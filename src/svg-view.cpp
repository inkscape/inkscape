#define __SP_SVG_VIEW_C__

/** \file
 * Functions and callbacks for generic SVG view and widget
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/canvas-arena.h"
#include "display/display-forward.h"
#include "document.h"
#include "sp-item.h"
#include "svg-view.h"


/**
 * Constructs new SPSVGView object and returns pointer to it.
 */
SPSVGView::SPSVGView (SPCanvasGroup *parent)
{
    _hscale = 1.0;
    _vscale = 1.0;
    _rescale = FALSE;
    _keepaspect = FALSE;
    _width = 0.0;
    _height = 0.0;

    _dkey = 0;
    _drawing = 0;
    _parent = parent;
}

SPSVGView::~SPSVGView()
{
    if (doc() && _drawing)
    {
        sp_item_invoke_hide (SP_ITEM (sp_document_root (doc())), _dkey);
        _drawing = NULL;
    }
}

/**
 * Rescales SPSVGView to given proportions.
 */
void
SPSVGView::setScale (gdouble hscale, gdouble vscale)
{
    if (!_rescale && ((hscale != _hscale) || (vscale != _vscale))) {
        _hscale = hscale;
        _vscale = vscale;
        doRescale (true);
    }
}

/**
 * Rescales SPSVGView and keeps aspect ratio.
 */
void
SPSVGView::setRescale
(bool rescale, bool keepaspect, gdouble width, gdouble height)
{
    g_return_if_fail (!rescale || (width >= 0.0));
    g_return_if_fail (!rescale || (height >= 0.0));

    _rescale = rescale;
    _keepaspect = keepaspect;
    _width = width;
    _height = height;

    doRescale (true);
}

/**
 * Helper function that sets rescale ratio and emits resize event.
 */
void
SPSVGView::doRescale (bool event)
{
    if (!doc()) return;
    if (sp_document_width (doc()) < 1e-9) return;
    if (sp_document_height (doc()) < 1e-9) return;

    if (_rescale) {
        _hscale = _width / sp_document_width (doc());
        _vscale = _height / sp_document_height (doc());
        if (_keepaspect) {
            if (_hscale > _vscale) {
                _hscale = _vscale;
                } else {
                    _vscale = _hscale;
                }
        }
    }

    if (_drawing) {
        sp_canvas_item_affine_absolute (_drawing, Geom::Scale(_hscale, _vscale));
    }

    if (event) {
        emitResized (sp_document_width (doc()) * _hscale,
                sp_document_height (doc()) * _vscale);
    }
}

void
SPSVGView::mouseover()
{
    GdkCursor *cursor = gdk_cursor_new(GDK_HAND2);
    gdk_window_set_cursor(GTK_WIDGET(SP_CANVAS_ITEM(_drawing)->canvas)->window, cursor);
    gdk_cursor_destroy(cursor);
}

void
SPSVGView::mouseout()
{
    gdk_window_set_cursor(GTK_WIDGET(SP_CANVAS_ITEM(_drawing)->canvas)->window, NULL);
}

//----------------------------------------------------------------
/**
 * Callback connected with arena_event.
 */
/// \todo fixme.
static gint
arena_handler (SPCanvasArena */*arena*/, NRArenaItem *ai, GdkEvent *event, SPSVGView *svgview)
{
	static gdouble x, y;
	static gboolean active = FALSE;
	SPEvent spev;

	SPItem *spitem = (ai) ? (SPItem*)NR_ARENA_ITEM_GET_DATA (ai) : 0;

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
			active = TRUE;
			x = event->button.x;
			y = event->button.y;
		}
		break;
	case GDK_BUTTON_RELEASE:
		if (event->button.button == 1) {
			if (active && (event->button.x == x) &&
                                      (event->button.y == y)) {
				spev.type = SP_EVENT_ACTIVATE;
                                if ( spitem != 0 )
				{
				  sp_item_event (spitem, &spev);
                                }
      			}
		}
		active = FALSE;
		break;
	case GDK_MOTION_NOTIFY:
		active = FALSE;
		break;
	case GDK_ENTER_NOTIFY:
		spev.type = SP_EVENT_MOUSEOVER;
		spev.data = svgview;
                if ( spitem != 0 )
		{
		  sp_item_event (spitem, &spev);
                }
		break;
	case GDK_LEAVE_NOTIFY:
		spev.type = SP_EVENT_MOUSEOUT;
		spev.data = svgview;
                if ( spitem != 0 )
		{
		  sp_item_event (spitem, &spev);
                }
		break;
	default:
		break;
	}

	return TRUE;
}

/**
 * Callback connected with set_document signal.
 */
void
SPSVGView::setDocument (SPDocument *document)
{
    if (doc()) {
        sp_item_invoke_hide (SP_ITEM (sp_document_root (doc())), _dkey);
    }

    if (!_drawing) {
        _drawing = sp_canvas_item_new (_parent, SP_TYPE_CANVAS_ARENA, NULL);
        g_signal_connect (G_OBJECT (_drawing), "arena_event", G_CALLBACK (arena_handler), this);
    }

    if (document) {
        NRArenaItem *ai = sp_item_invoke_show (
                SP_ITEM (sp_document_root (document)),
                SP_CANVAS_ARENA (_drawing)->arena,
                _dkey,
                SP_ITEM_SHOW_DISPLAY);

        if (ai) {
            nr_arena_item_add_child (SP_CANVAS_ARENA (_drawing)->root, ai, NULL);
        }

        doRescale (!_rescale);
    }

    View::setDocument (document);
}

/**
 * Callback connected with document_resized signal.
 */
void
SPSVGView::onDocumentResized (gdouble width, gdouble height)
{
    setScale (width, height);
    doRescale (!_rescale);
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

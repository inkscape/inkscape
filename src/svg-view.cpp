/*
 * Functions and callbacks for generic SVG view and widget.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/transforms.h>
#include "display/canvas-arena.h"
#include "display/drawing-group.h"
#include "document.h"
#include "sp-item.h"
#include "svg-view.h"
#include "sp-root.h"
#include "util/units.h"

SPSVGView::SPSVGView(SPCanvasGroup *parent)
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
        doc()->getRoot()->invoke_hide(_dkey);
        _drawing = NULL;
    }
}

void SPSVGView::setScale(gdouble hscale, gdouble vscale)
{
    if (!_rescale && ((hscale != _hscale) || (vscale != _vscale))) {
        _hscale = hscale;
        _vscale = vscale;
        doRescale (true);
    }
}

void SPSVGView::setRescale(bool rescale, bool keepaspect, gdouble width, gdouble height)
{
    g_return_if_fail (!rescale || (width >= 0.0));
    g_return_if_fail (!rescale || (height >= 0.0));

    _rescale = rescale;
    _keepaspect = keepaspect;
    _width = width;
    _height = height;

    doRescale (true);
}

void SPSVGView::doRescale(bool event)
{
    if (!doc()) {
        return;
    }
    if (doc()->getWidth().value("px") < 1e-9) {
        return;
    }
    if (doc()->getHeight().value("px") < 1e-9) {
        return;
    }

    if (_rescale) {
        _hscale = _width / doc()->getWidth().value("px");
        _vscale = _height / doc()->getHeight().value("px");
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
        emitResized (doc()->getWidth().value("px") * _hscale,
                doc()->getHeight().value("px") * _vscale);
    }
}

void SPSVGView::mouseover()
{
    GdkDisplay *display = gdk_display_get_default();
    GdkCursor  *cursor  = gdk_cursor_new_for_display(display, GDK_HAND2);
    GdkWindow *window = gtk_widget_get_window (GTK_WIDGET(SP_CANVAS_ITEM(_drawing)->canvas));
    gdk_window_set_cursor(window, cursor);
#if GTK_CHECK_VERSION(3,0,0)
    g_object_unref(cursor);
#else
    gdk_cursor_unref(cursor);
#endif
}

void SPSVGView::mouseout()
{
    GdkWindow *window = gtk_widget_get_window (GTK_WIDGET(SP_CANVAS_ITEM(_drawing)->canvas));
    gdk_window_set_cursor(window, NULL);
}

//----------------------------------------------------------------
/**
 * Callback connected with arena_event.
 */
/// \todo fixme.
static gint arena_handler(SPCanvasArena */*arena*/, Inkscape::DrawingItem *ai, GdkEvent *event, SPSVGView *svgview)
{
	static gdouble x, y;
	static gboolean active = FALSE;
	SPEvent spev;

	SPItem *spitem = (ai) ? (static_cast<SPItem*>(ai->data())) : 0;

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
				  spitem->emitEvent (spev);
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
		  spitem->emitEvent (spev);
                }
		break;
	case GDK_LEAVE_NOTIFY:
		spev.type = SP_EVENT_MOUSEOUT;
		spev.data = svgview;
                if ( spitem != 0 )
		{
		  spitem->emitEvent (spev);
                }
		break;
	default:
		break;
	}

	return TRUE;
}

void SPSVGView::setDocument(SPDocument *document)
{
    if (doc()) {
        doc()->getRoot()->invoke_hide(_dkey);
    }

    if (!_drawing) {
        _drawing = sp_canvas_item_new (_parent, SP_TYPE_CANVAS_ARENA, NULL);
        g_signal_connect (G_OBJECT (_drawing), "arena_event", G_CALLBACK (arena_handler), this);
    }

    View::setDocument (document);

    if (document) {
        Inkscape::DrawingItem *ai = document->getRoot()->invoke_show(
                SP_CANVAS_ARENA (_drawing)->drawing,
                _dkey,
                SP_ITEM_SHOW_DISPLAY);

        if (ai) {
            SP_CANVAS_ARENA (_drawing)->drawing.root()->prependChild(ai);
        }

        doRescale (!_rescale);
    }
}

void SPSVGView::onDocumentResized(gdouble width, gdouble height)
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

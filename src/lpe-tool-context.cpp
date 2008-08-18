/*
 * LPEToolContext: a context for a generic tool composed of subtools that are given by LPEs
 *
 * Authors:
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 2008 Maximilian Albert
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "forward.h"
#include "pixmaps/cursor-node.xpm"
#include "pixmaps/cursor-crosshairs.xpm"
#include <gtk/gtk.h>
#include "desktop.h"
#include "message-context.h"
#include "prefs-utils.h"
#include "shape-editor.h"
#include "selection.h"
#include "desktop-handles.h"
#include "document.h"
#include "display/curve.h"
#include "display/canvas-bpath.h"
#include "message-stack.h"

#include "lpe-tool-context.h"

static void sp_lpetool_context_class_init(SPLPEToolContextClass *klass);
static void sp_lpetool_context_init(SPLPEToolContext *erc);
static void sp_lpetool_context_dispose(GObject *object);

static void sp_lpetool_context_setup(SPEventContext *ec);
static void sp_lpetool_context_set(SPEventContext *ec, gchar const *key, gchar const *val);
static gint sp_lpetool_context_root_handler(SPEventContext *ec, GdkEvent *event);

void sp_lpetool_context_selection_changed(Inkscape::Selection *selection, gpointer data);

const int num_subtools = 7;

Inkscape::LivePathEffect::EffectType lpesubtools[] = {
    Inkscape::LivePathEffect::INVALID_LPE, // this must be here to account for the "all inactive" action
    Inkscape::LivePathEffect::LINE_SEGMENT,
    Inkscape::LivePathEffect::CIRCLE_3PTS,
    Inkscape::LivePathEffect::CIRCLE_WITH_RADIUS,
    Inkscape::LivePathEffect::PARALLEL,
    Inkscape::LivePathEffect::PERP_BISECTOR,
    Inkscape::LivePathEffect::ANGLE_BISECTOR,
};

static SPPenContextClass *lpetool_parent_class = 0;

GType sp_lpetool_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPLPEToolContextClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_lpetool_context_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPLPEToolContext),
            0, // n_preallocs
            (GInstanceInitFunc)sp_lpetool_context_init,
            0 // value_table
        };
        type = g_type_register_static(SP_TYPE_PEN_CONTEXT, "SPLPEToolContext", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}

static void
sp_lpetool_context_class_init(SPLPEToolContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    lpetool_parent_class = (SPPenContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_lpetool_context_dispose;

    event_context_class->setup = sp_lpetool_context_setup;
    event_context_class->set = sp_lpetool_context_set;
    event_context_class->root_handler = sp_lpetool_context_root_handler;
}

static void
sp_lpetool_context_init(SPLPEToolContext *lc)
{
    lc->cursor_shape = cursor_crosshairs_xpm;
    lc->hot_x = 7;
    lc->hot_y = 7;

    lc->canvas_bbox = NULL;

    new (&lc->sel_changed_connection) sigc::connection();
}

static void
sp_lpetool_context_dispose(GObject *object)
{
    SPLPEToolContext *lc = SP_LPETOOL_CONTEXT(object);
    delete lc->shape_editor;

    if (lc->canvas_bbox) {
        gtk_object_destroy(GTK_OBJECT(lc->canvas_bbox));
        lc->canvas_bbox = NULL;
    }

    lc->sel_changed_connection.disconnect();
    lc->sel_changed_connection.~connection();

    G_OBJECT_CLASS(lpetool_parent_class)->dispose(object);
}

static void
sp_lpetool_context_setup(SPEventContext *ec)
{
    SPLPEToolContext *lc = SP_LPETOOL_CONTEXT(ec);

    if (((SPEventContextClass *) lpetool_parent_class)->setup)
        ((SPEventContextClass *) lpetool_parent_class)->setup(ec);

    Inkscape::Selection *selection = sp_desktop_selection (ec->desktop);
    SPItem *item = selection->singleItem();

    lc->sel_changed_connection.disconnect();
    lc->sel_changed_connection =
        selection->connectChanged(sigc::bind(sigc::ptr_fun(&sp_lpetool_context_selection_changed), (gpointer)lc));

    lc->shape_editor = new ShapeEditor(ec->desktop);

    lpetool_context_switch_mode(lc, Inkscape::LivePathEffect::INVALID_LPE);
    lpetool_context_reset_limiting_bbox(lc);

// TODO temp force:
    ec->enableSelectionCue();

    if (item) {
        lc->shape_editor->set_item(item, SH_NODEPATH);
        lc->shape_editor->set_item(item, SH_KNOTHOLDER);
    }

    if (prefs_get_int_attribute("tools.lpetool", "selcue", 0) != 0) {
        ec->enableSelectionCue();
    }

    lc->_message_context = new Inkscape::MessageContext((ec->desktop)->messageStack());

    lc->shape_editor->update_statusbar();
}

/**
\brief  Callback that processes the "changed" signal on the selection;
destroys old and creates new nodepath and reassigns listeners to the new selected item's repr
*/
void
sp_lpetool_context_selection_changed(Inkscape::Selection *selection, gpointer data)
{
    SPLPEToolContext *lc = SP_LPETOOL_CONTEXT(data);

    // TODO: update ShapeEditorsCollective instead
    lc->shape_editor->unset_item(SH_NODEPATH);
    lc->shape_editor->unset_item(SH_KNOTHOLDER);
    SPItem *item = selection->singleItem();
    lc->shape_editor->set_item(item, SH_NODEPATH);
    lc->shape_editor->set_item(item, SH_KNOTHOLDER);
    lc->shape_editor->update_statusbar();
}

static void
sp_lpetool_context_set(SPEventContext *ec, gchar const *key, gchar const *val)
{
    // FIXME: how to set this correcly? the value from preferences-skeleton.h doesn't seem to get
    // read (it wants to set drag = 1)
    lpetool_parent_class->set(ec, key, "drag");

    /**
    //pass on up to parent class to handle common attributes.
    if ( lpetool_parent_class->set ) {
        lpetool_parent_class->set(ec, key, val);
    }
    **/
}

gint
sp_lpetool_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    //g_print ("sp_lpetool_context_root_handler()\n");
    SPLPEToolContext *lc = SP_LPETOOL_CONTEXT(event_context);
    SPDesktop *desktop = event_context->desktop;
    Inkscape::Selection *selection = sp_desktop_selection (desktop);

    bool ret = false;

    if (sp_pen_context_has_waiting_LPE(lc)) {
        // quit when we are waiting for a LPE to be applied
        g_print ("LPETool has waiting LPE. We call the pen tool parent context and return\n");
        ret = ((SPEventContextClass *) lpetool_parent_class)->root_handler(event_context, event);
        return ret;
    }

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            g_print ("GDK_BUTTON_PRESS\n");
            if (lc->mode == Inkscape::LivePathEffect::INVALID_LPE) {
                // don't do anything for now if we are inactive
                desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Choose a subtool from the toolbar to perform a geometric construction."));
                g_print ("Flash statusbar\n");
                ret = true;
                break;
            }

            if (event->button.button == 1 && !event_context->space_panning) {
                g_print ("   ... (passed if construct)\n");
                // save drag origin
                event_context->xp = (gint) event->button.x;
                event_context->yp = (gint) event->button.y;
                event_context->within_tolerance = true;
                lc->shape_editor->cancel_hit();

                using namespace Inkscape::LivePathEffect;

                int mode = prefs_get_int_attribute("tools.lpetool", "mode", 0);
                EffectType type = lpesubtools[mode];
                g_print ("Activating mode %d\n", mode);

                // save drag origin
                bool over_stroke = lc->shape_editor->is_over_stroke(NR::Point(event->button.x, event->button.y), true);
                g_print ("over_stroke: %s\n", over_stroke ? "true" : "false");

                sp_pen_context_wait_for_LPE_mouse_clicks(lc, type, Effect::acceptsNumClicks(type));

                // we pass the mouse click on to pen tool as the first click which it should collect
                ret = ((SPEventContextClass *) lpetool_parent_class)->root_handler(event_context, event);
            }
            break;
        case GDK_MOTION_NOTIFY:
        {
            if (!lc->shape_editor->has_nodepath() || selection->singleItem() == NULL) {
                break;
            }

            bool over_stroke = false;
            over_stroke = lc->shape_editor->is_over_stroke(NR::Point(event->motion.x, event->motion.y), false);

            if (over_stroke) {
                event_context->cursor_shape = cursor_node_xpm;
                event_context->hot_x = 1;
                event_context->hot_y = 1;
                sp_event_context_update_cursor(event_context);
            } else {
                lc->cursor_shape = cursor_crosshairs_xpm;
                lc->hot_x = 7;
                lc->hot_y = 7;
                sp_event_context_update_cursor(event_context);
            }
        }
        break;


    case GDK_BUTTON_RELEASE:
    {
        /**
        break;
        **/
    }

    case GDK_KEY_PRESS:
        /**
        switch (get_group0_keyval (&event->key)) {
        }
        break;
        **/

    case GDK_KEY_RELEASE:
        /**
        switch (get_group0_keyval(&event->key)) {
            case GDK_Control_L:
            case GDK_Control_R:
                dc->_message_context->clear();
                break;
            default:
                break;
        }
        **/

    default:
        break;
    }

    if (!ret) {
        if (((SPEventContextClass *) lpetool_parent_class)->root_handler) {
            ret = ((SPEventContextClass *) lpetool_parent_class)->root_handler(event_context, event);
        }
    }

    return ret;
}

static int
lpetool_mode_to_index(Inkscape::LivePathEffect::EffectType const type) {
    for (int i = 0; i < num_subtools; ++i) {
        if (lpesubtools[i] == type) {
            return i;
        }
    }
    return -1;
}

void
lpetool_context_switch_mode(SPLPEToolContext *lc, Inkscape::LivePathEffect::EffectType const type)
{
    int index = lpetool_mode_to_index(type);
    if (index != -1) {
        lc->mode = type;
        lc->desktop->setToolboxSelectOneValue ("lpetool_mode_action", index);
    } else {
        g_warning ("Invalid mode selected: %d", type);
        return;
    }
}

/*
 * Reads the limiting bounding box from preferences and draws it on the screen
 */
// TODO: Note that currently the bbox is not user-settable; we simply use the page borders
void
lpetool_context_reset_limiting_bbox(SPLPEToolContext *lc)
{
    if (lc->canvas_bbox) {
        gtk_object_destroy(GTK_OBJECT(lc->canvas_bbox));
        lc->canvas_bbox = NULL;
    }

    if (prefs_get_int_attribute("tools.lpetool", "show_bbox", 1) == 0)
        return;

    SPDocument *document = sp_desktop_document(lc->desktop);
    Geom::Coord w = sp_document_width(document);
    Geom::Coord h = sp_document_height(document);

    Geom::Point A(0,0);
    Geom::Point B(w,h);

    Geom::Rect rect(A, B);
    SPCurve *curve = SPCurve::new_from_rect(rect);

    lc->canvas_bbox = sp_canvas_bpath_new (sp_desktop_controls(lc->desktop), curve);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(lc->canvas_bbox), 0x0000ffff, 0.8, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT, 5, 5);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

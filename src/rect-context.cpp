#define __SP_RECT_CONTEXT_C__

/*
 * Rectangle drawing context
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2006      Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2000-2005 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <gdk/gdkkeysyms.h>
#include <cstring>
#include <string>

#include "macros.h"
#include "display/sp-canvas.h"
#include "sp-rect.h"
#include "document.h"
#include "sp-namedview.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "desktop-handles.h"
#include "snap.h"
#include "desktop.h"
#include "desktop-style.h"
#include "message-context.h"
#include "pixmaps/cursor-rect.xpm"
#include "rect-context.h"
#include "sp-metrics.h"
#include <glibmm/i18n.h>
#include "object-edit.h"
#include "xml/repr.h"
#include "xml/node-event-vector.h"
#include "preferences.h"
#include "context-fns.h"
#include "shape-editor.h"

//static const double goldenratio = 1.61803398874989484820; // golden ratio

static void sp_rect_context_class_init(SPRectContextClass *klass);
static void sp_rect_context_init(SPRectContext *rect_context);
static void sp_rect_context_dispose(GObject *object);

static void sp_rect_context_setup(SPEventContext *ec);
static void sp_rect_context_set(SPEventContext *ec, Inkscape::Preferences::Entry *val);

static gint sp_rect_context_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint sp_rect_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event);

static void sp_rect_drag(SPRectContext &rc, Geom::Point const pt, guint state);
static void sp_rect_finish(SPRectContext *rc);

static SPEventContextClass *parent_class;


GtkType sp_rect_context_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPRectContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_rect_context_class_init,
            NULL, NULL,
            sizeof(SPRectContext),
            4,
            (GInstanceInitFunc) sp_rect_context_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPRectContext", &info, (GTypeFlags) 0);
    }
    return type;
}

static void sp_rect_context_class_init(SPRectContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass *) g_type_class_peek_parent(klass);

    object_class->dispose = sp_rect_context_dispose;

    event_context_class->setup = sp_rect_context_setup;
    event_context_class->set = sp_rect_context_set;
    event_context_class->root_handler  = sp_rect_context_root_handler;
    event_context_class->item_handler  = sp_rect_context_item_handler;
}

static void sp_rect_context_init(SPRectContext *rect_context)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(rect_context);

    event_context->cursor_shape = cursor_rect_xpm;
    event_context->hot_x = 4;
    event_context->hot_y = 4;
    event_context->xp = 0;
    event_context->yp = 0;
    event_context->tolerance = 0;
    event_context->within_tolerance = false;
    event_context->item_to_select = NULL;

    rect_context->item = NULL;

    rect_context->rx = 0.0;
    rect_context->ry = 0.0;

    new (&rect_context->sel_changed_connection) sigc::connection();
}

static void sp_rect_context_dispose(GObject *object)
{
    SPRectContext *rc = SP_RECT_CONTEXT(object);
    SPEventContext *ec = SP_EVENT_CONTEXT(object);

    ec->enableGrDrag(false);

    rc->sel_changed_connection.disconnect();
    rc->sel_changed_connection.~connection();

    delete ec->shape_editor;
    ec->shape_editor = NULL;

    /* fixme: This is necessary because we do not grab */
    if (rc->item) {
        sp_rect_finish(rc);
    }

    if (rc->_message_context) {
        delete rc->_message_context;
    }

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

/**
\brief  Callback that processes the "changed" signal on the selection;
destroys old and creates new knotholder
*/
void sp_rect_context_selection_changed(Inkscape::Selection *selection, gpointer data)
{
    SPRectContext *rc = SP_RECT_CONTEXT(data);
    SPEventContext *ec = SP_EVENT_CONTEXT(rc);

    ec->shape_editor->unset_item(SH_KNOTHOLDER);
    SPItem *item = selection->singleItem();
    ec->shape_editor->set_item(item, SH_KNOTHOLDER);
}

static void sp_rect_context_setup(SPEventContext *ec)
{
    SPRectContext *rc = SP_RECT_CONTEXT(ec);

    if (((SPEventContextClass *) parent_class)->setup) {
        ((SPEventContextClass *) parent_class)->setup(ec);
    }

    ec->shape_editor = new ShapeEditor(ec->desktop);

    SPItem *item = sp_desktop_selection(ec->desktop)->singleItem();
    if (item) {
        ec->shape_editor->set_item(item, SH_KNOTHOLDER);
    }

    rc->sel_changed_connection.disconnect();
    rc->sel_changed_connection = sp_desktop_selection(ec->desktop)->connectChanged(
        sigc::bind(sigc::ptr_fun(&sp_rect_context_selection_changed), (gpointer)rc)
    );

    sp_event_context_read(ec, "rx");
    sp_event_context_read(ec, "ry");

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/shapes/selcue")) {
        ec->enableSelectionCue();
    }

    if (prefs->getBool("/tools/shapes/gradientdrag")) {
        ec->enableGrDrag();
    }

    rc->_message_context = new Inkscape::MessageContext((ec->desktop)->messageStack());
}

static void sp_rect_context_set(SPEventContext *ec, Inkscape::Preferences::Entry *val)
{
    SPRectContext *rc = SP_RECT_CONTEXT(ec);

    /* fixme: Proper error handling for non-numeric data.  Use a locale-independent function like
     * g_ascii_strtod (or a thin wrapper that does the right thing for invalid values inf/nan). */
    Glib::ustring name = val->getEntryName();
    if ( name == "rx" ) {
        rc->rx = val->getDoubleLimited(); // prevents NaN and +/-Inf from messing up
    } else if ( name == "ry" ) {
        rc->ry = val->getDoubleLimited();
    }
}

static gint sp_rect_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
    SPDesktop *desktop = event_context->desktop;

    gint ret = FALSE;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if ( event->button.button == 1 && !event_context->space_panning) {
            Inkscape::setup_for_drag_start(desktop, event_context, event);
            ret = TRUE;
        }
        break;
        // motion and release are always on root (why?)
    default:
        break;
    }

    if (((SPEventContextClass *) parent_class)->item_handler) {
        ret = ((SPEventContextClass *) parent_class)->item_handler(event_context, item, event);
    }

    return ret;
}

static gint sp_rect_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    static bool dragging;

    SPDesktop *desktop = event_context->desktop;
    Inkscape::Selection *selection = sp_desktop_selection (desktop);

    SPRectContext *rc = SP_RECT_CONTEXT(event_context);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    event_context->tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

    gint ret = FALSE;
    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if (event->button.button == 1 && !event_context->space_panning) {
            Geom::Point const button_w(event->button.x,
                                     event->button.y);

            // save drag origin
            event_context->xp = (gint) button_w[Geom::X];
            event_context->yp = (gint) button_w[Geom::Y];
            event_context->within_tolerance = true;

            // remember clicked item, disregarding groups, honoring Alt
            event_context->item_to_select = sp_event_context_find_item (desktop, button_w, event->button.state & GDK_MOD1_MASK, TRUE);

            dragging = true;
            sp_canvas_set_snap_delay_active(desktop->canvas, true);

            /* Position center */
            Geom::Point button_dt(desktop->w2d(button_w));
            rc->center = from_2geom(button_dt);

            /* Snap center */
            SnapManager &m = desktop->namedview->snap_manager;
            m.setup(desktop);
            m.freeSnapReturnByRef(Inkscape::SnapPreferences::SNAPPOINT_NODE, button_dt);
            rc->center = from_2geom(button_dt);

            sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                                ( GDK_KEY_PRESS_MASK |
                                  GDK_BUTTON_RELEASE_MASK       |
                                  GDK_POINTER_MOTION_MASK       |
                                  GDK_POINTER_MOTION_HINT_MASK       |
                                  GDK_BUTTON_PRESS_MASK ),
                                NULL, event->button.time);

            ret = TRUE;
        }
        break;
    case GDK_MOTION_NOTIFY:
        if ( dragging
             && (event->motion.state & GDK_BUTTON1_MASK) && !event_context->space_panning)
        {
            if ( event_context->within_tolerance
                 && ( abs( (gint) event->motion.x - event_context->xp ) < event_context->tolerance )
                 && ( abs( (gint) event->motion.y - event_context->yp ) < event_context->tolerance ) ) {
                break; // do not drag if we're within tolerance from origin
            }
            // Once the user has moved farther than tolerance from the original location
            // (indicating they intend to draw, not click), then always process the
            // motion notify coordinates as given (no snapping back to origin)
            event_context->within_tolerance = false;

            Geom::Point const motion_w(event->motion.x, event->motion.y);
            Geom::Point motion_dt(desktop->w2d(motion_w));

            sp_rect_drag(*rc, motion_dt, event->motion.state); // this will also handle the snapping
            gobble_motion_events(GDK_BUTTON1_MASK);
            ret = TRUE;
        }
        break;
    case GDK_BUTTON_RELEASE:
        event_context->xp = event_context->yp = 0;
        if (event->button.button == 1 && !event_context->space_panning) {
            dragging = false;
            sp_canvas_set_snap_delay_active(desktop->canvas, false);

            if (!event_context->within_tolerance) {
                // we've been dragging, finish the rect
                sp_rect_finish(rc);
            } else if (event_context->item_to_select) {
                // no dragging, select clicked item if any
                if (event->button.state & GDK_SHIFT_MASK) {
                    selection->toggle(event_context->item_to_select);
                } else {
                    selection->set(event_context->item_to_select);
                }
            } else {
                // click in an empty space
                selection->clear();
            }

            event_context->item_to_select = NULL;
            ret = TRUE;
            sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate),
                                  event->button.time);
        }
        break;
    case GDK_KEY_PRESS:
        switch (get_group0_keyval (&event->key)) {
        case GDK_Alt_L:
        case GDK_Alt_R:
        case GDK_Control_L:
        case GDK_Control_R:
        case GDK_Shift_L:
        case GDK_Shift_R:
        case GDK_Meta_L:  // Meta is when you press Shift+Alt (at least on my machine)
        case GDK_Meta_R:
            if (!dragging){
                sp_event_show_modifier_tip (event_context->defaultMessageContext(), event,
                                            _("<b>Ctrl</b>: make square or integer-ratio rect, lock a rounded corner circular"),
                                            _("<b>Shift</b>: draw around the starting point"),
                                            NULL);
            }
            break;
        case GDK_Up:
        case GDK_Down:
        case GDK_KP_Up:
        case GDK_KP_Down:
            // prevent the zoom field from activation
            if (!MOD__CTRL_ONLY)
                ret = TRUE;
            break;

        case GDK_x:
        case GDK_X:
            if (MOD__ALT_ONLY) {
                desktop->setToolboxFocusTo ("altx-rect");
                ret = TRUE;
            }
            break;

        case GDK_g:
        case GDK_G:
            if (MOD__SHIFT_ONLY) {
                sp_selection_to_guides(desktop);
                ret = true;
            }
            break;

        case GDK_Escape:
            sp_desktop_selection(desktop)->clear();
            //TODO: make dragging escapable by Esc
            break;

        case GDK_space:
            if (dragging) {
                sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate),
                                      event->button.time);
                dragging = false;
                sp_canvas_set_snap_delay_active(desktop->canvas, false);
                if (!event_context->within_tolerance) {
                    // we've been dragging, finish the rect
                    sp_rect_finish(rc);
                }
                // do not return true, so that space would work switching to selector
            }
            break;

        default:
            break;
        }
        break;
    case GDK_KEY_RELEASE:
        switch (get_group0_keyval (&event->key)) {
        case GDK_Alt_L:
        case GDK_Alt_R:
        case GDK_Control_L:
        case GDK_Control_R:
        case GDK_Shift_L:
        case GDK_Shift_R:
        case GDK_Meta_L:  // Meta is when you press Shift+Alt
        case GDK_Meta_R:
            event_context->defaultMessageContext()->clear();
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    if (!ret) {
        if (((SPEventContextClass *) parent_class)->root_handler) {
            ret = ((SPEventContextClass *) parent_class)->root_handler(event_context, event);
        }
    }

    return ret;
}

static void sp_rect_drag(SPRectContext &rc, Geom::Point const pt, guint state)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT(&rc)->desktop;

    if (!rc.item) {

        if (Inkscape::have_viable_layer(desktop, rc._message_context) == false) {
            return;
        }

        /* Create object */
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(SP_EVENT_CONTEXT_DOCUMENT(&rc));
        Inkscape::XML::Node *repr = xml_doc->createElement("svg:rect");

        /* Set style */
        sp_desktop_apply_style_tool (desktop, repr, "/tools/shapes/rect", false);

        rc.item = (SPItem *) desktop->currentLayer()->appendChildRepr(repr);
        Inkscape::GC::release(repr);
        rc.item->transform = sp_item_i2doc_affine(SP_ITEM(desktop->currentLayer())).inverse();
        rc.item->updateRepr();

        sp_canvas_force_full_redraw_after_interruptions(desktop->canvas, 5);
    }

    Geom::Rect const r = Inkscape::snap_rectangular_box(desktop, rc.item, pt, rc.center, state);

    sp_rect_position_set(SP_RECT(rc.item), r.min()[Geom::X], r.min()[Geom::Y], r.dimensions()[Geom::X], r.dimensions()[Geom::Y]);
    if ( rc.rx != 0.0 ) {
        sp_rect_set_rx (SP_RECT(rc.item), TRUE, rc.rx);
    }
    if ( rc.ry != 0.0 ) {
        if (rc.rx == 0.0)
            sp_rect_set_ry (SP_RECT(rc.item), TRUE, CLAMP(rc.ry, 0, MIN(r.dimensions()[Geom::X], r.dimensions()[Geom::Y])/2));
        else
            sp_rect_set_ry (SP_RECT(rc.item), TRUE, CLAMP(rc.ry, 0, r.dimensions()[Geom::Y]));
    }

    // status text
    double rdimx = r.dimensions()[Geom::X];
    double rdimy = r.dimensions()[Geom::Y];
    GString *xs = SP_PX_TO_METRIC_STRING(rdimx, desktop->namedview->getDefaultMetric());
    GString *ys = SP_PX_TO_METRIC_STRING(rdimy, desktop->namedview->getDefaultMetric());
    if (state & GDK_CONTROL_MASK) {
        int ratio_x, ratio_y;
        bool is_golden_ratio = false;
        if (fabs (rdimx) > fabs (rdimy)) {
            if (fabs(rdimx / rdimy - goldenratio) < 1e-6) {
                is_golden_ratio = true;
            }
            ratio_x = (int) rint (rdimx / rdimy);
            ratio_y = 1;
        } else {
            if (fabs(rdimy / rdimx - goldenratio) < 1e-6) {
                is_golden_ratio = true;
            }
            ratio_x = 1;
            ratio_y = (int) rint (rdimy / rdimx);
        }
        if (!is_golden_ratio) {
            rc._message_context->setF(Inkscape::IMMEDIATE_MESSAGE, _("<b>Rectangle</b>: %s &#215; %s (constrained to ratio %d:%d); with <b>Shift</b> to draw around the starting point"), xs->str, ys->str, ratio_x, ratio_y);
        } else {
            if (ratio_y == 1) {
                rc._message_context->setF(Inkscape::IMMEDIATE_MESSAGE, _("<b>Rectangle</b>: %s &#215; %s (constrained to golden ratio 1.618 : 1); with <b>Shift</b> to draw around the starting point"), xs->str, ys->str);
            } else {
                rc._message_context->setF(Inkscape::IMMEDIATE_MESSAGE, _("<b>Rectangle</b>: %s &#215; %s (constrained to golden ratio 1 : 1.618); with <b>Shift</b> to draw around the starting point"), xs->str, ys->str);
            }
        }
    } else {
        rc._message_context->setF(Inkscape::IMMEDIATE_MESSAGE, _("<b>Rectangle</b>: %s &#215; %s; with <b>Ctrl</b> to make square or integer-ratio rectangle; with <b>Shift</b> to draw around the starting point"), xs->str, ys->str);
    }
    g_string_free(xs, FALSE);
    g_string_free(ys, FALSE);
}

static void sp_rect_finish(SPRectContext *rc)
{
    rc->_message_context->clear();

    if ( rc->item != NULL ) {
        SPDesktop * desktop;

        desktop = SP_EVENT_CONTEXT_DESKTOP(rc);

        SP_OBJECT(rc->item)->updateRepr();

        sp_canvas_end_forced_full_redraws(desktop->canvas);

        sp_desktop_selection(desktop)->set(rc->item);
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_RECT,
                         _("Create rectangle"));

        rc->item = NULL;
    }
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

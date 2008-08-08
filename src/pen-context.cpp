/** \file
 * Pen event context implementation.
 */

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 * Copyright (C) 2004 Monash University
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gdk/gdkkeysyms.h>
#include <cstring>
#include <string>

#include "pen-context.h"
#include "sp-namedview.h"
#include "sp-metrics.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "draw-anchor.h"
#include "message-stack.h"
#include "message-context.h"
#include "prefs-utils.h"
#include "sp-path.h"
#include "display/curve.h"
#include "pixmaps/cursor-pen.xpm"
#include "display/canvas-bpath.h"
#include "display/sp-ctrlline.h"
#include "display/sodipodi-ctrl.h"
#include <glibmm/i18n.h>
#include "libnr/nr-point-ops.h"
#include "helper/units.h"
#include "macros.h"
#include "context-fns.h"

static void sp_pen_context_class_init(SPPenContextClass *klass);
static void sp_pen_context_init(SPPenContext *pc);
static void sp_pen_context_dispose(GObject *object);

static void sp_pen_context_setup(SPEventContext *ec);
static void sp_pen_context_finish(SPEventContext *ec);
static void sp_pen_context_set(SPEventContext *ec, gchar const *key, gchar const *val);
static gint sp_pen_context_root_handler(SPEventContext *ec, GdkEvent *event);
static gint sp_pen_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event);

static void spdc_pen_set_initial_point(SPPenContext *pc, NR::Point const p);
static void spdc_pen_set_subsequent_point(SPPenContext *const pc, NR::Point const p, bool statusbar, guint status = 0);
static void spdc_pen_set_ctrl(SPPenContext *pc, NR::Point const p, guint state);
static void spdc_pen_finish_segment(SPPenContext *pc, NR::Point p, guint state);

static void spdc_pen_finish(SPPenContext *pc, gboolean closed);

static gint pen_handle_button_press(SPPenContext *const pc, GdkEventButton const &bevent);
static gint pen_handle_motion_notify(SPPenContext *const pc, GdkEventMotion const &mevent);
static gint pen_handle_button_release(SPPenContext *const pc, GdkEventButton const &revent);
static gint pen_handle_2button_press(SPPenContext *const pc, GdkEventButton const &bevent);
static gint pen_handle_key_press(SPPenContext *const pc, GdkEvent *event);
static void spdc_reset_colors(SPPenContext *pc);

static void pen_disable_events(SPPenContext *const pc);
static void pen_enable_events(SPPenContext *const pc);

static NR::Point pen_drag_origin_w(0, 0);
static bool pen_within_tolerance = false;

static SPDrawContextClass *pen_parent_class;

static int pen_next_paraxial_direction(const SPPenContext *const pc, NR::Point const &pt, NR::Point const &origin, guint state);
static void pen_set_to_nearest_horiz_vert(const SPPenContext *const pc, NR::Point &pt, guint const state);
static NR::Point pen_get_intermediate_horiz_vert(const SPPenContext *const pc, NR::Point const &pt, guint const state);

static int pen_last_paraxial_dir = 0; // last used direction in horizontal/vertical mode; 0 = horizontal, 1 = vertical

/**
 * Register SPPenContext with Gdk and return its type.
 */
GType
sp_pen_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPPenContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_pen_context_class_init,
            NULL, NULL,
            sizeof(SPPenContext),
            4,
            (GInstanceInitFunc) sp_pen_context_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_DRAW_CONTEXT, "SPPenContext", &info, (GTypeFlags)0);
    }
    return type;
}

/**
 * Initialize the SPPenContext vtable.
 */
static void
sp_pen_context_class_init(SPPenContextClass *klass)
{
    GObjectClass *object_class;
    SPEventContextClass *event_context_class;

    object_class = (GObjectClass *) klass;
    event_context_class = (SPEventContextClass *) klass;

    pen_parent_class = (SPDrawContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_pen_context_dispose;

    event_context_class->setup = sp_pen_context_setup;
    event_context_class->finish = sp_pen_context_finish;
    event_context_class->set = sp_pen_context_set;
    event_context_class->root_handler = sp_pen_context_root_handler;
    event_context_class->item_handler = sp_pen_context_item_handler;
}

/**
 * Callback to initialize SPPenContext object.
 */
static void
sp_pen_context_init(SPPenContext *pc)
{

    SPEventContext *event_context = SP_EVENT_CONTEXT(pc);

    event_context->cursor_shape = cursor_pen_xpm;
    event_context->hot_x = 4;
    event_context->hot_y = 4;

    pc->npoints = 0;
    pc->mode = SP_PEN_CONTEXT_MODE_CLICK;
    pc->state = SP_PEN_CONTEXT_POINT;

    pc->c0 = NULL;
    pc->c1 = NULL;
    pc->cl0 = NULL;
    pc->cl1 = NULL;
    
    pc->events_disabled = 0;

    pc->num_clicks = 0;
    pc->waiting_LPE = NULL;
}

/**
 * Callback to destroy the SPPenContext object's members and itself.
 */
static void
sp_pen_context_dispose(GObject *object)
{
    SPPenContext *pc;

    pc = SP_PEN_CONTEXT(object);

    if (pc->c0) {
        gtk_object_destroy(GTK_OBJECT(pc->c0));
        pc->c0 = NULL;
    }
    if (pc->c1) {
        gtk_object_destroy(GTK_OBJECT(pc->c1));
        pc->c1 = NULL;
    }
    if (pc->cl0) {
        gtk_object_destroy(GTK_OBJECT(pc->cl0));
        pc->cl0 = NULL;
    }
    if (pc->cl1) {
        gtk_object_destroy(GTK_OBJECT(pc->cl1));
        pc->cl1 = NULL;
    }

    G_OBJECT_CLASS(pen_parent_class)->dispose(object);

    if (pc->expecting_clicks_for_LPE > 0) {
        // we received too few clicks to sanely set the parameter path so we remove the LPE from the item
        sp_lpe_item_remove_current_path_effect(pc->waiting_item, false);
    }
}

void
sp_pen_context_set_polyline_mode(SPPenContext *const pc) {
    guint mode = prefs_get_int_attribute("tools.freehand.pen", "freehand-mode", 0);
    pc->polylines_only = (mode == 2 || mode == 3);
    pc->polylines_paraxial = (mode == 3);
}

/**
 * Callback to initialize SPPenContext object.
 */
static void
sp_pen_context_setup(SPEventContext *ec)
{
    SPPenContext *pc;

    pc = SP_PEN_CONTEXT(ec);

    if (((SPEventContextClass *) pen_parent_class)->setup) {
        ((SPEventContextClass *) pen_parent_class)->setup(ec);
    }

    /* Pen indicators */
    pc->c0 = sp_canvas_item_new(sp_desktop_controls(SP_EVENT_CONTEXT_DESKTOP(ec)), SP_TYPE_CTRL, "shape", SP_CTRL_SHAPE_CIRCLE,
                                "size", 4.0, "filled", 0, "fill_color", 0xff00007f, "stroked", 1, "stroke_color", 0x0000ff7f, NULL);
    pc->c1 = sp_canvas_item_new(sp_desktop_controls(SP_EVENT_CONTEXT_DESKTOP(ec)), SP_TYPE_CTRL, "shape", SP_CTRL_SHAPE_CIRCLE,
                                "size", 4.0, "filled", 0, "fill_color", 0xff00007f, "stroked", 1, "stroke_color", 0x0000ff7f, NULL);
    pc->cl0 = sp_canvas_item_new(sp_desktop_controls(SP_EVENT_CONTEXT_DESKTOP(ec)), SP_TYPE_CTRLLINE, NULL);
    sp_ctrlline_set_rgba32(SP_CTRLLINE(pc->cl0), 0x0000007f);
    pc->cl1 = sp_canvas_item_new(sp_desktop_controls(SP_EVENT_CONTEXT_DESKTOP(ec)), SP_TYPE_CTRLLINE, NULL);
    sp_ctrlline_set_rgba32(SP_CTRLLINE(pc->cl1), 0x0000007f);

    sp_canvas_item_hide(pc->c0);
    sp_canvas_item_hide(pc->c1);
    sp_canvas_item_hide(pc->cl0);
    sp_canvas_item_hide(pc->cl1);

    sp_event_context_read(ec, "mode");

    pc->anchor_statusbar = false;

    sp_pen_context_set_polyline_mode(pc);

    if (prefs_get_int_attribute("tools.freehand.pen", "selcue", 0) != 0) {
        ec->enableSelectionCue();
    }
}

static void
pen_cancel (SPPenContext *const pc) 
{
    pc->num_clicks = 0;
    pc->state = SP_PEN_CONTEXT_STOP;
    spdc_reset_colors(pc);
    sp_canvas_item_hide(pc->c0);
    sp_canvas_item_hide(pc->c1);
    sp_canvas_item_hide(pc->cl0);
    sp_canvas_item_hide(pc->cl1);
    pc->_message_context->clear();
    pc->_message_context->flash(Inkscape::NORMAL_MESSAGE, _("Drawing cancelled"));

    sp_canvas_end_forced_full_redraws(pc->desktop->canvas);
}

/**
 * Finalization callback.
 */
static void
sp_pen_context_finish(SPEventContext *ec)
{
    SPPenContext *pc = SP_PEN_CONTEXT(ec);

    if (pc->npoints != 0) {
        pen_cancel (pc);
    }

    if (((SPEventContextClass *) pen_parent_class)->finish) {
        ((SPEventContextClass *) pen_parent_class)->finish(ec);
    }
}

/**
 * Callback that sets key to value in pen context.
 */
static void
sp_pen_context_set(SPEventContext *ec, gchar const *key, gchar const *val)
{
    SPPenContext *pc = SP_PEN_CONTEXT(ec);

    if (!strcmp(key, "mode")) {
        if ( val && !strcmp(val, "drag") ) {
            pc->mode = SP_PEN_CONTEXT_MODE_DRAG;
        } else {
            pc->mode = SP_PEN_CONTEXT_MODE_CLICK;
        }
    }
}

/**
 * Snaps new node relative to the previous node.
 */
static void
spdc_endpoint_snap(SPPenContext const *const pc, NR::Point &p, guint const state)
{
    if ((state & GDK_CONTROL_MASK)) { //CTRL enables angular snapping
        if (pc->npoints > 0) {
            spdc_endpoint_snap_rotation(pc, p, pc->p[0], state);
        }
    } else {
        if (!(state & GDK_SHIFT_MASK)) { //SHIFT disables all snapping, except the angular snapping above
                                         //After all, the user explicitely asked for angular snapping by
                                         //pressing CTRL
            spdc_endpoint_snap_free(pc, p, state);
        }
    }
    if (pc->polylines_paraxial) {
        // TODO: must we avoid one of the snaps in the previous case distinction in some situations?
        pen_set_to_nearest_horiz_vert(pc, p, state);
    }
}

/**
 * Snaps new node's handle relative to the new node.
 */
static void
spdc_endpoint_snap_handle(SPPenContext const *const pc, NR::Point &p, guint const state)
{
    g_return_if_fail(( pc->npoints == 2 ||
                       pc->npoints == 5   ));

    if ((state & GDK_CONTROL_MASK)) { //CTRL enables angular snapping
        spdc_endpoint_snap_rotation(pc, p, pc->p[pc->npoints - 2], state);
    } else {
        if (!(state & GDK_SHIFT_MASK)) { //SHIFT disables all snapping, except the angular snapping above
            spdc_endpoint_snap_free(pc, p, state);
        }
    }
}

static gint 
sp_pen_context_item_handler(SPEventContext *ec, SPItem *item, GdkEvent *event)
{
    SPPenContext *const pc = SP_PEN_CONTEXT(ec);

    gint ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            ret = pen_handle_button_press(pc, event->button);
            break;
        case GDK_BUTTON_RELEASE:
            ret = pen_handle_button_release(pc, event->button);
            break;
        default:
            break;
    }

    if (!ret) {
        if (((SPEventContextClass *) pen_parent_class)->item_handler)
            ret = ((SPEventContextClass *) pen_parent_class)->item_handler(ec, item, event);
    }

    return ret;
}

/**
 * Callback to handle all pen events.
 */
static gint
sp_pen_context_root_handler(SPEventContext *ec, GdkEvent *event)
{
    SPPenContext *const pc = SP_PEN_CONTEXT(ec);

    gint ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            ret = pen_handle_button_press(pc, event->button);
            break;

        case GDK_MOTION_NOTIFY:
            ret = pen_handle_motion_notify(pc, event->motion);
            break;

        case GDK_BUTTON_RELEASE:
            ret = pen_handle_button_release(pc, event->button);
            break;

        case GDK_2BUTTON_PRESS:
            ret = pen_handle_2button_press(pc, event->button);
            break;

        case GDK_KEY_PRESS:
            ret = pen_handle_key_press(pc, event);
            break;

        default:
            break;
    }

    if (!ret) {
        gint (*const parent_root_handler)(SPEventContext *, GdkEvent *)
            = ((SPEventContextClass *) pen_parent_class)->root_handler;
        if (parent_root_handler) {
            ret = parent_root_handler(ec, event);
        }
    }

    return ret;
}

/**
 * Handle mouse button press event.
 */
static gint pen_handle_button_press(SPPenContext *const pc, GdkEventButton const &bevent)
{
    if (pc->events_disabled) {
        // skip event processing if events are disabled
        return FALSE;
    }

    SPDrawContext * const dc = SP_DRAW_CONTEXT(pc);
    SPDesktop * const desktop = SP_EVENT_CONTEXT_DESKTOP(dc);
    NR::Point const event_w(bevent.x, bevent.y);
    NR::Point const event_dt(desktop->w2d(event_w));
    SPEventContext *event_context = SP_EVENT_CONTEXT(pc);

    gint ret = FALSE;
    if (bevent.button == 1 && !event_context->space_panning
        // make sure this is not the last click for a waiting LPE (otherwise we want to finish the path)
        && pc->expecting_clicks_for_LPE != 1) {

        if (Inkscape::have_viable_layer(desktop, dc->_message_context) == false) {
            return TRUE;
        }

        if (!pc->grab ) {
            /* Grab mouse, so release will not pass unnoticed */
            pc->grab = SP_CANVAS_ITEM(desktop->acetate);
            sp_canvas_item_grab(pc->grab, ( GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK   |
                                            GDK_BUTTON_RELEASE_MASK |
                                            GDK_POINTER_MOTION_MASK  ),
                                NULL, bevent.time);
        }

        pen_drag_origin_w = event_w;
        pen_within_tolerance = true;

        /* Test whether we hit any anchor. */
        SPDrawAnchor * const anchor = spdc_test_inside(pc, event_w);

        switch (pc->mode) {
            case SP_PEN_CONTEXT_MODE_CLICK:
                /* In click mode we add point on release */
                switch (pc->state) {
                    case SP_PEN_CONTEXT_POINT:
                    case SP_PEN_CONTEXT_CONTROL:
                    case SP_PEN_CONTEXT_CLOSE:
                        break;
                    case SP_PEN_CONTEXT_STOP:
                        /* This is allowed, if we just cancelled curve */
                        pc->state = SP_PEN_CONTEXT_POINT;
                        break;
                    default:
                        break;
                }
                break;
            case SP_PEN_CONTEXT_MODE_DRAG:
                switch (pc->state) {
                    case SP_PEN_CONTEXT_STOP:
                        /* This is allowed, if we just cancelled curve */
                    case SP_PEN_CONTEXT_POINT:
                        if (pc->npoints == 0) {

                            if (bevent.state & GDK_CONTROL_MASK) {
                                freehand_create_single_dot(event_context, event_dt, "tools.freehand.pen", bevent.state);
                                ret = TRUE;
                                break;
                            }

                            // TODO: Perhaps it would be nicer to rearrange the following case
                            // distinction so that the case of a waiting LPE is treated separately

                            /* Set start anchor */
                            pc->sa = anchor;
                            NR::Point p;
                            if (anchor && !sp_pen_context_has_waiting_LPE(pc)) {
                                /* Adjust point to anchor if needed; if we have a waiting LPE, we need
                                   a fresh path to be created so don't continue an existing one */
                                p = anchor->dp;
                                desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Continuing selected path"));
                            } else {
                                // This is the first click of a new curve; deselect item so that
                                // this curve is not combined with it (unless it is drawn from its
                                // anchor, which is handled by the sibling branch above)
                                Inkscape::Selection * const selection = sp_desktop_selection(desktop);
                                if (!(bevent.state & GDK_SHIFT_MASK) || sp_pen_context_has_waiting_LPE(pc)) {
                                    /* if we have a waiting LPE, we need a fresh path to be created
                                       so don't append to an existing one */
                                    selection->clear();
                                    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Creating new path"));
                                } else if (selection->singleItem() && SP_IS_PATH(selection->singleItem())) {
                                    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Appending to selected path"));
                                }

                                /* Create green anchor */
                                p = event_dt;
                                if (!pc->polylines_paraxial) {
                                    // only snap the starting point if we're not in horizontal/vertical mode
                                    // because otherwise it gets shifted; TODO: why do we snap here at all??
                                    spdc_endpoint_snap(pc, p, bevent.state);
                                }
                                pc->green_anchor = sp_draw_anchor_new(pc, pc->green_curve, TRUE, p);
                            }
                            spdc_pen_set_initial_point(pc, p);
                        } else {

                            /* Set end anchor */
                            pc->ea = anchor;
                            NR::Point p;
                            if (anchor) {
                                p = anchor->dp;
                                // we hit an anchor, will finish the curve (either with or without closing)
                                // in release handler
                                pc->state = SP_PEN_CONTEXT_CLOSE;

                                if (pc->green_anchor && pc->green_anchor->active) {
                                    // we clicked on the current curve start, so close it even if
                                    // we drag a handle away from it
                                    dc->green_closed = TRUE;
                                }
                                ret = TRUE;
                                break;

                            } else {
                                p = event_dt;
                                spdc_endpoint_snap(pc, p, bevent.state); /* Snap node only if not hitting anchor. */
                                spdc_pen_set_subsequent_point(pc, p, true);
                                if (pc->polylines_only) {
                                    spdc_pen_finish_segment(pc, p, bevent.state);
                                }
                            }
                        }

                        pc->state = pc->polylines_only ? SP_PEN_CONTEXT_POINT : SP_PEN_CONTEXT_CONTROL;
                        ret = TRUE;
                        break;
                    case SP_PEN_CONTEXT_CONTROL:
                        g_warning("Button down in CONTROL state");
                        break;
                    case SP_PEN_CONTEXT_CLOSE:
                        g_warning("Button down in CLOSE state");
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    } else if (bevent.button == 3 || pc->expecting_clicks_for_LPE == 1) { // when the last click for a waiting LPE occurs we want to finish the path
        if (pc->npoints != 0) {

            spdc_pen_finish_segment(pc, event_dt, bevent.state);
            if (pc->green_closed) {
                // finishing at the start anchor, close curve
                spdc_pen_finish(pc, TRUE);
            } else {
                // finishing at some other anchor, finish curve but not close
                spdc_pen_finish(pc, FALSE);
            }

            ret = TRUE;
        }
    }

    if (pc->expecting_clicks_for_LPE) {
        --pc->expecting_clicks_for_LPE;
    }

    return ret;
}

/**
 * Handle motion_notify event.
 */
static gint
pen_handle_motion_notify(SPPenContext *const pc, GdkEventMotion const &mevent)
{
    gint ret = FALSE;

    SPEventContext *event_context = SP_EVENT_CONTEXT(pc);
    SPDesktop * const dt = SP_EVENT_CONTEXT_DESKTOP(event_context);

    if (event_context->space_panning || mevent.state & GDK_BUTTON2_MASK || mevent.state & GDK_BUTTON3_MASK) {
        // allow scrolling
        return FALSE;
    }
   
    if (pc->events_disabled) {
        // skip motion events if pen events are disabled
        return FALSE;
    }

    NR::Point const event_w(mevent.x,
                            mevent.y);
    if (pen_within_tolerance) {
        gint const tolerance = prefs_get_int_attribute_limited("options.dragtolerance",
                                                               "value", 0, 0, 100);
        if ( NR::LInfty( event_w - pen_drag_origin_w ) < tolerance ) {
            return FALSE;   // Do not drag if we're within tolerance from origin.
        }
    }
    // Once the user has moved farther than tolerance from the original location
    // (indicating they intend to move the object, not click), then always process the
    // motion notify coordinates as given (no snapping back to origin)
    pen_within_tolerance = false;

    /* Find desktop coordinates */
    NR::Point p = dt->w2d(event_w);

    /* Test, whether we hit any anchor */
    SPDrawAnchor *anchor = spdc_test_inside(pc, event_w);

    switch (pc->mode) {
        case SP_PEN_CONTEXT_MODE_CLICK:
            switch (pc->state) {
                case SP_PEN_CONTEXT_POINT:
                    if ( pc->npoints != 0 ) {
                        /* Only set point, if we are already appending */
                        spdc_endpoint_snap(pc, p, mevent.state);
                        spdc_pen_set_subsequent_point(pc, p, true);
                        ret = TRUE;
                    }
                    break;
                case SP_PEN_CONTEXT_CONTROL:
                case SP_PEN_CONTEXT_CLOSE:
                    /* Placing controls is last operation in CLOSE state */
                    spdc_endpoint_snap(pc, p, mevent.state);
                    spdc_pen_set_ctrl(pc, p, mevent.state);
                    ret = TRUE;
                    break;
                case SP_PEN_CONTEXT_STOP:
                    /* This is perfectly valid */
                    break;
                default:
                    break;
            }
            break;
        case SP_PEN_CONTEXT_MODE_DRAG:
            switch (pc->state) {
                case SP_PEN_CONTEXT_POINT:
                    if ( pc->npoints > 0 ) {
                        /* Only set point, if we are already appending */

                        if (!anchor) {   /* Snap node only if not hitting anchor */
                            spdc_endpoint_snap(pc, p, mevent.state);
                        }

                        spdc_pen_set_subsequent_point(pc, p, !anchor, mevent.state);

                        if (anchor && !pc->anchor_statusbar) {
                            pc->_message_context->set(Inkscape::NORMAL_MESSAGE, _("<b>Click</b> or <b>click and drag</b> to close and finish the path."));
                            pc->anchor_statusbar = true;
                        } else if (!anchor && pc->anchor_statusbar) {
                            pc->_message_context->clear();
                            pc->anchor_statusbar = false;
                        }

                        ret = TRUE;
                    } else {
                        if (anchor && !pc->anchor_statusbar) {
                            pc->_message_context->set(Inkscape::NORMAL_MESSAGE, _("<b>Click</b> or <b>click and drag</b> to continue the path from this point."));
                            pc->anchor_statusbar = true;
                        } else if (!anchor && pc->anchor_statusbar) {
                            pc->_message_context->clear();
                            pc->anchor_statusbar = false;
                        }
                    }
                    break;
                case SP_PEN_CONTEXT_CONTROL:
                case SP_PEN_CONTEXT_CLOSE:
                    /* Placing controls is last operation in CLOSE state */

                    // snap the handle
                    spdc_endpoint_snap_handle(pc, p, mevent.state);

                    if (!pc->polylines_only) {
                        spdc_pen_set_ctrl(pc, p, mevent.state);
                    } else {
                        spdc_pen_set_ctrl(pc, pc->p[1], mevent.state);
                    }
                    gobble_motion_events(GDK_BUTTON1_MASK);
                    ret = TRUE;
                    break;
                case SP_PEN_CONTEXT_STOP:
                    /* This is perfectly valid */
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return ret;
}

/**
 * Handle mouse button release event.
 */
static gint
pen_handle_button_release(SPPenContext *const pc, GdkEventButton const &revent)
{
    if (pc->events_disabled) {
        // skip event processing if events are disabled
        return FALSE;
    }

    gint ret = FALSE;
    SPEventContext *event_context = SP_EVENT_CONTEXT(pc);
    if ( revent.button == 1  && !event_context->space_panning) {

        SPDrawContext *dc = SP_DRAW_CONTEXT (pc);

        NR::Point const event_w(revent.x,
                                revent.y);
        /* Find desktop coordinates */
        NR::Point p = pc->desktop->w2d(event_w);

        /* Test whether we hit any anchor. */
        SPDrawAnchor *anchor = spdc_test_inside(pc, event_w);

        switch (pc->mode) {
            case SP_PEN_CONTEXT_MODE_CLICK:
                switch (pc->state) {
                    case SP_PEN_CONTEXT_POINT:
                        if ( pc->npoints == 0 ) {
                            /* Start new thread only with button release */
                            if (anchor) {
                                p = anchor->dp;
                            }
                            pc->sa = anchor;
                            spdc_pen_set_initial_point(pc, p);
                        } else {
                            /* Set end anchor here */
                            pc->ea = anchor;
                            if (anchor) {
                                p = anchor->dp;
                            }
                        }
                        pc->state = SP_PEN_CONTEXT_CONTROL;
                        ret = TRUE;
                        break;
                    case SP_PEN_CONTEXT_CONTROL:
                        /* End current segment */
                        spdc_endpoint_snap(pc, p, revent.state);
                        spdc_pen_finish_segment(pc, p, revent.state);
                        pc->state = SP_PEN_CONTEXT_POINT;
                        ret = TRUE;
                        break;
                    case SP_PEN_CONTEXT_CLOSE:
                        /* End current segment */
                        if (!anchor) {   /* Snap node only if not hitting anchor */
                            spdc_endpoint_snap(pc, p, revent.state);
                        }
                        spdc_pen_finish_segment(pc, p, revent.state);
                        spdc_pen_finish(pc, TRUE);
                        pc->state = SP_PEN_CONTEXT_POINT;
                        ret = TRUE;
                        break;
                    case SP_PEN_CONTEXT_STOP:
                        /* This is allowed, if we just cancelled curve */
                        pc->state = SP_PEN_CONTEXT_POINT;
                        ret = TRUE;
                        break;
                    default:
                        break;
                }
                break;
            case SP_PEN_CONTEXT_MODE_DRAG:
                switch (pc->state) {
                    case SP_PEN_CONTEXT_POINT:
                    case SP_PEN_CONTEXT_CONTROL:
                        if (!pc->polylines_only) {
                            spdc_endpoint_snap(pc, p, revent.state);
                            spdc_pen_finish_segment(pc, p, revent.state);
                        }
                        break;
                    case SP_PEN_CONTEXT_CLOSE:
                        spdc_endpoint_snap(pc, p, revent.state);
                        spdc_pen_finish_segment(pc, p, revent.state);
                        if (pc->green_closed) {
                            // finishing at the start anchor, close curve
                            spdc_pen_finish(pc, TRUE);
                        } else {
                            // finishing at some other anchor, finish curve but not close
                            spdc_pen_finish(pc, FALSE);
                        }
                        break;
                    case SP_PEN_CONTEXT_STOP:
                        /* This is allowed, if we just cancelled curve */
                        break;
                    default:
                        break;
                }
                pc->state = SP_PEN_CONTEXT_POINT;
                ret = TRUE;
                break;
            default:
                break;
        }

        if (pc->grab) {
            /* Release grab now */
            sp_canvas_item_ungrab(pc->grab, revent.time);
            pc->grab = NULL;
        }

        ret = TRUE;

        dc->green_closed = FALSE;
    }

    // TODO: can we be sure that the path was created correctly?
    // TODO: should we offer an option to collect the clicks in a list?
    if (pc->expecting_clicks_for_LPE == 0 && sp_pen_context_has_waiting_LPE(pc)) {
        sp_pen_context_set_polyline_mode(pc);

        SPEventContext *ec = SP_EVENT_CONTEXT(pc);
        Inkscape::Selection *selection = sp_desktop_selection (ec->desktop);

        if (pc->waiting_LPE) {
            // we have an already created LPE waiting for a path
            pc->waiting_LPE->acceptParamPath(SP_PATH(selection->singleItem()));
            selection->add(SP_OBJECT(pc->waiting_item));
            pc->waiting_LPE = NULL;
        } else {
            // the case that we need to create a new LPE and apply it to the just-drawn path is
            // handled in spdc_check_for_and_apply_waiting_LPE() in draw-context.cpp
        }
    }

    return ret;
}

static gint
pen_handle_2button_press(SPPenContext *const pc, GdkEventButton const &bevent)
{
    gint ret = FALSE;
    if (pc->npoints != 0 && bevent.button != 2) {
        spdc_pen_finish(pc, FALSE);
        ret = TRUE;
    }
    return ret;
}

void
pen_redraw_all (SPPenContext *const pc)
{
    // green
    if (pc->green_bpaths) {
        // remove old piecewise green canvasitems
        while (pc->green_bpaths) {
            gtk_object_destroy(GTK_OBJECT(pc->green_bpaths->data));
            pc->green_bpaths = g_slist_remove(pc->green_bpaths, pc->green_bpaths->data);
        }
        // one canvas bpath for all of green_curve
        SPCanvasItem *cshape = sp_canvas_bpath_new(sp_desktop_sketch(pc->desktop), pc->green_curve);
        sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(cshape), pc->green_color, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
        sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(cshape), 0, SP_WIND_RULE_NONZERO);

        pc->green_bpaths = g_slist_prepend(pc->green_bpaths, cshape);
    }

    if (pc->green_anchor)
        SP_CTRL(pc->green_anchor->ctrl)->moveto(pc->green_anchor->dp);

    pc->red_curve->reset();
    pc->red_curve->moveto(pc->p[0]);
    pc->red_curve->curveto(pc->p[1], pc->p[2], pc->p[3]);
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->red_bpath), pc->red_curve);

    // handles
    if (pc->p[0] != pc->p[1]) {
        SP_CTRL(pc->c1)->moveto(pc->p[1]);
        sp_ctrlline_set_coords(SP_CTRLLINE(pc->cl1), pc->p[0], pc->p[1]);
        sp_canvas_item_show (pc->c1);
        sp_canvas_item_show (pc->cl1);
    } else {
        sp_canvas_item_hide (pc->c1);
        sp_canvas_item_hide (pc->cl1);
    }

    Geom::Curve const * last_seg = pc->green_curve->last_segment();
    if (last_seg) {
        Geom::CubicBezier const * cubic = dynamic_cast<Geom::CubicBezier const *>( last_seg );
        if ( cubic &&
             (*cubic)[2] != to_2geom(pc->p[0]) )
        {
            NR::Point p2 = (*cubic)[2];
            SP_CTRL(pc->c0)->moveto(p2);
            sp_ctrlline_set_coords(SP_CTRLLINE(pc->cl0), p2, pc->p[0]);
            sp_canvas_item_show (pc->c0);
            sp_canvas_item_show (pc->cl0);
        } else {
            sp_canvas_item_hide (pc->c0);
            sp_canvas_item_hide (pc->cl0);
        }
    }
}

void
pen_lastpoint_move (SPPenContext *const pc, gdouble x, gdouble y)
{
    if (pc->npoints != 5)
        return;

    // green
    if (!pc->green_curve->is_empty()) {
        pc->green_curve->last_point_additive_move( Geom::Point(x,y) );
    } else {
        // start anchor too
        if (pc->green_anchor) {
            pc->green_anchor->dp += NR::Point(x, y);
        }
    }

    // red
    pc->p[0] += NR::Point(x, y);
    pc->p[1] += NR::Point(x, y);
    pen_redraw_all(pc);
}

void
pen_lastpoint_move_screen (SPPenContext *const pc, gdouble x, gdouble y)
{
    pen_lastpoint_move (pc, x / pc->desktop->current_zoom(), y / pc->desktop->current_zoom());
}

void
pen_lastpoint_tocurve (SPPenContext *const pc)
{
    if (pc->npoints != 5)
        return;

    Geom::CubicBezier const * cubic = dynamic_cast<Geom::CubicBezier const *>( pc->green_curve->last_segment() );
    if ( cubic ) {
        pc->p[1] = pc->p[0] + (NR::Point)( (*cubic)[3] - (*cubic)[2] );
    } else {
        pc->p[1] = pc->p[0] + (1./3)*(pc->p[3] - pc->p[0]);
    }

    pen_redraw_all(pc);
}

void
pen_lastpoint_toline (SPPenContext *const pc)
{
    if (pc->npoints != 5)
        return;

    pc->p[1] = pc->p[0];

    pen_redraw_all(pc);
}


static gint
pen_handle_key_press(SPPenContext *const pc, GdkEvent *event)
{
    gint ret = FALSE;
    gdouble const nudge = prefs_get_double_attribute_limited("options.nudgedistance", "value", 2, 0, 1000); // in px

    switch (get_group0_keyval (&event->key)) {

        case GDK_Left: // move last point left
        case GDK_KP_Left:
        case GDK_KP_4:
            if (!MOD__CTRL) { // not ctrl
                if (MOD__ALT) { // alt
                    if (MOD__SHIFT) pen_lastpoint_move_screen(pc, -10, 0); // shift
                    else pen_lastpoint_move_screen(pc, -1, 0); // no shift
                }
                else { // no alt
                    if (MOD__SHIFT) pen_lastpoint_move(pc, -10*nudge, 0); // shift
                    else pen_lastpoint_move(pc, -nudge, 0); // no shift
                }
                ret = TRUE;
            }
            break;
        case GDK_Up: // move last point up
        case GDK_KP_Up:
        case GDK_KP_8:
            if (!MOD__CTRL) { // not ctrl
                if (MOD__ALT) { // alt
                    if (MOD__SHIFT) pen_lastpoint_move_screen(pc, 0, 10); // shift
                    else pen_lastpoint_move_screen(pc, 0, 1); // no shift
                }
                else { // no alt
                    if (MOD__SHIFT) pen_lastpoint_move(pc, 0, 10*nudge); // shift
                    else pen_lastpoint_move(pc, 0, nudge); // no shift
                }
                ret = TRUE;
            }
            break;
        case GDK_Right: // move last point right
        case GDK_KP_Right:
        case GDK_KP_6:
            if (!MOD__CTRL) { // not ctrl
                if (MOD__ALT) { // alt
                    if (MOD__SHIFT) pen_lastpoint_move_screen(pc, 10, 0); // shift
                    else pen_lastpoint_move_screen(pc, 1, 0); // no shift
                }
                else { // no alt
                    if (MOD__SHIFT) pen_lastpoint_move(pc, 10*nudge, 0); // shift
                    else pen_lastpoint_move(pc, nudge, 0); // no shift
                }
                ret = TRUE;
            }
            break;
        case GDK_Down: // move last point down
        case GDK_KP_Down:
        case GDK_KP_2:
            if (!MOD__CTRL) { // not ctrl
                if (MOD__ALT) { // alt
                    if (MOD__SHIFT) pen_lastpoint_move_screen(pc, 0, -10); // shift
                    else pen_lastpoint_move_screen(pc, 0, -1); // no shift
                }
                else { // no alt
                    if (MOD__SHIFT) pen_lastpoint_move(pc, 0, -10*nudge); // shift
                    else pen_lastpoint_move(pc, 0, -nudge); // no shift
                }
                ret = TRUE;
            }
            break;

        case GDK_P:
        case GDK_p:
            if (MOD__SHIFT_ONLY) {
                sp_pen_context_wait_for_LPE_mouse_clicks(pc, Inkscape::LivePathEffect::PARALLEL, 2);
                ret = TRUE;
            }
            break;

        case GDK_C:
        case GDK_c:
            if (MOD__SHIFT_ONLY) {
                sp_pen_context_wait_for_LPE_mouse_clicks(pc, Inkscape::LivePathEffect::CIRCLE_3PTS, 3);
                ret = TRUE;
            }
            break;

        case GDK_B:
        case GDK_b:
            if (MOD__SHIFT_ONLY) {
                sp_pen_context_wait_for_LPE_mouse_clicks(pc, Inkscape::LivePathEffect::PERP_BISECTOR, 2);
                ret = TRUE;
            }
            break;

        case GDK_A:
        case GDK_a:
            if (MOD__SHIFT_ONLY) {
                sp_pen_context_wait_for_LPE_mouse_clicks(pc, Inkscape::LivePathEffect::ANGLE_BISECTOR, 3);
                ret = TRUE;
            }
            break;

        case GDK_U:
        case GDK_u:
            if (MOD__SHIFT_ONLY) {
                pen_lastpoint_tocurve(pc);
                ret = TRUE;
            }
            break;
        case GDK_L:
        case GDK_l:
            if (MOD__SHIFT_ONLY) {
                pen_lastpoint_toline(pc);
                ret = TRUE;
            }
            break;

        case GDK_Return:
        case GDK_KP_Enter:
            if (pc->npoints != 0) {
                spdc_pen_finish(pc, FALSE);
                ret = TRUE;
            }
            break;
        case GDK_Escape:
            if (pc->npoints != 0) {
                // if drawing, cancel, otherwise pass it up for deselecting
                pen_cancel (pc);
                ret = TRUE;
            }
            break;
        case GDK_z:
        case GDK_Z:
            if (MOD__CTRL_ONLY && pc->npoints != 0) {
                // if drawing, cancel, otherwise pass it up for undo
                pen_cancel (pc);
                ret = TRUE;
            }
            break;
        case GDK_g:
        case GDK_G:
            if (MOD__SHIFT_ONLY) {
                sp_selection_to_guides();
                ret = true;
            }
            break;
        case GDK_BackSpace:
        case GDK_Delete:
        case GDK_KP_Delete:
            if (pc->green_curve->is_empty()) {
                if (!pc->red_curve->is_empty()) {
                    pen_cancel (pc);
                    ret = TRUE;
                } else {
                    // do nothing; this event should be handled upstream
                }
            } else {
                /* Reset red curve */
                pc->red_curve->reset();
                /* Destroy topmost green bpath */
                if (pc->green_bpaths) {
                    if (pc->green_bpaths->data)
                        gtk_object_destroy(GTK_OBJECT(pc->green_bpaths->data));
                    pc->green_bpaths = g_slist_remove(pc->green_bpaths, pc->green_bpaths->data);
                }
                /* Get last segment */
                if ( pc->green_curve->is_empty() ) {
                    g_warning("pen_handle_key_press, case GDK_KP_Delete: Green curve is empty");
                    break;
                }
                // The code below assumes that pc->green_curve has only ONE path !
                Geom::Path const & path = pc->green_curve->get_pathvector().back();
                Geom::Curve const * crv = &path.back_default();
                pc->p[0] = crv->initialPoint();
                if ( Geom::CubicBezier const * cubic = dynamic_cast<Geom::CubicBezier const *>(crv)) {
                    pc->p[1] = (*cubic)[1];
                } else {
                    pc->p[1] = pc->p[0];
                }
                NR::Point const pt(( pc->npoints < 4
                                     ? (NR::Point)(crv->finalPoint())
                                     : pc->p[3] ));
                pc->npoints = 2;
                pc->green_curve->backspace();
                sp_canvas_item_hide(pc->c0);
                sp_canvas_item_hide(pc->c1);
                sp_canvas_item_hide(pc->cl0);
                sp_canvas_item_hide(pc->cl1);
                pc->state = SP_PEN_CONTEXT_POINT;
                spdc_pen_set_subsequent_point(pc, pt, true);
                ret = TRUE;
            }
            break;
        default:
            break;
    }
    return ret;
}

static void
spdc_reset_colors(SPPenContext *pc)
{
    /* Red */
    pc->red_curve->reset();
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->red_bpath), NULL);
    /* Blue */
    pc->blue_curve->reset();
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->blue_bpath), NULL);
    /* Green */
    while (pc->green_bpaths) {
        gtk_object_destroy(GTK_OBJECT(pc->green_bpaths->data));
        pc->green_bpaths = g_slist_remove(pc->green_bpaths, pc->green_bpaths->data);
    }
    pc->green_curve->reset();
    if (pc->green_anchor) {
        pc->green_anchor = sp_draw_anchor_destroy(pc->green_anchor);
    }
    pc->sa = NULL;
    pc->ea = NULL;
    pc->npoints = 0;
    pc->red_curve_is_valid = false;
}


static void
spdc_pen_set_initial_point(SPPenContext *const pc, NR::Point const p)
{
    g_assert( pc->npoints == 0 );

    pc->p[0] = p;
    pc->p[1] = p;
    pc->npoints = 2;
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->red_bpath), NULL);

    sp_canvas_force_full_redraw_after_interruptions(pc->desktop->canvas, 5);
}

/**
 * Show the status message for the current line/curve segment.
 * This type of message always shows angle/distance as the last
 * two parameters ("angle %3.2f&#176;, distance %s").
 */ 
static void
spdc_pen_set_angle_distance_status_message(SPPenContext *const pc, NR::Point const p, int pc_point_to_compare, gchar const *message)
{
    g_assert(pc != NULL);
    g_assert((pc_point_to_compare == 0) || (pc_point_to_compare == 3)); // exclude control handles
    g_assert(message != NULL);

    SPDesktop *desktop = SP_EVENT_CONTEXT(pc)->desktop;
    NR::Point rel = p - pc->p[pc_point_to_compare];
    GString *dist = SP_PX_TO_METRIC_STRING(NR::L2(rel), desktop->namedview->getDefaultMetric());
    double angle = atan2(rel[NR::Y], rel[NR::X]) * 180 / M_PI;
    if (prefs_get_int_attribute("options.compassangledisplay", "value", 0) != 0)
        angle = angle_to_compass (angle);

    pc->_message_context->setF(Inkscape::IMMEDIATE_MESSAGE, message, angle, dist->str);
    g_string_free(dist, FALSE);
}

static void
spdc_pen_set_subsequent_point(SPPenContext *const pc, NR::Point const p, bool statusbar, guint status)
{
    g_assert( pc->npoints != 0 );
    /* todo: Check callers to see whether 2 <= npoints is guaranteed. */

    pc->p[2] = p;
    pc->p[3] = p;
    pc->p[4] = p;
    pc->npoints = 5;
    pc->red_curve->reset();
    bool is_curve;
    pc->red_curve->moveto(pc->p[0]);
    if (pc->polylines_paraxial && !statusbar) {
        // we are drawing horizontal/vertical lines and hit an anchor; draw an L-shaped path
        NR::Point intermed = p;
        pen_set_to_nearest_horiz_vert(pc, intermed, status);
        pc->red_curve->lineto(intermed);
        pc->red_curve->lineto(p);
        is_curve = false;
    } else {
        // one of the 'regular' modes
        if (pc->p[1] != pc->p[0])
        {
            pc->red_curve->curveto(pc->p[1], p, p);
            is_curve = true;
        } else {
            pc->red_curve->lineto(p);
            is_curve = false;
        }
    }

    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->red_bpath), pc->red_curve);

    if (statusbar) {
        gchar *message = is_curve ?
            _("<b>Curve segment</b>: angle %3.2f&#176;, distance %s; with <b>Ctrl</b> to snap angle, <b>Enter</b> to finish the path" ):
            _("<b>Line segment</b>: angle %3.2f&#176;, distance %s; with <b>Ctrl</b> to snap angle, <b>Enter</b> to finish the path");
        spdc_pen_set_angle_distance_status_message(pc, p, 0, message);
    }
}

static void
spdc_pen_set_ctrl(SPPenContext *const pc, NR::Point const p, guint const state)
{
    sp_canvas_item_show(pc->c1);
    sp_canvas_item_show(pc->cl1);

    if ( pc->npoints == 2 ) {
        pc->p[1] = p;
        sp_canvas_item_hide(pc->c0);
        sp_canvas_item_hide(pc->cl0);
        SP_CTRL(pc->c1)->moveto(pc->p[1]);
        sp_ctrlline_set_coords(SP_CTRLLINE(pc->cl1), pc->p[0], pc->p[1]);

        spdc_pen_set_angle_distance_status_message(pc, p, 0, _("<b>Curve handle</b>: angle %3.2f&#176;, length %s; with <b>Ctrl</b> to snap angle"));
    } else if ( pc->npoints == 5 ) {
        pc->p[4] = p;
        sp_canvas_item_show(pc->c0);
        sp_canvas_item_show(pc->cl0);
        bool is_symm = false;
        if ( ( ( pc->mode == SP_PEN_CONTEXT_MODE_CLICK ) && ( state & GDK_CONTROL_MASK ) ) ||
             ( ( pc->mode == SP_PEN_CONTEXT_MODE_DRAG ) &&  !( state & GDK_SHIFT_MASK  ) ) ) {
            NR::Point delta = p - pc->p[3];
            pc->p[2] = pc->p[3] - delta;
            is_symm = true;
            pc->red_curve->reset();
            pc->red_curve->moveto(pc->p[0]);
            pc->red_curve->curveto(pc->p[1], pc->p[2], pc->p[3]);
            sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->red_bpath), pc->red_curve);
        }
        SP_CTRL(pc->c0)->moveto(pc->p[2]);
        sp_ctrlline_set_coords(SP_CTRLLINE(pc->cl0), pc->p[3], pc->p[2]);
        SP_CTRL(pc->c1)->moveto(pc->p[4]);
        sp_ctrlline_set_coords(SP_CTRLLINE(pc->cl1), pc->p[3], pc->p[4]);

        gchar *message = is_symm ?
            _("<b>Curve handle, symmetric</b>: angle %3.2f&#176;, length %s; with <b>Ctrl</b> to snap angle, with <b>Shift</b> to move this handle only") :
            _("<b>Curve handle</b>: angle %3.2f&#176;, length %s; with <b>Ctrl</b> to snap angle, with <b>Shift</b> to move this handle only");
        spdc_pen_set_angle_distance_status_message(pc, p, 3, message);
    } else {
        g_warning("Something bad happened - npoints is %d", pc->npoints);
    }
}

static void
spdc_pen_finish_segment(SPPenContext *const pc, NR::Point const p, guint const state)
{
    if (pc->polylines_paraxial) {
        pen_last_paraxial_dir = pen_next_paraxial_direction(pc, p, pc->p[0], state);
    }
    ++pc->num_clicks;

    if (!pc->red_curve->is_empty()) {
        pc->green_curve->append_continuous(pc->red_curve, 0.0625);
        SPCurve *curve = pc->red_curve->copy();
        /// \todo fixme:
        SPCanvasItem *cshape = sp_canvas_bpath_new(sp_desktop_sketch(pc->desktop), curve);
        curve->unref();
        sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(cshape), pc->green_color, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);

        pc->green_bpaths = g_slist_prepend(pc->green_bpaths, cshape);

        pc->p[0] = pc->p[3];
        pc->p[1] = pc->p[4];
        pc->npoints = 2;

        pc->red_curve->reset();
    }
}

static void
spdc_pen_finish(SPPenContext *const pc, gboolean const closed)
{
    if (pc->expecting_clicks_for_LPE > 1) {
        // don't let the path be finished before we have collected the required number of mouse clicks
        return;
    }

    pc->num_clicks = 0;

    pen_disable_events(pc);
    
    SPDesktop *const desktop = pc->desktop;
    pc->_message_context->clear();
    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Drawing finished"));

    pc->red_curve->reset();
    spdc_concat_colors_and_flush(pc, closed);
    pc->sa = NULL;
    pc->ea = NULL;

    pc->npoints = 0;
    pc->state = SP_PEN_CONTEXT_POINT;

    sp_canvas_item_hide(pc->c0);
    sp_canvas_item_hide(pc->c1);
    sp_canvas_item_hide(pc->cl0);
    sp_canvas_item_hide(pc->cl1);

    if (pc->green_anchor) {
        pc->green_anchor = sp_draw_anchor_destroy(pc->green_anchor);
    }


    sp_canvas_end_forced_full_redraws(pc->desktop->canvas);

    pen_enable_events(pc);
}

static void
pen_disable_events(SPPenContext *const pc) {
  pc->events_disabled++;
}

static void
pen_enable_events(SPPenContext *const pc) {
  g_return_if_fail(pc->events_disabled != 0);
  
  pc->events_disabled--;
}

void
sp_pen_context_wait_for_LPE_mouse_clicks(SPPenContext *pc, Inkscape::LivePathEffect::EffectType effect_type,
                                         unsigned int num_clicks, bool use_polylines)
{
    g_print ("Now waiting for %s to be applied\n",
             Inkscape::LivePathEffect::LPETypeConverter.get_label(effect_type).c_str());
    pc->expecting_clicks_for_LPE = num_clicks;
    pc->polylines_only = use_polylines;
    pc->polylines_paraxial = false; // TODO: think if this is correct for all cases
    pc->waiting_LPE_type = effect_type;
}

static int pen_next_paraxial_direction(const SPPenContext *const pc,
                                       NR::Point const &pt, NR::Point const &origin, guint state) {
    /*
     * after the first mouse click we determine whether the mouse pointer is closest to a
     * horizontal or vertical segment; for all subsequent mouse clicks, we use the direction
     * orthogonal to the last one; pressing Shift toggles the direction
     */
    if (pc->num_clicks == 0) {
        // first mouse click
        double dist_h = fabs(pt[NR::X] - origin[NR::X]);
        double dist_v = fabs(pt[NR::Y] - origin[NR::Y]);
        int ret = (dist_h < dist_v) ? 1 : 0; // 0 = horizontal, 1 = vertical
        pen_last_paraxial_dir = (state & GDK_SHIFT_MASK) ? 1 - ret : ret;
        return pen_last_paraxial_dir;
    } else {
        // subsequent mouse click
        return (state & GDK_SHIFT_MASK) ? pen_last_paraxial_dir : 1 - pen_last_paraxial_dir;
    }
}

void pen_set_to_nearest_horiz_vert(const SPPenContext *const pc, NR::Point &pt, guint const state)
{
    NR::Point const &origin = pc->p[0];

    int next_dir = pen_next_paraxial_direction(pc, pt, origin, state);

    if (next_dir == 0) {
        // line is forced to be horizontal
        pt[NR::Y] = origin[NR::Y];
    } else {
        // line is forced to be vertical
        pt[NR::X] = origin[NR::X];
    }
}

NR::Point pen_get_intermediate_horiz_vert(const SPPenContext *const pc, NR::Point const &pt)
{
    NR::Point const &origin = pc->p[0];

    if (pen_last_paraxial_dir == 0) {
        return NR::Point (origin[NR::X], pt[NR::Y]);
    } else {
        return NR::Point (pt[NR::X], origin[NR::Y]);
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

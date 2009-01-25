/** \file
 * Pencil event context implementation.
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

#include "pencil-context.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "draw-anchor.h"
#include "message-stack.h"
#include "message-context.h"
#include "modifier-fns.h"
#include "sp-path.h"
#include "preferences.h"
#include "snap.h"
#include "pixmaps/cursor-pencil.xpm"
#include <2geom/sbasis-to-bezier.h>
#include <2geom/bezier-utils.h>
#include "display/canvas-bpath.h"
#include <glibmm/i18n.h>
#include "libnr/in-svg-plane.h"
#include "context-fns.h"
#include "sp-namedview.h"
#include "xml/repr.h"
#include "document.h"
#include "desktop-style.h"
#include "macros.h"
#include "display/curve.h"

static void sp_pencil_context_class_init(SPPencilContextClass *klass);
static void sp_pencil_context_init(SPPencilContext *pc);
static void sp_pencil_context_setup(SPEventContext *ec);
static void sp_pencil_context_dispose(GObject *object);

static gint sp_pencil_context_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint pencil_handle_button_press(SPPencilContext *const pc, GdkEventButton const &bevent);
static gint pencil_handle_motion_notify(SPPencilContext *const pc, GdkEventMotion const &mevent);
static gint pencil_handle_button_release(SPPencilContext *const pc, GdkEventButton const &revent);
static gint pencil_handle_key_press(SPPencilContext *const pc, guint const keyval, guint const state);
static gint pencil_handle_key_release(SPPencilContext *const pc, guint const keyval, guint const state);

static void spdc_set_startpoint(SPPencilContext *pc, Geom::Point const p);
static void spdc_set_endpoint(SPPencilContext *pc, Geom::Point const p);
static void spdc_finish_endpoint(SPPencilContext *pc);
static void spdc_add_freehand_point(SPPencilContext *pc, Geom::Point p, guint state);
static void fit_and_split(SPPencilContext *pc);
static void interpolate(SPPencilContext *pc);
static void sketch_interpolate(SPPencilContext *pc);

static SPDrawContextClass *pencil_parent_class;
static Geom::Point pencil_drag_origin_w(0, 0);
static bool pencil_within_tolerance = false;

/**
 * Register SPPencilContext class with Gdk and return its type number.
 */
GType
sp_pencil_context_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPPencilContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_pencil_context_class_init,
            NULL, NULL,
            sizeof(SPPencilContext),
            4,
            (GInstanceInitFunc) sp_pencil_context_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_DRAW_CONTEXT, "SPPencilContext", &info, (GTypeFlags)0);
    }
    return type;
}

/**
 * Initialize SPPencilContext vtable.
 */
static void
sp_pencil_context_class_init(SPPencilContextClass *klass)
{
    GObjectClass *object_class;
    SPEventContextClass *event_context_class;

    object_class = (GObjectClass *) klass;
    event_context_class = (SPEventContextClass *) klass;

    pencil_parent_class = (SPDrawContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_pencil_context_dispose;

    event_context_class->setup = sp_pencil_context_setup;
    event_context_class->root_handler = sp_pencil_context_root_handler;
}

/**
 * Callback to initialize SPPencilContext object.
 */
static void
sp_pencil_context_init(SPPencilContext *pc)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(pc);

    event_context->cursor_shape = cursor_pencil_xpm;
    event_context->hot_x = 4;
    event_context->hot_y = 4;

    pc->npoints = 0;
    pc->state = SP_PENCIL_CONTEXT_IDLE;
    pc->req_tangent = Geom::Point(0, 0);

    // since SPPencilContext is not properly constructed...
    pc->sketch_interpolation = Geom::Piecewise<Geom::D2<Geom::SBasis> >();
    pc->sketch_n = 0;
}

/**
 * Callback to setup SPPencilContext object.
 */
static void
sp_pencil_context_setup(SPEventContext *ec)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/freehand/pencil/selcue")) {
        ec->enableSelectionCue();
    }

    if (((SPEventContextClass *) pencil_parent_class)->setup) {
        ((SPEventContextClass *) pencil_parent_class)->setup(ec);
    }

    SPPencilContext *const pc = SP_PENCIL_CONTEXT(ec);
    pc->is_drawing = false;

    pc->anchor_statusbar = false;
}

static void
sp_pencil_context_dispose(GObject *object)
{
    G_OBJECT_CLASS(pencil_parent_class)->dispose(object);
}

/** Snaps new node relative to the previous node. */
static void
spdc_endpoint_snap(SPPencilContext const *pc, Geom::Point &p, guint const state)
{
    if ((state & GDK_CONTROL_MASK)) { //CTRL enables constrained snapping
        spdc_endpoint_snap_rotation(pc, p, pc->p[0], state);
    } else {
        if (!(state & GDK_SHIFT_MASK)) { //SHIFT disables all snapping, except the angular snapping above
                                         //After all, the user explicitely asked for angular snapping by
                                         //pressing CTRL
            spdc_endpoint_snap_free(pc, p, state);
        }
    }
}

/**
 * Callback for handling all pencil context events.
 */
gint
sp_pencil_context_root_handler(SPEventContext *const ec, GdkEvent *event)
{
    SPPencilContext *const pc = SP_PENCIL_CONTEXT(ec);

    gint ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            ret = pencil_handle_button_press(pc, event->button);
            break;

        case GDK_MOTION_NOTIFY:
            ret = pencil_handle_motion_notify(pc, event->motion);
            break;

        case GDK_BUTTON_RELEASE:
            ret = pencil_handle_button_release(pc, event->button);
            break;

        case GDK_KEY_PRESS:
            ret = pencil_handle_key_press(pc, get_group0_keyval (&event->key), event->key.state);
            break;

        case GDK_KEY_RELEASE:
            ret = pencil_handle_key_release(pc, get_group0_keyval (&event->key), event->key.state);
            break;

        default:
            break;
    }

    if (!ret) {
        gint (*const parent_root_handler)(SPEventContext *, GdkEvent *)
            = ((SPEventContextClass *) pencil_parent_class)->root_handler;
        if (parent_root_handler) {
            ret = parent_root_handler(ec, event);
        }
    }

    return ret;
}

static gint
pencil_handle_button_press(SPPencilContext *const pc, GdkEventButton const &bevent)
{
    gint ret = FALSE;
    SPEventContext *event_context = SP_EVENT_CONTEXT(pc);
    if ( bevent.button == 1  && !event_context->space_panning) {

        SPDrawContext *dc = SP_DRAW_CONTEXT (pc);
        SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(dc);
        Inkscape::Selection *selection = sp_desktop_selection(desktop);

        if (Inkscape::have_viable_layer(desktop, dc->_message_context) == false) {
            return TRUE;
        }

        Geom::Point const button_w(bevent.x, bevent.y);

        /* Find desktop coordinates */
        Geom::Point p = pc->desktop->w2d(button_w);

        /* Test whether we hit any anchor. */
        SPDrawAnchor *anchor = spdc_test_inside(pc, button_w);

        pencil_drag_origin_w = Geom::Point(bevent.x,bevent.y);
        pencil_within_tolerance = true;

        switch (pc->state) {
            case SP_PENCIL_CONTEXT_ADDLINE:
                /* Current segment will be finished with release */
                ret = TRUE;
                break;
            default:
                /* Set first point of sequence */
            	SnapManager &m = desktop->namedview->snap_manager;
				m.setup(desktop);
				sp_canvas_set_snap_delay_active(desktop->canvas, true);

                if (bevent.state & GDK_CONTROL_MASK) {
                	if (!(bevent.state & GDK_SHIFT_MASK)) {
                		m.freeSnapReturnByRef(Inkscape::SnapPreferences::SNAPPOINT_NODE, p);
                	}
					spdc_create_single_dot(event_context, p, "/tools/freehand/pencil", bevent.state);
					sp_canvas_set_snap_delay_active(desktop->canvas, false);
					ret = true;
                    break;
                }
                if (anchor) {
                    p = anchor->dp;
                    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Continuing selected path"));
                } else {

                    if (!(bevent.state & GDK_SHIFT_MASK)) {
						// This is the first click of a new curve; deselect item so that
                        // this curve is not combined with it (unless it is drawn from its
                        // anchor, which is handled by the sibling branch above)
                        selection->clear();
                        desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Creating new path"));
                        m.freeSnapReturnByRef(Inkscape::SnapPreferences::SNAPPOINT_NODE, p);
                    } else if (selection->singleItem() && SP_IS_PATH(selection->singleItem())) {
                        desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Appending to selected path"));
                        m.freeSnapReturnByRef(Inkscape::SnapPreferences::SNAPPOINT_NODE, p);
                    }
                }
                pc->sa = anchor;
                spdc_set_startpoint(pc, p);
                ret = TRUE;
                break;
        }

        pc->is_drawing = true;
    }
    return ret;
}

static gint
pencil_handle_motion_notify(SPPencilContext *const pc, GdkEventMotion const &mevent)
{
   	SPDesktop *const dt = pc->desktop;

	if ((mevent.state & GDK_CONTROL_MASK) && (mevent.state & GDK_BUTTON1_MASK)) {
        // mouse was accidentally moved during Ctrl+click;
        // ignore the motion and create a single point
        pc->is_drawing = false;
        return TRUE;
    }
    gint ret = FALSE;

    SPEventContext *event_context = SP_EVENT_CONTEXT(pc);
    if (event_context->space_panning || mevent.state & GDK_BUTTON2_MASK || mevent.state & GDK_BUTTON3_MASK) {
        // allow scrolling
        return FALSE;
    }

    if ( ( mevent.state & GDK_BUTTON1_MASK ) && !pc->grab && pc->is_drawing) {
        /* Grab mouse, so release will not pass unnoticed */
        pc->grab = SP_CANVAS_ITEM(dt->acetate);
        sp_canvas_item_grab(pc->grab, ( GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK   |
                                        GDK_BUTTON_RELEASE_MASK |
                                        GDK_POINTER_MOTION_MASK  ),
                            NULL, mevent.time);
    }

    /* Find desktop coordinates */
    Geom::Point p = dt->w2d(Geom::Point(mevent.x, mevent.y));

    /* Test whether we hit any anchor. */
    SPDrawAnchor *anchor = spdc_test_inside(pc, Geom::Point(mevent.x, mevent.y));

    if (pencil_within_tolerance) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        gint const tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);
        if ( Geom::LInfty( Geom::Point(mevent.x,mevent.y) - pencil_drag_origin_w ) < tolerance ) {
            return FALSE;   // Do not drag if we're within tolerance from origin.
        }
    }

    // Once the user has moved farther than tolerance from the original location
    // (indicating they intend to move the object, not click), then always process the
    // motion notify coordinates as given (no snapping back to origin)
    pencil_within_tolerance = false;

    switch (pc->state) {
        case SP_PENCIL_CONTEXT_ADDLINE:
            /* Set red endpoint */
            if (anchor) {
                p = anchor->dp;
            } else {
                Geom::Point ptnr(p);
                spdc_endpoint_snap(pc, ptnr, mevent.state);
                p = ptnr;
            }
            spdc_set_endpoint(pc, p);
            ret = TRUE;
            break;
        default:
            /* We may be idle or already freehand */
            if ( mevent.state & GDK_BUTTON1_MASK && pc->is_drawing ) {
                if (pc->state == SP_PENCIL_CONTEXT_IDLE) {
                	sp_canvas_set_snap_delay_active(dt->canvas, false);
                }
            	pc->state = SP_PENCIL_CONTEXT_FREEHAND;

                if ( !pc->sa && !pc->green_anchor ) {
                    /* Create green anchor */
                    pc->green_anchor = sp_draw_anchor_new(pc, pc->green_curve, TRUE, pc->p[0]);
                }
                /** \todo
                 * fixme: I am not sure whether we want to snap to anchors
                 * in middle of freehand (Lauris)
                 */
                if (anchor) {
                    p = anchor->dp;
                }

                if ( pc->npoints != 0) { // buttonpress may have happened before we entered draw context!
					spdc_add_freehand_point(pc, p, mevent.state);
					ret = TRUE;
                }

                if (anchor && !pc->anchor_statusbar) {
                    pc->_message_context->set(Inkscape::NORMAL_MESSAGE, _("<b>Release</b> here to close and finish the path."));
                    pc->anchor_statusbar = true;
                } else if (!anchor && pc->anchor_statusbar) {
                    pc->_message_context->clear();
                    pc->anchor_statusbar = false;
                } else if (!anchor) {
                    pc->_message_context->set(Inkscape::NORMAL_MESSAGE, _("Drawing a freehand path"));
                }

            } else {
                if (anchor && !pc->anchor_statusbar) {
                    pc->_message_context->set(Inkscape::NORMAL_MESSAGE, _("<b>Drag</b> to continue the path from this point."));
                    pc->anchor_statusbar = true;
                } else if (!anchor && pc->anchor_statusbar) {
                    pc->_message_context->clear();
                    pc->anchor_statusbar = false;
                }
            }
            break;
    }
    return ret;
}

static gint
pencil_handle_button_release(SPPencilContext *const pc, GdkEventButton const &revent)
{
    gint ret = FALSE;

    SPEventContext *event_context = SP_EVENT_CONTEXT(pc);
    if ( revent.button == 1 && pc->is_drawing && !event_context->space_panning) {
        SPDesktop *const dt = pc->desktop;

        pc->is_drawing = false;

        /* Find desktop coordinates */
        Geom::Point p = dt->w2d(Geom::Point(revent.x, revent.y));

        /* Test whether we hit any anchor. */
        SPDrawAnchor *anchor = spdc_test_inside(pc, Geom::Point(revent.x,
                                                              revent.y));

        switch (pc->state) {
            case SP_PENCIL_CONTEXT_IDLE:
                /* Releasing button in idle mode means single click */
                /* We have already set up start point/anchor in button_press */
                if (!(revent.state & GDK_CONTROL_MASK)) {
                    // Ctrl+click creates a single point so only set context in ADDLINE mode when Ctrl isn't pressed
                    pc->state = SP_PENCIL_CONTEXT_ADDLINE;
                    //sp_canvas_set_snap_delay_active(dt->canvas, true);
                }
                ret = TRUE;
                break;
            case SP_PENCIL_CONTEXT_ADDLINE:
                /* Finish segment now */
                if (anchor) {
                    p = anchor->dp;
                } else {
                    spdc_endpoint_snap(pc, p, revent.state);
                }
                pc->ea = anchor;
                spdc_set_endpoint(pc, p);
                spdc_finish_endpoint(pc);
                pc->state = SP_PENCIL_CONTEXT_IDLE;
                sp_canvas_set_snap_delay_active(dt->canvas, false);
                ret = TRUE;
                break;
            case SP_PENCIL_CONTEXT_FREEHAND:
                if (revent.state & GDK_MOD1_MASK) {
                    /* sketch mode: interpolate the sketched path and improve the current output path with the new interpolation. don't finish sketch */

                    sketch_interpolate(pc);

                    if (pc->green_anchor) {
                        pc->green_anchor = sp_draw_anchor_destroy(pc->green_anchor);
                    }

                    pc->state = SP_PENCIL_CONTEXT_SKETCH;
                    //sp_canvas_set_snap_delay_active(dt->canvas, true);
                } else {
                    /* Finish segment now */
                    /// \todo fixme: Clean up what follows (Lauris)
                    if (anchor) {
                        p = anchor->dp;
                    }
                    pc->ea = anchor;
                    /* Write curves to object */

                    dt->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Finishing freehand"));

                    interpolate(pc);
                    spdc_concat_colors_and_flush(pc, FALSE);
                    pc->sa = NULL;
                    pc->ea = NULL;
                    if (pc->green_anchor) {
                        pc->green_anchor = sp_draw_anchor_destroy(pc->green_anchor);
                    }
                    pc->state = SP_PENCIL_CONTEXT_IDLE;
                    // sp_canvas_set_snap_delay_active(dt->canvas, false);
                    // reset sketch mode too
                    pc->sketch_n = 0;
                }
                ret = TRUE;
                break;
            case SP_PENCIL_CONTEXT_SKETCH:
            default:
                break;
        }

        if (pc->grab) {
            /* Release grab now */
            sp_canvas_item_ungrab(pc->grab, revent.time);
            pc->grab = NULL;
        }

        ret = TRUE;
    }
    return ret;
}

static void
pencil_cancel (SPPencilContext *const pc)
{
    if (pc->grab) {
        /* Release grab now */
        sp_canvas_item_ungrab(pc->grab, 0);
        pc->grab = NULL;
    }

    pc->is_drawing = false;
    pc->state = SP_PENCIL_CONTEXT_IDLE;
    sp_canvas_set_snap_delay_active(pc->desktop->canvas, false);

    pc->red_curve->reset();
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->red_bpath), NULL);
    while (pc->green_bpaths) {
        gtk_object_destroy(GTK_OBJECT(pc->green_bpaths->data));
        pc->green_bpaths = g_slist_remove(pc->green_bpaths, pc->green_bpaths->data);
    }
    pc->green_curve->reset();
    if (pc->green_anchor) {
        pc->green_anchor = sp_draw_anchor_destroy(pc->green_anchor);
    }

    pc->_message_context->clear();
    pc->_message_context->flash(Inkscape::NORMAL_MESSAGE, _("Drawing cancelled"));

    sp_canvas_end_forced_full_redraws(pc->desktop->canvas);
}

static gint
pencil_handle_key_press(SPPencilContext *const pc, guint const keyval, guint const state)
{
    gint ret = FALSE;
    switch (keyval) {
        case GDK_Up:
        case GDK_Down:
        case GDK_KP_Up:
        case GDK_KP_Down:
            // Prevent the zoom field from activation.
            if (!mod_ctrl_only(state)) {
                ret = TRUE;
            }
            break;
        case GDK_Escape:
            if (pc->npoints != 0) {
                // if drawing, cancel, otherwise pass it up for deselecting
                if (pc->state != SP_PENCIL_CONTEXT_IDLE) {
                    pencil_cancel (pc);
                    ret = TRUE;
                }
            }
            break;
        case GDK_z:
        case GDK_Z:
            if (mod_ctrl_only(state) && pc->npoints != 0) {
                // if drawing, cancel, otherwise pass it up for undo
                if (pc->state != SP_PENCIL_CONTEXT_IDLE) {
                    pencil_cancel (pc);
                    ret = TRUE;
                }
            }
            break;
        case GDK_g:
        case GDK_G:
            if (mod_shift_only(state)) {
                sp_selection_to_guides(SP_EVENT_CONTEXT(pc)->desktop);
                ret = true;
            }
            break;
        case GDK_Alt_L:
        case GDK_Alt_R:
        case GDK_Meta_L:
        case GDK_Meta_R:
            if (pc->state == SP_PENCIL_CONTEXT_IDLE) {
                pc->desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("<b>Sketch mode</b>: holding <b>Alt</b> interpolates between sketched paths. Release <b>Alt</b> to finalize."));
            }
            break;
        default:
            break;
    }
    return ret;
}

static gint
pencil_handle_key_release(SPPencilContext *const pc, guint const keyval, guint const /*state*/)
{
    gint ret = FALSE;
    switch (keyval) {
        case GDK_Alt_L:
        case GDK_Alt_R:
        case GDK_Meta_L:
        case GDK_Meta_R:
            if (pc->state == SP_PENCIL_CONTEXT_SKETCH) {
                spdc_concat_colors_and_flush(pc, FALSE);
                pc->sketch_n = 0;
                pc->sa = NULL;
                pc->ea = NULL;
                if (pc->green_anchor) {
                    pc->green_anchor = sp_draw_anchor_destroy(pc->green_anchor);
                }
                pc->state = SP_PENCIL_CONTEXT_IDLE;
                sp_canvas_set_snap_delay_active(pc->desktop->canvas, false);
                pc->desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Finishing freehand sketch"));
                ret = TRUE;
            }
            break;
        default:
            break;
    }
    return ret;
}

/**
 * Reset points and set new starting point.
 */
static void
spdc_set_startpoint(SPPencilContext *const pc, Geom::Point const p)
{
    pc->npoints = 0;
    pc->red_curve_is_valid = false;
    if (in_svg_plane(p)) {
        pc->p[pc->npoints++] = p;
    }
}

/**
 * Change moving endpoint position.
 * <ul>
 * <li>Ctrl constrains to moving to H/V direction, snapping in given direction.
 * <li>Otherwise we snap freely to whatever attractors are available.
 * </ul>
 *
 * Number of points is (re)set to 2 always, 2nd point is modified.
 * We change RED curve.
 */
static void
spdc_set_endpoint(SPPencilContext *const pc, Geom::Point const p)
{
    if (pc->npoints == 0) {
        return;
        /* May occur if first point wasn't in SVG plane (e.g. weird w2d transform, perhaps from bad
         * zoom setting).
         */
    }
    g_return_if_fail( pc->npoints > 0 );

    pc->red_curve->reset();
    if ( ( p == pc->p[0] )
         || !in_svg_plane(p) )
    {
        pc->npoints = 1;
    } else {
        pc->p[1] = p;
        pc->npoints = 2;

        pc->red_curve->moveto(pc->p[0]);
        pc->red_curve->lineto(pc->p[1]);
        pc->red_curve_is_valid = true;

        sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->red_bpath), pc->red_curve);
    }
}

/**
 * Finalize addline.
 *
 * \todo
 * fixme: I'd like remove red reset from concat colors (lauris).
 * Still not sure, how it will make most sense.
 */
static void
spdc_finish_endpoint(SPPencilContext *const pc)
{
    if ( ( pc->red_curve->is_empty() )
         || ( *(pc->red_curve->first_point()) == *(pc->red_curve->second_point())   ) )
    {
        pc->red_curve->reset();
        sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->red_bpath), NULL);
    } else {
        /* Write curves to object. */
        spdc_concat_colors_and_flush(pc, FALSE);
        pc->sa = NULL;
        pc->ea = NULL;
    }
}


static void
spdc_add_freehand_point(SPPencilContext *pc, Geom::Point p, guint /*state*/)
{
    g_assert( pc->npoints > 0 );
    g_return_if_fail(unsigned(pc->npoints) < G_N_ELEMENTS(pc->p));

    if ( ( p != pc->p[ pc->npoints - 1 ] )
         && in_svg_plane(p) )
    {
        pc->ps.push_back(p);
        pc->p[pc->npoints++] = p;
        fit_and_split(pc);
    }
}

static inline double
square(double const x)
{
    return x * x;
}

static void
interpolate(SPPencilContext *pc)
{

    if ( pc->ps.size() <= 1 ) {
    	return;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    double const tol = prefs->getDoubleLimited("/tools/freehand/pencil/tolerance", 10.0, 1.0, 100.0) * 0.4;
    double const tolerance_sq = 0.02 * square( pc->desktop->w2d().descrim() *
                                               tol) * exp(0.2*tol - 2);

    g_assert(is_zero(pc->req_tangent)
             || is_unit_vector(pc->req_tangent));
    Geom::Point const tHatEnd(0, 0);

    guint n_points  = pc->ps.size();
    pc->green_curve->reset();
    pc->red_curve->reset();
    pc->red_curve_is_valid = false;

    Geom::Point * b = g_new(Geom::Point, 4*n_points);
    Geom::Point * points = g_new(Geom::Point, 4*n_points);
    for (unsigned int i = 0; i < pc->ps.size(); i++) {
        points[i] = pc->ps[i];
    }

    // worst case gives us a segment per point
    int max_segs = 4*n_points;

    int const n_segs = Geom::bezier_fit_cubic_r(b, points, n_points,
                                             tolerance_sq, max_segs);

    if ( n_segs > 0)
    {
        /* Fit and draw and reset state */
        pc->green_curve->moveto(b[0]);
        for (int c = 0; c < n_segs; c++) {
            pc->green_curve->curveto(b[4*c+1], b[4*c+2], b[4*c+3]);
        }
        sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->red_bpath), pc->green_curve);

        /* Fit and draw and copy last point */
        g_assert(!pc->green_curve->is_empty());

        /* Set up direction of next curve. */
        {
            Geom::CubicBezier const * last_seg = dynamic_cast<Geom::CubicBezier const *>(pc->green_curve->last_segment());
            g_assert( last_seg );      // Relevance: validity of (*last_seg)[2]
            pc->p[0] = last_seg->finalPoint();
            pc->npoints = 1;
            Geom::Point const req_vec( pc->p[0] - (*last_seg)[2] );
            pc->req_tangent = ( ( Geom::is_zero(req_vec) || !in_svg_plane(req_vec) )
                                ? Geom::Point(0, 0)
                                : Geom::unit_vector(req_vec) );
        }
    }
    g_free(b);
    g_free(points);
    pc->ps.clear();
}


/* interpolates the sketched curve and tweaks the current sketch interpolation*/
static void
sketch_interpolate(SPPencilContext *pc)
{
    g_assert( pc->ps.size() > 1 );

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    double const tol = prefs->getDoubleLimited("/tools/freehand/pencil/tolerance", 10.0, 1.0, 100.0) * 0.4;
    double const tolerance_sq = 0.02 * square( pc->desktop->w2d().descrim() *
                                               tol) * exp(0.2*tol - 2);

    bool average_all_sketches = prefs->getBool("/tools/freehand/pencil/average_all_sketches", true);

    g_assert(is_zero(pc->req_tangent)
             || is_unit_vector(pc->req_tangent));
    Geom::Point const tHatEnd(0, 0);

    guint n_points  = pc->ps.size();
    pc->red_curve->reset();
    pc->red_curve_is_valid = false;

    Geom::Point * b = g_new(Geom::Point, 4*n_points);
    Geom::Point * points = g_new(Geom::Point, 4*n_points);
    for (unsigned i = 0; i < pc->ps.size(); i++) {
        points[i] = pc->ps[i];
    }

    // worst case gives us a segment per point
    int max_segs = 4*n_points;

    int const n_segs = Geom::bezier_fit_cubic_r(b, points, n_points,
                                             tolerance_sq, max_segs);

    if ( n_segs > 0)
    {
        Geom::Path fit(b[0]);
        for (int c = 0; c < n_segs; c++) {
            fit.appendNew<Geom::CubicBezier>(b[4*c+1], b[4*c+2], b[4*c+3]);
        }
        Geom::Piecewise<Geom::D2<Geom::SBasis> > fit_pwd2 = fit.toPwSb();

        double t =0.;
        if ( pc->sketch_n > 0 ) {
            if (average_all_sketches) {
                // Average = (sum of all) / n
                //         = (sum of all + new one) / n+1
                //         = ((old average)*n + new one) / n+1
                t = pc->sketch_n / (pc->sketch_n + 1.);
            } else {
                t = 0.5;
            }
            pc->sketch_interpolation = Geom::lerp(t, fit_pwd2, pc->sketch_interpolation);
        } else {
            pc->sketch_interpolation = fit_pwd2;
        }
        pc->sketch_n++;

        pc->green_curve->reset();
        pc->green_curve->set_pathvector(Geom::path_from_piecewise(pc->sketch_interpolation, 0.01));
        sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->red_bpath), pc->green_curve);

        /* Fit and draw and copy last point */
        g_assert(!pc->green_curve->is_empty());

        /* Set up direction of next curve. */
        {
            Geom::CubicBezier const * last_seg = dynamic_cast<Geom::CubicBezier const *>(pc->green_curve->last_segment());
            g_assert( last_seg );      // Relevance: validity of (*last_seg)[2]
            pc->p[0] = last_seg->finalPoint();
            pc->npoints = 1;
            Geom::Point const req_vec( pc->p[0] - (*last_seg)[2] );
            pc->req_tangent = ( ( Geom::is_zero(req_vec) || !in_svg_plane(req_vec) )
                                ? Geom::Point(0, 0)
                                : Geom::unit_vector(req_vec) );
        }
    }
    g_free(b);
    g_free(points);
    pc->ps.clear();
}

static void
fit_and_split(SPPencilContext *pc)
{
    g_assert( pc->npoints > 1 );

    double const tolerance_sq = 0;

    Geom::Point b[4];
    g_assert(is_zero(pc->req_tangent)
             || is_unit_vector(pc->req_tangent));
    Geom::Point const tHatEnd(0, 0);
    int const n_segs = Geom::bezier_fit_cubic_full(b, NULL, pc->p, pc->npoints,
                                                pc->req_tangent, tHatEnd,
                                                tolerance_sq, 1);
    if ( n_segs > 0
         && unsigned(pc->npoints) < G_N_ELEMENTS(pc->p) )
    {
        /* Fit and draw and reset state */
        pc->red_curve->reset();
        pc->red_curve->moveto(b[0]);
        pc->red_curve->curveto(b[1], b[2], b[3]);
        sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->red_bpath), pc->red_curve);
        pc->red_curve_is_valid = true;
    } else {
        /* Fit and draw and copy last point */

        g_assert(!pc->red_curve->is_empty());

        /* Set up direction of next curve. */
        {
            Geom::CubicBezier const * last_seg = dynamic_cast<Geom::CubicBezier const *>(pc->red_curve->last_segment());
            g_assert( last_seg );      // Relevance: validity of (*last_seg)[2]
            pc->p[0] = last_seg->finalPoint();
            pc->npoints = 1;
            Geom::Point const req_vec( pc->p[0] - (*last_seg)[2] );
            pc->req_tangent = ( ( Geom::is_zero(req_vec) || !in_svg_plane(req_vec) )
                                ? Geom::Point(0, 0)
                                : Geom::unit_vector(req_vec) );
        }

        pc->green_curve->append_continuous(pc->red_curve, 0.0625);
        SPCurve *curve = pc->red_curve->copy();

        /// \todo fixme:
        SPCanvasItem *cshape = sp_canvas_bpath_new(sp_desktop_sketch(pc->desktop), curve);
        curve->unref();
        sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(cshape), pc->green_color, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);

        pc->green_bpaths = g_slist_prepend(pc->green_bpaths, cshape);

        pc->red_curve_is_valid = false;
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

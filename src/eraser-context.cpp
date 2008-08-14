/*
 * Eraser drawing mode
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   MenTaLguY <mental@rydia.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * The original dynadraw code:
 *   Paul Haeberli <paul@sgi.com>
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 2005-2007 bulia byak
 * Copyright (C) 2006 MenTaLguY
 * Copyright (C) 2008 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noERASER_VERBOSE

#include "config.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glibmm/i18n.h>
#include <string>
#include <cstring>
#include <numeric>

#include "svg/svg.h"
#include "display/canvas-bpath.h"
#include "display/bezier-utils.h"

#include <glib/gmem.h>
#include "macros.h"
#include "document.h"
#include "selection.h"
#include "desktop.h"
#include "desktop-events.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "desktop-style.h"
#include "message-context.h"
#include "prefs-utils.h"
#include "pixmaps/cursor-eraser.xpm"
#include "xml/repr.h"
#include "context-fns.h"
#include "sp-item.h"
#include "color.h"
#include "rubberband.h"
#include "splivarot.h"
#include "sp-item-group.h"
#include "sp-shape.h"
#include "sp-path.h"
#include "sp-text.h"
#include "display/canvas-bpath.h"
#include "display/canvas-arena.h"
#include "livarot/Shape.h"
#include <2geom/isnan.h>
#include <2geom/pathvector.h>

#include "eraser-context.h"

#define ERC_RED_RGBA 0xff0000ff

#define TOLERANCE_ERASER 0.1

#define ERASER_EPSILON 0.5e-6
#define ERASER_EPSILON_START 0.5e-2
#define ERASER_VEL_START 1e-5

#define DRAG_MIN 0.0
#define DRAG_DEFAULT 1.0
#define DRAG_MAX 1.0


static void sp_eraser_context_class_init(SPEraserContextClass *klass);
static void sp_eraser_context_init(SPEraserContext *erc);
static void sp_eraser_context_dispose(GObject *object);

static void sp_eraser_context_setup(SPEventContext *ec);
static void sp_eraser_context_set(SPEventContext *ec, gchar const *key, gchar const *val);
static gint sp_eraser_context_root_handler(SPEventContext *ec, GdkEvent *event);

static void clear_current(SPEraserContext *dc);
static void set_to_accumulated(SPEraserContext *dc);
static void add_cap(SPCurve *curve, NR::Point const &pre, NR::Point const &from, NR::Point const &to, NR::Point const &post, double rounding);
static void accumulate_eraser(SPEraserContext *dc);

static void fit_and_split(SPEraserContext *erc, gboolean release);

static void sp_eraser_reset(SPEraserContext *erc, NR::Point p);
static NR::Point sp_eraser_get_npoint(SPEraserContext const *erc, NR::Point v);
static NR::Point sp_eraser_get_vpoint(SPEraserContext const *erc, NR::Point n);
static void draw_temporary_box(SPEraserContext *dc);


static SPEventContextClass *eraser_parent_class = 0;

GType sp_eraser_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPEraserContextClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_eraser_context_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPEraserContext),
            0, // n_preallocs
            (GInstanceInitFunc)sp_eraser_context_init,
            0 // value_table
        };
        type = g_type_register_static(SP_TYPE_COMMON_CONTEXT, "SPEraserContext", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}

static void
sp_eraser_context_class_init(SPEraserContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    eraser_parent_class = (SPEventContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_eraser_context_dispose;

    event_context_class->setup = sp_eraser_context_setup;
    event_context_class->set = sp_eraser_context_set;
    event_context_class->root_handler = sp_eraser_context_root_handler;
}

static void
sp_eraser_context_init(SPEraserContext *erc)
{
    erc->cursor_shape = cursor_eraser_xpm;
    erc->hot_x = 4;
    erc->hot_y = 4;
}

static void
sp_eraser_context_dispose(GObject *object)
{
    //SPEraserContext *erc = SP_ERASER_CONTEXT(object);

    G_OBJECT_CLASS(eraser_parent_class)->dispose(object);
}

static void
sp_eraser_context_setup(SPEventContext *ec)
{
    SPEraserContext *erc = SP_ERASER_CONTEXT(ec);
    SPDesktop *desktop = ec->desktop;

    if (((SPEventContextClass *) eraser_parent_class)->setup)
        ((SPEventContextClass *) eraser_parent_class)->setup(ec);

    erc->accumulated = new SPCurve();
    erc->currentcurve = new SPCurve();

    erc->cal1 = new SPCurve();
    erc->cal2 = new SPCurve();

    erc->currentshape = sp_canvas_item_new(sp_desktop_sketch(desktop), SP_TYPE_CANVAS_BPATH, NULL);
    sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(erc->currentshape), ERC_RED_RGBA, SP_WIND_RULE_EVENODD);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(erc->currentshape), 0x00000000, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
    /* fixme: Cannot we cascade it to root more clearly? */
    g_signal_connect(G_OBJECT(erc->currentshape), "event", G_CALLBACK(sp_desktop_root_handler), desktop);

/*
static ProfileFloatElement f_profile[PROFILE_FLOAT_SIZE] = {
    {"mass",0.02, 0.0, 1.0},
    {"wiggle",0.0, 0.0, 1.0},
    {"angle",30.0, -90.0, 90.0},
    {"thinning",0.1, -1.0, 1.0},
    {"tremor",0.0, 0.0, 1.0},
    {"flatness",0.9, 0.0, 1.0},
    {"cap_rounding",0.0, 0.0, 5.0}
};
*/

    sp_event_context_read(ec, "mass");
    sp_event_context_read(ec, "wiggle");
    sp_event_context_read(ec, "angle");
    sp_event_context_read(ec, "width");
    sp_event_context_read(ec, "thinning");
    sp_event_context_read(ec, "tremor");
    sp_event_context_read(ec, "flatness");
    sp_event_context_read(ec, "tracebackground");
    sp_event_context_read(ec, "usepressure");
    sp_event_context_read(ec, "usetilt");
    sp_event_context_read(ec, "abs_width");
    sp_event_context_read(ec, "cap_rounding");

    erc->is_drawing = false;

    erc->_message_context = new Inkscape::MessageContext(desktop->messageStack());

    if (prefs_get_int_attribute("tools.eraser", "selcue", 0) != 0) {
        ec->enableSelectionCue();
    }
// TODO temp force:
    ec->enableSelectionCue();

}

static void
sp_eraser_context_set(SPEventContext *ec, gchar const *key, gchar const *val)
{
    //pass on up to parent class to handle common attributes.
    if ( eraser_parent_class->set ) {
        eraser_parent_class->set(ec, key, val);
    }
}

static double
flerp(double f0, double f1, double p)
{
    return f0 + ( f1 - f0 ) * p;
}

/* Get normalized point */
static NR::Point
sp_eraser_get_npoint(SPEraserContext const *dc, NR::Point v)
{
    NR::Rect drect = SP_EVENT_CONTEXT(dc)->desktop->get_display_area();
    double const max = MAX ( drect.dimensions()[NR::X], drect.dimensions()[NR::Y] );
    return NR::Point(( v[NR::X] - drect.min()[NR::X] ) / max,  ( v[NR::Y] - drect.min()[NR::Y] ) / max);
}

/* Get view point */
static NR::Point
sp_eraser_get_vpoint(SPEraserContext const *dc, NR::Point n)
{
    NR::Rect drect = SP_EVENT_CONTEXT(dc)->desktop->get_display_area();
    double const max = MAX ( drect.dimensions()[NR::X], drect.dimensions()[NR::Y] );
    return NR::Point(n[NR::X] * max + drect.min()[NR::X], n[NR::Y] * max + drect.min()[NR::Y]);
}

static void
sp_eraser_reset(SPEraserContext *dc, NR::Point p)
{
    dc->last = dc->cur = sp_eraser_get_npoint(dc, p);
    dc->vel = NR::Point(0,0);
    dc->vel_max = 0;
    dc->acc = NR::Point(0,0);
    dc->ang = NR::Point(0,0);
    dc->del = NR::Point(0,0);
}

static void
sp_eraser_extinput(SPEraserContext *dc, GdkEvent *event)
{
    if (gdk_event_get_axis (event, GDK_AXIS_PRESSURE, &dc->pressure))
        dc->pressure = CLAMP (dc->pressure, ERC_MIN_PRESSURE, ERC_MAX_PRESSURE);
    else
        dc->pressure = ERC_DEFAULT_PRESSURE;

    if (gdk_event_get_axis (event, GDK_AXIS_XTILT, &dc->xtilt))
        dc->xtilt = CLAMP (dc->xtilt, ERC_MIN_TILT, ERC_MAX_TILT);
    else
        dc->xtilt = ERC_DEFAULT_TILT;

    if (gdk_event_get_axis (event, GDK_AXIS_YTILT, &dc->ytilt))
        dc->ytilt = CLAMP (dc->ytilt, ERC_MIN_TILT, ERC_MAX_TILT);
    else
        dc->ytilt = ERC_DEFAULT_TILT;
}


static gboolean
sp_eraser_apply(SPEraserContext *dc, NR::Point p)
{
    NR::Point n = sp_eraser_get_npoint(dc, p);

    /* Calculate mass and drag */
    double const mass = flerp(1.0, 160.0, dc->mass);
    double const drag = flerp(0.0, 0.5, dc->drag * dc->drag);

    /* Calculate force and acceleration */
    NR::Point force = n - dc->cur;

    // If force is below the absolute threshold ERASER_EPSILON,
    // or we haven't yet reached ERASER_VEL_START (i.e. at the beginning of stroke)
    // _and_ the force is below the (higher) ERASER_EPSILON_START threshold,
    // discard this move. 
    // This prevents flips, blobs, and jerks caused by microscopic tremor of the tablet pen,
    // especially bothersome at the start of the stroke where we don't yet have the inertia to
    // smooth them out.
    if ( NR::L2(force) < ERASER_EPSILON || (dc->vel_max < ERASER_VEL_START && NR::L2(force) < ERASER_EPSILON_START)) {
        return FALSE;
    }

    dc->acc = force / mass;

    /* Calculate new velocity */
    dc->vel += dc->acc;

    if (NR::L2(dc->vel) > dc->vel_max)
        dc->vel_max = NR::L2(dc->vel);

    /* Calculate angle of drawing tool */

    double a1;
    if (dc->usetilt) {
        // 1a. calculate nib angle from input device tilt:
        gdouble length = std::sqrt(dc->xtilt*dc->xtilt + dc->ytilt*dc->ytilt);;

        if (length > 0) {
            NR::Point ang1 = NR::Point(dc->ytilt/length, dc->xtilt/length);
            a1 = atan2(ang1);
        }
        else
            a1 = 0.0;
    }
    else {
        // 1b. fixed dc->angle (absolutely flat nib):
        double const radians = ( (dc->angle - 90) / 180.0 ) * M_PI;
        NR::Point ang1 = NR::Point(-sin(radians),  cos(radians));
        a1 = atan2(ang1);
    }

    // 2. perpendicular to dc->vel (absolutely non-flat nib):
    gdouble const mag_vel = NR::L2(dc->vel);
    if ( mag_vel < ERASER_EPSILON ) {
        return FALSE;
    }
    NR::Point ang2 = NR::rot90(dc->vel) / mag_vel;

    // 3. Average them using flatness parameter:
    // calculate angles
    double a2 = atan2(ang2);
    // flip a2 to force it to be in the same half-circle as a1
    bool flipped = false;
    if (fabs (a2-a1) > 0.5*M_PI) {
        a2 += M_PI;
        flipped = true;
    }
    // normalize a2
    if (a2 > M_PI)
        a2 -= 2*M_PI;
    if (a2 < -M_PI)
        a2 += 2*M_PI;
    // find the flatness-weighted bisector angle, unflip if a2 was flipped
    // FIXME: when dc->vel is oscillating around the fixed angle, the new_ang flips back and forth. How to avoid this?
    double new_ang = a1 + (1 - dc->flatness) * (a2 - a1) - (flipped? M_PI : 0);

    // Try to detect a sudden flip when the new angle differs too much from the previous for the
    // current velocity; in that case discard this move
    double angle_delta = NR::L2(NR::Point (cos (new_ang), sin (new_ang)) - dc->ang);
    if ( angle_delta / NR::L2(dc->vel) > 4000 ) {
        return FALSE;
    }

    // convert to point
    dc->ang = NR::Point (cos (new_ang), sin (new_ang));

//    g_print ("force %g  acc %g  vel_max %g  vel %g  a1 %g  a2 %g  new_ang %g\n", NR::L2(force), NR::L2(dc->acc), dc->vel_max, NR::L2(dc->vel), a1, a2, new_ang);

    /* Apply drag */
    dc->vel *= 1.0 - drag;

    /* Update position */
    dc->last = dc->cur;
    dc->cur += dc->vel;

    return TRUE;
}

static void
sp_eraser_brush(SPEraserContext *dc)
{
    g_assert( dc->npoints >= 0 && dc->npoints < SAMPLING_SIZE );

    // How much velocity thins strokestyle
    double vel_thin = flerp (0, 160, dc->vel_thin);

    // Influence of pressure on thickness
    double pressure_thick = (dc->usepressure ? dc->pressure : 1.0);

    // get the real brush point, not the same as pointer (affected by hatch tracking and/or mass
    // drag)
    NR::Point brush = sp_eraser_get_vpoint(dc, dc->cur);
    NR::Point brush_w = SP_EVENT_CONTEXT(dc)->desktop->d2w(brush); 

    double trace_thick = 1;

    double width = (pressure_thick * trace_thick - vel_thin * NR::L2(dc->vel)) * dc->width;

    double tremble_left = 0, tremble_right = 0;
    if (dc->tremor > 0) {
        // obtain two normally distributed random variables, using polar Box-Muller transform
        double x1, x2, w, y1, y2;
        do {
            x1 = 2.0 * g_random_double_range(0,1) - 1.0;
            x2 = 2.0 * g_random_double_range(0,1) - 1.0;
            w = x1 * x1 + x2 * x2;
        } while ( w >= 1.0 );
        w = sqrt( (-2.0 * log( w ) ) / w );
        y1 = x1 * w;
        y2 = x2 * w;

        // deflect both left and right edges randomly and independently, so that:
        // (1) dc->tremor=1 corresponds to sigma=1, decreasing dc->tremor narrows the bell curve;
        // (2) deflection depends on width, but is upped for small widths for better visual uniformity across widths;
        // (3) deflection somewhat depends on speed, to prevent fast strokes looking
        // comparatively smooth and slow ones excessively jittery
        tremble_left  = (y1)*dc->tremor * (0.15 + 0.8*width) * (0.35 + 14*NR::L2(dc->vel));
        tremble_right = (y2)*dc->tremor * (0.15 + 0.8*width) * (0.35 + 14*NR::L2(dc->vel));
    }

    if ( width < 0.02 * dc->width ) {
        width = 0.02 * dc->width;
    }

    double dezoomify_factor = 0.05 * 1000;
    if (!dc->abs_width) {
        dezoomify_factor /= SP_EVENT_CONTEXT(dc)->desktop->current_zoom();
    }

    NR::Point del_left = dezoomify_factor * (width + tremble_left) * dc->ang;
    NR::Point del_right = dezoomify_factor * (width + tremble_right) * dc->ang;

    dc->point1[dc->npoints] = brush + del_left;
    dc->point2[dc->npoints] = brush - del_right;

    dc->del = 0.5*(del_left + del_right);

    dc->npoints++;
}

void
sp_erc_update_toolbox (SPDesktop *desktop, const gchar *id, double value)
{
    desktop->setToolboxAdjustmentValue (id, value);
}

static void
eraser_cancel(SPEraserContext *dc)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT(dc)->desktop;
    dc->dragging = FALSE;
    dc->is_drawing = false;
    sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate), 0);
            /* Remove all temporary line segments */
            while (dc->segments) {
                gtk_object_destroy(GTK_OBJECT(dc->segments->data));
                dc->segments = g_slist_remove(dc->segments, dc->segments->data);
            }
            /* reset accumulated curve */
            dc->accumulated->reset();
            clear_current(dc);
            if (dc->repr) {
                dc->repr = NULL;
            }
}


gint
sp_eraser_context_root_handler(SPEventContext *event_context,
                                  GdkEvent *event)
{
    SPEraserContext *dc = SP_ERASER_CONTEXT(event_context);
    SPDesktop *desktop = event_context->desktop;

    gint ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !event_context->space_panning) {

                if (Inkscape::have_viable_layer(desktop, dc->_message_context) == false) {
                    return TRUE;
                }

                NR::Point const button_w(event->button.x,
                                         event->button.y);
                NR::Point const button_dt(desktop->w2d(button_w));
                sp_eraser_reset(dc, button_dt);
                sp_eraser_extinput(dc, event);
                sp_eraser_apply(dc, button_dt);
                dc->accumulated->reset();
                if (dc->repr) {
                    dc->repr = NULL;
                }

                Inkscape::Rubberband::get(desktop)->start(desktop, button_dt);
                Inkscape::Rubberband::get(desktop)->setMode(RUBBERBAND_MODE_TOUCHPATH);

                /* initialize first point */
                dc->npoints = 0;

                sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                                    ( GDK_KEY_PRESS_MASK |
                                      GDK_BUTTON_RELEASE_MASK |
                                      GDK_POINTER_MOTION_MASK |
                                      GDK_BUTTON_PRESS_MASK ),
                                    NULL,
                                    event->button.time);

                ret = TRUE;

                sp_canvas_force_full_redraw_after_interruptions(desktop->canvas, 3);
                dc->is_drawing = true;
            }
            break;
        case GDK_MOTION_NOTIFY:
        {
            NR::Point const motion_w(event->motion.x,
                                     event->motion.y);
            NR::Point motion_dt(desktop->w2d(motion_w));
            sp_eraser_extinput(dc, event);

            dc->_message_context->clear();

            if ( dc->is_drawing && (event->motion.state & GDK_BUTTON1_MASK) && !event_context->space_panning) {
                dc->dragging = TRUE;

                dc->_message_context->set(Inkscape::NORMAL_MESSAGE, _("<b>Drawing</b> an eraser stroke"));

                if (!sp_eraser_apply(dc, motion_dt)) {
                    ret = TRUE;
                    break;
                }

                if ( dc->cur != dc->last ) {
                    sp_eraser_brush(dc);
                    g_assert( dc->npoints > 0 );
                    fit_and_split(dc, FALSE);
                }
                ret = TRUE;
            }
            Inkscape::Rubberband::get(desktop)->move(motion_dt);
        }
        break;


    case GDK_BUTTON_RELEASE:
    {
        NR::Point const motion_w(event->button.x, event->button.y);
        NR::Point const motion_dt(desktop->w2d(motion_w));

        sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate), event->button.time);
        sp_canvas_end_forced_full_redraws(desktop->canvas);
        dc->is_drawing = false;

        if (dc->dragging && event->button.button == 1 && !event_context->space_panning) {
            dc->dragging = FALSE;

            boost::optional<NR::Rect> const b = Inkscape::Rubberband::get(desktop)->getRectangle();

            sp_eraser_apply(dc, motion_dt);

            /* Remove all temporary line segments */
            while (dc->segments) {
                gtk_object_destroy(GTK_OBJECT(dc->segments->data));
                dc->segments = g_slist_remove(dc->segments, dc->segments->data);
            }

            /* Create object */
            fit_and_split(dc, TRUE);
            accumulate_eraser(dc);
            set_to_accumulated(dc); // performs document_done

            /* reset accumulated curve */
            dc->accumulated->reset();

            clear_current(dc);
            if (dc->repr) {
                dc->repr = NULL;
            }

            Inkscape::Rubberband::get(desktop)->stop();
            dc->_message_context->clear();
            ret = TRUE;
        }
        break;
    }

    case GDK_KEY_PRESS:
        switch (get_group0_keyval (&event->key)) {
        case GDK_Up:
        case GDK_KP_Up:
            if (!MOD__CTRL_ONLY) {
                dc->angle += 5.0;
                if (dc->angle > 90.0)
                    dc->angle = 90.0;
                sp_erc_update_toolbox (desktop, "eraser-angle", dc->angle);
                ret = TRUE;
            }
            break;
        case GDK_Down:
        case GDK_KP_Down:
            if (!MOD__CTRL_ONLY) {
                dc->angle -= 5.0;
                if (dc->angle < -90.0)
                    dc->angle = -90.0;
                sp_erc_update_toolbox (desktop, "eraser-angle", dc->angle);
                ret = TRUE;
            }
            break;
        case GDK_Right:
        case GDK_KP_Right:
            if (!MOD__CTRL_ONLY) {
                dc->width += 0.01;
                if (dc->width > 1.0)
                    dc->width = 1.0;
                sp_erc_update_toolbox (desktop, "altx-eraser", dc->width * 100); // the same spinbutton is for alt+x
                ret = TRUE;
            }
            break;
        case GDK_Left:
        case GDK_KP_Left:
            if (!MOD__CTRL_ONLY) {
                dc->width -= 0.01;
                if (dc->width < 0.01)
                    dc->width = 0.01;
                sp_erc_update_toolbox (desktop, "altx-eraser", dc->width * 100);
                ret = TRUE;
            }
            break;
        case GDK_Home:
        case GDK_KP_Home:
            dc->width = 0.01;
            sp_erc_update_toolbox (desktop, "altx-eraser", dc->width * 100);
            ret = TRUE;
            break;
        case GDK_End:
        case GDK_KP_End:
            dc->width = 1.0;
            sp_erc_update_toolbox (desktop, "altx-eraser", dc->width * 100);
            ret = TRUE;
            break;
        case GDK_x:
        case GDK_X:
            if (MOD__ALT_ONLY) {
                desktop->setToolboxFocusTo ("altx-eraser");
                ret = TRUE;
            }
            break;
        case GDK_Escape:
            Inkscape::Rubberband::get(desktop)->stop();
            if (dc->is_drawing) {
                // if drawing, cancel, otherwise pass it up for deselecting
                eraser_cancel (dc);
                ret = TRUE;
            }
            break;
        case GDK_z:
        case GDK_Z:
            if (MOD__CTRL_ONLY && dc->is_drawing) {
                // if drawing, cancel, otherwise pass it up for undo
                eraser_cancel (dc);
                ret = TRUE;
            }
            break;
        default:
            break;
        }
        break;

    case GDK_KEY_RELEASE:
        switch (get_group0_keyval(&event->key)) {
            case GDK_Control_L:
            case GDK_Control_R:
                dc->_message_context->clear();
                break;
            default:
                break;
        }

    default:
        break;
    }

    if (!ret) {
        if (((SPEventContextClass *) eraser_parent_class)->root_handler) {
            ret = ((SPEventContextClass *) eraser_parent_class)->root_handler(event_context, event);
        }
    }

    return ret;
}


static void
clear_current(SPEraserContext *dc)
{
    // reset bpath
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->currentshape), NULL);

    // reset curve
    dc->currentcurve->reset();
    dc->cal1->reset();
    dc->cal2->reset();

    // reset points
    dc->npoints = 0;
}

static void
set_to_accumulated(SPEraserContext *dc)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT(dc)->desktop;
    bool workDone = false;

    if (!dc->accumulated->is_empty()) {
        if (!dc->repr) {
            /* Create object */
            Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());
            Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");

            /* Set style */
            sp_desktop_apply_style_tool (desktop, repr, "tools.eraser", false);

            dc->repr = repr;

            SPItem *item=SP_ITEM(desktop->currentLayer()->appendChildRepr(dc->repr));
            Inkscape::GC::release(dc->repr);
            item->transform = SP_ITEM(desktop->currentRoot())->getRelativeTransform(desktop->currentLayer());
            item->updateRepr();
        }
        Geom::PathVector pathv = dc->accumulated->get_pathvector() * sp_desktop_dt2root_affine(desktop);
        gchar *str = sp_svg_write_path(pathv);
        g_assert( str != NULL );
        dc->repr->setAttribute("d", str);
        g_free(str);

        if ( dc->repr ) {
            bool wasSelection = false;
            Inkscape::Selection *selection = sp_desktop_selection(desktop);
            gint eraserMode = (prefs_get_int_attribute("tools.eraser", "mode", 0) != 0) ? 1 : 0;
            Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());

            SPItem* acid = SP_ITEM(desktop->doc()->getObjectByRepr(dc->repr));
            boost::optional<NR::Rect> eraserBbox = acid->getBounds(NR::identity());
            NR::Rect bounds = (*eraserBbox) * desktop->doc2dt();
            std::vector<SPItem*> remainingItems;
            GSList* toWorkOn = 0;
            if (selection->isEmpty()) {
                if ( eraserMode ) {
                    toWorkOn = sp_document_partial_items_in_box(sp_desktop_document(desktop), desktop->dkey, bounds);
                } else {
                    Inkscape::Rubberband::Rubberband *r = Inkscape::Rubberband::get(desktop);
                    toWorkOn = sp_document_items_at_points(sp_desktop_document(desktop), desktop->dkey, r->getPoints());
                }
                toWorkOn = g_slist_remove( toWorkOn, acid );
            } else {
                toWorkOn = g_slist_copy(const_cast<GSList*>(selection->itemList()));
                wasSelection = true;
            }

            if ( g_slist_length(toWorkOn) > 0 ) {
                if ( eraserMode ) {
                    for (GSList *i = toWorkOn ; i ; i = i->next ) {
                        SPItem *item = SP_ITEM(i->data);
                        if ( eraserMode ) {
                            boost::optional<NR::Rect> bbox = item->getBounds(NR::identity());
                            if (bbox && bbox->intersects(*eraserBbox)) {
                                Inkscape::XML::Node* dup = dc->repr->duplicate(xml_doc);
                                dc->repr->parent()->appendChild(dup);
                                Inkscape::GC::release(dup); // parent takes over

                                selection->set(item);
                                selection->add(dup);
                                sp_selected_path_diff_skip_undo();
                                workDone = true; // TODO set this only if something was cut.
                                if ( !selection->isEmpty() ) {
                                    // If the item was not completely erased, track the new remainder.
                                    GSList *nowSel = g_slist_copy(const_cast<GSList *>(selection->itemList()));
                                    for (GSList const *i2 = nowSel ; i2 ; i2 = i2->next ) {
                                        remainingItems.push_back(SP_ITEM(i2->data));
                                    }
                                    g_slist_free(nowSel);
                                }
                            } else {
                                remainingItems.push_back(item);
                            }
                        }
                    }
                } else {
                    for (GSList *i = toWorkOn ; i ; i = i->next ) {
                        sp_object_ref( SP_ITEM(i->data), 0 );
                    }
                    for (GSList *i = toWorkOn ; i ; i = i->next ) {
                        SPItem *item = SP_ITEM(i->data);
                        item->deleteObject(true);
                        sp_object_unref(item);
                        workDone = true;
                    }
                }

                g_slist_free(toWorkOn);

                if ( !eraserMode ) {
                    //sp_selection_delete(desktop);
                    remainingItems.clear();
                }

                selection->clear();
                if ( wasSelection ) {
                    if ( !remainingItems.empty() ) {
                        selection->add(remainingItems.begin(), remainingItems.end());
                    }
                }
            }

            // Remove the eraser stroke itself:
            sp_repr_unparent( dc->repr );
            dc->repr = 0;
        }
    } else {
        if (dc->repr) {
            sp_repr_unparent(dc->repr);
            dc->repr = 0;
        }
    }


    if ( workDone ) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_ERASER, 
                         _("Draw eraser stroke"));
    } else {
        sp_document_cancel(sp_desktop_document(desktop));
    }
}

static void
add_cap(SPCurve *curve,
        NR::Point const &pre, NR::Point const &from,
        NR::Point const &to, NR::Point const &post,
        double rounding)
{
    NR::Point vel = rounding * NR::rot90( to - from ) / sqrt(2.0);
    double mag = NR::L2(vel);

    NR::Point v_in = from - pre;
    double mag_in = NR::L2(v_in);
    if ( mag_in > ERASER_EPSILON ) {
        v_in = mag * v_in / mag_in;
    } else {
        v_in = NR::Point(0, 0);
    }

    NR::Point v_out = to - post;
    double mag_out = NR::L2(v_out);
    if ( mag_out > ERASER_EPSILON ) {
        v_out = mag * v_out / mag_out;
    } else {
        v_out = NR::Point(0, 0);
    }

    if ( NR::L2(v_in) > ERASER_EPSILON || NR::L2(v_out) > ERASER_EPSILON ) {
        curve->curveto(from + v_in, to + v_out, to);
    }
}

static void
accumulate_eraser(SPEraserContext *dc)
{
    if ( !dc->cal1->is_empty() && !dc->cal2->is_empty() ) {
        dc->accumulated->reset(); /*  Is this required ?? */
        SPCurve *rev_cal2 = dc->cal2->create_reverse();

        g_assert(dc->cal1->get_segment_count() > 0);
        g_assert(rev_cal2->get_segment_count() > 0);
        g_assert( ! dc->cal1->first_path()->closed() );
        g_assert( ! rev_cal2->first_path()->closed() );

        Geom::CubicBezier const * dc_cal1_firstseg  = dynamic_cast<Geom::CubicBezier const *>( dc->cal1->first_segment() );
        Geom::CubicBezier const * rev_cal2_firstseg = dynamic_cast<Geom::CubicBezier const *>( rev_cal2->first_segment() );
        Geom::CubicBezier const * dc_cal1_lastseg   = dynamic_cast<Geom::CubicBezier const *>( dc->cal1->last_segment() );
        Geom::CubicBezier const * rev_cal2_lastseg  = dynamic_cast<Geom::CubicBezier const *>( rev_cal2->last_segment() );
        g_assert( dc_cal1_firstseg );
        g_assert( rev_cal2_firstseg );
        g_assert( dc_cal1_lastseg );
        g_assert( rev_cal2_lastseg );

        dc->accumulated->append(dc->cal1, FALSE);

        add_cap(dc->accumulated, (*dc_cal1_lastseg)[2], (*dc_cal1_lastseg)[3], (*rev_cal2_firstseg)[0], (*rev_cal2_firstseg)[1], dc->cap_rounding);

        dc->accumulated->append(rev_cal2, TRUE);

        add_cap(dc->accumulated, (*rev_cal2_lastseg)[2], (*rev_cal2_lastseg)[3], (*dc_cal1_firstseg)[0], (*dc_cal1_firstseg)[1], dc->cap_rounding);

        dc->accumulated->closepath();

        rev_cal2->unref();

        dc->cal1->reset();
        dc->cal2->reset();
    }
}

static double square(double const x)
{
    return x * x;
}

static void
fit_and_split(SPEraserContext *dc, gboolean release)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT(dc)->desktop;

    double const tolerance_sq = square( NR::expansion(desktop->w2d()) * TOLERANCE_ERASER );

#ifdef ERASER_VERBOSE
    g_print("[F&S:R=%c]", release?'T':'F');
#endif

    if (!( dc->npoints > 0 && dc->npoints < SAMPLING_SIZE ))
        return; // just clicked

    if ( dc->npoints == SAMPLING_SIZE - 1 || release ) {
#define BEZIER_SIZE       4
#define BEZIER_MAX_BEZIERS  8
#define BEZIER_MAX_LENGTH ( BEZIER_SIZE * BEZIER_MAX_BEZIERS )

#ifdef ERASER_VERBOSE
        g_print("[F&S:#] dc->npoints:%d, release:%s\n",
                dc->npoints, release ? "TRUE" : "FALSE");
#endif

        /* Current eraser */
        if ( dc->cal1->is_empty() || dc->cal2->is_empty() ) {
            /* dc->npoints > 0 */
            /* g_print("erasers(1|2) reset\n"); */
            dc->cal1->reset();
            dc->cal2->reset();

            dc->cal1->moveto(dc->point1[0]);
            dc->cal2->moveto(dc->point2[0]);
        }

        NR::Point b1[BEZIER_MAX_LENGTH];
        gint const nb1 = sp_bezier_fit_cubic_r(b1, dc->point1, dc->npoints,
                                               tolerance_sq, BEZIER_MAX_BEZIERS);
        g_assert( nb1 * BEZIER_SIZE <= gint(G_N_ELEMENTS(b1)) );

        NR::Point b2[BEZIER_MAX_LENGTH];
        gint const nb2 = sp_bezier_fit_cubic_r(b2, dc->point2, dc->npoints,
                                               tolerance_sq, BEZIER_MAX_BEZIERS);
        g_assert( nb2 * BEZIER_SIZE <= gint(G_N_ELEMENTS(b2)) );

        if ( nb1 != -1 && nb2 != -1 ) {
            /* Fit and draw and reset state */
#ifdef ERASER_VERBOSE
            g_print("nb1:%d nb2:%d\n", nb1, nb2);
#endif
            /* CanvasShape */
            if (! release) {
                dc->currentcurve->reset();
                dc->currentcurve->moveto(b1[0]);
                for (NR::Point *bp1 = b1; bp1 < b1 + BEZIER_SIZE * nb1; bp1 += BEZIER_SIZE) {
                    dc->currentcurve->curveto(bp1[1],
                                     bp1[2], bp1[3]);
                }
                dc->currentcurve->lineto(b2[BEZIER_SIZE*(nb2-1) + 3]);
                for (NR::Point *bp2 = b2 + BEZIER_SIZE * ( nb2 - 1 ); bp2 >= b2; bp2 -= BEZIER_SIZE) {
                    dc->currentcurve->curveto(bp2[2], bp2[1], bp2[0]);
                }
                // FIXME: dc->segments is always NULL at this point??
                if (!dc->segments) { // first segment
                    add_cap(dc->currentcurve, b2[1], b2[0], b1[0], b1[1], dc->cap_rounding);
                }
                dc->currentcurve->closepath();
                sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->currentshape), dc->currentcurve);
            }

            /* Current eraser */
            for (NR::Point *bp1 = b1; bp1 < b1 + BEZIER_SIZE * nb1; bp1 += BEZIER_SIZE) {
                dc->cal1->curveto(bp1[1], bp1[2], bp1[3]);
            }
            for (NR::Point *bp2 = b2; bp2 < b2 + BEZIER_SIZE * nb2; bp2 += BEZIER_SIZE) {
                dc->cal2->curveto(bp2[1], bp2[2], bp2[3]);
            }
        } else {
            /* fixme: ??? */
#ifdef ERASER_VERBOSE
            g_print("[fit_and_split] failed to fit-cubic.\n");
#endif
            draw_temporary_box(dc);

            for (gint i = 1; i < dc->npoints; i++) {
                dc->cal1->lineto(dc->point1[i]);
            }
            for (gint i = 1; i < dc->npoints; i++) {
                dc->cal2->lineto(dc->point2[i]);
            }
        }

        /* Fit and draw and copy last point */
#ifdef ERASER_VERBOSE
        g_print("[%d]Yup\n", dc->npoints);
#endif
        if (!release) {
            gint eraserMode = (prefs_get_int_attribute("tools.eraser", "mode", 0) != 0) ? 1 : 0;
            g_assert(!dc->currentcurve->is_empty());

            SPCanvasItem *cbp = sp_canvas_item_new(sp_desktop_sketch(desktop),
                                                   SP_TYPE_CANVAS_BPATH,
                                                   NULL);
            SPCurve *curve = dc->currentcurve->copy();
            sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH (cbp), curve);
            curve->unref();

            guint32 fillColor = sp_desktop_get_color_tool (desktop, "tools.eraser", true);
            //guint32 strokeColor = sp_desktop_get_color_tool (desktop, "tools.eraser", false);
            double opacity = sp_desktop_get_master_opacity_tool (desktop, "tools.eraser");
            double fillOpacity = sp_desktop_get_opacity_tool (desktop, "tools.eraser", true);
            //double strokeOpacity = sp_desktop_get_opacity_tool (desktop, "tools.eraser", false);
            sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(cbp), ((fillColor & 0xffffff00) | SP_COLOR_F_TO_U(opacity*fillOpacity)), SP_WIND_RULE_EVENODD);
            //on second thougtht don't do stroke yet because we don't have stoke-width yet and because stoke appears between segments while drawing
            //sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(cbp), ((strokeColor & 0xffffff00) | SP_COLOR_F_TO_U(opacity*strokeOpacity)), 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
            sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(cbp), 0x00000000, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
            /* fixme: Cannot we cascade it to root more clearly? */
            g_signal_connect(G_OBJECT(cbp), "event", G_CALLBACK(sp_desktop_root_handler), desktop);

            dc->segments = g_slist_prepend(dc->segments, cbp);

            if ( !eraserMode ) {
                sp_canvas_item_hide(cbp);
                sp_canvas_item_hide(dc->currentshape);
            }
        }

        dc->point1[0] = dc->point1[dc->npoints - 1];
        dc->point2[0] = dc->point2[dc->npoints - 1];
        dc->npoints = 1;
    } else {
        draw_temporary_box(dc);
    }
}

static void
draw_temporary_box(SPEraserContext *dc)
{
    dc->currentcurve->reset();

    dc->currentcurve->moveto(dc->point1[dc->npoints-1]);
    for (gint i = dc->npoints-2; i >= 0; i--) {
        dc->currentcurve->lineto(dc->point1[i]);
    }
    for (gint i = 0; i < dc->npoints; i++) {
        dc->currentcurve->lineto(dc->point2[i]);
    }
    if (dc->npoints >= 2) {
        add_cap(dc->currentcurve, dc->point2[dc->npoints-2], dc->point2[dc->npoints-1], dc->point1[dc->npoints-1], dc->point1[dc->npoints-2], dc->cap_rounding);
    }

    dc->currentcurve->closepath();
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->currentshape), dc->currentcurve);
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

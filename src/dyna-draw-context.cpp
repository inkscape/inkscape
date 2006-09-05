#define __SP_DYNA_DRAW_CONTEXT_C__

/*
 * Handwriting-like drawing mode
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   MenTaLguY <mental@rydia.net>
 *
 * The original dynadraw code:
 *   Paul Haeberli <paul@sgi.com>
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 2005-2006 bulia byak
 * Copyright (C) 2006 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noDYNA_DRAW_VERBOSE

#include "config.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

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
#include "pixmaps/cursor-calligraphy.xpm"
#include "pixmaps/cursor-calligraphy.pixbuf"
#include "dyna-draw-context.h"
#include "libnr/n-art-bpath.h"
#include "libnr/nr-path.h"
#include "xml/repr.h"
#include "context-fns.h"
#include "sp-item.h"
#include "inkscape.h"
#include "color.h"

#define DDC_RED_RGBA 0xff0000ff

#define SAMPLE_TIMEOUT 10
#define TOLERANCE_LINE 1.0
#define TOLERANCE_CALLIGRAPHIC 3.0
#define DYNA_EPSILON 0.5e-2

#define DYNA_MIN_WIDTH 1.0e-6

#define DRAG_MIN 0.0
#define DRAG_DEFAULT 1.0
#define DRAG_MAX 1.0

static void sp_dyna_draw_context_class_init(SPDynaDrawContextClass *klass);
static void sp_dyna_draw_context_init(SPDynaDrawContext *ddc);
static void sp_dyna_draw_context_dispose(GObject *object);

static void sp_dyna_draw_context_setup(SPEventContext *ec);
static void sp_dyna_draw_context_set(SPEventContext *ec, gchar const *key, gchar const *val);
static gint sp_dyna_draw_context_root_handler(SPEventContext *ec, GdkEvent *event);

static void clear_current(SPDynaDrawContext *dc);
static void set_to_accumulated(SPDynaDrawContext *dc);
static void add_cap(SPCurve *curve, NR::Point const &pre, NR::Point const &from, NR::Point const &to, NR::Point const &post, double rounding);
static void accumulate_calligraphic(SPDynaDrawContext *dc);

static void fit_and_split(SPDynaDrawContext *ddc, gboolean release);

static void sp_dyna_draw_reset(SPDynaDrawContext *ddc, NR::Point p);
static NR::Point sp_dyna_draw_get_npoint(SPDynaDrawContext const *ddc, NR::Point v);
static NR::Point sp_dyna_draw_get_vpoint(SPDynaDrawContext const *ddc, NR::Point n);
static void draw_temporary_box(SPDynaDrawContext *dc);


static SPEventContextClass *parent_class;

GtkType
sp_dyna_draw_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPDynaDrawContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_dyna_draw_context_class_init,
            NULL, NULL,
            sizeof(SPDynaDrawContext),
            4,
            (GInstanceInitFunc) sp_dyna_draw_context_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPDynaDrawContext", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_dyna_draw_context_class_init(SPDynaDrawContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_dyna_draw_context_dispose;

    event_context_class->setup = sp_dyna_draw_context_setup;
    event_context_class->set = sp_dyna_draw_context_set;
    event_context_class->root_handler = sp_dyna_draw_context_root_handler;
}

static void
sp_dyna_draw_context_init(SPDynaDrawContext *ddc)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(ddc);

    event_context->cursor_shape = cursor_calligraphy_xpm;
    event_context->cursor_pixbuf = gdk_pixbuf_new_from_inline(
            -1,
            cursor_calligraphy_pixbuf,
            FALSE,
            NULL);  
    event_context->hot_x = 4;
    event_context->hot_y = 4;

    ddc->accumulated = NULL;
    ddc->segments = NULL;
    ddc->currentcurve = NULL;
    ddc->currentshape = NULL;
    ddc->npoints = 0;
    ddc->cal1 = NULL;
    ddc->cal2 = NULL;
    ddc->repr = NULL;

    /* DynaDraw values */
    ddc->cur = NR::Point(0,0);
    ddc->last = NR::Point(0,0);
    ddc->vel = NR::Point(0,0);
    ddc->acc = NR::Point(0,0);
    ddc->ang = NR::Point(0,0);
    ddc->del = NR::Point(0,0);

    /* attributes */
    ddc->dragging = FALSE;

    ddc->mass = 0.3;
    ddc->drag = DRAG_DEFAULT;
    ddc->angle = 30.0;
    ddc->width = 0.2;

    ddc->vel_thin = 0.1;
    ddc->flatness = 0.9;
    ddc->cap_rounding = 0.0;

    ddc->abs_width = false;
    ddc->keep_selected = true;
}

static void
sp_dyna_draw_context_dispose(GObject *object)
{
    SPDynaDrawContext *ddc = SP_DYNA_DRAW_CONTEXT(object);

    if (ddc->accumulated) {
        ddc->accumulated = sp_curve_unref(ddc->accumulated);
    }

    while (ddc->segments) {
        gtk_object_destroy(GTK_OBJECT(ddc->segments->data));
        ddc->segments = g_slist_remove(ddc->segments, ddc->segments->data);
    }

    if (ddc->currentcurve) ddc->currentcurve = sp_curve_unref(ddc->currentcurve);
    if (ddc->cal1) ddc->cal1 = sp_curve_unref(ddc->cal1);
    if (ddc->cal2) ddc->cal2 = sp_curve_unref(ddc->cal2);

    if (ddc->currentshape) {
        gtk_object_destroy(GTK_OBJECT(ddc->currentshape));
        ddc->currentshape = NULL;
    }

    if (ddc->_message_context) {
        delete ddc->_message_context;
    }

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

static void
sp_dyna_draw_context_setup(SPEventContext *ec)
{
    SPDynaDrawContext *ddc = SP_DYNA_DRAW_CONTEXT(ec);

    if (((SPEventContextClass *) parent_class)->setup)
        ((SPEventContextClass *) parent_class)->setup(ec);

    ddc->accumulated = sp_curve_new_sized(32);
    ddc->currentcurve = sp_curve_new_sized(4);

    ddc->cal1 = sp_curve_new_sized(32);
    ddc->cal2 = sp_curve_new_sized(32);

    ddc->currentshape = sp_canvas_item_new(sp_desktop_sketch(ec->desktop), SP_TYPE_CANVAS_BPATH, NULL);
    sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(ddc->currentshape), DDC_RED_RGBA, SP_WIND_RULE_EVENODD);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(ddc->currentshape), 0x00000000, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
    /* fixme: Cannot we cascade it to root more clearly? */
    g_signal_connect(G_OBJECT(ddc->currentshape), "event", G_CALLBACK(sp_desktop_root_handler), ec->desktop);

    sp_event_context_read(ec, "mass");
    sp_event_context_read(ec, "drag");
    sp_event_context_read(ec, "angle");
    sp_event_context_read(ec, "width");
    sp_event_context_read(ec, "thinning");
    sp_event_context_read(ec, "tremor");
    sp_event_context_read(ec, "flatness");
    sp_event_context_read(ec, "usepressure");
    sp_event_context_read(ec, "usetilt");
    sp_event_context_read(ec, "abs_width");
    sp_event_context_read(ec, "keep_selected");
    sp_event_context_read(ec, "cap_rounding");

    ddc->is_drawing = false;

    ddc->_message_context = new Inkscape::MessageContext((ec->desktop)->messageStack());
}

static void
sp_dyna_draw_context_set(SPEventContext *ec, gchar const *key, gchar const *val)
{
    SPDynaDrawContext *ddc = SP_DYNA_DRAW_CONTEXT(ec);

    if (!strcmp(key, "mass")) {
        double const dval = ( val ? g_ascii_strtod (val, NULL) : 0.2 );
        ddc->mass = CLAMP(dval, -1000.0, 1000.0);
    } else if (!strcmp(key, "drag")) {
        double const dval = ( val ? g_ascii_strtod (val, NULL) : DRAG_DEFAULT );
        ddc->drag = CLAMP(dval, DRAG_MIN, DRAG_MAX);
    } else if (!strcmp(key, "angle")) {
        double const dval = ( val ? g_ascii_strtod (val, NULL) : 0.0);
        ddc->angle = CLAMP (dval, -90, 90);
    } else if (!strcmp(key, "width")) {
        double const dval = ( val ? g_ascii_strtod (val, NULL) : 0.1 );
        ddc->width = CLAMP(dval, -1000.0, 1000.0);
    } else if (!strcmp(key, "thinning")) {
        double const dval = ( val ? g_ascii_strtod (val, NULL) : 0.1 );
        ddc->vel_thin = CLAMP(dval, -1.0, 1.0);
    } else if (!strcmp(key, "tremor")) {
        double const dval = ( val ? g_ascii_strtod (val, NULL) : 0.0 );
        ddc->tremor = CLAMP(dval, 0.0, 1.0);
    } else if (!strcmp(key, "flatness")) {
        double const dval = ( val ? g_ascii_strtod (val, NULL) : 1.0 );
        ddc->flatness = CLAMP(dval, 0, 1.0);
    } else if (!strcmp(key, "usepressure")) {
        ddc->usepressure = (val && strcmp(val, "0"));
    } else if (!strcmp(key, "usetilt")) {
        ddc->usetilt = (val && strcmp(val, "0"));
    } else if (!strcmp(key, "abs_width")) {
        ddc->abs_width = (val && strcmp(val, "0"));
    } else if (!strcmp(key, "keep_selected")) {
        ddc->keep_selected = (val && strcmp(val, "0"));
    } else if (!strcmp(key, "cap_rounding")) {
        ddc->cap_rounding = ( val ? g_ascii_strtod (val, NULL) : 0.0 );
    }

    //g_print("DDC: %g %g %g %g\n", ddc->mass, ddc->drag, ddc->angle, ddc->width);
}

static double
flerp(double f0, double f1, double p)
{
    return f0 + ( f1 - f0 ) * p;
}

/* Get normalized point */
static NR::Point
sp_dyna_draw_get_npoint(SPDynaDrawContext const *dc, NR::Point v)
{
    NR::Rect drect = SP_EVENT_CONTEXT(dc)->desktop->get_display_area();
    double const max = MAX ( drect.dimensions()[NR::X], drect.dimensions()[NR::Y] );
    return NR::Point(( v[NR::X] - drect.min()[NR::X] ) / max,  ( v[NR::Y] - drect.min()[NR::Y] ) / max);
}

/* Get view point */
static NR::Point
sp_dyna_draw_get_vpoint(SPDynaDrawContext const *dc, NR::Point n)
{
    NR::Rect drect = SP_EVENT_CONTEXT(dc)->desktop->get_display_area();
    double const max = MAX ( drect.dimensions()[NR::X], drect.dimensions()[NR::Y] );
    return NR::Point(n[NR::X] * max + drect.min()[NR::X], n[NR::Y] * max + drect.min()[NR::Y]);
}

static void
sp_dyna_draw_reset(SPDynaDrawContext *dc, NR::Point p)
{
    dc->last = dc->cur = sp_dyna_draw_get_npoint(dc, p);
    dc->vel = NR::Point(0,0);
    dc->acc = NR::Point(0,0);
    dc->ang = NR::Point(0,0);
    dc->del = NR::Point(0,0);
}

static void
sp_dyna_draw_extinput(SPDynaDrawContext *dc, GdkEvent *event)
{
    if (gdk_event_get_axis (event, GDK_AXIS_PRESSURE, &dc->pressure))
        dc->pressure = CLAMP (dc->pressure, DDC_MIN_PRESSURE, DDC_MAX_PRESSURE);
    else
        dc->pressure = DDC_DEFAULT_PRESSURE;

    if (gdk_event_get_axis (event, GDK_AXIS_XTILT, &dc->xtilt))
        dc->xtilt = CLAMP (dc->xtilt, DDC_MIN_TILT, DDC_MAX_TILT);
    else
        dc->xtilt = DDC_DEFAULT_TILT;

    if (gdk_event_get_axis (event, GDK_AXIS_YTILT, &dc->ytilt))
        dc->ytilt = CLAMP (dc->ytilt, DDC_MIN_TILT, DDC_MAX_TILT);
    else
        dc->ytilt = DDC_DEFAULT_TILT;
}


static gboolean
sp_dyna_draw_apply(SPDynaDrawContext *dc, NR::Point p)
{
    NR::Point n = sp_dyna_draw_get_npoint(dc, p);

    /* Calculate mass and drag */
    double const mass = flerp(1.0, 160.0, dc->mass);
    double const drag = flerp(0.0, 0.5, dc->drag * dc->drag);

    /* Calculate force and acceleration */
    NR::Point force = n - dc->cur;
    if ( NR::L2(force) < DYNA_EPSILON ) {
        return FALSE;
    }

    dc->acc = force / mass;

    /* Calculate new velocity */
    dc->vel += dc->acc;

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
    if ( mag_vel < DYNA_EPSILON ) {
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
    // convert to point
    dc->ang = NR::Point (cos (new_ang), sin (new_ang));

    /* Apply drag */
    dc->vel *= 1.0 - drag;

    /* Update position */
    dc->last = dc->cur;
    dc->cur += dc->vel;

    return TRUE;
}

static void
sp_dyna_draw_brush(SPDynaDrawContext *dc)
{
    g_assert( dc->npoints >= 0 && dc->npoints < SAMPLING_SIZE );

    // How much velocity thins strokestyle
    double vel_thin = flerp (0, 160, dc->vel_thin);

    // Influence of pressure on thickness
    double pressure_thick = (dc->usepressure ? dc->pressure : 1.0);

    double width = ( pressure_thick - vel_thin * NR::L2(dc->vel) ) * dc->width;

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

    NR::Point abs_middle = sp_dyna_draw_get_vpoint(dc, dc->cur);

    dc->point1[dc->npoints] = abs_middle + del_left;
    dc->point2[dc->npoints] = abs_middle - del_right;

    dc->del = 0.5*(del_left + del_right);

    dc->npoints++;
}

void
sp_ddc_update_toolbox (SPDesktop *desktop, const gchar *id, double value)
{
    desktop->setToolboxAdjustmentValue (id, value);
}

gint
sp_dyna_draw_context_root_handler(SPEventContext *event_context,
                                  GdkEvent *event)
{
    SPDynaDrawContext *dc = SP_DYNA_DRAW_CONTEXT(event_context);
    SPDesktop *desktop = event_context->desktop;

    gint ret = FALSE;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if ( event->button.button == 1 ) {

            SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(dc);

            if (Inkscape::have_viable_layer(desktop, dc->_message_context) == false) {
                return TRUE;
            }

            NR::Point const button_w(event->button.x,
                                     event->button.y);
            NR::Point const button_dt(desktop->w2d(button_w));
            sp_dyna_draw_reset(dc, button_dt);
            sp_dyna_draw_extinput(dc, event);
            sp_dyna_draw_apply(dc, button_dt);
            sp_curve_reset(dc->accumulated);
            if (dc->repr) {
                dc->repr = NULL;
            }

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

            dc->is_drawing = true;
        }
        break;
    case GDK_MOTION_NOTIFY:
        if ( dc->is_drawing && ( event->motion.state & GDK_BUTTON1_MASK ) ) {
            dc->dragging = TRUE;

            NR::Point const motion_w(event->motion.x,
                                     event->motion.y);
            NR::Point const motion_dt(desktop->w2d(motion_w));

            sp_dyna_draw_extinput(dc, event);
            if (!sp_dyna_draw_apply(dc, motion_dt)) {
                ret = TRUE;
                break;
            }

            if ( dc->cur != dc->last ) {
                sp_dyna_draw_brush(dc);
                g_assert( dc->npoints > 0 );
                fit_and_split(dc, FALSE);
            }
            ret = TRUE;
        }
        break;

    case GDK_BUTTON_RELEASE:
        sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate), event->button.time);
        dc->is_drawing = false;

        if ( dc->dragging && event->button.button == 1 ) {
            dc->dragging = FALSE;

            NR::Point const motion_w(event->button.x, event->button.y);
            NR::Point const motion_dt(desktop->w2d(motion_w));
            sp_dyna_draw_apply(dc, motion_dt);

            /* Remove all temporary line segments */
            while (dc->segments) {
                gtk_object_destroy(GTK_OBJECT(dc->segments->data));
                dc->segments = g_slist_remove(dc->segments, dc->segments->data);
            }
            /* Create object */
            fit_and_split(dc, TRUE);
            accumulate_calligraphic(dc);
            set_to_accumulated(dc); /* temporal implementation */
            /* reset accumulated curve */
            sp_curve_reset(dc->accumulated);
            clear_current(dc);
            if (dc->repr) {
                dc->repr = NULL;
            }
            ret = TRUE;
        }
        break;
    case GDK_KEY_PRESS:
        switch (get_group0_keyval (&event->key)) {
        case GDK_Up:
        case GDK_KP_Up:
            if (!MOD__CTRL_ONLY) {
                dc->angle += 5.0;
                if (dc->angle > 90.0)
                    dc->angle = 90.0;
                sp_ddc_update_toolbox (desktop, "calligraphy-angle", dc->angle);
                ret = TRUE;
            }
            break;
        case GDK_Down:
        case GDK_KP_Down:
            if (!MOD__CTRL_ONLY) {
                dc->angle -= 5.0;
                if (dc->angle < -90.0)
                    dc->angle = -90.0;
                sp_ddc_update_toolbox (desktop, "calligraphy-angle", dc->angle);
                ret = TRUE;
            }
            break;
        case GDK_Right:
        case GDK_KP_Right:
            if (!MOD__CTRL_ONLY) {
                dc->width += 0.01;
                if (dc->width > 1.0)
                    dc->width = 1.0;
                sp_ddc_update_toolbox (desktop, "altx-calligraphy", dc->width * 100); // the same spinbutton is for alt+x
                ret = TRUE;
            }
            break;
        case GDK_Left:
        case GDK_KP_Left:
            if (!MOD__CTRL_ONLY) {
                dc->width -= 0.01;
                if (dc->width < 0.01)
                    dc->width = 0.01;
                sp_ddc_update_toolbox (desktop, "altx-calligraphy", dc->width * 100);
                ret = TRUE;
            }
            break;
        case GDK_x:
        case GDK_X:
            if (MOD__ALT_ONLY) {
                desktop->setToolboxFocusTo ("altx-calligraphy");
                ret = TRUE;
            }
            break;
        case GDK_Escape:
            sp_desktop_selection(desktop)->clear();
            break;

        default:
            break;
        }
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


static void
clear_current(SPDynaDrawContext *dc)
{
    /* reset bpath */
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->currentshape), NULL);
    /* reset curve */
    sp_curve_reset(dc->currentcurve);
    sp_curve_reset(dc->cal1);
    sp_curve_reset(dc->cal2);
    /* reset points */
    dc->npoints = 0;
}

static void
set_to_accumulated(SPDynaDrawContext *dc)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT(dc)->desktop;

    if (!sp_curve_empty(dc->accumulated)) {
        NArtBpath *abp;
        gchar *str;

        if (!dc->repr) {
            /* Create object */
            Inkscape::XML::Node *repr = sp_repr_new("svg:path");

            /* Set style */
            sp_desktop_apply_style_tool (desktop, repr, "tools.calligraphic", false);

            dc->repr = repr;

            SPItem *item=SP_ITEM(desktop->currentLayer()->appendChildRepr(dc->repr));
            Inkscape::GC::release(dc->repr);
            item->transform = SP_ITEM(desktop->currentRoot())->getRelativeTransform(desktop->currentLayer());
            item->updateRepr();
            if (dc->keep_selected) {
                sp_desktop_selection(desktop)->set(dc->repr);
            } else {
                sp_desktop_selection(desktop)->clear();
            }
        }
        abp = nr_artpath_affine(sp_curve_first_bpath(dc->accumulated), sp_desktop_dt2root_affine(desktop));
        str = sp_svg_write_path(abp);
        g_assert( str != NULL );
        g_free(abp);
        dc->repr->setAttribute("d", str);
        g_free(str);
    } else {
        if (dc->repr) {
            sp_repr_unparent(dc->repr);
        }
        dc->repr = NULL;
    }

    sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_CALLIGRAPHIC, 
                     /* TODO: annotate */ "dyna-draw-context.cpp:689");
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
    if ( mag_in > DYNA_EPSILON ) {
        v_in = mag * v_in / mag_in;
    } else {
        v_in = NR::Point(0, 0);
    }

    NR::Point v_out = to - post;
    double mag_out = NR::L2(v_out);
    if ( mag_out > DYNA_EPSILON ) {
        v_out = mag * v_out / mag_out;
    } else {
        v_out = NR::Point(0, 0);
    }

    if ( NR::L2(v_in) > DYNA_EPSILON || NR::L2(v_out) > DYNA_EPSILON ) {
        sp_curve_curveto(curve, from + v_in, to + v_out, to);
    }
}

static void
accumulate_calligraphic(SPDynaDrawContext *dc)
{
    if ( !sp_curve_empty(dc->cal1) && !sp_curve_empty(dc->cal2) ) {
        sp_curve_reset(dc->accumulated); /*  Is this required ?? */
        SPCurve *rev_cal2 = sp_curve_reverse(dc->cal2);

        g_assert(dc->cal1->end > 1);
        g_assert(rev_cal2->end > 1);
        g_assert(SP_CURVE_SEGMENT(dc->cal1, 0)->code == NR_MOVETO_OPEN);
        g_assert(SP_CURVE_SEGMENT(rev_cal2, 0)->code == NR_MOVETO_OPEN);
        g_assert(SP_CURVE_SEGMENT(dc->cal1, 1)->code == NR_CURVETO);
        g_assert(SP_CURVE_SEGMENT(rev_cal2, 1)->code == NR_CURVETO);
        g_assert(SP_CURVE_SEGMENT(dc->cal1, dc->cal1->end-1)->code == NR_CURVETO);
        g_assert(SP_CURVE_SEGMENT(rev_cal2, rev_cal2->end-1)->code == NR_CURVETO);

        sp_curve_append(dc->accumulated, dc->cal1, FALSE);

        add_cap(dc->accumulated, SP_CURVE_SEGMENT(dc->cal1, dc->cal1->end-1)->c(2), SP_CURVE_SEGMENT(dc->cal1, dc->cal1->end-1)->c(3), SP_CURVE_SEGMENT(rev_cal2, 0)->c(3), SP_CURVE_SEGMENT(rev_cal2, 1)->c(1), dc->cap_rounding);

        sp_curve_append(dc->accumulated, rev_cal2, TRUE);

        add_cap(dc->accumulated, SP_CURVE_SEGMENT(rev_cal2, rev_cal2->end-1)->c(2), SP_CURVE_SEGMENT(rev_cal2, rev_cal2->end-1)->c(3), SP_CURVE_SEGMENT(dc->cal1, 0)->c(3), SP_CURVE_SEGMENT(dc->cal1, 1)->c(1), dc->cap_rounding);

        sp_curve_closepath(dc->accumulated);

        sp_curve_unref(rev_cal2);

        sp_curve_reset(dc->cal1);
        sp_curve_reset(dc->cal2);
    }
}

static double square(double const x)
{
    return x * x;
}

static void
fit_and_split(SPDynaDrawContext *dc, gboolean release)
{
    double const tolerance_sq = square( NR::expansion(SP_EVENT_CONTEXT(dc)->desktop->w2d()) * TOLERANCE_CALLIGRAPHIC );

#ifdef DYNA_DRAW_VERBOSE
    g_print("[F&S:R=%c]", release?'T':'F');
#endif

    if (!( dc->npoints > 0 && dc->npoints < SAMPLING_SIZE ))
        return; // just clicked

    if ( dc->npoints == SAMPLING_SIZE - 1 || release ) {
#define BEZIER_SIZE       4
#define BEZIER_MAX_BEZIERS  8
#define BEZIER_MAX_LENGTH ( BEZIER_SIZE * BEZIER_MAX_BEZIERS )

#ifdef DYNA_DRAW_VERBOSE
        g_print("[F&S:#] dc->npoints:%d, release:%s\n",
                dc->npoints, release ? "TRUE" : "FALSE");
#endif

        /* Current calligraphic */
        if ( dc->cal1->end == 0 || dc->cal2->end == 0 ) {
            /* dc->npoints > 0 */
            /* g_print("calligraphics(1|2) reset\n"); */
            sp_curve_reset(dc->cal1);
            sp_curve_reset(dc->cal2);

            sp_curve_moveto(dc->cal1, dc->point1[0]);
            sp_curve_moveto(dc->cal2, dc->point2[0]);
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
#ifdef DYNA_DRAW_VERBOSE
            g_print("nb1:%d nb2:%d\n", nb1, nb2);
#endif
            /* CanvasShape */
            if (! release) {
                sp_curve_reset(dc->currentcurve);
                sp_curve_moveto(dc->currentcurve, b1[0]);
                for (NR::Point *bp1 = b1; bp1 < b1 + BEZIER_SIZE * nb1; bp1 += BEZIER_SIZE) {
                    sp_curve_curveto(dc->currentcurve, bp1[1],
                                     bp1[2], bp1[3]);
                }
                sp_curve_lineto(dc->currentcurve,
                                b2[BEZIER_SIZE*(nb2-1) + 3]);
                for (NR::Point *bp2 = b2 + BEZIER_SIZE * ( nb2 - 1 ); bp2 >= b2; bp2 -= BEZIER_SIZE) {
                    sp_curve_curveto(dc->currentcurve, bp2[2], bp2[1], bp2[0]);
                }
                // FIXME: dc->segments is always NULL at this point??
                if (!dc->segments) { // first segment
                    add_cap(dc->currentcurve, b2[1], b2[0], b1[0], b1[1], dc->cap_rounding);
                }
                sp_curve_closepath(dc->currentcurve);
                sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->currentshape), dc->currentcurve);
            }

            /* Current calligraphic */
            for (NR::Point *bp1 = b1; bp1 < b1 + BEZIER_SIZE * nb1; bp1 += BEZIER_SIZE) {
                sp_curve_curveto(dc->cal1, bp1[1], bp1[2], bp1[3]);
            }
            for (NR::Point *bp2 = b2; bp2 < b2 + BEZIER_SIZE * nb2; bp2 += BEZIER_SIZE) {
                sp_curve_curveto(dc->cal2, bp2[1], bp2[2], bp2[3]);
            }
        } else {
            /* fixme: ??? */
#ifdef DYNA_DRAW_VERBOSE
            g_print("[fit_and_split] failed to fit-cubic.\n");
#endif
            draw_temporary_box(dc);

            for (gint i = 1; i < dc->npoints; i++) {
                sp_curve_lineto(dc->cal1, dc->point1[i]);
            }
            for (gint i = 1; i < dc->npoints; i++) {
                sp_curve_lineto(dc->cal2, dc->point2[i]);
            }
        }

        /* Fit and draw and copy last point */
#ifdef DYNA_DRAW_VERBOSE
        g_print("[%d]Yup\n", dc->npoints);
#endif
        if (!release) {
            g_assert(!sp_curve_empty(dc->currentcurve));

            SPCanvasItem *cbp = sp_canvas_item_new(sp_desktop_sketch(SP_EVENT_CONTEXT(dc)->desktop),
                                                   SP_TYPE_CANVAS_BPATH,
                                                   NULL);
            SPCurve *curve = sp_curve_copy(dc->currentcurve);
            sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH (cbp), curve);
            sp_curve_unref(curve);

            guint32 fillColor = sp_desktop_get_color_tool (SP_ACTIVE_DESKTOP, "tools.calligraphic", true);
            //guint32 strokeColor = sp_desktop_get_color_tool (SP_ACTIVE_DESKTOP, "tools.calligraphic", false);
            double opacity = sp_desktop_get_master_opacity_tool (SP_ACTIVE_DESKTOP, "tools.calligraphic");
            double fillOpacity = sp_desktop_get_opacity_tool (SP_ACTIVE_DESKTOP, "tools.calligraphic", true);
            //double strokeOpacity = sp_desktop_get_opacity_tool (SP_ACTIVE_DESKTOP, "tools.calligraphic", false);
            sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(cbp), ((fillColor & 0xffffff00) | SP_COLOR_F_TO_U(opacity*fillOpacity)), SP_WIND_RULE_EVENODD);
            //on second thougtht don't do stroke yet because we don't have stoke-width yet and because stoke appears between segments while drawing
            //sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(cbp), ((strokeColor & 0xffffff00) | SP_COLOR_F_TO_U(opacity*strokeOpacity)), 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
            sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(cbp), 0x00000000, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
            /* fixme: Cannot we cascade it to root more clearly? */
            g_signal_connect(G_OBJECT(cbp), "event", G_CALLBACK(sp_desktop_root_handler), SP_EVENT_CONTEXT(dc)->desktop);

            dc->segments = g_slist_prepend(dc->segments, cbp);
        }

        dc->point1[0] = dc->point1[dc->npoints - 1];
        dc->point2[0] = dc->point2[dc->npoints - 1];
        dc->npoints = 1;
    } else {
        draw_temporary_box(dc);
    }
}

static void
draw_temporary_box(SPDynaDrawContext *dc)
{
    sp_curve_reset(dc->currentcurve);
    sp_curve_moveto(dc->currentcurve, dc->point1[0]);
    for (gint i = 1; i < dc->npoints; i++) {
        sp_curve_lineto(dc->currentcurve, dc->point1[i]);
    }
    for (gint i = dc->npoints-1; i >= 0; i--) {
        sp_curve_lineto(dc->currentcurve, dc->point2[i]);
    }
    add_cap(dc->currentcurve, dc->point2[1], dc->point2[0], dc->point1[0], dc->point1[1], dc->cap_rounding);
    sp_curve_closepath(dc->currentcurve);
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

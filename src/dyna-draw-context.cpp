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
 * Copyright (C) 2005-2007 bulia byak
 * Copyright (C) 2006 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noDYNA_DRAW_VERBOSE

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
#include "display/curve.h"
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
#include "preferences.h"
#include "pixmaps/cursor-calligraphy.xpm"
#include "libnr/nr-matrix-ops.h"
#include "libnr/nr-scale-translate-ops.h"
#include "libnr/nr-convert2geom.h"
#include "xml/repr.h"
#include "context-fns.h"
#include "sp-item.h"
#include "inkscape.h"
#include "color.h"
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

#include "dyna-draw-context.h"

#define DDC_RED_RGBA 0xff0000ff

#define TOLERANCE_CALLIGRAPHIC 0.1

#define DYNA_EPSILON 0.5e-6
#define DYNA_EPSILON_START 0.5e-2
#define DYNA_VEL_START 1e-5

#define DYNA_MIN_WIDTH 1.0e-6

static void sp_dyna_draw_context_class_init(SPDynaDrawContextClass *klass);
static void sp_dyna_draw_context_init(SPDynaDrawContext *ddc);
static void sp_dyna_draw_context_dispose(GObject *object);

static void sp_dyna_draw_context_setup(SPEventContext *ec);
static void sp_dyna_draw_context_set(SPEventContext *ec, Inkscape::Preferences::Entry *value);
static gint sp_dyna_draw_context_root_handler(SPEventContext *ec, GdkEvent *event);

static void clear_current(SPDynaDrawContext *dc);
static void set_to_accumulated(SPDynaDrawContext *dc, bool unionize);
static void add_cap(SPCurve *curve, Geom::Point const &from, Geom::Point const &to, double rounding);
static bool accumulate_calligraphic(SPDynaDrawContext *dc);

static void fit_and_split(SPDynaDrawContext *ddc, gboolean release);

static void sp_dyna_draw_reset(SPDynaDrawContext *ddc, Geom::Point p);
static Geom::Point sp_dyna_draw_get_npoint(SPDynaDrawContext const *ddc, Geom::Point v);
static Geom::Point sp_dyna_draw_get_vpoint(SPDynaDrawContext const *ddc, Geom::Point n);
static void draw_temporary_box(SPDynaDrawContext *dc);


static SPEventContextClass *dd_parent_class = 0;

GType sp_dyna_draw_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPDynaDrawContextClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_dyna_draw_context_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPDynaDrawContext),
            0, // n_preallocs
            (GInstanceInitFunc)sp_dyna_draw_context_init,
            0 // value_table
        };
        type = g_type_register_static(SP_TYPE_COMMON_CONTEXT, "SPDynaDrawContext", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}

static void
sp_dyna_draw_context_class_init(SPDynaDrawContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    dd_parent_class = (SPEventContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_dyna_draw_context_dispose;

    event_context_class->setup = sp_dyna_draw_context_setup;
    event_context_class->set = sp_dyna_draw_context_set;
    event_context_class->root_handler = sp_dyna_draw_context_root_handler;
}

static void
sp_dyna_draw_context_init(SPDynaDrawContext *ddc)
{
    ddc->cursor_shape = cursor_calligraphy_xpm;
    ddc->hot_x = 4;
    ddc->hot_y = 4;

    ddc->vel_thin = 0.1;
    ddc->flatness = 0.9;
    ddc->cap_rounding = 0.0;

    ddc->abs_width = false;
    ddc->keep_selected = true;

    ddc->hatch_spacing = 0;
    ddc->hatch_spacing_step = 0;
    new (&ddc->hatch_pointer_past) std::list<double>();
    new (&ddc->hatch_nearest_past) std::list<double>();
    ddc->hatch_last_nearest = Geom::Point(0,0);
    ddc->hatch_last_pointer = Geom::Point(0,0);
    ddc->hatch_vector_accumulated = Geom::Point(0,0);
    ddc->hatch_escaped = false;
    ddc->hatch_area = NULL;
    ddc->hatch_item = NULL;
    ddc->hatch_livarot_path = NULL;

    ddc->trace_bg = false;
}

static void
sp_dyna_draw_context_dispose(GObject *object)
{
    SPDynaDrawContext *ddc = SP_DYNA_DRAW_CONTEXT(object);

    if (ddc->hatch_area) {
        gtk_object_destroy(GTK_OBJECT(ddc->hatch_area));
        ddc->hatch_area = NULL;
    }


    G_OBJECT_CLASS(dd_parent_class)->dispose(object);

    ddc->hatch_pointer_past.~list();
    ddc->hatch_nearest_past.~list();
}

static void
sp_dyna_draw_context_setup(SPEventContext *ec)
{
    SPDynaDrawContext *ddc = SP_DYNA_DRAW_CONTEXT(ec);

    if (((SPEventContextClass *) dd_parent_class)->setup)
        ((SPEventContextClass *) dd_parent_class)->setup(ec);

    ddc->accumulated = new SPCurve();
    ddc->currentcurve = new SPCurve();

    ddc->cal1 = new SPCurve();
    ddc->cal2 = new SPCurve();

    ddc->currentshape = sp_canvas_item_new(sp_desktop_sketch(ec->desktop), SP_TYPE_CANVAS_BPATH, NULL);
    sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(ddc->currentshape), DDC_RED_RGBA, SP_WIND_RULE_EVENODD);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(ddc->currentshape), 0x00000000, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
    /* fixme: Cannot we cascade it to root more clearly? */
    g_signal_connect(G_OBJECT(ddc->currentshape), "event", G_CALLBACK(sp_desktop_root_handler), ec->desktop);

    {
        /* TODO: this can be done either with an arcto, and should maybe also be put in a general file (other tools use this as well) */
        SPCurve *c = new SPCurve();
        const double C1 = 0.552;
        c->moveto(-1,0);
        c->curveto(-1, C1, -C1, 1, 0, 1 );
        c->curveto(C1, 1, 1, C1, 1, 0 );
        c->curveto(1, -C1, C1, -1, 0, -1 );
        c->curveto(-C1, -1, -1, -C1, -1, 0 );
        c->closepath();
        ddc->hatch_area = sp_canvas_bpath_new(sp_desktop_controls(ec->desktop), c);
        c->unref();
        sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(ddc->hatch_area), 0x00000000,(SPWindRule)0);
        sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(ddc->hatch_area), 0x0000007f, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
        sp_canvas_item_hide(ddc->hatch_area);
    }

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
    sp_event_context_read(ec, "keep_selected");
    sp_event_context_read(ec, "cap_rounding");

    ddc->is_drawing = false;
    ddc->_message_context = new Inkscape::MessageContext((ec->desktop)->messageStack());

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/calligraphic/selcue")) {
        ec->enableSelectionCue();
    }
}

static void
sp_dyna_draw_context_set(SPEventContext *ec, Inkscape::Preferences::Entry *val)
{
    SPDynaDrawContext *ddc = SP_DYNA_DRAW_CONTEXT(ec);
    Glib::ustring path = val->getEntryName();

    if (path == "tracebackground") {
        ddc->trace_bg = val->getBool();
    } else if (path == "keep_selected") {
        ddc->keep_selected = val->getBool();
    } else {
        //pass on up to parent class to handle common attributes.
        if ( dd_parent_class->set ) {
            dd_parent_class->set(ec, val);
        }
    }

    //g_print("DDC: %g %g %g %g\n", ddc->mass, ddc->drag, ddc->angle, ddc->width);
}

static double
flerp(double f0, double f1, double p)
{
    return f0 + ( f1 - f0 ) * p;
}

/* Get normalized point */
static Geom::Point
sp_dyna_draw_get_npoint(SPDynaDrawContext const *dc, Geom::Point v)
{
    Geom::Rect drect = SP_EVENT_CONTEXT(dc)->desktop->get_display_area();
    double const max = MAX ( drect.dimensions()[Geom::X], drect.dimensions()[Geom::Y] );
    return Geom::Point(( v[Geom::X] - drect.min()[Geom::X] ) / max,  ( v[Geom::Y] - drect.min()[Geom::Y] ) / max);
}

/* Get view point */
static Geom::Point
sp_dyna_draw_get_vpoint(SPDynaDrawContext const *dc, Geom::Point n)
{
    Geom::Rect drect = SP_EVENT_CONTEXT(dc)->desktop->get_display_area();
    double const max = MAX ( drect.dimensions()[Geom::X], drect.dimensions()[Geom::Y] );
    return Geom::Point(n[Geom::X] * max + drect.min()[Geom::X], n[Geom::Y] * max + drect.min()[Geom::Y]);
}

static void
sp_dyna_draw_reset(SPDynaDrawContext *dc, Geom::Point p)
{
    dc->last = dc->cur = sp_dyna_draw_get_npoint(dc, p);
    dc->vel = Geom::Point(0,0);
    dc->vel_max = 0;
    dc->acc = Geom::Point(0,0);
    dc->ang = Geom::Point(0,0);
    dc->del = Geom::Point(0,0);
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
sp_dyna_draw_apply(SPDynaDrawContext *dc, Geom::Point p)
{
    Geom::Point n = sp_dyna_draw_get_npoint(dc, p);

    /* Calculate mass and drag */
    double const mass = flerp(1.0, 160.0, dc->mass);
    double const drag = flerp(0.0, 0.5, dc->drag * dc->drag);

    /* Calculate force and acceleration */
    Geom::Point force = n - dc->cur;

    // If force is below the absolute threshold DYNA_EPSILON,
    // or we haven't yet reached DYNA_VEL_START (i.e. at the beginning of stroke)
    // _and_ the force is below the (higher) DYNA_EPSILON_START threshold,
    // discard this move. 
    // This prevents flips, blobs, and jerks caused by microscopic tremor of the tablet pen,
    // especially bothersome at the start of the stroke where we don't yet have the inertia to
    // smooth them out.
    if ( Geom::L2(force) < DYNA_EPSILON || (dc->vel_max < DYNA_VEL_START && Geom::L2(force) < DYNA_EPSILON_START)) {
        return FALSE;
    }

    dc->acc = force / mass;

    /* Calculate new velocity */
    dc->vel += dc->acc;

    if (Geom::L2(dc->vel) > dc->vel_max)
        dc->vel_max = Geom::L2(dc->vel);

    /* Calculate angle of drawing tool */

    double a1;
    if (dc->usetilt) {
        // 1a. calculate nib angle from input device tilt:
        gdouble length = std::sqrt(dc->xtilt*dc->xtilt + dc->ytilt*dc->ytilt);;

        if (length > 0) {
            Geom::Point ang1 = Geom::Point(dc->ytilt/length, dc->xtilt/length);
            a1 = atan2(ang1);
        }
        else
            a1 = 0.0;
    }
    else {
        // 1b. fixed dc->angle (absolutely flat nib):
        double const radians = ( (dc->angle - 90) / 180.0 ) * M_PI;
        Geom::Point ang1 = Geom::Point(-sin(radians),  cos(radians));
        a1 = atan2(ang1);
    }

    // 2. perpendicular to dc->vel (absolutely non-flat nib):
    gdouble const mag_vel = Geom::L2(dc->vel);
    if ( mag_vel < DYNA_EPSILON ) {
        return FALSE;
    }
    Geom::Point ang2 = Geom::rot90(dc->vel) / mag_vel;

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
    double angle_delta = Geom::L2(Geom::Point (cos (new_ang), sin (new_ang)) - dc->ang);
    if ( angle_delta / Geom::L2(dc->vel) > 4000 ) {
        return FALSE;
    }

    // convert to point
    dc->ang = Geom::Point (cos (new_ang), sin (new_ang));

//    g_print ("force %g  acc %g  vel_max %g  vel %g  a1 %g  a2 %g  new_ang %g\n", NR::L2(force), NR::L2(dc->acc), dc->vel_max, NR::L2(dc->vel), a1, a2, new_ang);

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

    // get the real brush point, not the same as pointer (affected by hatch tracking and/or mass
    // drag)
    Geom::Point brush = sp_dyna_draw_get_vpoint(dc, dc->cur);
    Geom::Point brush_w = SP_EVENT_CONTEXT(dc)->desktop->d2w(brush); 

    double trace_thick = 1;
    if (dc->trace_bg) {
        // pick single pixel
        NRPixBlock pb;
        int x = (int) floor(brush_w[Geom::X]);
        int y = (int) floor(brush_w[Geom::Y]);
        nr_pixblock_setup_fast(&pb, NR_PIXBLOCK_MODE_R8G8B8A8P, x, y, x+1, y+1, TRUE);
        sp_canvas_arena_render_pixblock(SP_CANVAS_ARENA(sp_desktop_drawing(SP_EVENT_CONTEXT(dc)->desktop)), &pb);
        const unsigned char *s = NR_PIXBLOCK_PX(&pb);
        double R = s[0] / 255.0;
        double G = s[1] / 255.0;
        double B = s[2] / 255.0;
        double A = s[3] / 255.0;
        double max = MAX (MAX (R, G), B);
        double min = MIN (MIN (R, G), B);
        double L = A * (max + min)/2 + (1 - A); // blend with white bg
        trace_thick = 1 - L;
        //g_print ("L %g thick %g\n", L, trace_thick);
    }

    double width = (pressure_thick * trace_thick - vel_thin * Geom::L2(dc->vel)) * dc->width;

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
        tremble_left  = (y1)*dc->tremor * (0.15 + 0.8*width) * (0.35 + 14*Geom::L2(dc->vel));
        tremble_right = (y2)*dc->tremor * (0.15 + 0.8*width) * (0.35 + 14*Geom::L2(dc->vel));
    }

    if ( width < 0.02 * dc->width ) {
        width = 0.02 * dc->width;
    }

    double dezoomify_factor = 0.05 * 1000;
    if (!dc->abs_width) {
        dezoomify_factor /= SP_EVENT_CONTEXT(dc)->desktop->current_zoom();
    }

    Geom::Point del_left = dezoomify_factor * (width + tremble_left) * dc->ang;
    Geom::Point del_right = dezoomify_factor * (width + tremble_right) * dc->ang;

    dc->point1[dc->npoints] = brush + del_left;
    dc->point2[dc->npoints] = brush - del_right;

    dc->del = 0.5*(del_left + del_right);

    dc->npoints++;
}

void
sp_ddc_update_toolbox (SPDesktop *desktop, const gchar *id, double value)
{
    desktop->setToolboxAdjustmentValue (id, value);
}

static void
calligraphic_cancel(SPDynaDrawContext *dc)
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
sp_dyna_draw_context_root_handler(SPEventContext *event_context,
                                  GdkEvent *event)
{
    SPDynaDrawContext *dc = SP_DYNA_DRAW_CONTEXT(event_context);
    SPDesktop *desktop = event_context->desktop;

    gint ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !event_context->space_panning) {

                SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(dc);

                if (Inkscape::have_viable_layer(desktop, dc->_message_context) == false) {
                    return TRUE;
                }

                Geom::Point const button_w(event->button.x,
                                         event->button.y);
                Geom::Point const button_dt(desktop->w2d(button_w));
                sp_dyna_draw_reset(dc, button_dt);
                sp_dyna_draw_extinput(dc, event);
                sp_dyna_draw_apply(dc, button_dt);
                dc->accumulated->reset();
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

                sp_canvas_force_full_redraw_after_interruptions(desktop->canvas, 3);
                dc->is_drawing = true;
            }
            break;
        case GDK_MOTION_NOTIFY:
        {
            Geom::Point const motion_w(event->motion.x,
                                     event->motion.y);
            Geom::Point motion_dt(desktop->w2d(motion_w));
            sp_dyna_draw_extinput(dc, event);

            dc->_message_context->clear();

            // for hatching:
            double hatch_dist = 0;
            Geom::Point hatch_unit_vector(0,0);
            Geom::Point nearest(0,0);
            Geom::Point pointer(0,0);
            Geom::Matrix motion_to_curve(Geom::identity());

            if (event->motion.state & GDK_CONTROL_MASK) { // hatching - sense the item

                SPItem *selected = sp_desktop_selection(desktop)->singleItem();
                if (selected && (SP_IS_SHAPE(selected) || SP_IS_TEXT(selected))) {
                    // One item selected, and it's a path;
                    // let's try to track it as a guide

                    if (selected != dc->hatch_item) {
                        dc->hatch_item = selected;
                        if (dc->hatch_livarot_path)
                            delete dc->hatch_livarot_path;
                        dc->hatch_livarot_path = Path_for_item (dc->hatch_item, true, true);
                        dc->hatch_livarot_path->ConvertWithBackData(0.01);
                    }

                    // calculate pointer point in the guide item's coords
                    motion_to_curve = sp_item_dt2i_affine(selected) * sp_item_i2doc_affine(selected);
                    pointer = motion_dt * motion_to_curve;

                    // calculate the nearest point on the guide path
                    boost::optional<Path::cut_position> position = get_nearest_position_on_Path(dc->hatch_livarot_path, pointer);
                    nearest = get_point_on_Path(dc->hatch_livarot_path, position->piece, position->t);


                    // distance from pointer to nearest
                    hatch_dist = NR::L2(pointer - nearest);
                    // unit-length vector
                    hatch_unit_vector = (pointer - nearest)/hatch_dist;

                    dc->_message_context->set(Inkscape::NORMAL_MESSAGE, _("<b>Guide path selected</b>; start drawing along the guide with <b>Ctrl</b>"));
                } else {
                    dc->_message_context->set(Inkscape::NORMAL_MESSAGE, _("<b>Select a guide path</b> to track with <b>Ctrl</b>"));
                }
            } 

            if ( dc->is_drawing && (event->motion.state & GDK_BUTTON1_MASK) && !event_context->space_panning) {
                dc->dragging = TRUE;

                if (event->motion.state & GDK_CONTROL_MASK && dc->hatch_item) { // hatching

#define SPEED_ELEMENTS 12
#define SPEED_MIN 0.12
#define SPEED_NORMAL 0.65

                    // speed is the movement of the nearest point along the guide path, divided by
                    // the movement of the pointer at the same period; it is averaged for the last
                    // SPEED_ELEMENTS motion events.  Normally, as you track the guide path, speed
                    // is about 1, i.e. the nearest point on the path is moved by about the same
                    // distance as the pointer. If the speed starts to decrease, we are losing
                    // contact with the guide; if it drops below SPEED_MIN, we are on our own and
                    // not attracted to guide anymore. Most often this happens when you have
                    // tracked to the end of a guide calligraphic stroke and keep moving
                    // further. We try to handle this situation gracefully: not stick with the
                    // guide forever but let go of it smoothly and without sharp jerks (non-zero
                    // mass recommended; with zero mass, jerks are still quite noticeable).

                    double speed = 1;
                    if (Geom::L2(dc->hatch_last_nearest) != 0) {
                        // the distance nearest moved since the last motion event
                        double nearest_moved = Geom::L2(nearest - dc->hatch_last_nearest);
                        // the distance pointer moved since the last motion event
                        double pointer_moved = Geom::L2(pointer - dc->hatch_last_pointer);
                        // store them in stacks limited to SPEED_ELEMENTS
                        dc->hatch_nearest_past.push_front(nearest_moved);
                        if (dc->hatch_nearest_past.size() > SPEED_ELEMENTS)
                            dc->hatch_nearest_past.pop_back();
                        dc->hatch_pointer_past.push_front(pointer_moved);
                        if (dc->hatch_pointer_past.size() > SPEED_ELEMENTS)
                            dc->hatch_pointer_past.pop_back();

                        // If the stacks are full,
                        if (dc->hatch_nearest_past.size() == SPEED_ELEMENTS) {
                            // calculate the sums of all stored movements
                            double nearest_sum = std::accumulate (dc->hatch_nearest_past.begin(), dc->hatch_nearest_past.end(), 0.0);
                            double pointer_sum = std::accumulate (dc->hatch_pointer_past.begin(), dc->hatch_pointer_past.end(), 0.0);
                            // and divide to get the speed
                            speed = nearest_sum/pointer_sum;
                            //g_print ("nearest sum %g  pointer_sum %g  speed %g\n", nearest_sum, pointer_sum, speed);
                        }
                    }

                    if (   dc->hatch_escaped  // already escaped, do not reattach
                        || (speed < SPEED_MIN) // stuck; most likely reached end of traced stroke
                        || (dc->hatch_spacing > 0 && hatch_dist > 50 * dc->hatch_spacing) // went too far from the guide
                        ) {
                        // We are NOT attracted to the guide!

                        //g_print ("\nlast_nearest %g %g   nearest %g %g  pointer %g %g  pos %d %g\n", dc->last_nearest[NR::X], dc->last_nearest[NR::Y], nearest[NR::X], nearest[NR::Y], pointer[NR::X], pointer[NR::Y], position->piece, position->t);

                        // Remember hatch_escaped so we don't get
                        // attracted again until the end of this stroke
                        dc->hatch_escaped = true;

                    } else {

                        // Calculate angle cosine of this vector-to-guide and all past vectors
                        // summed, to detect if we accidentally flipped to the other side of the
                        // guide
                        double dot = NR::dot (pointer - nearest, dc->hatch_vector_accumulated);
                        dot /= Geom::L2(pointer - nearest) * Geom::L2(dc->hatch_vector_accumulated);

                        if (dc->hatch_spacing != 0) { // spacing was already set
                            double target;
                            if (speed > SPEED_NORMAL) {
                                // all ok, strictly obey the spacing
                                target = dc->hatch_spacing;
                            } else {
                                // looks like we're starting to lose speed,
                                // so _gradually_ let go attraction to prevent jerks
                                target = (dc->hatch_spacing * speed + hatch_dist * (SPEED_NORMAL - speed))/SPEED_NORMAL;                            
                            }
                            if (!IS_NAN(dot) && dot < -0.5) {// flip
                                target = -target;
                            }

                            // This is the track pointer that we will use instead of the real one
                            Geom::Point new_pointer = nearest + target * hatch_unit_vector;

                            // some limited feedback: allow persistent pulling to slightly change
                            // the spacing
                            dc->hatch_spacing += (hatch_dist - dc->hatch_spacing)/3500;

                            // return it to the desktop coords
                            motion_dt = new_pointer * motion_to_curve.inverse();

                        } else {
                            // this is the first motion event, set the dist 
                            dc->hatch_spacing = hatch_dist;
                        }

                        // remember last points
                        dc->hatch_last_pointer = pointer;
                        dc->hatch_last_nearest = nearest;
                        dc->hatch_vector_accumulated += (pointer - nearest);
                    }

                    dc->_message_context->set(Inkscape::NORMAL_MESSAGE, dc->hatch_escaped? _("Tracking: <b>connection to guide path lost!</b>") : _("<b>Tracking</b> a guide path"));

                } else {
                    dc->_message_context->set(Inkscape::NORMAL_MESSAGE, _("<b>Drawing</b> a calligraphic stroke"));
                }

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

            // Draw the hatching circle if necessary
            if (event->motion.state & GDK_CONTROL_MASK) { 
                if (dc->hatch_spacing == 0 && hatch_dist != 0) { 
                    // Haven't set spacing yet: gray, center free, update radius live
                    Geom::Point c = desktop->w2d(motion_w);
                    NR::Matrix const sm (Geom::Scale(hatch_dist, hatch_dist) * Geom::Translate(c));
                    sp_canvas_item_affine_absolute(dc->hatch_area, sm);
                    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(dc->hatch_area), 0x7f7f7fff, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
                    sp_canvas_item_show(dc->hatch_area);
                } else if (dc->dragging && !dc->hatch_escaped) {
                    // Tracking: green, center snapped, fixed radius
                    Geom::Point c = motion_dt;
                    NR::Matrix const sm (Geom::Scale(dc->hatch_spacing, dc->hatch_spacing) * Geom::Translate(c));
                    sp_canvas_item_affine_absolute(dc->hatch_area, sm);
                    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(dc->hatch_area), 0x00FF00ff, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
                    sp_canvas_item_show(dc->hatch_area);
                } else if (dc->dragging && dc->hatch_escaped) {
                    // Tracking escaped: red, center free, fixed radius
                    Geom::Point c = desktop->w2d(motion_w);
                    NR::Matrix const sm (Geom::Scale(dc->hatch_spacing, dc->hatch_spacing) * Geom::Translate(c));

                    sp_canvas_item_affine_absolute(dc->hatch_area, sm);
                    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(dc->hatch_area), 0xFF0000ff, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
                    sp_canvas_item_show(dc->hatch_area);
                } else {
                    // Not drawing but spacing set: gray, center snapped, fixed radius
                    Geom::Point c = (nearest + dc->hatch_spacing * hatch_unit_vector) * motion_to_curve.inverse();
                    if (!IS_NAN(c[Geom::X]) && !IS_NAN(c[Geom::Y])) {
                        NR::Matrix const sm (Geom::Scale(dc->hatch_spacing, dc->hatch_spacing) * Geom::Translate(c));
                        sp_canvas_item_affine_absolute(dc->hatch_area, sm);
                        sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(dc->hatch_area), 0x7f7f7fff, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
                        sp_canvas_item_show(dc->hatch_area);
                    }
                }
            } else {
                sp_canvas_item_hide(dc->hatch_area);
            }
        }
        break;


    case GDK_BUTTON_RELEASE:
    {
        Geom::Point const motion_w(event->button.x, event->button.y);
        Geom::Point const motion_dt(desktop->w2d(motion_w));

        sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate), event->button.time);
        sp_canvas_end_forced_full_redraws(desktop->canvas);
        dc->is_drawing = false;

        if (dc->dragging && event->button.button == 1 && !event_context->space_panning) {
            dc->dragging = FALSE;

            sp_dyna_draw_apply(dc, motion_dt);

            /* Remove all temporary line segments */
            while (dc->segments) {
                gtk_object_destroy(GTK_OBJECT(dc->segments->data));
                dc->segments = g_slist_remove(dc->segments, dc->segments->data);
            }

            /* Create object */
            fit_and_split(dc, TRUE);
            if (accumulate_calligraphic(dc))
                set_to_accumulated(dc, event->button.state & GDK_SHIFT_MASK); // performs document_done
            else
                g_warning ("Failed to create path: invalid data in dc->cal1 or dc->cal2");

            /* reset accumulated curve */
            dc->accumulated->reset();

            clear_current(dc);
            if (dc->repr) {
                dc->repr = NULL;
            }

            if (!dc->hatch_pointer_past.empty()) dc->hatch_pointer_past.clear();
            if (!dc->hatch_nearest_past.empty()) dc->hatch_nearest_past.clear();
            dc->hatch_last_nearest = Geom::Point(0,0);
            dc->hatch_last_pointer = Geom::Point(0,0);
            dc->hatch_vector_accumulated = Geom::Point(0,0);
            dc->hatch_escaped = false;
            dc->hatch_item = NULL;
            dc->hatch_livarot_path = NULL;

            if (dc->hatch_spacing != 0 && !dc->keep_selected) { 
                // we do not select the newly drawn path, so increase spacing by step
                if (dc->hatch_spacing_step == 0) {
                    dc->hatch_spacing_step = dc->hatch_spacing;
                }
                dc->hatch_spacing += dc->hatch_spacing_step;
            }

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
        case GDK_Home:
        case GDK_KP_Home:
            dc->width = 0.01;
            sp_ddc_update_toolbox (desktop, "altx-calligraphy", dc->width * 100);
            ret = TRUE;
            break;
        case GDK_End:
        case GDK_KP_End:
            dc->width = 1.0;
            sp_ddc_update_toolbox (desktop, "altx-calligraphy", dc->width * 100);
            ret = TRUE;
            break;
        case GDK_x:
        case GDK_X:
            if (MOD__ALT_ONLY) {
                desktop->setToolboxFocusTo ("altx-calligraphy");
                ret = TRUE;
            }
            break;
        case GDK_Escape:
            if (dc->is_drawing) {
                // if drawing, cancel, otherwise pass it up for deselecting
                calligraphic_cancel (dc);
                ret = TRUE;
            }
            break;
        case GDK_z:
        case GDK_Z:
            if (MOD__CTRL_ONLY && dc->is_drawing) {
                // if drawing, cancel, otherwise pass it up for undo
                calligraphic_cancel (dc);
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
                dc->hatch_spacing = 0;
                dc->hatch_spacing_step = 0;
                break;
            default:
                break;
        }

    default:
        break;
    }

    if (!ret) {
        if (((SPEventContextClass *) dd_parent_class)->root_handler) {
            ret = ((SPEventContextClass *) dd_parent_class)->root_handler(event_context, event);
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
    dc->currentcurve->reset();
    dc->cal1->reset();
    dc->cal2->reset();
    /* reset points */
    dc->npoints = 0;
}

static void
set_to_accumulated(SPDynaDrawContext *dc, bool unionize)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT(dc)->desktop;

    if (!dc->accumulated->is_empty()) {
        if (!dc->repr) {
            /* Create object */
            Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());
            Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");

            /* Set style */
            sp_desktop_apply_style_tool (desktop, repr, "/tools/calligraphic", false);

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

        if (unionize) {
            sp_desktop_selection(desktop)->add(dc->repr);
            sp_selected_path_union_skip_undo(desktop);
        } else {
            if (dc->keep_selected) {
                sp_desktop_selection(desktop)->set(dc->repr);
            } 
        }

    } else {
        if (dc->repr) {
            sp_repr_unparent(dc->repr);
        }
        dc->repr = NULL;
    }

    sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_CALLIGRAPHIC, 
                     _("Draw calligraphic stroke"));
}

static void
add_cap(SPCurve *curve,
        Geom::Point const &from,
        Geom::Point const &to, 
        double rounding)
{
    if (Geom::L2( to - from ) > DYNA_EPSILON) {
        Geom::Point vel = rounding * NR::rot90( to - from ) / sqrt(2.0);
        double mag = Geom::L2(vel);

        Geom::Point v = mag * NR::rot90( to - from ) / Geom::L2( to - from );
        curve->curveto(from + v, to + v, to);
    }
}

static bool
accumulate_calligraphic(SPDynaDrawContext *dc)
{
        if (
            dc->cal1->is_empty() ||
            dc->cal2->is_empty() ||
            (dc->cal1->get_segment_count() <= 0) ||
            dc->cal1->first_path()->closed() 
            ) {
            dc->cal1->reset();
            dc->cal2->reset();
            return false; // failure
        }

        SPCurve *rev_cal2 = dc->cal2->create_reverse();
        if (
            (rev_cal2->get_segment_count() <= 0) ||
            rev_cal2->first_path()->closed() 
            ) {
            rev_cal2->unref();
            dc->cal1->reset();
            dc->cal2->reset();
            return false; // failure
        }

        Geom::CubicBezier const * dc_cal1_firstseg  = dynamic_cast<Geom::CubicBezier const *>( dc->cal1->first_segment() );
        Geom::CubicBezier const * rev_cal2_firstseg = dynamic_cast<Geom::CubicBezier const *>( rev_cal2->first_segment() );
        Geom::CubicBezier const * dc_cal1_lastseg   = dynamic_cast<Geom::CubicBezier const *>( dc->cal1->last_segment() );
        Geom::CubicBezier const * rev_cal2_lastseg  = dynamic_cast<Geom::CubicBezier const *>( rev_cal2->last_segment() );

        if (
            !dc_cal1_firstseg ||
            !rev_cal2_firstseg ||
            !dc_cal1_lastseg ||
            !rev_cal2_lastseg 
            ) {
            rev_cal2->unref();
            dc->cal1->reset();
            dc->cal2->reset();
            return false; // failure
        }

        dc->accumulated->reset(); /*  Is this required ?? */

        dc->accumulated->append(dc->cal1, false);

        add_cap(dc->accumulated, (*dc_cal1_lastseg)[3], (*rev_cal2_firstseg)[0], dc->cap_rounding);

        dc->accumulated->append(rev_cal2, true);

        add_cap(dc->accumulated, (*rev_cal2_lastseg)[3], (*dc_cal1_firstseg)[0], dc->cap_rounding);

        dc->accumulated->closepath();

        rev_cal2->unref();

        dc->cal1->reset();
        dc->cal2->reset();

        return true; // success
}

static double square(double const x)
{
    return x * x;
}

static void
fit_and_split(SPDynaDrawContext *dc, gboolean release)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT(dc)->desktop;

    double const tolerance_sq = square( desktop->w2d().descrim() * TOLERANCE_CALLIGRAPHIC );

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
        if ( dc->cal1->is_empty() || dc->cal2->is_empty() ) {
            /* dc->npoints > 0 */
            /* g_print("calligraphics(1|2) reset\n"); */
            dc->cal1->reset();
            dc->cal2->reset();

            dc->cal1->moveto(dc->point1[0]);
            dc->cal2->moveto(dc->point2[0]);
        }

        Geom::Point b1[BEZIER_MAX_LENGTH];
        gint const nb1 = sp_bezier_fit_cubic_r(b1, dc->point1, dc->npoints,
                                               tolerance_sq, BEZIER_MAX_BEZIERS);
        g_assert( nb1 * BEZIER_SIZE <= gint(G_N_ELEMENTS(b1)) );

        Geom::Point b2[BEZIER_MAX_LENGTH];
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
                dc->currentcurve->reset();
                dc->currentcurve->moveto(b1[0]);
                for (Geom::Point *bp1 = b1; bp1 < b1 + BEZIER_SIZE * nb1; bp1 += BEZIER_SIZE) {
                    dc->currentcurve->curveto(bp1[1], bp1[2], bp1[3]);
                }
                dc->currentcurve->lineto(b2[BEZIER_SIZE*(nb2-1) + 3]);
                for (Geom::Point *bp2 = b2 + BEZIER_SIZE * ( nb2 - 1 ); bp2 >= b2; bp2 -= BEZIER_SIZE) {
                    dc->currentcurve->curveto(bp2[2], bp2[1], bp2[0]);
                }
                // FIXME: dc->segments is always NULL at this point??
                if (!dc->segments) { // first segment
                    add_cap(dc->currentcurve, b2[0], b1[0], dc->cap_rounding);
                }
                dc->currentcurve->closepath();
                sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->currentshape), dc->currentcurve);
            }

            /* Current calligraphic */
            for (Geom::Point *bp1 = b1; bp1 < b1 + BEZIER_SIZE * nb1; bp1 += BEZIER_SIZE) {
                dc->cal1->curveto(bp1[1], bp1[2], bp1[3]);
            }
            for (Geom::Point *bp2 = b2; bp2 < b2 + BEZIER_SIZE * nb2; bp2 += BEZIER_SIZE) {
                dc->cal2->curveto(bp2[1], bp2[2], bp2[3]);
            }
        } else {
            /* fixme: ??? */
#ifdef DYNA_DRAW_VERBOSE
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
#ifdef DYNA_DRAW_VERBOSE
        g_print("[%d]Yup\n", dc->npoints);
#endif
        if (!release) {
            g_assert(!dc->currentcurve->is_empty());

            SPCanvasItem *cbp = sp_canvas_item_new(sp_desktop_sketch(desktop),
                                                   SP_TYPE_CANVAS_BPATH,
                                                   NULL);
            SPCurve *curve = dc->currentcurve->copy();
            sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH (cbp), curve);
            curve->unref();

            guint32 fillColor = sp_desktop_get_color_tool (desktop, "/tools/calligraphic", true);
            //guint32 strokeColor = sp_desktop_get_color_tool (desktop, "/tools/calligraphic", false);
            double opacity = sp_desktop_get_master_opacity_tool (desktop, "/tools/calligraphic");
            double fillOpacity = sp_desktop_get_opacity_tool (desktop, "/tools/calligraphic", true);
            //double strokeOpacity = sp_desktop_get_opacity_tool (desktop, "/tools/calligraphic", false);
            sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(cbp), ((fillColor & 0xffffff00) | SP_COLOR_F_TO_U(opacity*fillOpacity)), SP_WIND_RULE_EVENODD);
            //on second thougtht don't do stroke yet because we don't have stoke-width yet and because stoke appears between segments while drawing
            //sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(cbp), ((strokeColor & 0xffffff00) | SP_COLOR_F_TO_U(opacity*strokeOpacity)), 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
            sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(cbp), 0x00000000, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
            /* fixme: Cannot we cascade it to root more clearly? */
            g_signal_connect(G_OBJECT(cbp), "event", G_CALLBACK(sp_desktop_root_handler), desktop);

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
    dc->currentcurve->reset();

    dc->currentcurve->moveto(dc->point2[dc->npoints-1]);
    for (gint i = dc->npoints-2; i >= 0; i--) {
        dc->currentcurve->lineto(dc->point2[i]);
    }
    for (gint i = 0; i < dc->npoints; i++) {
        dc->currentcurve->lineto(dc->point1[i]);
    }

    if (dc->npoints >= 2) {
        add_cap(dc->currentcurve, dc->point1[dc->npoints-1], dc->point2[dc->npoints-1], dc->cap_rounding);
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

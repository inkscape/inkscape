#define __SP_TWEAK_CONTEXT_C__

/*
 * tweaking paths without node editing
 *
 * Authors:
 *   bulia byak
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glibmm/i18n.h>

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
#include "pixmaps/cursor-thin.xpm"
#include "pixmaps/cursor-thicken.xpm"
#include "pixmaps/cursor-attract.xpm"
#include "pixmaps/cursor-repel.xpm"
#include "pixmaps/cursor-push.xpm"
#include "pixmaps/cursor-roughen.xpm"
#include "pixmaps/cursor-color.xpm"
#include "libnr/n-art-bpath.h"
#include "libnr/nr-path.h"
#include "libnr/nr-maybe.h"
#include "libnr/nr-matrix-ops.h"
#include "libnr/nr-scale-translate-ops.h"
#include "xml/repr.h"
#include "context-fns.h"
#include "sp-item.h"
#include "inkscape.h"
#include "color.h"
#include "svg/svg-color.h"
#include "splivarot.h"
#include "sp-item-group.h"
#include "sp-shape.h"
#include "sp-path.h"
#include "path-chemistry.h"
#include "sp-gradient.h"
#include "sp-stop.h"
#include "sp-stop-fns.h"
#include "sp-gradient-reference.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "gradient-chemistry.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "display/canvas-bpath.h"
#include "display/canvas-arena.h"
#include "livarot/Shape.h"
#include "isnan.h"
#include "prefs-utils.h"
#include "style.h"
#include "box3d.h"

#include "tweak-context.h"

#define DDC_RED_RGBA 0xff0000ff

#define DYNA_MIN_WIDTH 1.0e-6

// FIXME: move it to some shared file to be reused by both calligraphy and dropper
#define C1 0.552
static NArtBpath const hatch_area_circle[] = {
    { NR_MOVETO, 0, 0, 0, 0, -1, 0 },
    { NR_CURVETO, -1, C1, -C1, 1, 0, 1 },
    { NR_CURVETO, C1, 1, 1, C1, 1, 0 },
    { NR_CURVETO, 1, -C1, C1, -1, 0, -1 },
    { NR_CURVETO, -C1, -1, -1, -C1, -1, 0 },
    { NR_END, 0, 0, 0, 0, 0, 0 }
};
#undef C1


static void sp_tweak_context_class_init(SPTweakContextClass *klass);
static void sp_tweak_context_init(SPTweakContext *ddc);
static void sp_tweak_context_dispose(GObject *object);

static void sp_tweak_context_setup(SPEventContext *ec);
static void sp_tweak_context_set(SPEventContext *ec, gchar const *key, gchar const *val);
static gint sp_tweak_context_root_handler(SPEventContext *ec, GdkEvent *event);

static SPEventContextClass *parent_class;

GtkType
sp_tweak_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPTweakContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_tweak_context_class_init,
            NULL, NULL,
            sizeof(SPTweakContext),
            4,
            (GInstanceInitFunc) sp_tweak_context_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPTweakContext", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_tweak_context_class_init(SPTweakContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_tweak_context_dispose;

    event_context_class->setup = sp_tweak_context_setup;
    event_context_class->set = sp_tweak_context_set;
    event_context_class->root_handler = sp_tweak_context_root_handler;
}

static void
sp_tweak_context_init(SPTweakContext *tc)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(tc);

    event_context->cursor_shape = cursor_push_xpm;
    event_context->hot_x = 4;
    event_context->hot_y = 4;

    /* attributes */
    tc->dragging = FALSE;

    tc->width = 0.2;
    tc->force = 0.2;
    tc->pressure = TC_DEFAULT_PRESSURE;

    tc->is_dilating = false;
    tc->has_dilated = false;

    tc->do_h = true;
    tc->do_s = true;
    tc->do_l = true;
    tc->do_o = false;

    new (&tc->style_set_connection) sigc::connection();
}

static void
sp_tweak_context_dispose(GObject *object)
{
    SPTweakContext *tc = SP_TWEAK_CONTEXT(object);

    tc->style_set_connection.disconnect();
    tc->style_set_connection.~connection();

    if (tc->dilate_area) {
        gtk_object_destroy(GTK_OBJECT(tc->dilate_area));
        tc->dilate_area = NULL;
    }

    if (tc->_message_context) {
        delete tc->_message_context;
    }

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

void
sp_tweak_update_cursor (SPTweakContext *tc, gint mode)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(tc);
   switch (mode) {
       case TWEAK_MODE_PUSH:
           event_context->cursor_shape = cursor_push_xpm;
           break;
       case TWEAK_MODE_SHRINK:
           event_context->cursor_shape = cursor_thin_xpm;
           break;
       case TWEAK_MODE_GROW:
           event_context->cursor_shape = cursor_thicken_xpm;
           break;
       case TWEAK_MODE_ATTRACT:
           event_context->cursor_shape = cursor_attract_xpm;
           break;
       case TWEAK_MODE_REPEL:
           event_context->cursor_shape = cursor_repel_xpm;
           break;
       case TWEAK_MODE_ROUGHEN:
           event_context->cursor_shape = cursor_roughen_xpm;
           break;
       case TWEAK_MODE_COLORPAINT:
       case TWEAK_MODE_COLORJITTER:
           event_context->cursor_shape = cursor_color_xpm;
           break;
   }
   sp_event_context_update_cursor(event_context);
}

static bool
sp_tweak_context_style_set(SPCSSAttr const *css, SPTweakContext *tc)
{
    if (tc->mode == TWEAK_MODE_COLORPAINT) { // intercept color setting only in this mode
        // we cannot store properties with uris
        css = sp_css_attr_unset_uris ((SPCSSAttr *) css);

        sp_repr_css_change (inkscape_get_repr (INKSCAPE, "tools.tweak"), (SPCSSAttr *) css, "style");

        return true;
    }
    return false;
}


static void
sp_tweak_context_setup(SPEventContext *ec)
{
    SPTweakContext *tc = SP_TWEAK_CONTEXT(ec);

    if (((SPEventContextClass *) parent_class)->setup)
        ((SPEventContextClass *) parent_class)->setup(ec);

    {
        SPCurve *c = sp_curve_new_from_foreign_bpath(hatch_area_circle);
        tc->dilate_area = sp_canvas_bpath_new(sp_desktop_controls(ec->desktop), c);
        sp_curve_unref(c);
        sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(tc->dilate_area), 0x00000000,(SPWindRule)0);
        sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(tc->dilate_area), 0xff9900ff, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
        sp_canvas_item_hide(tc->dilate_area);
    }

    sp_event_context_read(ec, "width");
    sp_event_context_read(ec, "mode");
    sp_event_context_read(ec, "fidelity");
    sp_event_context_read(ec, "force");
    sp_event_context_read(ec, "usepressure");
    sp_event_context_read(ec, "doh");
    sp_event_context_read(ec, "dol");
    sp_event_context_read(ec, "dos");
    sp_event_context_read(ec, "doo");

    tc->is_drawing = false;

    tc->_message_context = new Inkscape::MessageContext((ec->desktop)->messageStack());

    tc->style_set_connection = ec->desktop->connectSetStyle( // catch style-setting signal in this tool
        sigc::bind(sigc::ptr_fun(&sp_tweak_context_style_set), tc)
    );

    if (prefs_get_int_attribute("tools.tweak", "selcue", 0) != 0) {
        ec->enableSelectionCue();
    }

    if (prefs_get_int_attribute("tools.tweak", "gradientdrag", 0) != 0) {
        ec->enableGrDrag();
    }
}

static void
sp_tweak_context_set(SPEventContext *ec, gchar const *key, gchar const *val)
{
    SPTweakContext *tc = SP_TWEAK_CONTEXT(ec);

    if (!strcmp(key, "width")) {
        double const dval = ( val ? g_ascii_strtod (val, NULL) : 0.1 );
        tc->width = CLAMP(dval, -1000.0, 1000.0);
    } else if (!strcmp(key, "mode")) {
        gint64 const dval = ( val ? g_ascii_strtoll (val, NULL, 10) : 0 );
        tc->mode = dval;
        sp_tweak_update_cursor(tc, tc->mode);
    } else if (!strcmp(key, "fidelity")) {
        double const dval = ( val ? g_ascii_strtod (val, NULL) : 0.0 );
        tc->fidelity = CLAMP(dval, 0.0, 1.0);
    } else if (!strcmp(key, "force")) {
        double const dval = ( val ? g_ascii_strtod (val, NULL) : 1.0 );
        tc->force = CLAMP(dval, 0, 1.0);
    } else if (!strcmp(key, "usepressure")) {
        tc->usepressure = (val && strcmp(val, "0"));
    } else if (!strcmp(key, "doh")) {
        tc->do_h = (val && strcmp(val, "0"));
    } else if (!strcmp(key, "dos")) {
        tc->do_s = (val && strcmp(val, "0"));
    } else if (!strcmp(key, "dol")) {
        tc->do_l = (val && strcmp(val, "0"));
    } else if (!strcmp(key, "doo")) {
        tc->do_o = (val && strcmp(val, "0"));
    }
}

static void
sp_tweak_extinput(SPTweakContext *tc, GdkEvent *event)
{
    if (gdk_event_get_axis (event, GDK_AXIS_PRESSURE, &tc->pressure))
        tc->pressure = CLAMP (tc->pressure, TC_MIN_PRESSURE, TC_MAX_PRESSURE);
    else
        tc->pressure = TC_DEFAULT_PRESSURE;
}

double
get_dilate_radius (SPTweakContext *tc)
{
    // 10 times the pen width:
    return 500 * tc->width/SP_EVENT_CONTEXT(tc)->desktop->current_zoom();
}

double
get_dilate_force (SPTweakContext *tc)
{
    double force = 8 * (tc->usepressure? tc->pressure : TC_DEFAULT_PRESSURE)
        /sqrt(SP_EVENT_CONTEXT(tc)->desktop->current_zoom());
    if (force > 3) {
        force += 4 * (force - 3);
    }
    return force * tc->force;
}

bool
sp_tweak_dilate_recursive (Inkscape::Selection *selection, SPItem *item, NR::Point p, NR::Point vector, gint mode, double radius, double force, double fidelity)
{
    bool did = false;

    if (SP_IS_BOX3D(item)) {
        // convert 3D boxes to ordinary groups before tweaking their shapes
        item = SP_ITEM(box3d_convert_to_group(SP_BOX3D(item)));
        selection->add(item);
    }

    if (SP_IS_GROUP(item)) {
        for (SPObject *child = sp_object_first_child(SP_OBJECT(item)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
            if (SP_IS_ITEM(child)) {
                if (sp_tweak_dilate_recursive (selection, SP_ITEM(child), p, vector, mode, radius, force, fidelity))
                    did = true;
            }
        }

    } else if (SP_IS_PATH(item) || SP_IS_SHAPE(item) || SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item)) {

        Inkscape::XML::Node *newrepr = NULL;
        gint pos = 0;
        Inkscape::XML::Node *parent = NULL;
        char const *id = NULL;
        if (!SP_IS_PATH(item)) {
            newrepr = sp_selected_item_to_curved_repr(item, 0);
            if (!newrepr)
                return false;

            // remember the position of the item
            pos = SP_OBJECT_REPR(item)->position();
            // remember parent
            parent = SP_OBJECT_REPR(item)->parent();
            // remember id
            id = SP_OBJECT_REPR(item)->attribute("id");
        }


        // skip those paths whose bboxes are entirely out of reach with our radius
        NR::Maybe<NR::Rect> bbox = item->getBounds(sp_item_i2doc_affine(item));
        if (bbox) {
            bbox->growBy(radius);
            if (!bbox->contains(p)) {
                return false;
            }
        }

        Path *orig = Path_for_item(item, false);
        if (orig == NULL) {
            return false;
        }

        Path *res = new Path;
        res->SetBackData(false);

        Shape *theShape = new Shape;
        Shape *theRes = new Shape;
        NR::Matrix i2doc(sp_item_i2doc_affine(item));

        orig->ConvertWithBackData((0.08 - (0.07 * fidelity)) / NR::expansion(i2doc)); // default 0.059
        orig->Fill(theShape, 0);

        SPCSSAttr *css = sp_repr_css_attr(SP_OBJECT_REPR(item), "style");
        gchar const *val = sp_repr_css_property(css, "fill-rule", NULL);
        if (val && strcmp(val, "nonzero") == 0)
        {
            theRes->ConvertToShape(theShape, fill_nonZero);
        }
        else if (val && strcmp(val, "evenodd") == 0)
        {
            theRes->ConvertToShape(theShape, fill_oddEven);
        }
        else
        {
            theRes->ConvertToShape(theShape, fill_nonZero);
        }

        if (NR::L2(vector) != 0)
            vector = 1/NR::L2(vector) * vector;

        bool did_this = false;
        if (mode == TWEAK_MODE_SHRINK || mode == TWEAK_MODE_GROW) {
            if (theShape->MakeTweak(tweak_mode_grow, theRes,
                                 mode == TWEAK_MODE_GROW? force : -force,
                                 join_straight, 4.0,
                                 true, p, NR::Point(0,0), radius, &i2doc) == 0) // 0 means the shape was actually changed
              did_this = true;
        } else if (mode == TWEAK_MODE_ATTRACT || mode == TWEAK_MODE_REPEL) {
            if (theShape->MakeTweak(tweak_mode_repel, theRes,
                                 mode == TWEAK_MODE_REPEL? force : -force,
                                 join_straight, 4.0,
                                 true, p, NR::Point(0,0), radius, &i2doc) == 0)
              did_this = true;
        } else if (mode == TWEAK_MODE_PUSH) {
            if (theShape->MakeTweak(tweak_mode_push, theRes,
                                 1.0,
                                 join_straight, 4.0,
                                 true, p, force*2*vector, radius, &i2doc) == 0)
              did_this = true;
        } else if (mode == TWEAK_MODE_ROUGHEN) {
            if (theShape->MakeTweak(tweak_mode_roughen, theRes,
                                 force,
                                 join_straight, 4.0,
                                 true, p, NR::Point(0,0), radius, &i2doc) == 0)
              did_this = true;
        }

        // the rest only makes sense if we actually changed the path
        if (did_this) {
            theRes->ConvertToShape(theShape, fill_positive);

            res->Reset();
            theRes->ConvertToForme(res);

            double th_max = (0.6 - 0.59*sqrt(fidelity)) / NR::expansion(i2doc);
            double threshold = MAX(th_max, th_max*force);
            res->ConvertEvenLines(threshold);
            res->Simplify(threshold / (SP_ACTIVE_DESKTOP->current_zoom()));

            if (newrepr) { // converting to path, need to replace the repr
                bool is_selected = selection->includes(item);
                if (is_selected)
                    selection->remove(item);

                // It's going to resurrect, so we delete without notifying listeners.
                SP_OBJECT(item)->deleteObject(false);

                // restore id
                newrepr->setAttribute("id", id);
                // add the new repr to the parent
                parent->appendChild(newrepr);
                // move to the saved position
                newrepr->setPosition(pos > 0 ? pos : 0);

                if (is_selected)
                    selection->add(newrepr);
            }

            if (res->descr_cmd.size() > 1) {
                gchar *str = res->svg_dump_path();
                if (newrepr) {
                    newrepr->setAttribute("d", str);
                } else {
                    if (SP_IS_LPE_ITEM(item) && sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(item))) {
                        SP_OBJECT_REPR(item)->setAttribute("inkscape:original-d", str);
                    } else {
                        SP_OBJECT_REPR(item)->setAttribute("d", str);
                    }
                }
                g_free(str);
            } else {
                // TODO: if there's 0 or 1 node left, delete this path altogether
            }

            if (newrepr) {
                Inkscape::GC::release(newrepr);
                newrepr = NULL;
            }
        }

        delete theShape;
        delete theRes;
        delete orig;
        delete res;

        if (did_this)
            did = true;
    }

    return did;
}

void
tweak_colorpaint (float *color, guint32 goal, double force, bool do_h, bool do_s, bool do_l)
{
    float rgb_g[3];

    if (!do_h || !do_s || !do_l) {
        float hsl_g[3];
        sp_color_rgb_to_hsl_floatv (hsl_g, SP_RGBA32_R_F(goal), SP_RGBA32_G_F(goal), SP_RGBA32_B_F(goal));
        float hsl_c[3];
        sp_color_rgb_to_hsl_floatv (hsl_c, color[0], color[1], color[2]);
        if (!do_h)
            hsl_g[0] = hsl_c[0];
        if (!do_s)
            hsl_g[1] = hsl_c[1];
        if (!do_l)
            hsl_g[2] = hsl_c[2];
        sp_color_hsl_to_rgb_floatv (rgb_g, hsl_g[0], hsl_g[1], hsl_g[2]);
    } else {
        rgb_g[0] = SP_RGBA32_R_F(goal);
        rgb_g[1] = SP_RGBA32_G_F(goal);
        rgb_g[2] = SP_RGBA32_B_F(goal);
    }

    for (int i = 0; i < 3; i++) {
        double d = rgb_g[i] - color[i];
        color[i] += d * force;
    }
}

void
tweak_colorjitter (float *color, double force, bool do_h, bool do_s, bool do_l)
{
    float hsl_c[3];
    sp_color_rgb_to_hsl_floatv (hsl_c, color[0], color[1], color[2]);

    if (do_h) {
        hsl_c[0] += g_random_double_range(-0.5, 0.5) * force;
        if (hsl_c[0] > 1)
            hsl_c[0] -= 1;
        if (hsl_c[0] < 0)
            hsl_c[0] += 1;
    }
    if (do_s) {
        hsl_c[1] += g_random_double_range(-hsl_c[1], 1 - hsl_c[1]) * force;
    }
    if (do_l) {
        hsl_c[2] += g_random_double_range(-hsl_c[2], 1 - hsl_c[2]) * force;
    }

    sp_color_hsl_to_rgb_floatv (color, hsl_c[0], hsl_c[1], hsl_c[2]);
}

void
tweak_color (guint mode, float *color, guint32 goal, double force, bool do_h, bool do_s, bool do_l)
{
    if (mode == TWEAK_MODE_COLORPAINT) {
        tweak_colorpaint (color, goal, force, do_h, do_s, do_l);
    } else if (mode == TWEAK_MODE_COLORJITTER) {
        tweak_colorjitter (color, force, do_h, do_s, do_l);
    }
}

void
tweak_opacity (guint mode, SPIScale24 *style_opacity, double opacity_goal, double force)
{
    double opacity = SP_SCALE24_TO_FLOAT (style_opacity->value);

    if (mode == TWEAK_MODE_COLORPAINT) {
        double d = opacity_goal - opacity;
        opacity += d * force;
    } else if (mode == TWEAK_MODE_COLORJITTER) {
        opacity += g_random_double_range(-opacity, 1 - opacity) * force;
    }

    style_opacity->value = SP_SCALE24_FROM_FLOAT(opacity);
}


double
tweak_profile (double dist, double radius)
{
    if (radius == 0)
        return 0;
    double x = dist / radius;
    double alpha = 1;
    if (x >= 1) {
        return 0;
    } else if (x <= 0) {
        return 1;
    } else {
        return (0.5 * cos (M_PI * (pow(x, alpha))) + 0.5);
    }
}

void
tweak_colors_in_gradient (SPItem *item, bool fill_or_stroke,
                          guint32 const rgb_goal, NR::Point p_w, double radius, double force, guint mode,
                          bool do_h, bool do_s, bool do_l, bool /*do_o*/)
{
    SPGradient *gradient = sp_item_gradient (item, fill_or_stroke);

    if (!gradient || !SP_IS_GRADIENT(gradient))
        return;

    NR::Matrix i2d = sp_item_i2doc_affine (item);
    NR::Point p = p_w * i2d.inverse();
    p *= (gradient->gradientTransform).inverse();
    // now p is in gradient's original coordinates

    double pos = 0;
    double r = 0;
    if (SP_IS_LINEARGRADIENT(gradient)) {
        SPLinearGradient *lg = SP_LINEARGRADIENT(gradient);

        NR::Point p1(lg->x1.computed, lg->y1.computed);
        NR::Point p2(lg->x2.computed, lg->y2.computed);
        NR::Point pdiff(p2 - p1);
        double vl = NR::L2(pdiff);

        // This is the matrix which moves and rotates the gradient line
        // so it's oriented along the X axis:
        NR::Matrix norm = NR::Matrix(NR::translate(-p1)) * NR::Matrix(NR::rotate(-atan2(pdiff[NR::Y], pdiff[NR::X])));

        // Transform the mouse point by it to find out its projection onto the gradient line:
        NR::Point pnorm = p * norm;

        // Scale its X coordinate to match the length of the gradient line:
        pos = pnorm[NR::X] / vl;
        // Calculate radius in lenfth-of-gradient-line units
        r = radius / vl;

    } else if (SP_IS_RADIALGRADIENT(gradient)) {
        SPRadialGradient *rg = SP_RADIALGRADIENT(gradient);
        NR::Point c (rg->cx.computed, rg->cy.computed);
        pos = NR::L2(p - c) / rg->r.computed;
        r = radius / rg->r.computed;
    }

    // Normalize pos to 0..1, taking into accound gradient spread:
    double pos_e = pos;
    if (gradient->spread == SP_GRADIENT_SPREAD_PAD) {
        if (pos > 1)
            pos_e = 1;
        if (pos < 0)
            pos_e = 0;
    } else if (gradient->spread == SP_GRADIENT_SPREAD_REPEAT) {
        if (pos > 1 || pos < 0)
            pos_e = pos - floor(pos);
    } else if (gradient->spread == SP_GRADIENT_SPREAD_REFLECT) {
        if (pos > 1 || pos < 0) {
            bool odd = ((int)(floor(pos)) % 2 == 1);
            pos_e = pos - floor(pos);
            if (odd)
                pos_e = 1 - pos_e;
        }
    }

    SPGradient *vector = sp_gradient_get_forked_vector_if_necessary(gradient, false);

    double offset_l = 0;
    double offset_h = 0;
    SPObject *child_prev = NULL;
    for (SPObject *child = sp_object_first_child(vector);
         child != NULL; child = SP_OBJECT_NEXT(child)) {
        if (!SP_IS_STOP(child))
            continue;
        SPStop *stop = SP_STOP (child);

        offset_h = stop->offset;

        if (child_prev) {

            if (offset_h - offset_l > r && pos_e >= offset_l && pos_e <= offset_h) {
                // the summit falls in this interstop, and the radius is small,
                // so it only affects the ends of this interstop;
                // distribute the force between the two endstops so that they
                // get all the painting even if they are not touched by the brush
                tweak_color (mode, stop->specified_color.v.c, rgb_goal,
                                  force * (pos_e - offset_l) / (offset_h - offset_l),
                                  do_h, do_s, do_l);
                tweak_color (mode, SP_STOP(child_prev)->specified_color.v.c, rgb_goal,
                                  force * (offset_h - pos_e) / (offset_h - offset_l),
                                  do_h, do_s, do_l);
                SP_OBJECT(stop)->updateRepr();
                child_prev->updateRepr();
                break;
            } else {
                // wide brush, may affect more than 2 stops,
                // paint each stop by the force from the profile curve
                if (offset_l <= pos_e && offset_l > pos_e - r) {
                    tweak_color (mode, SP_STOP(child_prev)->specified_color.v.c, rgb_goal,
                                 force * tweak_profile (fabs (pos_e - offset_l), r),
                                 do_h, do_s, do_l);
                    child_prev->updateRepr();
                }

                if (offset_h >= pos_e && offset_h < pos_e + r) {
                    tweak_color (mode, stop->specified_color.v.c, rgb_goal,
                                 force * tweak_profile (fabs (pos_e - offset_h), r),
                                 do_h, do_s, do_l);
                    SP_OBJECT(stop)->updateRepr();
                }
            }
        }

        offset_l = offset_h;
        child_prev = child;
    }
}

bool
sp_tweak_color_recursive (guint mode, SPItem *item, SPItem *item_at_point,
                          guint32 fill_goal, bool do_fill,
                          guint32 stroke_goal, bool do_stroke,
                          float opacity_goal, bool do_opacity,
                          NR::Point p, double radius, double force,
                          bool do_h, bool do_s, bool do_l, bool do_o)
{
    bool did = false;

    if (SP_IS_GROUP(item)) {
        for (SPObject *child = sp_object_first_child(SP_OBJECT(item)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
            if (SP_IS_ITEM(child)) {
                if (sp_tweak_color_recursive (mode, SP_ITEM(child), item_at_point,
                                          fill_goal, do_fill,
                                          stroke_goal, do_stroke,
                                          opacity_goal, do_opacity,
                                          p, radius, force, do_h, do_s, do_l, do_o));
                    did = true;
            }
        }

    } else {
        SPStyle *style = SP_OBJECT_STYLE(item);
        if (!style) {
            return false;
        }
        NR::Maybe<NR::Rect> bbox = item->getBounds(sp_item_i2doc_affine(item),
                                                        SPItem::GEOMETRIC_BBOX);
        if (!bbox) {
            return false;
        }

        NR::Rect brush(p - NR::Point(radius, radius), p + NR::Point(radius, radius));

        NR::Point center = bbox->midpoint();
        double this_force;

// if item == item_at_point, use max force
        if (item == item_at_point) {
            this_force = force;
// else if no overlap of bbox and brush box, skip:
        } else if (!bbox->intersects(brush)) {
            return false;
//TODO:
// else if object > 1.5 brush: test 4/8/16 points in the brush on hitting the object, choose max
        //} else if (bbox->maxExtent() > 3 * radius) {
        //}
// else if object > 0.5 brush: test 4 corners of bbox and center on being in the brush, choose max
// else if still smaller, then check only the object center:
        } else {
            this_force = force * tweak_profile (NR::L2 (p - center), radius);
        }

        if (this_force > 0.002) {

            if (do_fill) {
                if (style->fill.isPaintserver()) {
                    tweak_colors_in_gradient (item, true, fill_goal, p, radius, this_force, mode, do_h, do_s, do_l, do_o);
                    did = true;
                } else if (style->fill.isColor()) {
                    tweak_color (mode, style->fill.value.color.v.c, fill_goal, this_force, do_h, do_s, do_l);
                    item->updateRepr();
                    did = true;
                }
            }
            if (do_stroke) {
                if (style->stroke.isPaintserver()) {
                    tweak_colors_in_gradient (item, false, stroke_goal, p, radius, this_force, mode, do_h, do_s, do_l, do_o);
                    did = true;
                } else if (style->stroke.isColor()) {
                    tweak_color (mode, style->stroke.value.color.v.c, stroke_goal, this_force, do_h, do_s, do_l);
                    item->updateRepr();
                    did = true;
                }
            }
            if (do_opacity && do_o) {
                tweak_opacity (mode, &style->opacity, opacity_goal, this_force);
            }
        }
    }

    return did;
}


bool
sp_tweak_dilate (SPTweakContext *tc, NR::Point event_p, NR::Point p, NR::Point vector)
{
    Inkscape::Selection *selection = sp_desktop_selection(SP_EVENT_CONTEXT(tc)->desktop);

    if (selection->isEmpty()) {
        return false;
    }

    bool did = false;
    double radius = get_dilate_radius(tc);
    double force = get_dilate_force(tc);
    if (radius == 0 || force == 0) {
        return false;
    }
    double color_force = MIN(sqrt(force)/20.0, 1);

    SPItem *item_at_point = SP_EVENT_CONTEXT(tc)->desktop->item_at_point(event_p, TRUE);
    Inkscape::XML::Node *tool_repr = inkscape_get_repr(INKSCAPE, "tools.tweak");
    SPCSSAttr *css = sp_repr_css_attr_inherited(tool_repr, "style");
    if (tc->mode == TWEAK_MODE_COLORPAINT && !css)
        return false;

    bool do_fill = false, do_stroke = false, do_opacity = false;
    guint32 fill_goal = 0, stroke_goal = 0;
    float opacity_goal = 1;

    const gchar *fill_prop = sp_repr_css_property(css, "fill", NULL);
    if (fill_prop && strcmp(fill_prop, "none")) {
        do_fill = true;
        fill_goal = sp_svg_read_color(fill_prop, 0);
    }
    const gchar *stroke_prop = sp_repr_css_property(css, "stroke", NULL);
    if (stroke_prop && strcmp(stroke_prop, "none")) {
        do_stroke = true;
        stroke_goal = sp_svg_read_color(stroke_prop, 0);
    }
    const gchar *opacity_prop = sp_repr_css_property(css, "opacity", NULL);
    if (opacity_prop) {
        do_opacity = true;
        sp_svg_number_read_f(opacity_prop, &opacity_goal);
    }

    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next) {

        SPItem *item = (SPItem *) items->data;

        if (tc->mode == TWEAK_MODE_COLORPAINT || tc->mode == TWEAK_MODE_COLORJITTER) {
            if (do_fill || do_stroke || do_opacity) {
                if (sp_tweak_color_recursive (tc->mode, item, item_at_point,
                                          fill_goal, do_fill,
                                          stroke_goal, do_stroke,
                                          opacity_goal, do_opacity,
                                          p, radius, color_force, tc->do_h, tc->do_s, tc->do_l, tc->do_o))
                did = true;
            }
        } else {
            if (sp_tweak_dilate_recursive (selection, item, p, vector, tc->mode, radius, force, tc->fidelity))
                did = true;
        }
    }

    return did;
}

void
sp_tweak_update_area (SPTweakContext *tc)
{
        double radius = get_dilate_radius(tc);
        NR::Matrix const sm (NR::scale(radius, radius) * NR::translate(SP_EVENT_CONTEXT(tc)->desktop->point()));
        sp_canvas_item_affine_absolute(tc->dilate_area, sm);
        sp_canvas_item_show(tc->dilate_area);
}

void
sp_tweak_switch_mode (SPTweakContext *tc, gint mode)
{
    SP_EVENT_CONTEXT(tc)->desktop->setToolboxSelectOneValue ("tweak_tool_mode", mode);
    // need to set explicitly, because the prefs may not have changed by the previous
    tc->mode = mode;
    sp_tweak_update_cursor (tc, mode);
}

void
sp_tweak_switch_mode_temporarily (SPTweakContext *tc, gint mode)
{
   // Juggling about so that prefs have the old value but tc->mode and the button show new mode:
   gint now_mode = prefs_get_int_attribute("tools.tweak", "mode", 0);
   SP_EVENT_CONTEXT(tc)->desktop->setToolboxSelectOneValue ("tweak_tool_mode", mode);
   // button has changed prefs, restore
   prefs_set_int_attribute("tools.tweak", "mode", now_mode);
   // changing prefs changed tc->mode, restore back :)
   tc->mode = mode;
   sp_tweak_update_cursor (tc, mode);
}

gint
sp_tweak_context_root_handler(SPEventContext *event_context,
                                  GdkEvent *event)
{
    SPTweakContext *tc = SP_TWEAK_CONTEXT(event_context);
    SPDesktop *desktop = event_context->desktop;

    gint ret = FALSE;

    switch (event->type) {
        case GDK_ENTER_NOTIFY:
            sp_canvas_item_show(tc->dilate_area);
            break;
        case GDK_LEAVE_NOTIFY:
            sp_canvas_item_hide(tc->dilate_area);
            break;
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !event_context->space_panning) {

                if (Inkscape::have_viable_layer(desktop, tc->_message_context) == false) {
                    return TRUE;
                }

                NR::Point const button_w(event->button.x,
                                         event->button.y);
                NR::Point const button_dt(desktop->w2d(button_w));
                tc->last_push = desktop->dt2doc(button_dt);

                sp_tweak_extinput(tc, event);

                sp_canvas_force_full_redraw_after_interruptions(desktop->canvas, 3);
                tc->is_drawing = true;
                tc->is_dilating = true;
                tc->has_dilated = false;

                ret = TRUE;
            }
            break;
        case GDK_MOTION_NOTIFY:
        {
            NR::Point const motion_w(event->motion.x,
                                     event->motion.y);
            NR::Point motion_dt(desktop->w2d(motion_w));
            NR::Point motion_doc(desktop->dt2doc(motion_dt));
            sp_tweak_extinput(tc, event);

            // draw the dilating cursor
                double radius = get_dilate_radius(tc);
                NR::Matrix const sm (NR::scale(radius, radius) * NR::translate(desktop->w2d(motion_w)));
                sp_canvas_item_affine_absolute(tc->dilate_area, sm);
                sp_canvas_item_show(tc->dilate_area);

                guint num = 0;
                if (!desktop->selection->isEmpty()) {
                    num = g_slist_length((GSList *) desktop->selection->itemList());
                }
                if (num == 0) {
                    tc->_message_context->set(Inkscape::NORMAL_MESSAGE, _("<b>Nothing selected!</b> Select objects to tweak."));
                } else {
                    switch (tc->mode) {
                        case TWEAK_MODE_PUSH:
                           tc->_message_context->setF(Inkscape::NORMAL_MESSAGE,
                                                      ngettext("<b>Pushing %d</b> selected object",
                                                      "<b>Pushing %d</b> selected objects", num), num);
                           break;
                        case TWEAK_MODE_SHRINK:
                           tc->_message_context->setF(Inkscape::NORMAL_MESSAGE,
                                                      ngettext("<b>Shrinking %d</b> selected object",
                                                      "<b>Shrinking %d</b> selected objects", num), num);
                           break;
                        case TWEAK_MODE_GROW:
                           tc->_message_context->setF(Inkscape::NORMAL_MESSAGE,
                                                      ngettext("<b>Growing %d</b> selected object",
                                                      "<b>Growing %d</b> selected objects", num), num);
                           break;
                        case TWEAK_MODE_ATTRACT:
                           tc->_message_context->setF(Inkscape::NORMAL_MESSAGE,
                                                      ngettext("<b>Attracting %d</b> selected object",
                                                      "<b>Attracting %d</b> selected objects", num), num);
                           break;
                        case TWEAK_MODE_REPEL:
                           tc->_message_context->setF(Inkscape::NORMAL_MESSAGE,
                                                      ngettext("<b>Repelling %d</b> selected object",
                                                      "<b>Repelling %d</b> selected objects", num), num);
                           break;
                        case TWEAK_MODE_ROUGHEN:
                           tc->_message_context->setF(Inkscape::NORMAL_MESSAGE,
                                                      ngettext("<b>Roughening %d</b> selected object",
                                                      "<b>Roughening %d</b> selected objects", num), num);
                        case TWEAK_MODE_COLORPAINT:
                           tc->_message_context->setF(Inkscape::NORMAL_MESSAGE,
                                                      ngettext("<b>Painting %d</b> selected object",
                                                      "<b>Painting %d</b> selected objects", num), num);
                           break;
                        case TWEAK_MODE_COLORJITTER:
                           tc->_message_context->setF(Inkscape::NORMAL_MESSAGE,
                                                      ngettext("<b>Jittering colors in %d</b> selected object",
                                                      "<b>Jittering colors in %d</b> selected objects", num), num);
                           break;
                    }
                }


            // dilating:
            if (tc->is_drawing && ( event->motion.state & GDK_BUTTON1_MASK )) {
                sp_tweak_dilate (tc, motion_w, motion_doc, motion_doc - tc->last_push);
                //tc->last_push = motion_doc;
                tc->has_dilated = true;
                // it's slow, so prevent clogging up with events
                gobble_motion_events(GDK_BUTTON1_MASK);
                return TRUE;
            }

        }
        break;


    case GDK_BUTTON_RELEASE:
    {
        NR::Point const motion_w(event->button.x, event->button.y);
        NR::Point const motion_dt(desktop->w2d(motion_w));

        sp_canvas_end_forced_full_redraws(desktop->canvas);
        tc->is_drawing = false;

        if (tc->is_dilating && event->button.button == 1 && !event_context->space_panning) {
            if (!tc->has_dilated) {
                // if we did not rub, do a light tap
                tc->pressure = 0.03;
                sp_tweak_dilate (tc, motion_w, desktop->dt2doc(motion_dt), NR::Point(0,0));
            }
            tc->is_dilating = false;
            tc->has_dilated = false;
            switch (tc->mode) {
                case TWEAK_MODE_PUSH:
                    sp_document_done(sp_desktop_document(SP_EVENT_CONTEXT(tc)->desktop),
                                     SP_VERB_CONTEXT_TWEAK, _("Push tweak"));
                    break;
                case TWEAK_MODE_SHRINK:
                    sp_document_done(sp_desktop_document(SP_EVENT_CONTEXT(tc)->desktop),
                                     SP_VERB_CONTEXT_TWEAK, _("Shrink tweak"));
                    break;
                case TWEAK_MODE_GROW:
                    sp_document_done(sp_desktop_document(SP_EVENT_CONTEXT(tc)->desktop),
                                     SP_VERB_CONTEXT_TWEAK, _("Grow tweak"));
                    break;
                case TWEAK_MODE_ATTRACT:
                    sp_document_done(sp_desktop_document(SP_EVENT_CONTEXT(tc)->desktop),
                                     SP_VERB_CONTEXT_TWEAK, _("Attract tweak"));
                    break;
                case TWEAK_MODE_REPEL:
                    sp_document_done(sp_desktop_document(SP_EVENT_CONTEXT(tc)->desktop),
                                     SP_VERB_CONTEXT_TWEAK, _("Repel tweak"));
                    break;
                case TWEAK_MODE_ROUGHEN:
                    sp_document_done(sp_desktop_document(SP_EVENT_CONTEXT(tc)->desktop),
                                     SP_VERB_CONTEXT_TWEAK, _("Roughen tweak"));
                    break;
                case TWEAK_MODE_COLORPAINT:
                    sp_document_done(sp_desktop_document(SP_EVENT_CONTEXT(tc)->desktop),
                                     SP_VERB_CONTEXT_TWEAK, _("Color paint tweak"));
                case TWEAK_MODE_COLORJITTER:
                    sp_document_done(sp_desktop_document(SP_EVENT_CONTEXT(tc)->desktop),
                                     SP_VERB_CONTEXT_TWEAK, _("Color jitter tweak"));
                    break;
            }
        }
        break;
    }

    case GDK_KEY_PRESS:
        switch (get_group0_keyval (&event->key)) {
        case GDK_p:
        case GDK_P:
        case GDK_1:
            if (MOD__SHIFT_ONLY) {
                sp_tweak_switch_mode(tc, TWEAK_MODE_PUSH);
                ret = TRUE;
            }
            break;
        case GDK_s:
        case GDK_S:
        case GDK_2:
            if (MOD__SHIFT_ONLY) {
                sp_tweak_switch_mode(tc, TWEAK_MODE_SHRINK);
                ret = TRUE;
            }
            break;
        case GDK_g:
        case GDK_G:
        case GDK_3:
            if (MOD__SHIFT_ONLY) {
                sp_tweak_switch_mode(tc, TWEAK_MODE_GROW);
                ret = TRUE;
            }
            break;
        case GDK_a:
        case GDK_A:
        case GDK_4:
            if (MOD__SHIFT_ONLY) {
                sp_tweak_switch_mode(tc, TWEAK_MODE_ATTRACT);
                ret = TRUE;
            }
            break;
        case GDK_e:
        case GDK_E:
        case GDK_5:
            if (MOD__SHIFT_ONLY) {
                sp_tweak_switch_mode(tc, TWEAK_MODE_REPEL);
                ret = TRUE;
            }
            break;
        case GDK_r:
        case GDK_R:
        case GDK_6:
            if (MOD__SHIFT_ONLY) {
                sp_tweak_switch_mode(tc, TWEAK_MODE_ROUGHEN);
                ret = TRUE;
            }
            break;
        case GDK_c:
        case GDK_C:
        case GDK_7:
            if (MOD__SHIFT_ONLY) {
                sp_tweak_switch_mode(tc, TWEAK_MODE_COLORPAINT);
                ret = TRUE;
            }
            break;
        case GDK_j:
        case GDK_J:
        case GDK_8:
            if (MOD__SHIFT_ONLY) {
                sp_tweak_switch_mode(tc, TWEAK_MODE_COLORJITTER);
                ret = TRUE;
            }
            break;

        case GDK_Up:
        case GDK_KP_Up:
            if (!MOD__CTRL_ONLY) {
                tc->force += 0.05;
                if (tc->force > 1.0)
                    tc->force = 1.0;
                desktop->setToolboxAdjustmentValue ("tweak-force", tc->force * 100);
                ret = TRUE;
            }
            break;
        case GDK_Down:
        case GDK_KP_Down:
            if (!MOD__CTRL_ONLY) {
                tc->force -= 0.05;
                if (tc->force < 0.0)
                    tc->force = 0.0;
                desktop->setToolboxAdjustmentValue ("tweak-force", tc->force * 100);
                ret = TRUE;
            }
            break;
        case GDK_Right:
        case GDK_KP_Right:
            if (!MOD__CTRL_ONLY) {
                tc->width += 0.01;
                if (tc->width > 1.0)
                    tc->width = 1.0;
                desktop->setToolboxAdjustmentValue ("altx-tweak", tc->width * 100); // the same spinbutton is for alt+x
                sp_tweak_update_area(tc);
                ret = TRUE;
            }
            break;
        case GDK_Left:
        case GDK_KP_Left:
            if (!MOD__CTRL_ONLY) {
                tc->width -= 0.01;
                if (tc->width < 0.01)
                    tc->width = 0.01;
                desktop->setToolboxAdjustmentValue ("altx-tweak", tc->width * 100);
                sp_tweak_update_area(tc);
                ret = TRUE;
            }
            break;
        case GDK_Home:
        case GDK_KP_Home:
            tc->width = 0.01;
            desktop->setToolboxAdjustmentValue ("altx-tweak", tc->width * 100);
            sp_tweak_update_area(tc);
            ret = TRUE;
            break;
        case GDK_End:
        case GDK_KP_End:
            tc->width = 1.0;
            desktop->setToolboxAdjustmentValue ("altx-tweak", tc->width * 100);
            sp_tweak_update_area(tc);
            ret = TRUE;
            break;
        case GDK_x:
        case GDK_X:
            if (MOD__ALT_ONLY) {
                desktop->setToolboxFocusTo ("altx-tweak");
                ret = TRUE;
            }
            break;

        case GDK_Control_L:
        case GDK_Control_R:
            if (MOD__SHIFT) {
                sp_tweak_switch_mode_temporarily(tc, TWEAK_MODE_GROW);
            } else {
                sp_tweak_switch_mode_temporarily(tc, TWEAK_MODE_SHRINK);
            }
            break;
        case GDK_Shift_L:
        case GDK_Shift_R:
            if (MOD__CTRL) {
                sp_tweak_switch_mode_temporarily(tc, TWEAK_MODE_GROW);
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
                sp_tweak_switch_mode (tc, prefs_get_int_attribute("tools.tweak", "mode", 0));
                tc->_message_context->clear();
                break;
            case GDK_Shift_L:
            case GDK_Shift_R:
                if (MOD__CTRL) {
                    sp_tweak_switch_mode_temporarily(tc, TWEAK_MODE_SHRINK);
                }
                break;
            break;
            default:
                sp_tweak_switch_mode (tc, prefs_get_int_attribute("tools.tweak", "mode", 0));
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

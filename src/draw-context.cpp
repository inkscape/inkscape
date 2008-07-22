#define __SP_DRAW_CONTEXT_C__

/*
 * Generic drawing context
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define DRAW_VERBOSE

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <gdk/gdkkeysyms.h>

#include "display/canvas-bpath.h"
#include "xml/repr.h"
#include "svg/svg.h"
#include <glibmm/i18n.h>
#include "libnr/n-art-bpath.h"
#include "display/curve.h"
#include "desktop.h"
#include "desktop-affine.h"
#include "desktop-handles.h"
#include "desktop-style.h"
#include "document.h"
#include "draw-anchor.h"
#include "macros.h"
#include "message-stack.h"
#include "pen-context.h"
#include "prefs-utils.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "snap.h"
#include "sp-path.h"
#include "sp-namedview.h"
#include "live_effects/lpe-patternalongpath.h"
#include "style.h"

static void sp_draw_context_class_init(SPDrawContextClass *klass);
static void sp_draw_context_init(SPDrawContext *dc);
static void sp_draw_context_dispose(GObject *object);

static void sp_draw_context_setup(SPEventContext *ec);
static void sp_draw_context_set(SPEventContext *ec, gchar const *key, gchar const *value);
static void sp_draw_context_finish(SPEventContext *ec);

static gint sp_draw_context_root_handler(SPEventContext *event_context, GdkEvent *event);

static void spdc_selection_changed(Inkscape::Selection *sel, SPDrawContext *dc);
static void spdc_selection_modified(Inkscape::Selection *sel, guint flags, SPDrawContext *dc);

static void spdc_attach_selection(SPDrawContext *dc, Inkscape::Selection *sel);

static void spdc_flush_white(SPDrawContext *dc, SPCurve *gc);

static void spdc_reset_white(SPDrawContext *dc);
static void spdc_free_colors(SPDrawContext *dc);


static SPEventContextClass *draw_parent_class;


GType
sp_draw_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPDrawContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_draw_context_class_init,
            NULL, NULL,
            sizeof(SPDrawContext),
            4,
            (GInstanceInitFunc) sp_draw_context_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPDrawContext", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_draw_context_class_init(SPDrawContextClass *klass)
{
    GObjectClass *object_class;
    SPEventContextClass *ec_class;

    object_class = (GObjectClass *)klass;
    ec_class = (SPEventContextClass *) klass;

    draw_parent_class = (SPEventContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_draw_context_dispose;

    ec_class->setup = sp_draw_context_setup;
    ec_class->set = sp_draw_context_set;
    ec_class->finish = sp_draw_context_finish;
    ec_class->root_handler = sp_draw_context_root_handler;
}

static void
sp_draw_context_init(SPDrawContext *dc)
{
    dc->attach = FALSE;

    dc->red_color = 0xff00007f;
    dc->blue_color = 0x0000ff7f;
    dc->green_color = 0x00ff007f;
    dc->red_curve_is_valid = false;

    dc->waiting_LPE_type = Inkscape::LivePathEffect::INVALID_LPE;

    new (&dc->sel_changed_connection) sigc::connection();
    new (&dc->sel_modified_connection) sigc::connection();
}

static void
sp_draw_context_dispose(GObject *object)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(object);

    dc->sel_changed_connection.~connection();
    dc->sel_modified_connection.~connection();

    if (dc->grab) {
        sp_canvas_item_ungrab(dc->grab, GDK_CURRENT_TIME);
        dc->grab = NULL;
    }

    if (dc->selection) {
        dc->selection = NULL;
    }

    dc->waiting_LPE_type = Inkscape::LivePathEffect::INVALID_LPE;

    spdc_free_colors(dc);

    G_OBJECT_CLASS(draw_parent_class)->dispose(object);
}

static void
sp_draw_context_setup(SPEventContext *ec)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(ec);
    SPDesktop *dt = ec->desktop;

    if (((SPEventContextClass *) draw_parent_class)->setup) {
        ((SPEventContextClass *) draw_parent_class)->setup(ec);
    }

    dc->selection = sp_desktop_selection(dt);

    /* Connect signals to track selection changes */
    dc->sel_changed_connection = dc->selection->connectChanged(
        sigc::bind(sigc::ptr_fun(&spdc_selection_changed), dc)
    );
    dc->sel_modified_connection = dc->selection->connectModified(
        sigc::bind(sigc::ptr_fun(&spdc_selection_modified), dc)
    );

    /* Create red bpath */
    dc->red_bpath = sp_canvas_bpath_new(sp_desktop_sketch(ec->desktop), NULL);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(dc->red_bpath), dc->red_color, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
    /* Create red curve */
    dc->red_curve = new SPCurve();

    /* Create blue bpath */
    dc->blue_bpath = sp_canvas_bpath_new(sp_desktop_sketch(ec->desktop), NULL);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(dc->blue_bpath), dc->blue_color, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
    /* Create blue curve */
    dc->blue_curve = new SPCurve();

    /* Create green curve */
    dc->green_curve = new SPCurve();
    /* No green anchor by default */
    dc->green_anchor = NULL;
    dc->green_closed = FALSE;

    dc->attach = TRUE;
    spdc_attach_selection(dc, dc->selection);
}

static void
sp_draw_context_finish(SPEventContext *ec)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(ec);

    dc->sel_changed_connection.disconnect();
    dc->sel_modified_connection.disconnect();

    if (dc->grab) {
        sp_canvas_item_ungrab(dc->grab, GDK_CURRENT_TIME);
    }

    if (dc->selection) {
        dc->selection = NULL;
    }

    spdc_free_colors(dc);
}

static void
sp_draw_context_set(SPEventContext */*ec*/, const gchar */*key*/, const gchar */*value*/)
{
}

gint
sp_draw_context_root_handler(SPEventContext *ec, GdkEvent *event)
{
    gint ret = FALSE;

    switch (event->type) {
        case GDK_KEY_PRESS:
            switch (get_group0_keyval (&event->key)) {
                case GDK_Up:
                case GDK_Down:
                case GDK_KP_Up:
                case GDK_KP_Down:
                    // prevent the zoom field from activation
                    if (!MOD__CTRL_ONLY) {
                        ret = TRUE;
                    }
                    break;
                default:
            break;
        }
        break;
    default:
        break;
    }

    if (!ret) {
        if (((SPEventContextClass *) draw_parent_class)->root_handler) {
            ret = ((SPEventContextClass *) draw_parent_class)->root_handler(ec, event);
        }
    }

    return ret;
}

static char const *
tool_name(SPDrawContext *dc)
{
    return ( SP_IS_PEN_CONTEXT(dc)
             ? "tools.freehand.pen"
             : "tools.freehand.pencil" );
}

static void
spdc_paste_curve_as_freehand_shape(const SPCurve *c, SPDrawContext *dc, SPItem *item)
{
    using namespace Inkscape::LivePathEffect;

    // TODO: Don't paste path if nothing is on the clipboard

    Effect::createAndApply(Inkscape::LivePathEffect::FREEHAND_SHAPE, dc->desktop->doc(), item);
    Effect* lpe = sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item));
    gchar *svgd = sp_svg_write_path(c->get_pathvector());
    static_cast<LPEPatternAlongPath*>(lpe)->pattern.paste_param_path(svgd);
}

/*
 * If we have an item and a waiting LPE, apply the effect to the item
 * (spiro spline mode is treated separately)
 */
void
spdc_check_for_and_apply_waiting_LPE(SPDrawContext *dc, SPItem *item)
{
    using namespace Inkscape::LivePathEffect;

    if (item && SP_IS_LPE_ITEM(item)) {
        if (prefs_get_int_attribute(tool_name(dc), "spiro-spline-mode", 0)) {
            Effect::createAndApply(SPIRO, dc->desktop->doc(), item);
        }

        int shape = prefs_get_int_attribute(tool_name(dc), "shape", 0);
        bool shape_applied = false;
        SPCSSAttr *css_item = sp_css_attr_from_object (SP_OBJECT(item), SP_STYLE_FLAG_ALWAYS);
        const char *cstroke = sp_repr_css_property(css_item, "stroke", "none");

        switch (shape) {
            case 0:
                // don't apply any shape
                break;
            case 1:
            {
                // take shape from clipboard; TODO: catch the case where clipboard is empty
                Effect::createAndApply(FREEHAND_SHAPE, dc->desktop->doc(), item);
                Effect* lpe = sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item));
                static_cast<LPEPatternAlongPath*>(lpe)->pattern.on_paste_button_click();

                shape_applied = true;
                break;
            }
            case 2:
            {
                // "crescendo"
                // TODO: this is only for illustration (we create a "crescendo"-shaped path
                //       manually; eventually we should read the path from a separate file)
                SPCurve *c = new SPCurve();
                c->moveto(0,5);
                c->lineto(200,10);
                c->lineto(200,0);
                c->closepath();
                spdc_paste_curve_as_freehand_shape(c, dc, item);
                c->unref();

                shape_applied = true;
                break;
            }
            case 3:
            {
                // "decrescendo"
                // TODO: this is only for illustration (we create a "decrescendo"-shaped path
                //       manually; eventually we should read the path from a separate file)
                SPCurve *c = new SPCurve();
                c->moveto(0,0);
                c->lineto(0,10);
                c->lineto(200,5);
                c->closepath();
                spdc_paste_curve_as_freehand_shape(c, dc, item);
                c->unref();

                shape_applied = true;
                break;
            }
            default:
                break;
        }
        if (shape_applied) {
            // apply original stroke color as fill and unset stroke; then return
            SPCSSAttr *css = sp_repr_css_attr_new();
            sp_repr_css_set_property (css, "fill", cstroke);
            sp_repr_css_set_property (css, "stroke", "none");
            sp_desktop_apply_css_recursive(SP_OBJECT(item), css, true);
            sp_repr_css_attr_unref(css);
            return;
        }

        if (dc->waiting_LPE_type != INVALID_LPE) {
            Effect::createAndApply(dc->waiting_LPE_type, dc->desktop->doc(), item);
            dc->waiting_LPE_type = INVALID_LPE;
        }
        if (SP_IS_PEN_CONTEXT(dc)) {
            SP_PEN_CONTEXT(dc)->polylines_only = false;
        }
    }
}

/*
 * Selection handlers
 */

static void
spdc_selection_changed(Inkscape::Selection *sel, SPDrawContext *dc)
{
    if (dc->attach) {
        spdc_attach_selection(dc, sel);
    }
}

/* fixme: We have to ensure this is not delayed (Lauris) */

static void
spdc_selection_modified(Inkscape::Selection *sel, guint /*flags*/, SPDrawContext *dc)
{
    if (dc->attach) {
        spdc_attach_selection(dc, sel);
    }
}

static void
spdc_attach_selection(SPDrawContext *dc, Inkscape::Selection */*sel*/)
{
    /* We reset white and forget white/start/end anchors */
    spdc_reset_white(dc);
    dc->sa = NULL;
    dc->ea = NULL;

    SPItem *item = dc->selection ? dc->selection->singleItem() : NULL;

    if ( item && SP_IS_PATH(item) ) {
        /* Create new white data */
        /* Item */
        dc->white_item = item;
        /* Curve list */
        /* We keep it in desktop coordinates to eliminate calculation errors */
        SPCurve *norm = sp_path_get_curve_for_edit (SP_PATH(item));
        norm->transform(sp_item_i2d_affine(dc->white_item));
        g_return_if_fail( norm != NULL );
        dc->white_curves = g_slist_reverse(norm->split());
        norm->unref();
        /* Anchor list */
        for (GSList *l = dc->white_curves; l != NULL; l = l->next) {
            SPCurve *c;
            c = (SPCurve*)l->data;
            g_return_if_fail( c->get_segment_count() > 0 );
            if ( !c->is_closed() ) {
                SPDrawAnchor *a;
                a = sp_draw_anchor_new(dc, c, TRUE, c->first_point());
                dc->white_anchors = g_slist_prepend(dc->white_anchors, a);
                a = sp_draw_anchor_new(dc, c, FALSE, c->last_point());
                dc->white_anchors = g_slist_prepend(dc->white_anchors, a);
            }
        }
        /* fixme: recalculate active anchor? */
    }
}


/**
 *  Snaps node or handle to PI/rotationsnapsperpi degree increments.
 *
 *  \param dc draw context
 *  \param p cursor point (to be changed by snapping)
 *  \param o origin point
 *  \param state  keyboard state to check if ctrl was pressed
*/

void spdc_endpoint_snap_rotation(SPEventContext const *const ec, NR::Point &p, NR::Point const o,
                                 guint state)
{
    /* Control must be down for this snap to work */
    if ((state & GDK_CONTROL_MASK) == 0) {
        return;
    }

    unsigned const snaps = abs(prefs_get_int_attribute("options.rotationsnapsperpi", "value", 12));
    /* 0 means no snapping. */

    /* mirrored by fabs, so this corresponds to 15 degrees */
    NR::Point best; /* best solution */
    double bn = NR_HUGE; /* best normal */
    double bdot = 0;
    NR::Point v = NR::Point(0, 1);
    double const r00 = cos(M_PI / snaps), r01 = sin(M_PI / snaps);
    double const r10 = -r01, r11 = r00;

    NR::Point delta = p - o;

    for (unsigned i = 0; i < snaps; i++) {
        double const ndot = fabs(dot(v,NR::rot90(delta)));
        NR::Point t(r00*v[NR::X] + r01*v[NR::Y],
                    r10*v[NR::X] + r11*v[NR::Y]);
        if (ndot < bn) {
            /* I think it is better numerically to use the normal, rather than the dot product
             * to assess solutions, but I haven't proven it. */
            bn = ndot;
            best = v;
            bdot = dot(v, delta);
        }
        v = t;
    }

    if (fabs(bdot) > 0) {
        p = o + bdot * best;

        /* Snap it along best vector */
        SnapManager &m = SP_EVENT_CONTEXT_DESKTOP(ec)->namedview->snap_manager;
        m.setup(SP_EVENT_CONTEXT_DESKTOP(ec), NULL);
        m.constrainedSnapReturnByRef( Inkscape::Snapper::SNAPPOINT_NODE, p, Inkscape::Snapper::ConstraintLine(best));
    }
}


void spdc_endpoint_snap_free(SPEventContext const * const ec, NR::Point& p, guint const state)
{
    /* Shift disables this snap */
    if (state & GDK_SHIFT_MASK) {
        return;
    }

    SnapManager &m = SP_EVENT_CONTEXT_DESKTOP(ec)->namedview->snap_manager;
    m.setup(SP_EVENT_CONTEXT_DESKTOP(ec), NULL);
    m.freeSnapReturnByRef(Inkscape::Snapper::SNAPPOINT_NODE, p);
}

static SPCurve *
reverse_then_unref(SPCurve *orig)
{
    SPCurve *ret = orig->create_reverse();
    orig->unref();
    return ret;
}

/**
 * Concats red, blue and green.
 * If any anchors are defined, process these, optionally removing curves from white list
 * Invoke _flush_white to write result back to object.
 */
void
spdc_concat_colors_and_flush(SPDrawContext *dc, gboolean forceclosed)
{
    /* Concat RBG */
    SPCurve *c = dc->green_curve;

    /* Green */
    dc->green_curve = new SPCurve();
    while (dc->green_bpaths) {
        gtk_object_destroy(GTK_OBJECT(dc->green_bpaths->data));
        dc->green_bpaths = g_slist_remove(dc->green_bpaths, dc->green_bpaths->data);
    }
    /* Blue */
    c->append_continuous(dc->blue_curve, 0.0625);
    dc->blue_curve->reset();
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->blue_bpath), NULL);
    /* Red */
    if (dc->red_curve_is_valid) {
        c->append_continuous(dc->red_curve, 0.0625);
    }
    dc->red_curve->reset();
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->red_bpath), NULL);

    if (c->is_empty()) {
        c->unref();
        return;
    }

    /* Step A - test, whether we ended on green anchor */
    if ( forceclosed || ( dc->green_anchor && dc->green_anchor->active ) ) {
        // We hit green anchor, closing Green-Blue-Red
        SP_EVENT_CONTEXT_DESKTOP(dc)->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Path is closed."));
        c->closepath_current();
        /* Closed path, just flush */
        spdc_flush_white(dc, c);
        c->unref();
        return;
    }

    /* Step B - both start and end anchored to same curve */
    if ( dc->sa && dc->ea
         && ( dc->sa->curve == dc->ea->curve )
         && ( ( dc->sa != dc->ea )
              || dc->sa->curve->is_closed() ) )
    {
        // We hit bot start and end of single curve, closing paths
        SP_EVENT_CONTEXT_DESKTOP(dc)->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Closing path."));
        if (dc->sa->start && !(dc->sa->curve->is_closed()) ) {
            c = reverse_then_unref(c);
        }
        dc->sa->curve->append_continuous(c, 0.0625);
        c->unref();
        dc->sa->curve->closepath_current();
        spdc_flush_white(dc, NULL);
        return;
    }

    /* Step C - test start */
    if (dc->sa) {
        SPCurve *s = dc->sa->curve;
        dc->white_curves = g_slist_remove(dc->white_curves, s);
        if (dc->sa->start) {
            s = reverse_then_unref(s);
        }
        s->append_continuous(c, 0.0625);
        c->unref();
        c = s;
    } else /* Step D - test end */ if (dc->ea) {
        SPCurve *e = dc->ea->curve;
        dc->white_curves = g_slist_remove(dc->white_curves, e);
        if (!dc->ea->start) {
            e = reverse_then_unref(e);
        }
        c->append_continuous(e, 0.0625);
        e->unref();
    }


    spdc_flush_white(dc, c);

    c->unref();
}

/*
 * Flushes white curve(s) and additional curve into object
 *
 * No cleaning of colored curves - this has to be done by caller
 * No rereading of white data, so if you cannot rely on ::modified, do it in caller
 *
 */

static void
spdc_flush_white(SPDrawContext *dc, SPCurve *gc)
{
    SPCurve *c;

    if (dc->white_curves) {
        g_assert(dc->white_item);
        c = SPCurve::concat(dc->white_curves);
        g_slist_free(dc->white_curves);
        dc->white_curves = NULL;
        if (gc) {
            c->append(gc, FALSE);
        }
    } else if (gc) {
        c = gc;
        c->ref();
    } else {
        return;
    }

    /* Now we have to go back to item coordinates at last */
    c->transform(( dc->white_item
                            ? sp_item_dt2i_affine(dc->white_item)
                            : to_2geom(sp_desktop_dt2root_affine(SP_EVENT_CONTEXT_DESKTOP(dc))) ));

    SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(dc);
    SPDocument *doc = sp_desktop_document(desktop);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

    if ( c && !c->is_empty() ) {
        /* We actually have something to write */

        bool has_lpe = false;
        Inkscape::XML::Node *repr;
        if (dc->white_item) {
            repr = SP_OBJECT_REPR(dc->white_item);
            has_lpe = sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(dc->white_item));
        } else {
            repr = xml_doc->createElement("svg:path");
            /* Set style */
            sp_desktop_apply_style_tool(desktop, repr, tool_name(dc), false);
        }

        gchar *str = sp_svg_write_path( c->get_pathvector() );
        g_assert( str != NULL );
        if (has_lpe)
            repr->setAttribute("inkscape:original-d", str);
        else
            repr->setAttribute("d", str);
        g_free(str);

        if (!dc->white_item) {
            /* Attach repr */
            SPItem *item = SP_ITEM(desktop->currentLayer()->appendChildRepr(repr));
            dc->selection->set(repr);
            Inkscape::GC::release(repr);
            item->transform = from_2geom(i2i_affine(desktop->currentRoot(), desktop->currentLayer()));
            item->updateRepr();
        }


        // we finished the path; now apply any waiting LPEs or freehand shapes
        // FIXME: placing this here seems to cause issues with undo!
        spdc_check_for_and_apply_waiting_LPE(dc, dc->selection->singleItem());

        sp_document_done(doc, SP_IS_PEN_CONTEXT(dc)? SP_VERB_CONTEXT_PEN : SP_VERB_CONTEXT_PENCIL, 
                         _("Draw path"));

        // When quickly drawing several subpaths with Shift, the next subpath may be finished and
        // flushed before the selection_modified signal is fired by the previous change, which
        // results in the tool losing all of the selected path's curve except that last subpath. To
        // fix this, we force the selection_modified callback now, to make sure the tool's curve is
        // in sync immediately.
        spdc_selection_modified(sp_desktop_selection(desktop), 0, dc);
    }

    c->unref();

    /* Flush pending updates */
    sp_document_ensure_up_to_date(doc);
}

/**
 * Returns FIRST active anchor (the activated one).
 */
SPDrawAnchor *
spdc_test_inside(SPDrawContext *dc, NR::Point p)
{
    SPDrawAnchor *active = NULL;

    /* Test green anchor */
    if (dc->green_anchor) {
        active = sp_draw_anchor_test(dc->green_anchor, p, TRUE);
    }

    for (GSList *l = dc->white_anchors; l != NULL; l = l->next) {
        SPDrawAnchor *na = sp_draw_anchor_test((SPDrawAnchor *) l->data, p, !active);
        if ( !active && na ) {
            active = na;
        }
    }

    return active;
}

static void
spdc_reset_white(SPDrawContext *dc)
{
    if (dc->white_item) {
        /* We do not hold refcount */
        dc->white_item = NULL;
    }
    while (dc->white_curves) {
        reinterpret_cast<SPCurve *>(dc->white_curves->data)->unref();
        dc->white_curves = g_slist_remove(dc->white_curves, dc->white_curves->data);
    }
    while (dc->white_anchors) {
        sp_draw_anchor_destroy((SPDrawAnchor *) dc->white_anchors->data);
        dc->white_anchors = g_slist_remove(dc->white_anchors, dc->white_anchors->data);
    }
}

static void
spdc_free_colors(SPDrawContext *dc)
{
    /* Red */
    if (dc->red_bpath) {
        gtk_object_destroy(GTK_OBJECT(dc->red_bpath));
        dc->red_bpath = NULL;
    }
    if (dc->red_curve) {
        dc->red_curve = dc->red_curve->unref();
    }
    /* Blue */
    if (dc->blue_bpath) {
        gtk_object_destroy(GTK_OBJECT(dc->blue_bpath));
        dc->blue_bpath = NULL;
    }
    if (dc->blue_curve) {
        dc->blue_curve = dc->blue_curve->unref();
    }
    /* Green */
    while (dc->green_bpaths) {
        gtk_object_destroy(GTK_OBJECT(dc->green_bpaths->data));
        dc->green_bpaths = g_slist_remove(dc->green_bpaths, dc->green_bpaths->data);
    }
    if (dc->green_curve) {
        dc->green_curve = dc->green_curve->unref();
    }
    if (dc->green_anchor) {
        dc->green_anchor = sp_draw_anchor_destroy(dc->green_anchor);
    }
    /* White */
    if (dc->white_item) {
        /* We do not hold refcount */
        dc->white_item = NULL;
    }
    while (dc->white_curves) {
        reinterpret_cast<SPCurve *>(dc->white_curves->data)->unref();
        dc->white_curves = g_slist_remove(dc->white_curves, dc->white_curves->data);
    }
    while (dc->white_anchors) {
        sp_draw_anchor_destroy((SPDrawAnchor *) dc->white_anchors->data);
        dc->white_anchors = g_slist_remove(dc->white_anchors, dc->white_anchors->data);
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

/*
 * Generic drawing context
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 * Copyright (C) 2012 Johan Engelen
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define DRAW_VERBOSE

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "live_effects/lpe-patternalongpath.h"
#include "display/canvas-bpath.h"
#include "xml/repr.h"
#include "svg/svg.h"
#include <glibmm/i18n.h>
#include "display/curve.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-style.h"
#include "document.h"
#include "ui/draw-anchor.h"
#include "macros.h"
#include "message-stack.h"
#include "ui/tools/pen-tool.h"
#include "ui/tools/lpe-tool.h"
#include "preferences.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "snap.h"
#include "sp-path.h"
#include "sp-namedview.h"
#include "live_effects/lpe-powerstroke.h"
#include "style.h"
#include "ui/control-manager.h"
// clipboard support
#include "ui/clipboard.h"
#include "ui/tools/freehand-base.h"

#include <gdk/gdkkeysyms.h>

using Inkscape::DocumentUndo;

namespace Inkscape {
namespace UI {
namespace Tools {

static void spdc_selection_changed(Inkscape::Selection *sel, FreehandBase *dc);
static void spdc_selection_modified(Inkscape::Selection *sel, guint flags, FreehandBase *dc);

static void spdc_attach_selection(FreehandBase *dc, Inkscape::Selection *sel);

/**
 * Flushes white curve(s) and additional curve into object.
 *
 * No cleaning of colored curves - this has to be done by caller
 * No rereading of white data, so if you cannot rely on ::modified, do it in caller
 */
static void spdc_flush_white(FreehandBase *dc, SPCurve *gc);

static void spdc_reset_white(FreehandBase *dc);
static void spdc_free_colors(FreehandBase *dc);

FreehandBase::FreehandBase(gchar const *const *cursor_shape, gint hot_x, gint hot_y)
    : ToolBase(cursor_shape, hot_x, hot_y)
    , selection(NULL)
    , grab(NULL)
    , attach(false)
    , red_color(0xff00007f)
    , blue_color(0x0000ff7f)
    , green_color(0x00ff007f)
    , highlight_color(0x0000007f)
    , red_bpath(NULL)
    , red_curve(NULL)
    , blue_bpath(NULL)
    , blue_curve(NULL)
    , green_bpaths(NULL)
    , green_curve(NULL)
    , green_anchor(NULL)
    , green_closed(false)
    , white_item(NULL)
    , white_curves(NULL)
    , white_anchors(NULL)
    , overwriteCurve(NULL)
    , sa(NULL)
    , ea(NULL)
    , waiting_LPE_type(Inkscape::LivePathEffect::INVALID_LPE)
    , red_curve_is_valid(false)
    , anchor_statusbar(false)
{
}

FreehandBase::~FreehandBase() {
    if (this->grab) {
        sp_canvas_item_ungrab(this->grab, GDK_CURRENT_TIME);
        this->grab = NULL;
    }

    if (this->selection) {
        this->selection = NULL;
    }

    spdc_free_colors(this);
}

void FreehandBase::setup() {
    ToolBase::setup();

    this->selection = desktop->getSelection();

    // Connect signals to track selection changes
    this->sel_changed_connection = this->selection->connectChanged(
        sigc::bind(sigc::ptr_fun(&spdc_selection_changed), this)
    );
    this->sel_modified_connection = this->selection->connectModified(
        sigc::bind(sigc::ptr_fun(&spdc_selection_modified), this)
    );

    // Create red bpath
    this->red_bpath = sp_canvas_bpath_new(this->desktop->getSketch(), NULL);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(this->red_bpath), this->red_color, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);

    // Create red curve
    this->red_curve = new SPCurve();

    // Create blue bpath
    this->blue_bpath = sp_canvas_bpath_new(this->desktop->getSketch(), NULL);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(this->blue_bpath), this->blue_color, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);

    // Create blue curve
    this->blue_curve = new SPCurve();

    // Create green curve
    this->green_curve = new SPCurve();

    // No green anchor by default
    this->green_anchor = NULL;
    this->green_closed = FALSE;

    // Create start anchor alternative curve
    this->overwriteCurve = new SPCurve();

    this->attach = TRUE;
    spdc_attach_selection(this, this->selection);
}

void FreehandBase::finish() {
    this->sel_changed_connection.disconnect();
    this->sel_modified_connection.disconnect();

    if (this->grab) {
        sp_canvas_item_ungrab(this->grab, GDK_CURRENT_TIME);
    }

    if (this->selection) {
        this->selection = NULL;
    }

    spdc_free_colors(this);

    ToolBase::finish();
}

void FreehandBase::set(const Inkscape::Preferences::Entry& /*value*/) {
}

bool FreehandBase::root_handler(GdkEvent* event) {
    gint ret = FALSE;

    switch (event->type) {
        case GDK_KEY_PRESS:
            switch (get_group0_keyval (&event->key)) {
                case GDK_KEY_Up:
                case GDK_KEY_Down:
                case GDK_KEY_KP_Up:
                case GDK_KEY_KP_Down:
                    // prevent the zoom field from activation
                    if (!MOD__CTRL_ONLY(event)) {
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
    	ret = ToolBase::root_handler(event);
    }

    return ret;
}

static Glib::ustring const tool_name(FreehandBase *dc)
{
    return ( SP_IS_PEN_CONTEXT(dc)
             ? "/tools/freehand/pen"
             : "/tools/freehand/pencil" );
}

static void spdc_paste_curve_as_freehand_shape(const SPCurve *c, FreehandBase *dc, SPItem *item)
{
    using namespace Inkscape::LivePathEffect;

    // TODO: Don't paste path if nothing is on the clipboard

    Effect::createAndApply(PATTERN_ALONG_PATH, dc->desktop->doc(), item);
    Effect* lpe = SP_LPE_ITEM(item)->getCurrentLPE();
    gchar *svgd = sp_svg_write_path(c->get_pathvector());
    static_cast<LPEPatternAlongPath*>(lpe)->pattern.paste_param_path(svgd);
}

static void spdc_apply_powerstroke_shape(const std::vector<Geom::Point> & points, FreehandBase *dc, SPItem *item)
{
    using namespace Inkscape::LivePathEffect;

    Effect::createAndApply(POWERSTROKE, dc->desktop->doc(), item);
    Effect* lpe = SP_LPE_ITEM(item)->getCurrentLPE();
    static_cast<LPEPowerStroke*>(lpe)->offset_points.param_set_and_write_new_value(points);

    // find out stroke width (TODO: is there an easier way??)
    SPDesktop *desktop = dc->desktop;
    Inkscape::XML::Document *xml_doc = desktop->doc()->getReprDoc();
    Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");
    Inkscape::GC::release(repr);

    char const* tool = SP_IS_PEN_CONTEXT(dc) ? "/tools/freehand/pen" : "/tools/freehand/pencil";

    // apply the tool's current style
    sp_desktop_apply_style_tool(desktop, repr, tool, false);

    double stroke_width = 1.0;
    char const *style_str = NULL;
    style_str = repr->attribute("style");
    if (style_str) {
        SPStyle *style = sp_style_new(SP_ACTIVE_DOCUMENT);
        sp_style_merge_from_style_string(style, style_str);
        stroke_width = style->stroke_width.computed;
        style->stroke_width.computed = 0;
        sp_style_unref(style);
    }

    std::ostringstream s;
    s.imbue(std::locale::classic());
    s << points[0][Geom::X] << "," << stroke_width / 2.;

    // write powerstroke parameters:
    lpe->getRepr()->setAttribute("start_linecap_type", "zerowidth");
    lpe->getRepr()->setAttribute("end_linecap_type", "zerowidth");
    lpe->getRepr()->setAttribute("cusp_linecap_type", "round");
    lpe->getRepr()->setAttribute("sort_points", "true");
    lpe->getRepr()->setAttribute("interpolator_type", "CubicBezierJohan");
    lpe->getRepr()->setAttribute("interpolator_beta", "0.2");
    lpe->getRepr()->setAttribute("offset_points", s.str().c_str());
}

static void spdc_check_for_and_apply_waiting_LPE(FreehandBase *dc, SPItem *item, SPCurve *curve)
{
    using namespace Inkscape::LivePathEffect;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (item && SP_IS_LPE_ITEM(item)) {
        if (prefs->getInt(tool_name(dc) + "/freehand-mode", 0) == 1) {
            Effect::createAndApply(SPIRO, dc->desktop->doc(), item);
        }
        //add the bspline node in the waiting effects
        if (prefs->getInt(tool_name(dc) + "/freehand-mode", 0) == 2) {
            Effect::createAndApply(BSPLINE, dc->desktop->doc(), item);
        }

        //Store the clipboard path to apply in the future without the use of clipboard
        static Geom::PathVector previous_shape_pathv;
        enum shapeType { NONE, TRIANGLE_IN, TRIANGLE_OUT, ELLIPSE, CLIPBOARD, LAST_APPLIED };
        static shapeType previous_shape_type = NONE;


        shapeType shape = (shapeType)prefs->getInt(tool_name(dc) + "/shape", 0);
        bool shape_applied = false;
        SPCSSAttr *css_item = sp_css_attr_from_object(item, SP_STYLE_FLAG_ALWAYS);
        const char *cstroke = sp_repr_css_property(css_item, "stroke", "none");

#define SHAPE_LENGTH 10
#define SHAPE_HEIGHT 10

        if(shape == LAST_APPLIED){
            shape = previous_shape_type;
            if(shape == CLIPBOARD){
                shape = LAST_APPLIED;
            }
        }

        switch (shape) {
            case NONE:
                // don't apply any shape
                break;
            case TRIANGLE_IN:
            {
                // "triangle in"
                std::vector<Geom::Point> points(1);
                points[0] = Geom::Point(0., SHAPE_HEIGHT/2);
                spdc_apply_powerstroke_shape(points, dc, item);

                shape_applied = true;
                break;
            }
            case TRIANGLE_OUT:
            {
                // "triangle out"
                guint curve_length = curve->get_segment_count();
                std::vector<Geom::Point> points(1);
                points[0] = Geom::Point((double)curve_length, SHAPE_HEIGHT/2);
                spdc_apply_powerstroke_shape(points, dc, item);

                shape_applied = true;
                break;
            }
            case ELLIPSE:
            {
                // "ellipse"
                SPCurve *c = new SPCurve();
                const double C1 = 0.552;
                c->moveto(0, SHAPE_HEIGHT/2);
                c->curveto(0, (1 - C1) * SHAPE_HEIGHT/2, (1 - C1) * SHAPE_LENGTH/2, 0, SHAPE_LENGTH/2, 0);
                c->curveto((1 + C1) * SHAPE_LENGTH/2, 0, SHAPE_LENGTH, (1 - C1) * SHAPE_HEIGHT/2, SHAPE_LENGTH, SHAPE_HEIGHT/2);
                c->curveto(SHAPE_LENGTH, (1 + C1) * SHAPE_HEIGHT/2, (1 + C1) * SHAPE_LENGTH/2, SHAPE_HEIGHT, SHAPE_LENGTH/2, SHAPE_HEIGHT);
                c->curveto((1 - C1) * SHAPE_LENGTH/2, SHAPE_HEIGHT, 0, (1 + C1) * SHAPE_HEIGHT/2, 0, SHAPE_HEIGHT/2);
                c->closepath();
                spdc_paste_curve_as_freehand_shape(c, dc, item);
                c->unref();

                shape_applied = true;
                break;
            }
            case CLIPBOARD:
            {
                // take shape from clipboard; TODO: catch the case where clipboard is empty
                Effect::createAndApply(PATTERN_ALONG_PATH, dc->desktop->doc(), item);
                Effect* lpe = SP_LPE_ITEM(item)->getCurrentLPE();
                static_cast<LPEPatternAlongPath*>(lpe)->pattern.on_paste_button_click();
                Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
                Glib::ustring svgd = cm->getPathParameter(SP_ACTIVE_DESKTOP);
                previous_shape_pathv = sp_svg_read_pathv(svgd.data());

                shape_applied = true;
                break;
            }
            case LAST_APPLIED:
            {
                if(previous_shape_pathv.size() != 0){
                    SPCurve * c = new SPCurve();
                    c->set_pathvector(previous_shape_pathv);
                    spdc_paste_curve_as_freehand_shape(c, dc, item);
                    c->unref();

                    shape_applied = true;
                }
                break;
            }
            default:
                break;
        }
        previous_shape_type = shape;

        if (shape_applied) {
            // apply original stroke color as fill and unset stroke; then return
            SPCSSAttr *css = sp_repr_css_attr_new();

            if (!strcmp(cstroke, "none")){
                sp_repr_css_set_property (css, "fill", "black");
            } else {
                sp_repr_css_set_property (css, "fill", cstroke);
            }
            sp_repr_css_set_property (css, "stroke", "none");
            sp_desktop_apply_css_recursive(item, css, true);
            sp_repr_css_attr_unref(css);
            return;
        }

        if (dc->waiting_LPE_type != INVALID_LPE) {
            Effect::createAndApply(dc->waiting_LPE_type, dc->desktop->doc(), item);
            dc->waiting_LPE_type = INVALID_LPE;

            if (SP_IS_LPETOOL_CONTEXT(dc)) {
                // since a geometric LPE was applied, we switch back to "inactive" mode
                lpetool_context_switch_mode(SP_LPETOOL_CONTEXT(dc), INVALID_LPE);
            }
        }
        if (SP_IS_PEN_CONTEXT(dc)) {
            SP_PEN_CONTEXT(dc)->setPolylineMode();
        }
    }
}

/*
 * Selection handlers
 */

static void spdc_selection_changed(Inkscape::Selection *sel, FreehandBase *dc)
{
    if (dc->attach) {
        spdc_attach_selection(dc, sel);
    }
}

/* fixme: We have to ensure this is not delayed (Lauris) */

static void spdc_selection_modified(Inkscape::Selection *sel, guint /*flags*/, FreehandBase *dc)
{
    if (dc->attach) {
        spdc_attach_selection(dc, sel);
    }
}

static void spdc_attach_selection(FreehandBase *dc, Inkscape::Selection */*sel*/)
{
    // We reset white and forget white/start/end anchors
    spdc_reset_white(dc);
    dc->sa = NULL;
    dc->ea = NULL;

    SPItem *item = dc->selection ? dc->selection->singleItem() : NULL;

    if ( item && SP_IS_PATH(item) ) {
        // Create new white data
        // Item
        dc->white_item = item;

        // Curve list
        // We keep it in desktop coordinates to eliminate calculation errors
        SPCurve *norm = SP_PATH(item)->get_curve_for_edit();
        norm->transform((dc->white_item)->i2dt_affine());
        g_return_if_fail( norm != NULL );
        dc->white_curves = g_slist_reverse(norm->split());
        norm->unref();

        // Anchor list
        for (GSList *l = dc->white_curves; l != NULL; l = l->next) {
            SPCurve *c;
            c = static_cast<SPCurve*>(l->data);
            g_return_if_fail( c->get_segment_count() > 0 );
            if ( !c->is_closed() ) {
                SPDrawAnchor *a;
                a = sp_draw_anchor_new(dc, c, TRUE, *(c->first_point()));
                if (a)
                    dc->white_anchors = g_slist_prepend(dc->white_anchors, a);
                a = sp_draw_anchor_new(dc, c, FALSE, *(c->last_point()));
                if (a)
                    dc->white_anchors = g_slist_prepend(dc->white_anchors, a);
            }
        }
        // fixme: recalculate active anchor?
    }
}


void spdc_endpoint_snap_rotation(ToolBase const *const ec, Geom::Point &p, Geom::Point const &o,
                                 guint state)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    unsigned const snaps = abs(prefs->getInt("/options/rotationsnapsperpi/value", 12));

    SnapManager &m = ec->desktop->namedview->snap_manager;
    m.setup(ec->desktop);

    bool snap_enabled = m.snapprefs.getSnapEnabledGlobally();
    if (state & GDK_SHIFT_MASK) {
        // SHIFT disables all snapping, except the angular snapping. After all, the user explicitly asked for angular
        // snapping by pressing CTRL, otherwise we wouldn't have arrived here. But although we temporarily disable
        // the snapping here, we must still call for a constrained snap in order to apply the constraints (i.e. round
        // to the nearest angle increment)
        m.snapprefs.setSnapEnabledGlobally(false);
    }

    Inkscape::SnappedPoint dummy = m.constrainedAngularSnap(Inkscape::SnapCandidatePoint(p, Inkscape::SNAPSOURCE_NODE_HANDLE), boost::optional<Geom::Point>(), o, snaps);
    p = dummy.getPoint();

    if (state & GDK_SHIFT_MASK) {
        m.snapprefs.setSnapEnabledGlobally(snap_enabled); // restore the original setting
    }

    m.unSetup();
}


void spdc_endpoint_snap_free(ToolBase const * const ec, Geom::Point& p, boost::optional<Geom::Point> &start_of_line, guint const /*state*/)
{
    SPDesktop *dt = ec->desktop;
    SnapManager &m = dt->namedview->snap_manager;
    Inkscape::Selection *selection = dt->getSelection();

    // selection->singleItem() is the item that is currently being drawn. This item will not be snapped to (to avoid self-snapping)
    // TODO: Allow snapping to the stationary parts of the item, and only ignore the last segment

    m.setup(dt, true, selection->singleItem());
    Inkscape::SnapCandidatePoint scp(p, Inkscape::SNAPSOURCE_NODE_HANDLE);
    if (start_of_line) {
        scp.addOrigin(*start_of_line);
    }

    Inkscape::SnappedPoint sp = m.freeSnap(scp);
    p = sp.getPoint();

    m.unSetup();
}

static SPCurve *reverse_then_unref(SPCurve *orig)
{
    SPCurve *ret = orig->create_reverse();
    orig->unref();
    return ret;
}

void spdc_concat_colors_and_flush(FreehandBase *dc, gboolean forceclosed)
{
    // Concat RBG
    SPCurve *c = dc->green_curve;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    // Green
    dc->green_curve = new SPCurve();
    while (dc->green_bpaths) {
        sp_canvas_item_destroy(SP_CANVAS_ITEM(dc->green_bpaths->data));
        dc->green_bpaths = g_slist_remove(dc->green_bpaths, dc->green_bpaths->data);
    }

    // Blue
    c->append_continuous(dc->blue_curve, 0.0625);
    dc->blue_curve->reset();
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->blue_bpath), NULL);

    // Red
    if (dc->red_curve_is_valid) {
        c->append_continuous(dc->red_curve, 0.0625);
    }
    dc->red_curve->reset();
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->red_bpath), NULL);

    if (c->is_empty()) {
        c->unref();
        return;
    }

    // Step A - test, whether we ended on green anchor
    if ( forceclosed || ( dc->green_anchor && dc->green_anchor->active ) ) {
        // We hit green anchor, closing Green-Blue-Red
        dc->desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Path is closed."));
        c->closepath_current();
        // Closed path, just flush
        spdc_flush_white(dc, c);
        c->unref();
        return;
    }

    // Step B - both start and end anchored to same curve
    if ( dc->sa && dc->ea
         && ( dc->sa->curve == dc->ea->curve )
         && ( ( dc->sa != dc->ea )
              || dc->sa->curve->is_closed() ) )
    {
        // We hit bot start and end of single curve, closing paths
        dc->desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Closing path."));
        if (dc->sa->start && !(dc->sa->curve->is_closed()) ) {
            c = reverse_then_unref(c);
        }
        if(prefs->getInt(tool_name(dc) + "/freehand-mode", 0) == 1 || 
            prefs->getInt(tool_name(dc) + "/freehand-mode", 0) == 2){
            dc->overwriteCurve->append_continuous(c, 0.0625);
            c->unref();
            dc->overwriteCurve->closepath_current();
            if(dc->sa){
                dc->white_curves = g_slist_remove(dc->white_curves, dc->sa->curve);
                dc->white_curves = g_slist_append(dc->white_curves, dc->overwriteCurve);
            }
        }else{
            dc->sa->curve->append_continuous(c, 0.0625);
            c->unref();
            dc->sa->curve->closepath_current();
        }
        spdc_flush_white(dc, NULL);
        return;
    }

    // Step C - test start
    if (dc->sa) {
        SPCurve *s = dc->sa->curve;
        dc->white_curves = g_slist_remove(dc->white_curves, s);
        if(prefs->getInt(tool_name(dc) + "/freehand-mode", 0) == 1 || 
            prefs->getInt(tool_name(dc) + "/freehand-mode", 0) == 2){
                s = dc->overwriteCurve;
        }
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
        if(prefs->getInt(tool_name(dc) + "/freehand-mode", 0) == 1 || 
            prefs->getInt(tool_name(dc) + "/freehand-mode", 0) == 2){
                e = reverse_then_unref(e);
                Geom::CubicBezier const * cubic = dynamic_cast<Geom::CubicBezier const*>(&*e->last_segment());
                SPCurve *lastSeg = new SPCurve();
                if(cubic){
                    lastSeg->moveto((*cubic)[0]);
                    lastSeg->curveto((*cubic)[1],(*cubic)[3],(*cubic)[3]);
                    if( e->get_segment_count() == 1){
                        e = lastSeg;
                    }else{
                        //we eliminate the last segment
                        e->backspace();
                        //and we add it again with the recreation
                        e->append_continuous(lastSeg, 0.0625);
                    }
                }
                e = reverse_then_unref(e);
        }
        c->append_continuous(e, 0.0625);
        e->unref();
    }


    spdc_flush_white(dc, c);

    c->unref();
}

static void spdc_flush_white(FreehandBase *dc, SPCurve *gc)
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

    // Now we have to go back to item coordinates at last
    c->transform( dc->white_item
                            ? (dc->white_item)->dt2i_affine()
                            : dc->desktop->dt2doc() );

    SPDesktop *desktop = dc->desktop;
    SPDocument *doc = sp_desktop_document(desktop);
    Inkscape::XML::Document *xml_doc = doc->getReprDoc();

    if ( c && !c->is_empty() ) {
        // We actually have something to write

        bool has_lpe = false;
        Inkscape::XML::Node *repr;

        if (dc->white_item) {
            repr = dc->white_item->getRepr();
            has_lpe = SP_LPE_ITEM(dc->white_item)->hasPathEffectRecursive();
        } else {
            repr = xml_doc->createElement("svg:path");
            // Set style
            sp_desktop_apply_style_tool(desktop, repr, tool_name(dc).data(), false);
        }

        gchar *str = sp_svg_write_path( c->get_pathvector() );
        g_assert( str != NULL );
        if (has_lpe)
            repr->setAttribute("inkscape:original-d", str);
        else
            repr->setAttribute("d", str);
        g_free(str);

        if (!dc->white_item) {
            // Attach repr
            SPItem *item = SP_ITEM(desktop->currentLayer()->appendChildRepr(repr));

            // we finished the path; now apply any waiting LPEs or freehand shapes
            spdc_check_for_and_apply_waiting_LPE(dc, item, c);

            dc->selection->set(repr);
            Inkscape::GC::release(repr);
            item->transform = SP_ITEM(desktop->currentLayer())->i2doc_affine().inverse();
            item->updateRepr();
            item->doWriteTransform(item->getRepr(), item->transform, NULL, true);
        }

        DocumentUndo::done(doc, SP_IS_PEN_CONTEXT(dc)? SP_VERB_CONTEXT_PEN : SP_VERB_CONTEXT_PENCIL,
                         _("Draw path"));

        // When quickly drawing several subpaths with Shift, the next subpath may be finished and
        // flushed before the selection_modified signal is fired by the previous change, which
        // results in the tool losing all of the selected path's curve except that last subpath. To
        // fix this, we force the selection_modified callback now, to make sure the tool's curve is
        // in sync immediately.
        spdc_selection_modified(desktop->getSelection(), 0, dc);
    }

    c->unref();

    // Flush pending updates
    doc->ensureUpToDate();
}

SPDrawAnchor *spdc_test_inside(FreehandBase *dc, Geom::Point p)
{
    SPDrawAnchor *active = NULL;

    // Test green anchor
    if (dc->green_anchor) {
        active = sp_draw_anchor_test(dc->green_anchor, p, TRUE);
    }

    for (GSList *l = dc->white_anchors; l != NULL; l = l->next) {
        SPDrawAnchor *na = sp_draw_anchor_test(static_cast<SPDrawAnchor*>(l->data), p, !active);
        if ( !active && na ) {
            active = na;
        }
    }
    return active;
}

static void spdc_reset_white(FreehandBase *dc)
{
    if (dc->white_item) {
        // We do not hold refcount
        dc->white_item = NULL;
    }
    while (dc->white_curves) {
        reinterpret_cast<SPCurve *>(dc->white_curves->data)->unref();
        dc->white_curves = g_slist_remove(dc->white_curves, dc->white_curves->data);
    }
    while (dc->white_anchors) {
        sp_draw_anchor_destroy(static_cast<SPDrawAnchor*>(dc->white_anchors->data));
        dc->white_anchors = g_slist_remove(dc->white_anchors, dc->white_anchors->data);
    }
}

static void spdc_free_colors(FreehandBase *dc)
{
    // Red
    if (dc->red_bpath) {
        sp_canvas_item_destroy(SP_CANVAS_ITEM(dc->red_bpath));
        dc->red_bpath = NULL;
    }
    if (dc->red_curve) {
        dc->red_curve = dc->red_curve->unref();
    }

    // Blue
    if (dc->blue_bpath) {
        sp_canvas_item_destroy(SP_CANVAS_ITEM(dc->blue_bpath));
        dc->blue_bpath = NULL;
    }
    if (dc->blue_curve) {
        dc->blue_curve = dc->blue_curve->unref();
    }

    // Green
    while (dc->green_bpaths) {
        sp_canvas_item_destroy(SP_CANVAS_ITEM(dc->green_bpaths->data));
        dc->green_bpaths = g_slist_remove(dc->green_bpaths, dc->green_bpaths->data);
    }
    if (dc->green_curve) {
        dc->green_curve = dc->green_curve->unref();
    }
    if (dc->green_anchor) {
        dc->green_anchor = sp_draw_anchor_destroy(dc->green_anchor);
    }

    // White
    if (dc->white_item) {
        // We do not hold refcount
        dc->white_item = NULL;
    }
    while (dc->white_curves) {
        reinterpret_cast<SPCurve *>(dc->white_curves->data)->unref();
        dc->white_curves = g_slist_remove(dc->white_curves, dc->white_curves->data);
    }
    while (dc->white_anchors) {
        sp_draw_anchor_destroy(static_cast<SPDrawAnchor *>(dc->white_anchors->data));
        dc->white_anchors = g_slist_remove(dc->white_anchors, dc->white_anchors->data);
    }
}

void spdc_create_single_dot(ToolBase *ec, Geom::Point const &pt, char const *tool, guint event_state) {
    g_return_if_fail(!strcmp(tool, "/tools/freehand/pen") || !strcmp(tool, "/tools/freehand/pencil"));
    Glib::ustring tool_path = tool;

    SPDesktop *desktop = ec->desktop;
    Inkscape::XML::Document *xml_doc = desktop->doc()->getReprDoc();
    Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");
    repr->setAttribute("sodipodi:type", "arc");
    SPItem *item = SP_ITEM(desktop->currentLayer()->appendChildRepr(repr));
    Inkscape::GC::release(repr);

    // apply the tool's current style
    sp_desktop_apply_style_tool(desktop, repr, tool, false);

    // find out stroke width (TODO: is there an easier way??)
    double stroke_width = 3.0;
    gchar const *style_str = NULL;
    style_str = repr->attribute("style");
    if (style_str) {
        SPStyle *style = sp_style_new(SP_ACTIVE_DOCUMENT);
        sp_style_merge_from_style_string(style, style_str);
        stroke_width = style->stroke_width.computed;
        style->stroke_width.computed = 0;
        sp_style_unref(style);
    }

    // unset stroke and set fill color to former stroke color
    gchar * str;
    str = g_strdup_printf("fill:#%06x;stroke:none;", sp_desktop_get_color_tool(desktop, tool, false) >> 8);
    repr->setAttribute("style", str);
    g_free(str);

    // put the circle where the mouse click occurred and set the diameter to the
    // current stroke width, multiplied by the amount specified in the preferences
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    Geom::Affine const i2d (item->i2dt_affine ());
    Geom::Point pp = pt * i2d.inverse();
    double rad = 0.5 * prefs->getDouble(tool_path + "/dot-size", 3.0);
    if (event_state & GDK_MOD1_MASK) {
        // TODO: We vary the dot size between 0.5*rad and 1.5*rad, where rad is the dot size
        // as specified in prefs. Very simple, but it might be sufficient in practice. If not,
        // we need to devise something more sophisticated.
        double s = g_random_double_range(-0.5, 0.5);
        rad *= (1 + s);
    }
    if (event_state & GDK_SHIFT_MASK) {
        // double the point size
        rad *= 2;
    }

    sp_repr_set_svg_double (repr, "sodipodi:cx", pp[Geom::X]);
    sp_repr_set_svg_double (repr, "sodipodi:cy", pp[Geom::Y]);
    sp_repr_set_svg_double (repr, "sodipodi:rx", rad * stroke_width);
    sp_repr_set_svg_double (repr, "sodipodi:ry", rad * stroke_width);
    item->updateRepr();

    desktop->getSelection()->set(item);

    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Creating single dot"));
    DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_NONE, _("Create single dot"));
}

}
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

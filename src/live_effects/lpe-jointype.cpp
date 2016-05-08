/* Authors:
*
*	 Liam P White
*
* Copyright (C) 2014 Authors
*
* Released under GNU GPL v2+, read the file 'COPYING' for more information
*/

#include "live_effects/parameter/enum.h"
#include "helper/geom-pathstroke.h"

#include "sp-shape.h"
#include "style.h"
#include "xml/repr.h"
#include "sp-paint-server.h"
#include "svg/svg-color.h"
#include "desktop-style.h"
#include "svg/css-ostringstream.h"
#include "display/curve.h"

#include <2geom/path.h>
#include <2geom/elliptical-arc.h>

#include "lpe-jointype.h"

namespace Inkscape {
namespace LivePathEffect {

static const Util::EnumData<unsigned> JoinTypeData[] = {
    {JOIN_BEVEL,       N_("Beveled"),    "bevel"},
    {JOIN_ROUND,       N_("Rounded"),    "round"},
    {JOIN_MITER,       N_("Miter"),      "miter"},
    {JOIN_MITER_CLIP,  N_("Miter Clip"), "miter-clip"},
    {JOIN_EXTRAPOLATE, N_("Extrapolated arc"), "extrp_arc"},
    {JOIN_EXTRAPOLATE1, N_("Extrapolated arc Alt1"), "extrp_arc1"},
    {JOIN_EXTRAPOLATE2, N_("Extrapolated arc Alt2"), "extrp_arc2"},
    {JOIN_EXTRAPOLATE3, N_("Extrapolated arc Alt3"), "extrp_arc3"},
};

static const Util::EnumData<unsigned> CapTypeData[] = {
    {BUTT_FLAT, N_("Butt"), "butt"},
    {BUTT_ROUND, N_("Rounded"), "round"},
    {BUTT_SQUARE, N_("Square"), "square"},
    {BUTT_PEAK, N_("Peak"), "peak"},
    //{BUTT_LEANED, N_("Leaned"), "leaned"}
};

static const Util::EnumDataConverter<unsigned> CapTypeConverter(CapTypeData, sizeof(CapTypeData)/sizeof(*CapTypeData));
static const Util::EnumDataConverter<unsigned> JoinTypeConverter(JoinTypeData, sizeof(JoinTypeData)/sizeof(*JoinTypeData));

LPEJoinType::LPEJoinType(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    line_width(_("Line width"), _("Thickness of the stroke"), "line_width", &wr, this, 1.),
    linecap_type(_("Line cap"), _("The end shape of the stroke"), "linecap_type", CapTypeConverter, &wr, this, butt_straight),
    linejoin_type(_("Join:"), _("Determines the shape of the path's corners"),  "linejoin_type", JoinTypeConverter, &wr, this, JOIN_EXTRAPOLATE),
    //start_lean(_("Start path lean"), _("Start path lean"), "start_lean", &wr, this, 0.),
    //end_lean(_("End path lean"), _("End path lean"), "end_lean", &wr, this, 0.),
    miter_limit(_("Miter limit:"), _("Maximum length of the miter join (in units of stroke width)"), "miter_limit", &wr, this, 100.),
    attempt_force_join(_("Force miter"), _("Overrides the miter limit and forces a join."), "attempt_force_join", &wr, this, true)
{
    show_orig_path = true;
    registerParameter(&linecap_type);
    registerParameter(&line_width);
    registerParameter(&linejoin_type);
    //registerParameter(&start_lean);
    //registerParameter(&end_lean);
    registerParameter(&miter_limit);
    registerParameter(&attempt_force_join);
    //start_lean.param_set_range(-1,1);
    //start_lean.param_set_increments(0.1, 0.1);
    //start_lean.param_set_digits(4);
    //end_lean.param_set_range(-1,1);
    //end_lean.param_set_increments(0.1, 0.1);
    //end_lean.param_set_digits(4);
}

LPEJoinType::~LPEJoinType()
{
}

//from LPEPowerStroke -- sets fill if stroke color because we will
//be converting to a fill to make the new join.

void LPEJoinType::doOnApply(SPLPEItem const* lpeitem)
{
    if (SP_IS_SHAPE(lpeitem)) {
        SPLPEItem* item = const_cast<SPLPEItem*>(lpeitem);
        double width = (lpeitem && lpeitem->style) ? lpeitem->style->stroke_width.computed : 1.;

        SPCSSAttr *css = sp_repr_css_attr_new ();
        if (lpeitem->style->stroke.isPaintserver()) {
            SPPaintServer * server = lpeitem->style->getStrokePaintServer();
            if (server) {
                Glib::ustring str;
                str += "url(#";
                str += server->getId();
                str += ")";
                sp_repr_css_set_property (css, "fill", str.c_str());
            }
        } else if (lpeitem->style->stroke.isColor()) {
            gchar c[64];
            sp_svg_write_color (c, sizeof(c), lpeitem->style->stroke.value.color.toRGBA32(SP_SCALE24_TO_FLOAT(lpeitem->style->stroke_opacity.value)));
            sp_repr_css_set_property (css, "fill", c);
        } else {
            sp_repr_css_set_property (css, "fill", "none");
        }

        sp_repr_css_set_property(css, "fill-rule", "nonzero");
        sp_repr_css_set_property(css, "stroke", "none");

        sp_desktop_apply_css_recursive(item, css, true);
        sp_repr_css_attr_unref (css);

        line_width.param_set_value(width);
        line_width.write_to_SVG();
    }
}

//from LPEPowerStroke -- sets stroke color from existing fill color

void LPEJoinType::doOnRemove(SPLPEItem const* lpeitem)
{
    if (SP_IS_SHAPE(lpeitem)) {
        SPLPEItem *item = const_cast<SPLPEItem*>(lpeitem);

        SPCSSAttr *css = sp_repr_css_attr_new ();
        if (lpeitem->style->fill.isPaintserver()) {
            SPPaintServer * server = lpeitem->style->getFillPaintServer();
            if (server) {
                Glib::ustring str;
                str += "url(#";
                str += server->getId();
                str += ")";
                sp_repr_css_set_property (css, "stroke", str.c_str());
            }
        } else if (lpeitem->style->fill.isColor()) {
            gchar c[64];
            sp_svg_write_color (c, sizeof(c), lpeitem->style->fill.value.color.toRGBA32(SP_SCALE24_TO_FLOAT(lpeitem->style->fill_opacity.value)));
            sp_repr_css_set_property (css, "stroke", c);
        } else {
            sp_repr_css_set_property (css, "stroke", "none");
        }

        Inkscape::CSSOStringStream os;
        os << fabs(line_width);
        sp_repr_css_set_property (css, "stroke-width", os.str().c_str());

        sp_repr_css_set_property(css, "fill", "none");

        sp_desktop_apply_css_recursive(item, css, true);
        sp_repr_css_attr_unref (css);
        item->updateRepr();
    }
}

Geom::PathVector LPEJoinType::doEffect_path(Geom::PathVector const & path_in)
{
    Geom::PathVector ret;
    for (size_t i = 0; i < path_in.size(); ++i) {
        Geom::PathVector tmp = Inkscape::outline(path_in[i], line_width, 
                                                 (attempt_force_join ? std::numeric_limits<double>::max() : miter_limit),
                                                 static_cast<LineJoinType>(linejoin_type.get_value()),
                                                 static_cast<LineCapType>(linecap_type.get_value()));
        ret.insert(ret.begin(), tmp.begin(), tmp.end());
    }

    return ret;
}

} // namespace LivePathEffect
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :

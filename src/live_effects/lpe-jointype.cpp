/* Authors:
*
*	 Liam P White
*
* Copyright (C) 2014 Authors
*
* Released under GNU GPL v2, read the file 'COPYING' for more information
*/

#include "live_effects/parameter/enum.h"
#include "live_effects/pathoutlineprovider.h"

#include "sp-shape.h"
#include "style.h"
#include "xml/repr.h"
#include "sp-paint-server.h"
#include "svg/svg-color.h"
#include "desktop-style.h"
#include "svg/css-ostringstream.h"
#include "display/curve.h"

#include <2geom/path.h>
#include <2geom/svg-elliptical-arc.h>

#include "lpe-jointype.h"

namespace Inkscape {
namespace LivePathEffect {

static const Util::EnumData<unsigned> JoinTypeData[] = {
    {LINEJOIN_STRAIGHT, N_("Beveled"), "bevel"},
    {LINEJOIN_ROUND, N_("Rounded"), "round"},
    {LINEJOIN_POINTY, N_("Miter"), "miter"},
    {LINEJOIN_REFLECTED,  N_("Reflected"), "extrapolated"},
    {LINEJOIN_EXTRAPOLATED, N_("Extrapolated arc"), "extrp_arc"}
};

static const Util::EnumData<unsigned> CapTypeData[] = {
    {BUTT_STRAIGHT, N_("Butt"), "butt"},
    {BUTT_ROUND, N_("Rounded"), "round"},
    {BUTT_SQUARE, N_("Square"), "square"},
    {BUTT_POINTY, N_("Peak"), "peak"},
    {BUTT_LEANED, N_("Leaned"), "leaned"}
};

static const Util::EnumDataConverter<unsigned> CapTypeConverter(CapTypeData, sizeof(CapTypeData)/sizeof(*CapTypeData));
static const Util::EnumDataConverter<unsigned> JoinTypeConverter(JoinTypeData, sizeof(JoinTypeData)/sizeof(*JoinTypeData));

LPEJoinType::LPEJoinType(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    line_width(_("Line width"), _("Thickness of the stroke"), "line_width", &wr, this, 1.),
    linecap_type(_("Line cap"), _("The end shape of the stroke"), "linecap_type", CapTypeConverter, &wr, this, butt_straight),
    linejoin_type(_("Join:"), _("Determines the shape of the path's corners"),  "linejoin_type", JoinTypeConverter, &wr, this, LINEJOIN_EXTRAPOLATED),
    start_lean(_("Start path lean"), _("Start path lean"), "start_lean", &wr, this, 0.),
    end_lean(_("End path lean"), _("End path lean"), "end_lean", &wr, this, 0.),
    miter_limit(_("Miter limit:"), _("Maximum length of the miter join (in units of stroke width)"), "miter_limit", &wr, this, 100.),
    attempt_force_join(_("Force miter"), _("Overrides the miter limit and forces a join."), "attempt_force_join", &wr, this, true)
{
    show_orig_path = true;
    registerParameter(&linecap_type);
    registerParameter(&line_width);
    registerParameter(&linejoin_type);
    registerParameter(&start_lean);
    registerParameter(&end_lean);
    registerParameter(&miter_limit);
    registerParameter(&attempt_force_join);
    was_initialized = false;
    start_lean.param_set_range(-1,1);
    start_lean.param_set_increments(0.1, 0.1);
    start_lean.param_set_digits(4);
    end_lean.param_set_range(-1,1);
    end_lean.param_set_increments(0.1, 0.1);
    end_lean.param_set_digits(4);
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
        if (true) {
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
        } else {
            sp_repr_css_unset_property (css, "fill");
        }

        sp_repr_css_set_property(css, "stroke", "none");

        sp_desktop_apply_css_recursive(item, css, true);
        sp_repr_css_attr_unref (css);
        if (!was_initialized)
        {
            was_initialized = true;
            line_width.param_set_value(width);
        }
    } else {
        g_warning("LPE Join Type can only be applied to paths (not groups).");
    }
}

//from LPEPowerStroke -- sets stroke color from existing fill color

void LPEJoinType::doOnRemove(SPLPEItem const* lpeitem)
{

    if (SP_IS_SHAPE(lpeitem)) {
        SPLPEItem *item = const_cast<SPLPEItem*>(lpeitem);

        SPCSSAttr *css = sp_repr_css_attr_new ();
        if (true) {
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
                sp_svg_write_color (c, sizeof(c), lpeitem->style->stroke.value.color.toRGBA32(SP_SCALE24_TO_FLOAT(lpeitem->style->stroke_opacity.value)));
                sp_repr_css_set_property (css, "stroke", c);
            } else {
                sp_repr_css_set_property (css, "stroke", "none");
            }
        } else {
            sp_repr_css_unset_property (css, "stroke");
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

// NOTE: I originally had all the outliner functions defined in here, but they were actually useful
// enough for other LPEs so I moved them all into pathoutlineprovider.cpp. The code here is just a
// wrapper around it.
std::vector<Geom::Path> LPEJoinType::doEffect_path(std::vector<Geom::Path> const & path_in)
{
    return Outline::PathVectorOutline(path_in, line_width, static_cast<ButtTypeMod>(linecap_type.get_value()),
                                      static_cast<LineJoinType>(linejoin_type.get_value()),
                                      (attempt_force_join ? std::numeric_limits<double>::max() : miter_limit),
                                      start_lean/2 ,end_lean/2);
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

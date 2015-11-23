/** \file
 * LPE interpolate_points implementation
 *   Interpolates between knots of the input path.
 */
/*
 * Authors:
 *   Johan Engelen
 *
 * Copyright (C) Johan Engelen 2014 <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-interpolate_points.h"

#include <2geom/path.h>

#include "live_effects/lpe-powerstroke-interpolators.h"

namespace Inkscape {
namespace LivePathEffect {


static const Util::EnumData<unsigned> InterpolatorTypeData[] = {
    {Geom::Interpolate::INTERP_LINEAR          , N_("Linear"), "Linear"},
    {Geom::Interpolate::INTERP_CUBICBEZIER          , N_("CubicBezierFit"), "CubicBezierFit"},
    {Geom::Interpolate::INTERP_CUBICBEZIER_JOHAN     , N_("CubicBezierJohan"), "CubicBezierJohan"},
    {Geom::Interpolate::INTERP_SPIRO  , N_("SpiroInterpolator"), "SpiroInterpolator"},
    {Geom::Interpolate::INTERP_CENTRIPETAL_CATMULLROM, N_("Centripetal Catmull-Rom"), "CentripetalCatmullRom"}
};
static const Util::EnumDataConverter<unsigned> InterpolatorTypeConverter(InterpolatorTypeData, sizeof(InterpolatorTypeData)/sizeof(*InterpolatorTypeData));


LPEInterpolatePoints::LPEInterpolatePoints(LivePathEffectObject *lpeobject)
    : Effect(lpeobject)
    , interpolator_type(
          _("Interpolator type:"),
          _("Determines which kind of interpolator will be used to interpolate between stroke width along the path"),
          "interpolator_type", InterpolatorTypeConverter, &wr, this, Geom::Interpolate::INTERP_CENTRIPETAL_CATMULLROM)
{
    show_orig_path = false;

    registerParameter( &interpolator_type );
}

LPEInterpolatePoints::~LPEInterpolatePoints()
{
}


Geom::PathVector
LPEInterpolatePoints::doEffect_path (Geom::PathVector const & path_in)
{
    Geom::PathVector path_out;
#if __cplusplus <= 199711L
    std::auto_ptr<Geom::Interpolate::Interpolator> interpolator(  Geom::Interpolate::Interpolator::create(static_cast<Geom::Interpolate::InterpolatorType>(interpolator_type.get_value())) );
#else
    std::unique_ptr<Geom::Interpolate::Interpolator> interpolator(  Geom::Interpolate::Interpolator::create(static_cast<Geom::Interpolate::InterpolatorType>(interpolator_type.get_value())) );
#endif

    for(Geom::PathVector::const_iterator path_it = path_in.begin(); path_it != path_in.end(); ++path_it) {
        if (path_it->empty())
            continue;

        if (path_it->closed()) {
            g_warning("Interpolate points LPE currently ignores whether path is closed or not.");
        }

        std::vector<Geom::Point> pts;
        pts.push_back(path_it->initialPoint());

        for (Geom::Path::const_iterator it = path_it->begin(), e = path_it->end_default(); it != e; ++it) {
            pts.push_back((*it).finalPoint());
        }

        Geom::Path path = interpolator->interpolateToPath(pts);

        path_out.push_back(path);
    }

    return path_out;
}


} //namespace LivePathEffect
} /* namespace Inkscape */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

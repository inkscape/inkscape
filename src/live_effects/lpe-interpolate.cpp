/** \file
 * LPE interpolate implementation
 */
/*
 * Authors:
 *   Johan Engelen
 *
 * Copyright (C) Johan Engelen 2007-2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "live_effects/lpe-interpolate.h"

#include <2geom/path.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/piecewise.h>
#include <2geom/sbasis-geometric.h>

#include "sp-path.h"
#include "display/curve.h"

namespace Inkscape {
namespace LivePathEffect {

LPEInterpolate::LPEInterpolate(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    trajectory_path(_("Trajectory:"), _("Path along which intermediate steps are created."), "trajectory", &wr, this, "M0,0 L0,0"),
    number_of_steps(_("Steps_:"), _("Determines the number of steps from start to end path."), "steps", &wr, this, 5),
    equidistant_spacing(_("E_quidistant spacing"), _("If true, the spacing between intermediates is constant along the length of the path. If false, the distance depends on the location of the nodes of the trajectory path."), "equidistant_spacing", &wr, this, true)
{
    show_orig_path = true;

    registerParameter( dynamic_cast<Parameter *>(&trajectory_path) );
    registerParameter( dynamic_cast<Parameter *>(&equidistant_spacing) );
    registerParameter( dynamic_cast<Parameter *>(&number_of_steps) );

    number_of_steps.param_make_integer();
    number_of_steps.param_set_range(2, Geom::infinity());
}

LPEInterpolate::~LPEInterpolate()
{

}

/*
 * interpolate path_in[0] to path_in[1]
 */
Geom::PathVector
LPEInterpolate::doEffect_path (Geom::PathVector const & path_in)
{
    if ( (path_in.size() < 2) || (number_of_steps < 2)) {
        return path_in;
    }
    // Don't allow empty path parameter:
    if ( trajectory_path.get_pathvector().empty() ) {
        return path_in;
    }

    Geom::PathVector path_out;

    Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_A = path_in[0].toPwSb();
    Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_B = path_in[1].toPwSb();

    // Transform both paths to (0,0) midpoint, so they can easily be positioned along interpolate_path
    if (Geom::OptRect bounds = Geom::bounds_exact(pwd2_A)) {
        pwd2_A -= bounds->midpoint();
    }
    if (Geom::OptRect bounds = Geom::bounds_exact(pwd2_B)) {
        pwd2_B -= bounds->midpoint();
    }

    // Make sure both paths have the same number of segments and cuts at the same locations
    pwd2_B.setDomain(pwd2_A.domain());
    Geom::Piecewise<Geom::D2<Geom::SBasis> > pA = Geom::partition(pwd2_A, pwd2_B.cuts);
    Geom::Piecewise<Geom::D2<Geom::SBasis> > pB = Geom::partition(pwd2_B, pwd2_A.cuts);

    Geom::Piecewise<Geom::D2<Geom::SBasis> > trajectory = trajectory_path.get_pathvector()[0].toPwSb();
    if (equidistant_spacing)
        trajectory = Geom::arc_length_parametrization(trajectory);

    Geom::Interval trajectory_domain = trajectory.domain();

    for (int i = 0; i < number_of_steps; ++i) {
        double fraction = i / (number_of_steps-1);

        Geom::Piecewise<Geom::D2<Geom::SBasis> > pResult = pA*(1-fraction)  +  pB*fraction;
        pResult += trajectory.valueAt(trajectory_domain.min() + fraction*trajectory_domain.extent());

        Geom::PathVector pathv = Geom::path_from_piecewise(pResult, LPE_CONVERSION_TOLERANCE);
        path_out.push_back( pathv[0] );
    }

    return path_out;
}

void
LPEInterpolate::resetDefaults(SPItem const* item)
{
    Effect::resetDefaults(item);

    if (!SP_IS_PATH(item))
        return;

    SPCurve const *crv = SP_PATH(item)->get_curve_reference();
    Geom::PathVector const &pathv = crv->get_pathvector();
    if ( (pathv.size() < 2) )
        return;

    Geom::OptRect bounds_A = pathv[0].boundsExact();
    Geom::OptRect bounds_B = pathv[1].boundsExact();

    if (bounds_A && bounds_B) {
        Geom::PathVector traj_pathv;
        traj_pathv.push_back( Geom::Path() );
        traj_pathv[0].start( bounds_A->midpoint() );
        traj_pathv[0].appendNew<Geom::LineSegment>( bounds_B->midpoint() );
        trajectory_path.set_new_value( traj_pathv, true );
    } else {
        trajectory_path.param_set_and_write_default();
    }
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

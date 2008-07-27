#define INKSCAPE_LPE_INTERPOLATE_CPP
/** \file
 * LPE boolops implementation
 */
/*
 * Authors:
 *   Johan Engelen
 *
 * Copyright (C) Johan Engelen 2007-2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-interpolate.h"

#include <2geom/path.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2-sbasis.h>
#include <2geom/piecewise.h>

#include "sp-path.h"
#include "display/curve.h"

namespace Inkscape {
namespace LivePathEffect {

LPEInterpolate::LPEInterpolate(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    interpolate_path(_("Interpolate trajectory"), _("Path along which intermediate steps are created."), "trajectory", &wr, this, "M0,0 L0,0"),
    number_of_steps(_("Steps"), _("Determines the number of steps from start to end path."), "steps", &wr, this, 5),
    spacing(_("Spacing"), _("Determines the spacing between intermediate steps."), "spacing", &wr, this, 1)
{
    show_orig_path = true;

    registerParameter( dynamic_cast<Parameter *>(&interpolate_path) );
    registerParameter( dynamic_cast<Parameter *>(&number_of_steps) );

    number_of_steps.param_make_integer();
    number_of_steps.param_set_range(2, NR_HUGE);
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
    if ( (path_in.size() < 2) || (number_of_steps < 2))
        return path_in;

    std::vector<Geom::Path> path_out;

    Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_A = path_in[0].toPwSb();
    Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_B = path_in[1].toPwSb();

    // Transform both paths to (0,0) midpoint, so they can easily be positioned along interpolate_path
    pwd2_A -= Geom::bounds_exact(pwd2_A).midpoint();
    pwd2_B -= Geom::bounds_exact(pwd2_B).midpoint();

    // Make sure both paths have the same number of segments and cuts at the same locations
    pwd2_B.setDomain(pwd2_A.domain());
    Geom::Piecewise<Geom::D2<Geom::SBasis> > pA = Geom::partition(pwd2_A, pwd2_B.cuts);
    Geom::Piecewise<Geom::D2<Geom::SBasis> > pB = Geom::partition(pwd2_B, pwd2_A.cuts);

    Geom::Path const &trajectory = interpolate_path.get_pathvector()[0];
    double trajectory_length = trajectory.size();

    for (int i = 0; i < number_of_steps; ++i) {
        double fraction = i / (number_of_steps-1);

        Geom::Piecewise<Geom::D2<Geom::SBasis> > pResult = pA*fraction  +  pB*(1-fraction);
        pResult += trajectory.pointAt((1.-fraction)*trajectory_length);

        Geom::PathVector pathv = Geom::path_from_piecewise(pResult, LPE_CONVERSION_TOLERANCE);
        path_out.push_back( pathv[0] );
    }

    return path_out;
}

void
LPEInterpolate::resetDefaults(SPItem * item)
{
    if (!SP_IS_PATH(item))
        return;

    SPCurve const *crv = sp_path_get_curve_reference(SP_PATH(item));
    Geom::PathVector const &pathv = crv->get_pathvector();
    if ( (pathv.size() < 2) )
        return;

    Geom::Rect bounds_A = pathv[0].boundsExact();
    Geom::Rect bounds_B = pathv[1].boundsExact();

    Geom::PathVector traj_pathv;
    traj_pathv.push_back( Geom::Path() );
    traj_pathv[0].start( bounds_A.midpoint() );
    traj_pathv[0].appendNew<Geom::LineSegment>( bounds_B.midpoint() );
    interpolate_path.set_new_value( traj_pathv, true );
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

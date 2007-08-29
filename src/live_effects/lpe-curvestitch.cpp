#define INKSCAPE_LPE_EXPRESSION_CPP
/** \file
 * SVG <skeleton> implementation, used as an example for a base starting class
 * when implementing new LivePathEffects.
 *
 */
/*
 * Authors:
 *   Johan Engelen
*
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-curvestitch.h"
#include "display/curve.h"
#include <libnr/n-art-bpath.h>

#include <2geom/path.h>
#include <2geom/piecewise.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>

#include "ui/widget/scalar.h"
#include "libnr/nr-values.h"

namespace Inkscape {
namespace LivePathEffect {

using namespace Geom;

LPECurveStitch::LPECurveStitch(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    strokepath(_("Stroke path"), _("The path that will be stroked, whatever, think of good text here."), "strokepath", &wr, this, "M0,0 L1,1"),
    nrofpaths(_("Nr of paths"), _("The number of paths that will be generated."), "count", &wr, this, 5),
    startpoint_variation(_("Startpoint variation"), _("..."), "startpoint_variation", &wr, this, 0),
    endpoint_variation(_("Endpoint variation"), _("..."), "endpoint_variation", &wr, this, 0)
{
    registerParameter( dynamic_cast<Parameter *>(&nrofpaths) );
//    registerParameter( dynamic_cast<Parameter *>(&startpoint_variation) );
//    registerParameter( dynamic_cast<Parameter *>(&endpoint_variation) );

    nrofpaths.param_make_integer();
    nrofpaths.param_set_range(2, NR_HUGE);

//    startpoint_variation.param_set_range(-NR_HUGE, 1);
//    endpoint_variation.param_set_range(-1, NR_HUGE);
}

LPECurveStitch::~LPECurveStitch()
{

}

std::vector<Geom::Path>
LPECurveStitch::doEffect (std::vector<Geom::Path> & path_in)
{
    if (path_in.size() >= 2) {
        std::vector<Geom::Path> path_out (nrofpaths);

        // do this for all permutations if there are more than 2 paths? realllly cool!
        Piecewise<D2<SBasis> > A = arc_length_parametrization(Piecewise<D2<SBasis> >(path_in[0].toPwSb()),2,.1);
        Piecewise<D2<SBasis> > B = arc_length_parametrization(Piecewise<D2<SBasis> >(path_in[1].toPwSb()),2,.1);
        Interval bndsA = A.domain();
        Interval bndsB = B.domain();
        gdouble incrementA = (bndsA.max()-bndsA.min()) / (nrofpaths-1);
        gdouble incrementB = (bndsB.max()-bndsB.min()) / (nrofpaths-1);
        gdouble tA = bndsA.min();
        gdouble tB = bndsB.min();
        for (int i = 0; i < nrofpaths; i++) {
            Point start = A(tA);
            Point end = B(tB);
            if (startpoint_variation != 0)
                start = start + g_random_double_range(0, startpoint_variation) * (end - start);
            if (endpoint_variation != 0)
                end = end + g_random_double_range(0, endpoint_variation) * (end - start);

            path_out[i].start( start );
            path_out[i].appendNew<LineSegment>( end );
            tA += incrementA;
            tB += incrementB;
        }

        return path_out;
    } else {
        return path_in;
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

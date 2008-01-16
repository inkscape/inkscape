#define INKSCAPE_LPE_CURVESTITCH_CPP
/** \file
 * LPE Curve Stitching implementation, used as an example for a base starting class
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
#include "sp-item.h"
#include "sp-path.h"
#include "live_effects/n-art-bpath-2geom.h"

#include <2geom/path.h>
#include <2geom/piecewise.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>
#include <2geom/matrix.h>


#include "ui/widget/scalar.h"
#include "libnr/nr-values.h"

namespace Inkscape {
namespace LivePathEffect {

using namespace Geom;

LPECurveStitch::LPECurveStitch(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    strokepath(_("Stroke path"), _("The path that will be used as stitch."), "strokepath", &wr, this, "M0,0 L1,0"),
    nrofpaths(_("Number of paths"), _("The number of paths that will be generated."), "count", &wr, this, 5),
    startpoint_variation(_("Start point jitter"), _("The amount of random jitter to apply to the start points of the stitches"), "startpoint_variation", &wr, this, 0),
    endpoint_variation(_("End point jitter"), _("The amount of random jitter to apply to the end points of the stitches"), "endpoint_variation", &wr, this, 0),
    spacing_variation(_("Spacing variation"), _("Determines the random deviation from the normal start and end points along the sub-paths (whether lines cluster together or have an equal spacing between each other)."), "spacing_variation", &wr, this, 0),
    prop_scale(_("Scale width"), _("Scaling of the width of the stroke path"), "prop_scale", &wr, this, 1),
    scale_y_rel(_("Scale width relative"), _("Scale the width of the stroke path relative to its length"), "scale_y_rel", &wr, this, false)
{
    registerParameter( dynamic_cast<Parameter *>(&nrofpaths) );
    registerParameter( dynamic_cast<Parameter *>(&startpoint_variation) );
    registerParameter( dynamic_cast<Parameter *>(&endpoint_variation) );
    registerParameter( dynamic_cast<Parameter *>(&spacing_variation) );
    registerParameter( dynamic_cast<Parameter *>(&strokepath) );
    registerParameter( dynamic_cast<Parameter *>(&prop_scale) );
    registerParameter( dynamic_cast<Parameter *>(&scale_y_rel) );

    nrofpaths.param_make_integer();
    nrofpaths.param_set_range(2, NR_HUGE);

    prop_scale.param_set_digits(3);
    prop_scale.param_set_increments(0.01, 0.10);
}

LPECurveStitch::~LPECurveStitch()
{

}

std::vector<Geom::Path>
LPECurveStitch::doEffect_path (std::vector<Geom::Path> & path_in)
{
    bool scislac = true;

    if (path_in.size() >= 2) {
        startpoint_variation.resetRandomizer();
        endpoint_variation.resetRandomizer();
        spacing_variation.resetRandomizer();

        D2<Piecewise<SBasis> > stroke = make_cuts_independant(strokepath);
        Interval bndsStroke = bounds_exact(stroke[0]);
        gdouble scaling = bndsStroke.max() - bndsStroke.min();
        Interval bndsStrokeY = bounds_exact(stroke[1]);
        Point stroke_origin(bndsStroke.min(), (bndsStrokeY.max()+bndsStrokeY.min())/2);

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
        gdouble tAclean = tA; // the tA without spacing_variation
        gdouble tBclean = tB; // the tB without spacing_variation
        for (int i = 0; i < nrofpaths; i++) {
            Point start = A(tA);
            Point end = B(tB);
            if (startpoint_variation.get_value() != 0)
                start = start + (startpoint_variation - startpoint_variation.get_value()/2) * (end - start);
            if (endpoint_variation.get_value() != 0)
                end = end + (endpoint_variation - endpoint_variation.get_value()/2)* (end - start);
    
            gdouble scaling_y = 1.0;
            if (scale_y_rel.get_value()) {
                scaling_y = (L2(end-start)/scaling)*prop_scale;
            } else {
                scaling_y = prop_scale;
            }

            Matrix transform;
            transform.setXAxis( (end-start) / scaling );
            transform.setYAxis( rot90(unit_vector(end-start)) * scaling_y);
            transform.setTranslation( start );
            Piecewise<D2<SBasis> > pwd2_out = (strokepath-stroke_origin) * transform;
            // add stuff to one big pw<d2<sbasis> > and then outside the loop convert to path?
            std::vector<Geom::Path> result = Geom::path_from_piecewise(pwd2_out, LPE_CONVERSION_TOLERANCE);
            path_out[i] = result[0];
            gdouble svA = spacing_variation - spacing_variation.get_value()/2;
            gdouble svB = scislac ? 0 : svA;
            tAclean += incrementA;
            tBclean += incrementB;
            tA = tAclean + incrementA * svA;
            tB = tBclean + incrementB * svB;
            if (tA > bndsA.max())
                tA = bndsA.max();
            if (tB > bndsB.max())
                tB = bndsB.max();
        }

        return path_out;
    } else {
        return path_in;
    }
}

void
LPECurveStitch::resetDefaults(SPItem * item)
{
    if (!SP_IS_PATH(item)) return;

    using namespace Geom;

    // set the stroke path to run horizontally in the middle of the bounding box of the original path
    Piecewise<D2<SBasis> > pwd2;
    std::vector<Geom::Path> temppath = SVGD_to_2GeomPath( SP_OBJECT_REPR(item)->attribute("inkscape:original-d"));
    for (unsigned int i=0; i < temppath.size(); i++) {
        pwd2.concat( temppath[i].toPwSb() );
    }

    D2<Piecewise<SBasis> > d2pw = make_cuts_independant(pwd2);
    Interval bndsX = bounds_exact(d2pw[0]);
    Interval bndsY = bounds_exact(d2pw[1]);
    Point start(bndsX.min(), (bndsY.max()+bndsY.min())/2);
    Point end(bndsX.max(), (bndsY.max()+bndsY.min())/2);

    Geom::Path path;
    path.start( start );
    path.appendNew<Geom::LineSegment>( end );
    strokepath.param_set_and_write_new_value( path.toPwSb() );
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

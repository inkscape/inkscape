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

#include "ui/widget/scalar.h"
#include <glibmm/i18n.h>

#include "live_effects/lpe-curvestitch.h"

#include "sp-item.h"
#include "sp-path.h"
#include "svg/svg.h"
#include "xml/repr.h"

#include <2geom/path.h>
#include <2geom/piecewise.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>
#include <2geom/affine.h>

namespace Inkscape {
namespace LivePathEffect {

using namespace Geom;

LPECurveStitch::LPECurveStitch(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    strokepath(_("Stitch path:"), _("The path that will be used as stitch."), "strokepath", &wr, this, "M0,0 L1,0"),
    nrofpaths(_("N_umber of paths:"), _("The number of paths that will be generated."), "count", &wr, this, 5),
    startpoint_edge_variation(_("Sta_rt edge variance:"), _("The amount of random jitter to move the start points of the stitches inside & outside the guide path"), "startpoint_edge_variation", &wr, this, 0),
    startpoint_spacing_variation(_("Sta_rt spacing variance:"), _("The amount of random shifting to move the start points of the stitches back & forth along the guide path"), "startpoint_spacing_variation", &wr, this, 0),
    endpoint_edge_variation(_("End ed_ge variance:"), _("The amount of randomness that moves the end points of the stitches inside & outside the guide path"), "endpoint_edge_variation", &wr, this, 0),
    endpoint_spacing_variation(_("End spa_cing variance:"), _("The amount of random shifting to move the end points of the stitches back & forth along the guide path"), "endpoint_spacing_variation", &wr, this, 0),
    prop_scale(_("Scale _width:"), _("Scale the width of the stitch path"), "prop_scale", &wr, this, 1),
    scale_y_rel(_("Scale _width relative to length"), _("Scale the width of the stitch path relative to its length"), "scale_y_rel", &wr, this, false)
{
    registerParameter( dynamic_cast<Parameter *>(&nrofpaths) );
    registerParameter( dynamic_cast<Parameter *>(&startpoint_edge_variation) );
    registerParameter( dynamic_cast<Parameter *>(&startpoint_spacing_variation) );
    registerParameter( dynamic_cast<Parameter *>(&endpoint_edge_variation) );
    registerParameter( dynamic_cast<Parameter *>(&endpoint_spacing_variation) );
    registerParameter( dynamic_cast<Parameter *>(&strokepath) );
    registerParameter( dynamic_cast<Parameter *>(&prop_scale) );
    registerParameter( dynamic_cast<Parameter *>(&scale_y_rel) );

    nrofpaths.param_make_integer();
    nrofpaths.param_set_range(2, Geom::infinity());

    prop_scale.param_set_digits(3);
    prop_scale.param_set_increments(0.01, 0.10);
}

LPECurveStitch::~LPECurveStitch()
{

}

Geom::PathVector
LPECurveStitch::doEffect_path (Geom::PathVector const & path_in)
{
    if (path_in.size() >= 2) {
        startpoint_edge_variation.resetRandomizer();
        endpoint_edge_variation.resetRandomizer();
        startpoint_spacing_variation.resetRandomizer();
        endpoint_spacing_variation.resetRandomizer();

        D2<Piecewise<SBasis> > stroke = make_cuts_independent(strokepath.get_pwd2());
        OptInterval bndsStroke = bounds_exact(stroke[0]);
        OptInterval bndsStrokeY = bounds_exact(stroke[1]);
        if (!bndsStroke && !bndsStrokeY) {
            return path_in;
        }
        gdouble scaling = bndsStroke->max() - bndsStroke->min();
        Point stroke_origin(bndsStroke->min(), (bndsStrokeY->max()+bndsStrokeY->min())/2);

        Geom::PathVector path_out;

        // do this for all permutations (ii,jj) if there are more than 2 paths? realllly cool!
        for (unsigned ii = 0   ; ii < path_in.size() - 1; ii++)
        for (unsigned jj = ii+1; jj < path_in.size(); jj++)
        {
            Piecewise<D2<SBasis> > A = arc_length_parametrization(Piecewise<D2<SBasis> >(path_in[ii].toPwSb()),2,.1);
            Piecewise<D2<SBasis> > B = arc_length_parametrization(Piecewise<D2<SBasis> >(path_in[jj].toPwSb()),2,.1);
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
                if (startpoint_edge_variation.get_value() != 0)
                    start = start + (startpoint_edge_variation - startpoint_edge_variation.get_value()/2) * (end - start);
                if (endpoint_edge_variation.get_value() != 0)
                    end = end + (endpoint_edge_variation - endpoint_edge_variation.get_value()/2)* (end - start);
        
                if (!Geom::are_near(start,end)) {
                    gdouble scaling_y = 1.0;
                    if (scale_y_rel.get_value()) {
                        scaling_y = (L2(end-start)/scaling)*prop_scale;
                    } else {
                        scaling_y = prop_scale;
                    }

                    Affine transform;
                    transform.setXAxis( (end-start) / scaling );
                    transform.setYAxis( rot90(unit_vector(end-start)) * scaling_y);
                    transform.setTranslation( start );
                    Piecewise<D2<SBasis> > pwd2_out = (strokepath.get_pwd2()-stroke_origin) * transform;

                    // add stuff to one big pw<d2<sbasis> > and then outside the loop convert to path?
                    // No: this way, the separate result paths are kept separate which might come in handy some time!
                    Geom::PathVector result = Geom::path_from_piecewise(pwd2_out, LPE_CONVERSION_TOLERANCE);
                    path_out.push_back(result[0]);
                }
                gdouble svA = startpoint_spacing_variation - startpoint_spacing_variation.get_value()/2;
                gdouble svB = endpoint_spacing_variation - endpoint_spacing_variation.get_value()/2;
                tAclean += incrementA;
                tBclean += incrementB;
                tA = tAclean + incrementA * svA;
                tB = tBclean + incrementB * svB;
                if (tA > bndsA.max())
                    tA = bndsA.max();
                if (tB > bndsB.max())
                    tB = bndsB.max();
            }
        }

        return path_out;
    } else {
        return path_in;
    }
}

void
LPECurveStitch::resetDefaults(SPItem const* item)
{
    Effect::resetDefaults(item);

    if (!SP_IS_PATH(item)) return;

    using namespace Geom;

    // set the stroke path to run horizontally in the middle of the bounding box of the original path
    
    // calculate bounding box:  (isn't there a simpler way?)
    Piecewise<D2<SBasis> > pwd2;
    Geom::PathVector temppath = sp_svg_read_pathv( item->getRepr()->attribute("inkscape:original-d"));
    for (unsigned int i=0; i < temppath.size(); i++) {
        pwd2.concat( temppath[i].toPwSb() );
    }
    D2<Piecewise<SBasis> > d2pw = make_cuts_independent(pwd2);
    OptInterval bndsX = bounds_exact(d2pw[0]);
    OptInterval bndsY = bounds_exact(d2pw[1]);
    if (bndsX && bndsY) {
        Point start(bndsX->min(), (bndsY->max()+bndsY->min())/2);
        Point end(bndsX->max(), (bndsY->max()+bndsY->min())/2);
        if ( !Geom::are_near(start,end) ) {
            Geom::Path path;
            path.start( start );
            path.appendNew<Geom::LineSegment>( end );
            strokepath.set_new_value( path.toPwSb(), true );
        } else {
            // bounding box is too small to make decent path. set to default default. :-)
            strokepath.param_set_and_write_default();
        }
    } else {
        // bounding box is non-existent. set to default default. :-)
        strokepath.param_set_and_write_default();
    }
}


/** /todo check whether this special case is necessary. It seems to "bug" editing behavior:
 * scaling an object with transforms preserved behaves differently from scaling with
 * transforms optimized (difference caused by this special method).
 * special casing is probably needed, because rotation should not be propagated to the strokepath.
 */
void
LPECurveStitch::transform_multiply(Geom::Affine const& postmul, bool set)
{
    // only take translations into account
    if (postmul.isTranslation()) {
        strokepath.param_transform_multiply(postmul, set);
    } else if (!scale_y_rel.get_value()) {
  // this basically means that for this transformation, the result should be the same as normal scaling the result path
  // don't know how to do this yet.
//        Geom::Affine new_postmul;
        //new_postmul.setIdentity();
//        new_postmul.setTranslation(postmul.translation());
//        Effect::transform_multiply(new_postmul, set);
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

/*
 * Copyright (C) Steren Giannini 2008 <steren.giannini@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-envelope.h"
#include "sp-shape.h"
#include "sp-item.h"
#include "sp-path.h"
#include "sp-item-group.h"
#include "display/curve.h"
#include "svg/svg.h"
#include "ui/widget/scalar.h"

#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>
#include <2geom/piecewise.h>

#include <algorithm>
using std::vector;

namespace Inkscape {
namespace LivePathEffect {

LPEEnvelope::LPEEnvelope(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    bend_path1(_("Top bend path:"), _("Top path along which to bend the original path"), "bendpath1", &wr, this, "M0,0 L1,0"),
    bend_path2(_("Right bend path:"), _("Right path along which to bend the original path"), "bendpath2", &wr, this, "M0,0 L1,0"),
    bend_path3(_("Bottom bend path:"), _("Bottom path along which to bend the original path"), "bendpath3", &wr, this, "M0,0 L1,0"),
    bend_path4(_("Left bend path:"), _("Left path along which to bend the original path"), "bendpath4", &wr, this, "M0,0 L1,0"),
    xx(_("_Enable left & right paths"), _("Enable the left and right deformation paths"), "xx", &wr, this, true),
    yy(_("_Enable top & bottom paths"), _("Enable the top and bottom deformation paths"), "yy", &wr, this, true)
{
    registerParameter( dynamic_cast<Parameter *>(&yy) );
    registerParameter( dynamic_cast<Parameter *>(&xx) );
    registerParameter( dynamic_cast<Parameter *>(&bend_path1) );
    registerParameter( dynamic_cast<Parameter *>(&bend_path2) );
    registerParameter( dynamic_cast<Parameter *>(&bend_path3) );
    registerParameter( dynamic_cast<Parameter *>(&bend_path4) );
    concatenate_before_pwd2 = true;
    apply_to_clippath_and_mask = true;
}

LPEEnvelope::~LPEEnvelope()
{

}

void
LPEEnvelope::doBeforeEffect (SPLPEItem const* lpeitem)
{
    // get the item bounding box
    original_bbox(lpeitem);
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEEnvelope::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{

    if(xx.get_value() == false && yy.get_value() == false)
    {
        return pwd2_in;
    }

    using namespace Geom;

    // Don't allow empty path parameters:
    if ( bend_path1.get_pathvector().empty()
         || bend_path2.get_pathvector().empty()
         || bend_path3.get_pathvector().empty()
         || bend_path4.get_pathvector().empty() )
    {
        return pwd2_in;
    }

    /*
    The code below is inspired from the Bend Path code developed by jfb and mgsloan
    Please, read it before tring to understand this one
    */

    Piecewise<D2<SBasis> > uskeleton1 = arc_length_parametrization(bend_path1.get_pwd2(),2,.1);
    uskeleton1 = remove_short_cuts(uskeleton1,.01);
    Piecewise<D2<SBasis> > n1 = rot90(derivative(uskeleton1));
    n1 = force_continuity(remove_short_cuts(n1,.1));

    Piecewise<D2<SBasis> > uskeleton2 = arc_length_parametrization(bend_path2.get_pwd2(),2,.1);
    uskeleton2 = remove_short_cuts(uskeleton2,.01);
    Piecewise<D2<SBasis> > n2 = rot90(derivative(uskeleton2));
    n2 = force_continuity(remove_short_cuts(n2,.1));

    Piecewise<D2<SBasis> > uskeleton3 = arc_length_parametrization(bend_path3.get_pwd2(),2,.1);
    uskeleton3 = remove_short_cuts(uskeleton3,.01);
    Piecewise<D2<SBasis> > n3 = rot90(derivative(uskeleton3));
    n3 = force_continuity(remove_short_cuts(n3,.1));

    Piecewise<D2<SBasis> > uskeleton4 = arc_length_parametrization(bend_path4.get_pwd2(),2,.1);
    uskeleton4 = remove_short_cuts(uskeleton4,.01);
    Piecewise<D2<SBasis> > n4 = rot90(derivative(uskeleton4));
    n4 = force_continuity(remove_short_cuts(n4,.1));


    D2<Piecewise<SBasis> > patternd2 = make_cuts_independent(pwd2_in);
    Piecewise<SBasis> x = Piecewise<SBasis>(patternd2[0]);
    Piecewise<SBasis> y = Piecewise<SBasis>(patternd2[1]);

    /*The *1.001 is a hack to avoid a small bug : path at x=0 and y=0 don't work well. */
    x-= boundingbox_X.min()*1.001;
    y-= boundingbox_Y.min()*1.001;

    Piecewise<SBasis> x1 = x ;
    Piecewise<SBasis> y1 = y ;

    Piecewise<SBasis> x2 = x ;
    Piecewise<SBasis> y2 = y ;
    x2 -= boundingbox_X.extent();

    Piecewise<SBasis> x3 = x ;
    Piecewise<SBasis> y3 = y ;
    y3 -= boundingbox_Y.extent();

    Piecewise<SBasis> x4 = x ;
    Piecewise<SBasis> y4 = y ;


    /*Scaling to the Bend Path length*/
    double scaling1 = uskeleton1.cuts.back()/boundingbox_X.extent();
    if (scaling1 != 1.0) {
        x1*=scaling1;
    }

    double scaling2 = uskeleton2.cuts.back()/boundingbox_Y.extent();
    if (scaling2 != 1.0) {
        y2*=scaling2;
    }

    double scaling3 = uskeleton3.cuts.back()/boundingbox_X.extent();
    if (scaling3 != 1.0) {
        x3*=scaling3;
    }

    double scaling4 = uskeleton4.cuts.back()/boundingbox_Y.extent();
    if (scaling4 != 1.0) {
        y4*=scaling4;
    }



    Piecewise<SBasis> xbis = x;
    Piecewise<SBasis> ybis = y;
    xbis *= -1.0;
    xbis += boundingbox_X.extent();
    ybis *= -1.0;
    ybis += boundingbox_Y.extent();
    /* This is important : y + ybis = constant  and x +xbis = constant */

    Piecewise<D2<SBasis> > output;
    Piecewise<D2<SBasis> > output1;
    Piecewise<D2<SBasis> > output2;
    Piecewise<D2<SBasis> > output_x;
    Piecewise<D2<SBasis> > output_y;

    /*
    output_y : Deformation by Up and Down Bend Paths
    We use weighting : The closer a point is to a Band Path, the more it will be affected by this Bend Path.
    This is done by the line "ybis*Derformation1 + y*Deformation2"
    The result is a mix between the 2 deformed paths
    */
    output_y =  ybis*(compose((uskeleton1),x1) + y1*compose(n1,x1) )
            +    y*(compose((uskeleton3),x3) + y3*compose(n3,x3) );
    output_y /= (boundingbox_Y.extent());
    if(xx.get_value() == false && yy.get_value() == true)
    {
            return output_y;
    }

    /*output_x : Deformation by Left and Right Bend Paths*/
    output_x =    x*(compose((uskeleton2),y2) + -x2*compose(n2,y2) )
            + xbis*(compose((uskeleton4),y4) + -x4*compose(n4,y4) );
    output_x /= (boundingbox_X.extent());
    if(xx.get_value() == true && yy.get_value() == false)
    {
            return output_x;
    }

    /*output : Deformation by Up, Left, Right and Down Bend Paths*/
    if(xx.get_value() == true && yy.get_value() == true)
    {
        Piecewise<SBasis> xsqr = x*xbis; /* xsqr = x * (BBox_X - x) */
        Piecewise<SBasis> ysqr = y*ybis; /* xsqr = y * (BBox_Y - y) */
        Piecewise<SBasis> xsqrbis = xsqr;
        Piecewise<SBasis> ysqrbis = ysqr;
        xsqrbis *= -1;
        xsqrbis += boundingbox_X.extent()*boundingbox_X.extent()/4.;
        ysqrbis *= -1;
        ysqrbis += boundingbox_Y.extent()*boundingbox_Y.extent()/4.;
        /*This is important : xsqr + xsqrbis = constant*/


        /*
        Here we mix the last two results : output_x and output_y
        output1 : The more a point is close to Up and Down, the less it will be affected by output_x.
        (This is done with the polynomial function)
        output2 : The more a point is close to Left and Right, the less it will be affected by output_y.
        output : we do the mean between output1 and output2 for all points.
        */
        output1 =  (ysqrbis*output_y) + (ysqr*output_x);
        output1 /= (boundingbox_Y.extent()*boundingbox_Y.extent()/4.);

        output2 =  (xsqrbis*output_x) + (xsqr*output_y);
        output2 /= (boundingbox_X.extent()*boundingbox_X.extent()/4.);

        output = output1 + output2;
        output /= 2.;

        return output;
        /*Of course, the result is not perfect, but on a graphical point of view, this is sufficent.*/

    }

    // do nothing when xx and yy are both false
    return pwd2_in;
}

void
LPEEnvelope::resetDefaults(SPItem const* item)
{
    Effect::resetDefaults(item);

    original_bbox(SP_LPE_ITEM(item));

    Geom::Point Up_Left(boundingbox_X.min(), boundingbox_Y.min());
    Geom::Point Up_Right(boundingbox_X.max(), boundingbox_Y.min());
    Geom::Point Down_Left(boundingbox_X.min(), boundingbox_Y.max());
    Geom::Point Down_Right(boundingbox_X.max(), boundingbox_Y.max());

    Geom::Path path1;
    path1.start( Up_Left );
    path1.appendNew<Geom::LineSegment>( Up_Right );
    bend_path1.set_new_value( path1.toPwSb(), true );

    Geom::Path path2;
    path2.start( Up_Right );
    path2.appendNew<Geom::LineSegment>( Down_Right );
    bend_path2.set_new_value( path2.toPwSb(), true );

    Geom::Path path3;
    path3.start( Down_Left );
    path3.appendNew<Geom::LineSegment>( Down_Right );
    bend_path3.set_new_value( path3.toPwSb(), true );

    Geom::Path path4;
    path4.start( Up_Left );
    path4.appendNew<Geom::LineSegment>( Down_Left );
    bend_path4.set_new_value( path4.toPwSb(), true );
}


} // namespace LivePathEffect
} /* namespace Inkscape */

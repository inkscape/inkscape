#define INKSCAPE_LPE_POWERSTROKE_CPP
/** \file
 * @brief  PowerStroke LPE implementation. Creates curves with modifiable stroke width.
 */
/* Authors:
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *
 * Copyright (C) 2010 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-powerstroke.h"

#include "sp-shape.h"
#include "display/curve.h"

#include <2geom/path.h>
#include <2geom/piecewise.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/svg-elliptical-arc.h>
#include <2geom/transforms.h>

namespace Inkscape {
namespace LivePathEffect {

LPEPowerStroke::LPEPowerStroke(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    offset_points(_("Offset points"), _("Offset points"), "offset_points", &wr, this),
    sort_points(_("Sort points"), _("Sort offset points according to their time value along the curve."), "sort_points", &wr, this, true)
{
    show_orig_path = true;

    registerParameter( dynamic_cast<Parameter *>(&offset_points) );
    registerParameter( dynamic_cast<Parameter *>(&sort_points) );
}

LPEPowerStroke::~LPEPowerStroke()
{

}


void
LPEPowerStroke::doOnApply(SPLPEItem *lpeitem)
{
    std::vector<Geom::Point> points;
    points.push_back( *(SP_SHAPE(lpeitem)->curve->first_point()) );
    Geom::Path const *path = SP_SHAPE(lpeitem)->curve->first_path();
    points.push_back( path->pointAt(path->size()/2) );
    points.push_back( *(SP_SHAPE(lpeitem)->curve->last_point()) );
    offset_points.param_set_and_write_new_value(points);
}

static void append_half_circle(Geom::Piecewise<Geom::D2<Geom::SBasis> > &pwd2,
                               Geom::Point const center, Geom::Point const &dir) {
    using namespace Geom;

    double r = L2(dir);
    SVGEllipticalArc cap(center + dir, r, r, angle_between(Point(1,0), dir), false, false, center - dir);
    Piecewise<D2<SBasis> > cap_pwd2(cap.toSBasis());
    pwd2.continuousConcat(cap_pwd2);
}

static bool compare_offsets (Geom::Point first, Geom::Point second)
{
    return first[Geom::X] <= second[Geom::X];
}


Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEPowerStroke::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    // perhaps use std::list instead of std::vector?
    std::vector<Geom::Point> ts(offset_points.data().size());

    for (unsigned int i; i < ts.size(); ++i) {
        double t = nearest_point(offset_points.data().at(i), pwd2_in);
        double offset = L2(pwd2_in.valueAt(t) - offset_points.data().at(i));
        ts.at(i) = Geom::Point(t, offset);
    }
    if (sort_points) {
        sort(ts.begin(), ts.end(), compare_offsets);
    }

    // create stroke path where points (x,y) = (t, offset)
    Path strokepath;
    strokepath.start( Point(pwd2_in.domain().min(),0) );
    for (unsigned int i = 0 ; i < ts.size(); ++i) {
        strokepath.appendNew<Geom::LineSegment>(ts.at(i));
    }
    strokepath.appendNew<Geom::LineSegment>( Point(pwd2_in.domain().max(), 0) );
    for (unsigned int i = 0; i < ts.size(); ++i) {
        Geom::Point temp = ts.at(ts.size() - 1 - i);
        strokepath.appendNew<Geom::LineSegment>( Geom::Point(temp[X], - temp[Y]) );
    }
    strokepath.close();

    D2<Piecewise<SBasis> > patternd2 = make_cuts_independent(strokepath.toPwSb());
    Piecewise<SBasis> x = Piecewise<SBasis>(patternd2[0]);
    Piecewise<SBasis> y = Piecewise<SBasis>(patternd2[1]);

    Piecewise<D2<SBasis> > der = unitVector(derivative(pwd2_in));
    Piecewise<D2<SBasis> > n   = rot90(der);

//    output  = pwd2_in + n * offset;
//    append_half_circle(output, pwd2_in.lastValue(), n.lastValue() * offset);
//    output.continuousConcat(reverse(pwd2_in - n * offset));
//    append_half_circle(output, pwd2_in.firstValue(), -n.firstValue() * offset);

    Piecewise<D2<SBasis> > output = compose(pwd2_in,x) + y*compose(n,x);
    return output;
}

/* ######################## */

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

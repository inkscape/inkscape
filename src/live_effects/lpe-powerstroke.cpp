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
    offset_1(_("Start Offset"), _("Handle to control the distance of the offset from the curve"), "offset_1", &wr, this),
    offset_2(_("End Offset"), _("Handle to control the distance of the offset from the curve"), "offset_2", &wr, this),
    offset_3(_("End Offset"), _("Handle to control the distance of the offset from the curve"), "offset_3", &wr, this)
//    offset_points(_("Offset points"), _("Offset points"), "offset_points", &wr, this)
{
    show_orig_path = true;

    registerParameter( dynamic_cast<Parameter *>(&offset_1) );
    registerParameter( dynamic_cast<Parameter *>(&offset_2) );
    registerParameter( dynamic_cast<Parameter *>(&offset_3) );
//    registerParameter( dynamic_cast<Parameter *>(&offset_points) );

    /* register all your knotholder handles here: */
    //registerKnotHolderHandle(new PowerStroke::KnotHolderEntityAttachMyHandle(), _("help message"));
}

LPEPowerStroke::~LPEPowerStroke()
{

}


void
LPEPowerStroke::doOnApply(SPLPEItem *lpeitem)
{
    offset_1.param_set_and_write_new_value(*(SP_SHAPE(lpeitem)->curve->first_point()));
    offset_2.param_set_and_write_new_value(*(SP_SHAPE(lpeitem)->curve->last_point()));
    offset_3.param_set_and_write_new_value(*(SP_SHAPE(lpeitem)->curve->last_point()));
}

static void append_half_circle(Geom::Piecewise<Geom::D2<Geom::SBasis> > &pwd2,
                               Geom::Point const center, Geom::Point const &dir) {
    using namespace Geom;

    double r = L2(dir);
    SVGEllipticalArc cap(center + dir, r, r, angle_between(Point(1,0), dir), false, false, center - dir);
    Piecewise<D2<SBasis> > cap_pwd2(cap.toSBasis());
    pwd2.continuousConcat(cap_pwd2);
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEPowerStroke::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    double t1 = nearest_point(offset_1, pwd2_in);
    double offset1 = L2(pwd2_in.valueAt(t1) - offset_1);

    double t2 = nearest_point(offset_2, pwd2_in);
    double offset2 = L2(pwd2_in.valueAt(t2) - offset_2);

    double t3 = nearest_point(offset_3, pwd2_in);
    double offset3 = L2(pwd2_in.valueAt(t3) - offset_3);

    /*
    unsigned number_of_points = offset_points.data.size();
    double* t = new double[number_of_points];
    Point*  offset = new double[number_of_points];
    for (unsigned i = 0; i < number_of_points; ++i) {
        t[i] = nearest_point(offset_points.data[i], pwd2_in);
        Point A = pwd2_in.valueAt(t[i]);
        offset[i] = L2(A - offset_points.data[i]);
    }
    */

    // create stroke path where points (x,y) = (t, offset)
    Path strokepath;
    strokepath.start( Point(pwd2_in.domain().min(),0) );
    strokepath.appendNew<Geom::LineSegment>( Point(t1, offset1) );
    strokepath.appendNew<Geom::LineSegment>( Point(t2, offset2) );
    strokepath.appendNew<Geom::LineSegment>( Point(t3, offset3) );
    strokepath.appendNew<Geom::LineSegment>( Point(pwd2_in.domain().max(), 0) );
    strokepath.appendNew<Geom::LineSegment>( Point(t3, -offset3) );
    strokepath.appendNew<Geom::LineSegment>( Point(t2, -offset2) );
    strokepath.appendNew<Geom::LineSegment>( Point(t1, -offset1) );
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


/* ########################
 *  Define the classes for your knotholder handles here
 */

/*
namespace PowerStroke {

class KnotHolderEntityMyHandle : public LPEKnotHolderEntity
{
public:
    // the set() and get() methods must be implemented, click() is optional
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get();
    //virtual void knot_click(guint state);
};

} // namespace PowerStroke
*/

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

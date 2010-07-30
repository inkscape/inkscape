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
#include <2geom/transforms.h>
#include <2geom/bezier-utils.h>


/// @TODO  Move this to 2geom
namespace Geom {
namespace Interpolate {

class Interpolator {
public:
    Interpolator() {};
    virtual ~Interpolator() {};

//    virtual Piecewise<D2<SBasis> > interpolateToPwD2Sb(std::vector<Point> points) = 0;
    virtual Path interpolateToPath(std::vector<Point> points) = 0;

private:
    Interpolator(const Interpolator&);
    Interpolator& operator=(const Interpolator&);
};

class Linear : public Interpolator {
public:
    Linear() {};
    virtual ~Linear() {};

    virtual Path interpolateToPath(std::vector<Point> points) {
        Path path;
        path.start( points.at(0) );
        for (unsigned int i = 1 ; i < points.size(); ++i) {
            path.appendNew<Geom::LineSegment>(points.at(i));
        }
        return path;
    };

private:
    Linear(const Linear&);
    Linear& operator=(const Linear&);
};

// this class is terrible
class CubicBezierFit : public Interpolator {
public:
    CubicBezierFit() {};
    virtual ~CubicBezierFit() {};

    virtual Path interpolateToPath(std::vector<Point> points) {
        unsigned int n_points = points.size();
        // worst case gives us 2 segment per point
        int max_segs = 8*n_points;
        Geom::Point * b = g_new(Geom::Point, max_segs);
        Geom::Point * points_array = g_new(Geom::Point, 4*n_points);
        for (unsigned i = 0; i < n_points; ++i) {
            points_array[i] = points.at(i);
        }

        double tolerance_sq = 0; // this value is just a random guess

        int const n_segs = Geom::bezier_fit_cubic_r(b, points_array, n_points,
                                                 tolerance_sq, max_segs);

        Geom::Path fit;
        if ( n_segs > 0)
        {
            fit.start(b[0]);
            for (int c = 0; c < n_segs; c++) {
                fit.appendNew<Geom::CubicBezier>(b[4*c+1], b[4*c+2], b[4*c+3]);
            }
        }
        g_free(b);
        g_free(points_array);
        return fit;
    };

private:
    CubicBezierFit(const CubicBezierFit&);
    CubicBezierFit& operator=(const CubicBezierFit&);
};

/// @todo invent name for this class
class CubicBezierJohan : public Interpolator {
public:
    CubicBezierJohan() {};
    virtual ~CubicBezierJohan() {};

    virtual Path interpolateToPath(std::vector<Point> points) {
        Path fit;
        fit.start(points.at(0));
        for (unsigned int i = 1; i < points.size(); ++i) {
            Point p0 = points.at(i-1);
            Point p1 = points.at(i);
            Point dx = Point(p1[X] - p0[X], 0);
            fit.appendNew<CubicBezier>(p0+0.2*dx, p1-0.2*dx, p1);
        }
        return fit;
    };

private:
    CubicBezierJohan(const CubicBezierJohan&);
    CubicBezierJohan& operator=(const CubicBezierJohan&);
};

} //namespace Interpolate
} //namespace Geom

namespace Inkscape {
namespace LivePathEffect {

LPEPowerStroke::LPEPowerStroke(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    offset_points(_("Offset points"), _("Offset points"), "offset_points", &wr, this),
    sort_points(_("Sort points"), _("Sort offset points according to their time value along the curve."), "sort_points", &wr, this, true)
{
    show_orig_path = true;

    /// @todo offset_points are initialized with empty path, is that bug-save?

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
    Geom::Path::size_type size = SP_SHAPE(lpeitem)->curve->get_pathvector().front().size_open();
    points.push_back( Geom::Point(0,0) );
    points.push_back( Geom::Point(0.5*size,0) );
    points.push_back( Geom::Point(size,0) );
    offset_points.param_set_and_write_new_value(points);
}

static bool compare_offsets (Geom::Point first, Geom::Point second)
{
    return first[Geom::X] < second[Geom::X];
}


Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEPowerStroke::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    offset_points.set_pwd2(pwd2_in);

    // perhaps use std::list instead of std::vector?
    std::vector<Geom::Point> ts(offset_points.data().size() + 2);
    // first and last point coincide with input path (for now at least)
    ts.front() = Point(pwd2_in.domain().min(),0);
    ts.back()  = Point(pwd2_in.domain().max(),0);
    for (unsigned int i = 0; i < offset_points.data().size(); ++i) {
        ts.at(i+1) = offset_points.data().at(i);
    }

    if (sort_points) {
        sort(ts.begin(), ts.end(), compare_offsets);
    }

    // create stroke path where points (x,y) = (t, offset)
    Geom::Interpolate::CubicBezierJohan interpolator;
    Path strokepath = interpolator.interpolateToPath(ts);
    Path mirroredpath = strokepath.reverse() * Geom::Scale(1,-1);
    strokepath.append(mirroredpath, Geom::Path::STITCH_DISCONTINUOUS);
    strokepath.close();

    D2<Piecewise<SBasis> > patternd2 = make_cuts_independent(strokepath.toPwSb());
    Piecewise<SBasis> x = Piecewise<SBasis>(patternd2[0]);
    Piecewise<SBasis> y = Piecewise<SBasis>(patternd2[1]);

    Piecewise<D2<SBasis> > der = unitVector(derivative(pwd2_in));
    Piecewise<D2<SBasis> > n = rot90(der);
    offset_points.set_pwd2_normal(n);

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

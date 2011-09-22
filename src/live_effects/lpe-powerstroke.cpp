#define INKSCAPE_LPE_POWERSTROKE_CPP
/** \file
 * @brief  PowerStroke LPE implementation. Creates curves with modifiable stroke width.
 */
/* Authors:
 *   Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Copyright (C) 2010-2011 Authors
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
#include <2geom/svg-elliptical-arc.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/svg-path.h>

// for the spiro interpolator:
#include "live_effects/bezctx.h"
#include "live_effects/bezctx_intf.h"
#include "live_effects/spiro.h"


/// @TODO  Move this to 2geom
namespace Geom {
namespace Interpolate {

enum InterpolatorType {
  INTERP_LINEAR,
  INTERP_CUBICBEZIER,
  INTERP_CUBICBEZIER_JOHAN,
  INTERP_SPIRO
};

class Interpolator {
public:
    Interpolator() {};
    virtual ~Interpolator() {};

    static Interpolator* create(InterpolatorType type);

//    virtual Piecewise<D2<SBasis> > interpolateToPwD2Sb(std::vector<Point> points) = 0;
    virtual Geom::Path interpolateToPath(std::vector<Point> points) = 0;

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


#define SPIRO_SHOW_INFINITE_COORDINATE_CALLS
class SpiroInterpolator : public Interpolator {
public:
    SpiroInterpolator() {};
    virtual ~SpiroInterpolator() {};

    virtual Path interpolateToPath(std::vector<Point> points) {
        Path fit;

        Coord scale_y = 100.;

        guint len = points.size();
        bezctx *bc = new_bezctx_ink(&fit);
        spiro_cp *controlpoints = g_new (spiro_cp, len);
        for (unsigned int i = 0; i < len; ++i) {
            controlpoints[i].x = points[i][X];
            controlpoints[i].y = points[i][Y] / scale_y;
            controlpoints[i].ty = 'c';
        }
        controlpoints[0].ty = '{';
        controlpoints[1].ty = 'v';
        controlpoints[len-2].ty = 'v';
        controlpoints[len-1].ty = '}';

        spiro_seg *s = run_spiro(controlpoints, len);
        spiro_to_bpath(s, len, bc);
        free(s);
        free(bc);

        fit *= Scale(1,scale_y);
        return fit;
    };

private:
    typedef struct {
        bezctx base;
        Path *path;
        int is_open;
    } bezctx_ink;

    static void bezctx_ink_moveto(bezctx *bc, double x, double y, int /*is_open*/)
    {
        bezctx_ink *bi = (bezctx_ink *) bc;
        if ( IS_FINITE(x) && IS_FINITE(y) ) {
            bi->path->start(Point(x, y));
        }
    #ifdef SPIRO_SHOW_INFINITE_COORDINATE_CALLS
        else {
            g_message("spiro moveto not finite");
        }
    #endif
    }

    static void bezctx_ink_lineto(bezctx *bc, double x, double y)
    {
        bezctx_ink *bi = (bezctx_ink *) bc;
        if ( IS_FINITE(x) && IS_FINITE(y) ) {
            bi->path->appendNew<LineSegment>( Point(x, y) );
        }
    #ifdef SPIRO_SHOW_INFINITE_COORDINATE_CALLS
        else {
            g_message("spiro lineto not finite");
        }
    #endif
    }

    static void bezctx_ink_quadto(bezctx *bc, double xm, double ym, double x3, double y3)
    {
        bezctx_ink *bi = (bezctx_ink *) bc;

        if ( IS_FINITE(xm) && IS_FINITE(ym) && IS_FINITE(x3) && IS_FINITE(y3) ) {
            bi->path->appendNew<QuadraticBezier>(Point(xm, ym), Point(x3, y3));
        }
    #ifdef SPIRO_SHOW_INFINITE_COORDINATE_CALLS
        else {
            g_message("spiro quadto not finite");
        }
    #endif
    }

    static void bezctx_ink_curveto(bezctx *bc, double x1, double y1, double x2, double y2,
                double x3, double y3)
    {
        bezctx_ink *bi = (bezctx_ink *) bc;
        if ( IS_FINITE(x1) && IS_FINITE(y1) && IS_FINITE(x2) && IS_FINITE(y2) ) {
            bi->path->appendNew<CubicBezier>(Point(x1, y1), Point(x2, y2), Point(x3, y3));
        }
    #ifdef SPIRO_SHOW_INFINITE_COORDINATE_CALLS
        else {
            g_message("spiro curveto not finite");
        }
    #endif
    }

    bezctx *
    new_bezctx_ink(Geom::Path *path) {
        bezctx_ink *result = g_new(bezctx_ink, 1);
        result->base.moveto = bezctx_ink_moveto;
        result->base.lineto = bezctx_ink_lineto;
        result->base.quadto = bezctx_ink_quadto;
        result->base.curveto = bezctx_ink_curveto;
        result->base.mark_knot = NULL;
        result->path = path;
        return &result->base;
    }

    SpiroInterpolator(const SpiroInterpolator&);
    SpiroInterpolator& operator=(const SpiroInterpolator&);
};


Interpolator*
Interpolator::create(InterpolatorType type) {
    switch (type) {
      case INTERP_LINEAR:
        return new Geom::Interpolate::Linear();
      case INTERP_CUBICBEZIER:
        return new Geom::Interpolate::CubicBezierFit();
      case INTERP_CUBICBEZIER_JOHAN:
        return new Geom::Interpolate::CubicBezierJohan();
      case INTERP_SPIRO:
        return new Geom::Interpolate::SpiroInterpolator();
      default:
        return new Geom::Interpolate::Linear();
    }
}

} //namespace Interpolate
} //namespace Geom

namespace Inkscape {
namespace LivePathEffect {

static const Util::EnumData<unsigned> InterpolatorTypeData[] = {
    {Geom::Interpolate::INTERP_LINEAR          , N_("Linear"), "Linear"},
    {Geom::Interpolate::INTERP_CUBICBEZIER          , N_("CubicBezierFit"), "CubicBezierFit"},
    {Geom::Interpolate::INTERP_CUBICBEZIER_JOHAN     , N_("CubicBezierJohan"), "CubicBezierJohan"},
    {Geom::Interpolate::INTERP_SPIRO  , N_("SpiroInterpolator"), "SpiroInterpolator"}
};
static const Util::EnumDataConverter<unsigned> InterpolatorTypeConverter(InterpolatorTypeData, sizeof(InterpolatorTypeData)/sizeof(*InterpolatorTypeData));

enum LineCapType {
  LINECAP_BUTT,
  LINECAP_SQUARE,
  LINECAP_ROUND,
  LINECAP_PEAK
};
static const Util::EnumData<unsigned> LineCapTypeData[] = {
    {LINECAP_BUTT   ,  N_("Butt"),  "butt"},
    {LINECAP_SQUARE,  N_("Square"),  "square"},
    {LINECAP_ROUND  ,  N_("Round"), "round"},
    {LINECAP_PEAK  , N_("Peak"), "peak"}
};
static const Util::EnumDataConverter<unsigned> LineCapTypeConverter(LineCapTypeData, sizeof(LineCapTypeData)/sizeof(*LineCapTypeData));

enum LineCuspType {
  LINECUSP_BEVEL,
  LINECUSP_ROUND,
  LINECUSP_SHARP
};
static const Util::EnumData<unsigned> LineCuspTypeData[] = {
    {LINECUSP_BEVEL ,  N_("Beveled"),  "bevel"},
    {LINECUSP_ROUND ,  N_("Rounded"), "round"},
    {LINECUSP_SHARP  , N_("Sharp"), "sharp"}
};
static const Util::EnumDataConverter<unsigned> LineCuspTypeConverter(LineCuspTypeData, sizeof(LineCuspTypeData)/sizeof(*LineCuspTypeData));

LPEPowerStroke::LPEPowerStroke(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    offset_points(_("Offset points"), _("Offset points"), "offset_points", &wr, this),
    sort_points(_("Sort points"), _("Sort offset points according to their time value along the curve."), "sort_points", &wr, this, true),
    interpolator_type(_("Interpolator type"), _("Determines which kind of interpolator will be used to interpolate between stroke width along the path."), "interpolator_type", InterpolatorTypeConverter, &wr, this, Geom::Interpolate::INTERP_CUBICBEZIER_JOHAN),
    start_linecap_type(_("Start line cap type"), _("Determines the shape of the path's start."), "start_linecap_type", LineCapTypeConverter, &wr, this, LINECAP_ROUND),
    cusp_linecap_type(_("Cusp line cap type"), _("Determines the shape of the cusps along the path."), "cusp_linecap_type", LineCuspTypeConverter, &wr, this, LINECUSP_ROUND),
    end_linecap_type(_("End line cap type"), _("Determines the shape of the path's end."), "end_linecap_type", LineCapTypeConverter, &wr, this, LINECAP_ROUND)
{
    show_orig_path = true;

    /// @todo offset_points are initialized with empty path, is that bug-save?

    registerParameter( dynamic_cast<Parameter *>(&offset_points) );
    registerParameter( dynamic_cast<Parameter *>(&sort_points) );
    registerParameter( dynamic_cast<Parameter *>(&interpolator_type) );
    registerParameter( dynamic_cast<Parameter *>(&start_linecap_type) );
    registerParameter( dynamic_cast<Parameter *>(&cusp_linecap_type) );
    registerParameter( dynamic_cast<Parameter *>(&end_linecap_type) );
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

    // find discontinuities in piecewise
std::vector<unsigned> find_discontinuities(Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in, double eps=Geom::EPSILON)
{
    std::vector<unsigned> indices;
    for(unsigned i = 1; i < pwd2_in.size(); i++) {
        if ( ! are_near(pwd2_in[i-1].at1(), pwd2_in[i].at0(), eps) ) {
            indices.push_back(i);
        }
    }
    return indices;
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEPowerStroke::doEffect_pwd2_open ( Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in,
                                     Geom::Piecewise<Geom::D2<Geom::SBasis> > const & der,
                                     Geom::Piecewise<Geom::D2<Geom::SBasis> > const & n )
{
    using namespace Geom;

    Piecewise<D2<SBasis> > output;

    LineCapType start_linecap = static_cast<LineCapType>(start_linecap_type.get_value());
    LineCapType end_linecap = static_cast<LineCapType>(end_linecap_type.get_value());

    // perhaps use std::list instead of std::vector?
    std::vector<Geom::Point> ts(offset_points.data().size() + 2);
    for (unsigned int i = 0; i < offset_points.data().size(); ++i) {
        ts.at(i+1) = offset_points.data().at(i);
    }
    if (sort_points) {
        sort(ts.begin()+1, ts.end()-1, compare_offsets);
    }

    // first and last point have same distance from path as second and second to last points, respectively.
    ts.front() = Point(pwd2_in.domain().min(), (*(ts.begin()+1))[Geom::Y] );
    ts.back()  = Point(pwd2_in.domain().max(), (*(ts.end()-2))[Geom::Y] );

    // create stroke path where points (x,y) := (t, offset)
    Geom::Interpolate::Interpolator *interpolator = Geom::Interpolate::Interpolator::create(static_cast<Geom::Interpolate::InterpolatorType>(interpolator_type.get_value()));
    Geom::Path strokepath = interpolator->interpolateToPath(ts);
    delete interpolator;

    D2<Piecewise<SBasis> > patternd2 = make_cuts_independent(strokepath.toPwSb());
    Piecewise<SBasis> x = Piecewise<SBasis>(patternd2[0]);
    Piecewise<SBasis> y = Piecewise<SBasis>(patternd2[1]);

    // find time values for which x lies outside path domain
    // and only take portion of x and y that lies within those time values
    std::vector< double > rtsmin = roots (x - pwd2_in.domain().min());
    std::vector< double > rtsmax = roots (x - pwd2_in.domain().max());
    if ( !rtsmin.empty() && !rtsmax.empty() ) {
        x = portion(x, rtsmin.at(0), rtsmax.at(0));
        y = portion(y, rtsmin.at(0), rtsmax.at(0));
    }

    output = compose(pwd2_in,x) + y*compose(n,x);

    x = reverse(x);
    y = reverse(y);
    Piecewise<D2<SBasis> > mirrorpath = compose(pwd2_in,x) - y*compose(n,x);

    switch (end_linecap) {
        case LINECAP_PEAK:
        {
            Geom::Point end_deriv = der.lastValue();
            double radius = 0.5 * distance(output.lastValue(), mirrorpath.firstValue());
            Geom::Point midpoint = 0.5*(output.lastValue() + mirrorpath.firstValue()) + radius*end_deriv;
            Geom::LineSegment cap11(output.lastValue(), midpoint);
            Geom::LineSegment cap12(midpoint, mirrorpath.firstValue());
            output.continuousConcat(Piecewise<D2<SBasis> >(cap11.toSBasis()));
            output.continuousConcat(Piecewise<D2<SBasis> >(cap12.toSBasis()));
            break;
        }
        case LINECAP_SQUARE:
        {
            Geom::Point end_deriv = der.lastValue();
            double radius = 0.5 * distance(output.lastValue(), mirrorpath.firstValue());
            Geom::LineSegment cap11(output.lastValue(), output.lastValue() + radius*end_deriv);
            Geom::LineSegment cap12(output.lastValue() + radius*end_deriv, mirrorpath.firstValue() + radius*end_deriv);
            Geom::LineSegment cap13(mirrorpath.firstValue() + radius*end_deriv, mirrorpath.firstValue());
            output.continuousConcat(Piecewise<D2<SBasis> >(cap11.toSBasis()));
            output.continuousConcat(Piecewise<D2<SBasis> >(cap12.toSBasis()));
            output.continuousConcat(Piecewise<D2<SBasis> >(cap13.toSBasis()));
            break;
        }
        case LINECAP_BUTT:
        {
            Geom::LineSegment cap1(output.lastValue(), mirrorpath.firstValue());
            output.continuousConcat(Piecewise<D2<SBasis> >(cap1.toSBasis()));
            break;
        }
        case LINECAP_ROUND:
        default:
        {
            double radius1 = 0.5 * distance(output.lastValue(), mirrorpath.firstValue());
            Geom::SVGEllipticalArc cap1(output.lastValue(), radius1, radius1, M_PI/2., false, y.firstValue() < 0, mirrorpath.firstValue()); // note that y is reversed above!
            output.continuousConcat(Piecewise<D2<SBasis> >(cap1.toSBasis()));
            break;
        }
    }

    output.continuousConcat(mirrorpath);

    switch (start_linecap) {
        case LINECAP_PEAK:
        {
            Geom::Point start_deriv = der.firstValue();
            double radius = 0.5 * distance(output.firstValue(), output.lastValue());
            Geom::Point midpoint = 0.5*(output.lastValue() + output.firstValue()) - radius*start_deriv;
            Geom::LineSegment cap21(output.lastValue(), midpoint);
            Geom::LineSegment cap22(midpoint, output.firstValue());
            output.continuousConcat(Piecewise<D2<SBasis> >(cap21.toSBasis()));
            output.continuousConcat(Piecewise<D2<SBasis> >(cap22.toSBasis()));
            break;
        }
        case LINECAP_SQUARE:
        {
            Geom::Point start_deriv = der.firstValue();
            double radius = 0.5 * distance(output.firstValue(), output.lastValue());
            Geom::LineSegment cap21(output.lastValue(), output.lastValue() - radius*start_deriv);
            Geom::LineSegment cap22(output.lastValue() - radius*start_deriv, output.firstValue() - radius*start_deriv);
            Geom::LineSegment cap23(output.firstValue() - radius*start_deriv, output.firstValue());
            output.continuousConcat(Piecewise<D2<SBasis> >(cap21.toSBasis()));
            output.continuousConcat(Piecewise<D2<SBasis> >(cap22.toSBasis()));
            output.continuousConcat(Piecewise<D2<SBasis> >(cap23.toSBasis()));
            break;
        }
        case LINECAP_BUTT:
        {
            Geom::LineSegment cap2(output.lastValue(), output.firstValue());
            output.continuousConcat(Piecewise<D2<SBasis> >(cap2.toSBasis()));
            break;
        }
        case LINECAP_ROUND:
        default:
        {
            double radius2 = 0.5 * distance(output.firstValue(), output.lastValue());
            Geom::SVGEllipticalArc cap2(output.lastValue(), radius2, radius2, M_PI/2., false, y.lastValue() < 0, output.firstValue()); // note that y is reversed above!
            output.continuousConcat(Piecewise<D2<SBasis> >(cap2.toSBasis()));
            break;
        }
    }

    return output;
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEPowerStroke::doEffect_pwd2_closed ( Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in,
                                       Geom::Piecewise<Geom::D2<Geom::SBasis> > const & /*der*/,
                                       Geom::Piecewise<Geom::D2<Geom::SBasis> > const & n )
{
    using namespace Geom;

    Piecewise<D2<SBasis> > output;

    // path is closed
    // linecap parameter can be ignored

    // perhaps use std::list instead of std::vector?
    std::vector<Geom::Point> ts = offset_points.data();
    if (sort_points) {
        sort(ts.begin(), ts.end(), compare_offsets);
    }
    // add extra points for interpolation between first and last point
    Point first_point = ts.front();
    Point last_point = ts.back();
    ts.insert(ts.begin(), last_point - Point(pwd2_in.domain().extent() ,0));
    ts.push_back( first_point + Point(pwd2_in.domain().extent() ,0) );
    // create stroke path where points (x,y) := (t, offset)
    Geom::Interpolate::Interpolator *interpolator = Geom::Interpolate::Interpolator::create(static_cast<Geom::Interpolate::InterpolatorType>(interpolator_type.get_value()));
    Geom::Path strokepath = interpolator->interpolateToPath(ts);
    delete interpolator;

    // output 2 separate paths
    D2<Piecewise<SBasis> > patternd2 = make_cuts_independent(strokepath.toPwSb());
    Piecewise<SBasis> x = Piecewise<SBasis>(patternd2[0]);
    Piecewise<SBasis> y = Piecewise<SBasis>(patternd2[1]);
    // find time values for which x lies outside path domain
    // and only take portion of x and y that lies within those time values
    std::vector< double > rtsmin = roots (x - pwd2_in.domain().min());
    std::vector< double > rtsmax = roots (x - pwd2_in.domain().max());
    if ( !rtsmin.empty() && !rtsmax.empty() ) {
        x = portion(x, rtsmin.at(0), rtsmax.at(0));
        y = portion(y, rtsmin.at(0), rtsmax.at(0));
    }
    output = compose(pwd2_in,x) + y*compose(n,x);
    x = reverse(x);
    y = reverse(y);
    output.concat(compose(pwd2_in,x) - y*compose(n,x));

    return output;
}

std::vector<Geom::Path>
LPEPowerStroke::doEffect_path (std::vector<Geom::Path> const & path_in)
{
    using namespace Geom;

    std::vector<Geom::Path> path_out;

    for (unsigned int i=0; i < path_in.size(); i++) {
        Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_in = path_in[i].toPwSb();

        offset_points.set_pwd2(pwd2_in);
        Piecewise<D2<SBasis> > der = unitVector(derivative(pwd2_in));
        Piecewise<D2<SBasis> > n = rot90(der);
        offset_points.set_pwd2_normal(n);

        Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_out;
        if (path_in[i].closed()) {
            pwd2_out = doEffect_pwd2_closed(pwd2_in, der, n);
        } else {
            pwd2_out = doEffect_pwd2_open(pwd2_in, der, n);
        }
        
        std::vector<Geom::Path> path = path_from_piecewise_fix_cusps( pwd2_out, LPE_CONVERSION_TOLERANCE);
        // add the output path vector to the already accumulated vector:
        for (unsigned int j=0; j < path.size(); j++) {
            path_out.push_back(path[j]);
        }
    }

    return path_out;
}

std::vector<Geom::Path>
LPEPowerStroke::path_from_piecewise_fix_cusps(Geom::Piecewise<Geom::D2<Geom::SBasis> > const &B, double tol) {

/* per definition, the input piecewise should be closed. each discontinuity should be fixed with a cusp-ending,
   as defined by cusp_linecap_type
*/
    Geom::PathBuilder pb;
    if(B.size() == 0) return pb.peek();
    Geom::Point start = B[0].at0();
    pb.moveTo(start);
    for(unsigned i = 0; ; i++) {
        if ( (i+1 == B.size()) 
             || !are_near(B[i+1].at0(), B[i].at1(), tol) )
        {
            //start of a new path
            if (are_near(start, B[i].at1()) && sbasis_size(B[i]) <= 1) {
                pb.closePath();
                //last line seg already there (because of .closePath())
                goto no_add;
            }
            build_from_sbasis(pb, B[i], tol, false);
            if (are_near(start, B[i].at1())) {
                //it's closed, the last closing segment was not a straight line so it needed to be added, but still make it closed here with degenerate straight line.
                pb.closePath();
            }
          no_add:
            if (i+1 >= B.size()) {
                break;
            }
            start = B[i+1].at0();
            pb.moveTo(start);
        } else {
            build_from_sbasis(pb, B[i], tol, false);
        }
    }
    pb.finish();
    return pb.peek();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

/**
 * @file
 * PowerStroke LPE implementation. Creates curves with modifiable stroke width.
 */
/* Authors:
 *   Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Copyright (C) 2010-2011 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-powerstroke.h"
#include "live_effects/lpe-powerstroke-interpolators.h"

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
// not yet supported    {LINECUSP_SHARP  , N_("Sharp"), "sharp"}
};
static const Util::EnumDataConverter<unsigned> LineCuspTypeConverter(LineCuspTypeData, sizeof(LineCuspTypeData)/sizeof(*LineCuspTypeData));

LPEPowerStroke::LPEPowerStroke(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    offset_points(_("Offset points"), _("Offset points"), "offset_points", &wr, this),
    sort_points(_("Sort points"), _("Sort offset points according to their time value along the curve."), "sort_points", &wr, this, true),
    interpolator_type(_("Interpolator type"), _("Determines which kind of interpolator will be used to interpolate between stroke width along the path."), "interpolator_type", InterpolatorTypeConverter, &wr, this, Geom::Interpolate::INTERP_CUBICBEZIER_JOHAN),
    interpolator_beta(_("Smoothness"), _("Sets the smoothness for the CubicBezierJohan interpolator. 0 = linear interpolation, 1 = smooth"), "interpolator_beta", &wr, this, 0.2),
    start_linecap_type(_("Start line cap type"), _("Determines the shape of the path's start."), "start_linecap_type", LineCapTypeConverter, &wr, this, LINECAP_ROUND),
    cusp_linecap_type(_("Cusp line cap type"), _("Determines the shape of the cusps along the path."), "cusp_linecap_type", LineCuspTypeConverter, &wr, this, LINECUSP_ROUND),
    end_linecap_type(_("End line cap type"), _("Determines the shape of the path's end."), "end_linecap_type", LineCapTypeConverter, &wr, this, LINECAP_ROUND)
{
    show_orig_path = true;

    /// @todo offset_points are initialized with empty path, is that bug-save?

    interpolator_beta.addSlider(true);
    interpolator_beta.param_set_range(0.,1.);

    registerParameter( dynamic_cast<Parameter *>(&offset_points) );
    registerParameter( dynamic_cast<Parameter *>(&sort_points) );
    registerParameter( dynamic_cast<Parameter *>(&interpolator_type) );
    registerParameter( dynamic_cast<Parameter *>(&interpolator_beta) );
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

void
LPEPowerStroke::adjustForNewPath(std::vector<Geom::Path> const & path_in)
{
    offset_points.recalculate_controlpoints_for_new_pwd2(path_in[0].toPwSb());
}

static bool compare_offsets (Geom::Point first, Geom::Point second)
{
    return first[Geom::X] < second[Geom::X];
}

// find discontinuities in input path
struct discontinuity_data {
    Geom::Point der0; // unit derivative of 'left' side of cusp
    Geom::Point der1; // unit derivative of 'right' side of cusp
    double width; // intended stroke width at cusp
};
std::vector<discontinuity_data> find_discontinuities( Geom::Piecewise<Geom::D2<Geom::SBasis> > const & der,
                                                      Geom::Piecewise<Geom::SBasis> const & x,
                                                      Geom::Piecewise<Geom::SBasis> const & y,
                                                      double eps=Geom::EPSILON )
{
    std::vector<discontinuity_data> vect;
    for(unsigned i = 1; i < der.size(); i++) {
        if ( ! are_near(der[i-1].at1(), der[i].at0(), eps) ) {
            discontinuity_data data;
            data.der0 = der[i-1].at1();
            data.der1 = der[i].at0();
            double t = der.cuts[i];
            std::vector< double > rts = roots (x - t);  /// @todo this has multiple solutions for general strokewidth paths (generated by spiro interpolator...), ignore for now
            if (!rts.empty()) {
                data.width = y(rts.front());
            } else {
                data.width = 1;
            }
            vect.push_back(data);
        }
    }
    return vect;
}


Geom::Path path_from_piecewise_fix_cusps( Geom::Piecewise<Geom::D2<Geom::SBasis> > const & B,
                                          std::vector<discontinuity_data> const & cusps,
                                          LineCuspType cusp_linecap,
                                          double tol=Geom::EPSILON)
{
/* per definition, each discontinuity should be fixed with a cusp-ending, as defined by cusp_linecap_type
*/
    Geom::PathBuilder pb;
    if (B.size() == 0) {
        return pb.peek().front();
    }

    unsigned int cusp_i = 0;
    Geom::Point start = B[0].at0();
    pb.moveTo(start);
    build_from_sbasis(pb, B[0], tol, false);
    for (unsigned i=1; i < B.size(); i++) {
        if (!are_near(B[i-1].at1(), B[i].at0(), tol) )
        { // discontinuity found, so fix it :-)
            discontinuity_data const &cusp = cusps[cusp_i];

            switch (cusp_linecap) {
            case LINECUSP_ROUND:  // properly bugged ^_^
                pb.arcTo( abs(cusp.width), abs(cusp.width),
                          angle_between(cusp.der0, cusp.der1), false, cusp.width < 0,
                          B[i].at0() );
                break;
            case LINECUSP_SHARP: // no clue yet what to do here :)
            case LINECUSP_BEVEL:
            default:
                pb.lineTo(B[i].at0());
                break;
            }

            cusp_i++;
        }
        build_from_sbasis(pb, B[i], tol, false);
    }
    pb.finish();
    return pb.peek().front();
}


std::vector<Geom::Path>
LPEPowerStroke::doEffect_path (std::vector<Geom::Path> const & path_in)
{
    using namespace Geom;

    std::vector<Geom::Path> path_out;
    if (path_in.size() == 0) {
        return path_out;
    }

    // for now, only regard first subpath and ignore the rest
    Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_in = path_in[0].toPwSb();

    Piecewise<D2<SBasis> > der = unitVector(derivative(pwd2_in));
    Piecewise<D2<SBasis> > n = rot90(der);
    offset_points.set_pwd2(pwd2_in, n);

    std::vector<Geom::Point> ts = offset_points.data();
    if (sort_points) {
        sort(ts.begin(), ts.end(), compare_offsets);
    }
    if (path_in[0].closed()) {
        // add extra points for interpolation between first and last point
        Point first_point = ts.front();
        Point last_point = ts.back();
        ts.insert(ts.begin(), last_point - Point(pwd2_in.domain().extent() ,0));
        ts.push_back( first_point + Point(pwd2_in.domain().extent() ,0) );
    } else {
        // first and last point have same distance from path as second and second to last points, respectively.
        ts.insert(ts.begin(), Point(pwd2_in.domain().min(), ts.front()[Geom::Y]) );
        ts.push_back( Point(pwd2_in.domain().max(), ts.back()[Geom::Y]) );
    }
    // create stroke path where points (x,y) := (t, offset)
    Geom::Interpolate::Interpolator *interpolator = Geom::Interpolate::Interpolator::create(static_cast<Geom::Interpolate::InterpolatorType>(interpolator_type.get_value()));
    if (Geom::Interpolate::CubicBezierJohan *johan = dynamic_cast<Geom::Interpolate::CubicBezierJohan*>(interpolator)) {
        johan->setBeta(interpolator_beta);
    }
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

    std::vector<discontinuity_data> cusps = find_discontinuities(der, x, y);
    LineCuspType cusp_linecap = static_cast<LineCuspType>(cusp_linecap_type.get_value());

    Piecewise<D2<SBasis> > pwd2_out   = compose(pwd2_in,x) + y*compose(n,x);
    Piecewise<D2<SBasis> > mirrorpath = reverse(compose(pwd2_in,x) - y*compose(n,x));

    Geom::Path fixed_path       = path_from_piecewise_fix_cusps( pwd2_out,   cusps, cusp_linecap, LPE_CONVERSION_TOLERANCE);
    Geom::Path fixed_mirrorpath = path_from_piecewise_fix_cusps( mirrorpath, cusps, cusp_linecap, LPE_CONVERSION_TOLERANCE);

    if (path_in[0].closed()) {
        fixed_path.close(true);
        path_out.push_back(fixed_path);
        fixed_mirrorpath.close(true);
        path_out.push_back(fixed_mirrorpath);
    } else {
        // add linecaps...
        LineCapType end_linecap = static_cast<LineCapType>(end_linecap_type.get_value());
        LineCapType start_linecap = static_cast<LineCapType>(start_linecap_type.get_value());
        switch (end_linecap) {
            case LINECAP_PEAK:
            {
                Geom::Point end_deriv = der.lastValue();
                double radius = 0.5 * distance(pwd2_out.lastValue(), mirrorpath.firstValue());
                Geom::Point midpoint = 0.5*(pwd2_out.lastValue() + mirrorpath.firstValue()) + radius*end_deriv;
                fixed_path.appendNew<LineSegment>(midpoint);
                fixed_path.appendNew<LineSegment>(mirrorpath.firstValue());
                break;
            }
            case LINECAP_SQUARE:
            {
                Geom::Point end_deriv = der.lastValue();
                double radius = 0.5 * distance(pwd2_out.lastValue(), mirrorpath.firstValue());
                fixed_path.appendNew<LineSegment>( pwd2_out.lastValue() + radius*end_deriv );
                fixed_path.appendNew<LineSegment>( mirrorpath.firstValue() + radius*end_deriv );
                fixed_path.appendNew<LineSegment>( mirrorpath.firstValue() );
                break;
            }
            case LINECAP_BUTT:
            {
                fixed_path.appendNew<LineSegment>( mirrorpath.firstValue() );
                break;
            }
            case LINECAP_ROUND:
            default:
            {
                double radius1 = 0.5 * distance(pwd2_out.lastValue(), mirrorpath.firstValue());
                fixed_path.appendNew<SVGEllipticalArc>( radius1, radius1, M_PI/2., false, y.lastValue() < 0, mirrorpath.firstValue() );
                break;
            }
        }

        fixed_path.append(fixed_mirrorpath, Geom::Path::STITCH_DISCONTINUOUS);

        switch (start_linecap) {
            case LINECAP_PEAK:
            {
                Geom::Point start_deriv = der.firstValue();
                double radius = 0.5 * distance(pwd2_out.firstValue(), mirrorpath.lastValue());
                Geom::Point midpoint = 0.5*(mirrorpath.lastValue() + pwd2_out.firstValue()) - radius*start_deriv;
                fixed_path.appendNew<LineSegment>( midpoint );
                fixed_path.appendNew<LineSegment>( pwd2_out.firstValue() );
                break;
            }
            case LINECAP_SQUARE:
            {
                Geom::Point start_deriv = der.firstValue();
                double radius = 0.5 * distance(pwd2_out.firstValue(), mirrorpath.lastValue());
                fixed_path.appendNew<LineSegment>( mirrorpath.lastValue() - radius*start_deriv );
                fixed_path.appendNew<LineSegment>( pwd2_out.firstValue() - radius*start_deriv );
                fixed_path.appendNew<LineSegment>( pwd2_out.firstValue() );
                break;
            }
            case LINECAP_BUTT:
            {
                fixed_path.appendNew<LineSegment>( pwd2_out.firstValue() );
                break;
            }
            case LINECAP_ROUND:
            default:
            {
                double radius2 = 0.5 * distance(pwd2_out.firstValue(), mirrorpath.lastValue());
                fixed_path.appendNew<SVGEllipticalArc>( radius2, radius2, M_PI/2., false, y.firstValue() < 0, pwd2_out.firstValue() );
                break;
            }
        }

        fixed_path.close(true);
        path_out.push_back(fixed_path);
    }

    return path_out;
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

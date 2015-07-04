/** @file
 * @brief Fitting elliptical arc to SBasis
 *
 * This file contains the implementation of the function arc_from_sbasis.
 *//*
 * Copyright 2008  Marco Cecchetti <mrcekets at gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

#include <2geom/curve.h>
#include <2geom/angle.h>
#include <2geom/utils.h>
#include <2geom/bezier-curve.h>
#include <2geom/elliptical-arc.h>
#include <2geom/sbasis-curve.h>  // for non-native methods
#include <2geom/numeric/vector.h>
#include <2geom/numeric/fitting-tool.h>
#include <2geom/numeric/fitting-model.h>
#include <algorithm>

namespace Geom {

// forward declation
namespace detail
{
    struct ellipse_equation;
}

/*
 * make_elliptical_arc
 *
 * convert a parametric polynomial curve given in symmetric power basis form
 * into an EllipticalArc type; in order to be successfull the input curve
 * has to look like an actual elliptical arc even if a certain tolerance
 * is allowed through an ad-hoc parameter.
 * The conversion is performed through an interpolation on a certain amount of
 * sample points computed on the input curve;
 * the interpolation computes the coefficients of the general implicit equation
 * of an ellipse (A*X^2 + B*XY + C*Y^2 + D*X + E*Y + F = 0), then from the
 * implicit equation we compute the parametric form.
 *
 */
class make_elliptical_arc
{
  public:
    typedef D2<SBasis> curve_type;

    /*
     * constructor
     *
     * it doesn't execute the conversion but set the input and output parameters
     *
     * _ea:         the output EllipticalArc that will be generated;
     * _curve:      the input curve to be converted;
     * _total_samples: the amount of sample points to be taken
     *                 on the input curve for performing the conversion
     * _tolerance:     how much likelihood is required between the input curve
     *                 and the generated elliptical arc; the smaller it is the
     *                 the tolerance the higher it is the likelihood.
     */
    make_elliptical_arc( EllipticalArc& _ea,
                         curve_type const& _curve,
                         unsigned int _total_samples,
                         double _tolerance );

  private:
    bool bound_exceeded( unsigned int k, detail::ellipse_equation const & ee,
                         double e1x, double e1y, double e2 );

    bool check_bound(double A, double B, double C, double D, double E, double F);

    void fit();

    bool make_elliptiarc();

    void print_bound_error(unsigned int k)
    {
        std::cerr
            << "tolerance error" << std::endl
            << "at point: " << k << std::endl
            << "error value: "<< dist_err << std::endl
            << "bound: " << dist_bound << std::endl
            << "angle error: " << angle_err
            << " (" << angle_tol << ")" << std::endl;
    }

  public:
    /*
     * perform the actual conversion
     * return true if the conversion is successfull, false on the contrary
     */
    bool operator()()
    {
        // initialize the reference
        const NL::Vector & coeff = fitter.result();
        fit();
        if ( !check_bound(1, coeff[0], coeff[1], coeff[2], coeff[3], coeff[4]) )
            return false;
        if ( !(make_elliptiarc()) ) return false;
        return true;
    }

  private:
      EllipticalArc& ea;                 // output elliptical arc
      const curve_type & curve;             // input curve
      Piecewise<D2<SBasis> > dcurve;        // derivative of the input curve
      NL::LFMEllipse model;                 // model used for fitting
      // perform the actual fitting task
      NL::least_squeares_fitter<NL::LFMEllipse> fitter;
      // tolerance: the user-defined tolerance parameter;
      // tol_at_extr: the tolerance at end-points automatically computed
      // on the value of "tolerance", and usually more strict;
      // tol_at_center: tolerance at the center of the ellipse
      // angle_tol: tolerance for the angle btw the input curve tangent
      // versor and the ellipse normal versor at the sample points
      double tolerance, tol_at_extr, tol_at_center, angle_tol;
      Point initial_point, final_point;     // initial and final end-points
      unsigned int N;                       // total samples
      unsigned int last; // N-1
      double partitions; // N-1
      std::vector<Point> p;                 // sample points
      double dist_err, dist_bound, angle_err;
};

namespace detail
{
/*
 * ellipse_equation
 *
 * this is an helper struct, it provides two routines:
 * the first one evaluates the implicit form of an ellipse on a given point
 * the second one computes the normal versor at a given point of an ellipse
 * in implicit form
 */
struct ellipse_equation
{
    ellipse_equation(double a, double b, double c, double d, double e, double f)
        : A(a), B(b), C(c), D(d), E(e), F(f)
    {
    }

    double operator()(double x, double y) const
    {
        // A * x * x + B * x * y + C * y * y + D * x + E * y + F;
        return (A * x + B * y + D) * x + (C * y + E) * y + F;
    }

    double operator()(Point const& p) const
    {
        return (*this)(p[X], p[Y]);
    }

    Point normal(double x, double y) const
    {
        Point n( 2 * A * x + B * y + D, 2 * C * y + B * x + E );
        return unit_vector(n);
    }

    Point normal(Point const& p) const
    {
        return normal(p[X], p[Y]);
    }

    double A, B, C, D, E, F;
};

} // end namespace detail

make_elliptical_arc::
make_elliptical_arc( EllipticalArc& _ea,
                     curve_type const& _curve,
                     unsigned int _total_samples,
                     double _tolerance )
    : ea(_ea), curve(_curve),
      dcurve( unitVector(derivative(curve)) ),
      model(), fitter(model, _total_samples),
      tolerance(_tolerance), tol_at_extr(tolerance/2),
      tol_at_center(0.1), angle_tol(0.1),
      initial_point(curve.at0()), final_point(curve.at1()),
      N(_total_samples), last(N-1), partitions(N-1), p(N)
{
}

/*
 * check that the coefficients computed by the fit method satisfy
 * the tolerance parameters at the k-th sample point
 */
bool
make_elliptical_arc::
bound_exceeded( unsigned int k, detail::ellipse_equation const & ee,
                double e1x, double e1y, double e2 )
{
    dist_err = std::fabs( ee(p[k]) );
    dist_bound = std::fabs( e1x * p[k][X] + e1y * p[k][Y] + e2 );
    // check that the angle btw the tangent versor to the input curve
    // and the normal versor of the elliptical arc, both evaluate
    // at the k-th sample point, are really othogonal
    angle_err = std::fabs( dot( dcurve(k/partitions), ee.normal(p[k]) ) );
    //angle_err *= angle_err;
    return ( dist_err  > dist_bound || angle_err > angle_tol );
}

/*
 * check that the coefficients computed by the fit method satisfy
 * the tolerance parameters at each sample point
 */
bool
make_elliptical_arc::
check_bound(double A, double B, double C, double D, double E, double F)
{
    detail::ellipse_equation ee(A, B, C, D, E, F);

    // check error magnitude at the end-points
    double e1x = (2*A + B) * tol_at_extr;
    double e1y = (B + 2*C) * tol_at_extr;
    double e2 = ((D + E)  + (A + B + C) * tol_at_extr) * tol_at_extr;
    if (bound_exceeded(0, ee, e1x, e1y, e2))
    {
        print_bound_error(0);
        return false;
    }
    if (bound_exceeded(0, ee, e1x, e1y, e2))
    {
        print_bound_error(last);
        return false;
    }

    // e1x = derivative((ee(x,y), x) | x->tolerance, y->tolerance
    e1x = (2*A + B) * tolerance;
    // e1y = derivative((ee(x,y), y) | x->tolerance, y->tolerance
    e1y = (B + 2*C) * tolerance;
    // e2 = ee(tolerance, tolerance) - F;
    e2 = ((D + E)  + (A + B + C) * tolerance) * tolerance;
//  std::cerr << "e1x = " << e1x << std::endl;
//  std::cerr << "e1y = " << e1y << std::endl;
//  std::cerr << "e2 = " << e2 << std::endl;

    // check error magnitude at sample points
    for ( unsigned int k = 1; k < last; ++k )
    {
        if ( bound_exceeded(k, ee, e1x, e1y, e2) )
        {
            print_bound_error(k);
            return false;
        }
    }

    return true;
}

/*
 * fit
 *
 * supply the samples to the fitter and compute
 * the ellipse implicit equation coefficients
 */
void make_elliptical_arc::fit()
{
    for (unsigned int k = 0; k < N; ++k)
    {
        p[k] = curve( k / partitions );
        fitter.append(p[k]);
    }
    fitter.update();

    NL::Vector z(N, 0.0);
    fitter.result(z);
}

bool make_elliptical_arc::make_elliptiarc()
{
    const NL::Vector & coeff = fitter.result();
    Ellipse e;
    try
    {
        e.setCoefficients(1, coeff[0], coeff[1], coeff[2], coeff[3], coeff[4]);
    }
    catch(LogicalError const &exc)
    {
        return false;
    }

    Point inner_point = curve(0.5);

#ifdef CPP11
    std::unique_ptr<EllipticalArc> arc( e.arc(initial_point, inner_point, final_point) );
#else
    std::auto_ptr<EllipticalArc> arc( e.arc(initial_point, inner_point, final_point) );
#endif
    ea = *arc;

    if ( !are_near( e.center(),
                    ea.center(),
                    tol_at_center * std::min(e.ray(X),e.ray(Y))
                  )
       )
    {
        return false;
    }
    return true;
}



bool arc_from_sbasis(EllipticalArc &ea, D2<SBasis> const &in,
                     double tolerance, unsigned num_samples)
{
    make_elliptical_arc convert(ea, in, num_samples, tolerance);
    return convert();
}

} // end namespace Geom

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

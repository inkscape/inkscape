/*
 * SVG Elliptical Arc Class
 *
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


#include <2geom/bezier-curve.h>
#include <2geom/ellipse.h>
#include <2geom/numeric/fitting-model.h>
#include <2geom/numeric/fitting-tool.h>
#include <2geom/numeric/vector.h>
#include <2geom/poly.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/svg-elliptical-arc.h>

#include <cfloat>
#include <limits>
#include <memory>


namespace Geom
{

/**
 * @class SVGEllipticalArc
 * @brief SVG 1.1-compliant elliptical arc.
 *
 * This class is almost identical to the normal elliptical arc, but it differs slightly
 * in the handling of degenerate arcs to be compliant with SVG 1.1 implementation guidelines.
 *
 * @ingroup Curves
 */

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

}

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
      N(_total_samples), last(N-1), partitions(N-1), p(N),
      svg_compliant(true)
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
        e.set(1, coeff[0], coeff[1], coeff[2], coeff[3], coeff[4]);
    }
    catch(LogicalError const &exc)
    {
        return false;
    }

    Point inner_point = curve(0.5);

    if (svg_compliant_flag())
    {
#ifdef CPP11
        std::unique_ptr<EllipticalArc> arc( e.arc(initial_point, inner_point, final_point, true) );
#else
        std::auto_ptr<EllipticalArc> arc( e.arc(initial_point, inner_point, final_point, true) );
#endif
        ea = *arc;
    }
    else
    {
        try
        {
#ifdef CPP11
            std::unique_ptr<EllipticalArc>
#else
            std::auto_ptr<EllipticalArc>
#endif
                eap( e.arc(initial_point, inner_point, final_point, false) );
            ea = *eap;
        }
        catch(RangeError const &exc)
        {
            return false;
        }
    }


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


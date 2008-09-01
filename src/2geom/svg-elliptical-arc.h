/*
 * Elliptical Arc - implementation of the svg elliptical arc path element
 *
 * Authors:
 *      MenTaLguY <mental@rydia.net>
 *      Marco Cecchetti <mrcekets at gmail.com>
 *
 * Copyright 2007-2008  authors
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


#ifndef _2GEOM_SVG_ELLIPTICAL_ARC_H_
#define _2GEOM_SVG_ELLIPTICAL_ARC_H_


#include <2geom/curve.h>
#include <2geom/angle.h>
#include <2geom/utils.h>
#include <2geom/bezier-curve.h>
#include <2geom/sbasis-curve.h>  // for non-native methods
#include <2geom/numeric/vector.h>
#include <2geom/numeric/fitting-tool.h>
#include <2geom/numeric/fitting-model.h>


#include <algorithm>



namespace Geom
{

class SVGEllipticalArc : public Curve
{
  public:
    SVGEllipticalArc(bool _svg_compliant = true)
        : m_initial_point(Point(0,0)), m_final_point(Point(0,0)),
          m_rx(0), m_ry(0), m_rot_angle(0),
          m_large_arc(true), m_sweep(true),
          m_svg_compliant(_svg_compliant),
          m_start_angle(0), m_end_angle(0),
          m_center(Point(0,0))
    {
    }

    /*
     * constructor
     *
     * input parameters:
     * _initial_point:     initial arc end point;
     * _rx:                ellipse x-axis ray length
     * _ry:                ellipse y-axis ray length
     * _rot_angle:         ellipse x-axis rotation angle;
     * _large_arc:         if true the largest arc is chosen,
     *                     if false the smallest arc is chosen;
     * _sweep :            if true the clockwise arc is chosen,
     *                     if false the counter-clockwise arc is chosen;
     * _final_point:       final arc end point;
     * _svg_compliant:     if true the class behaviour follows the Standard
     *                     SVG 1.1 implementation guidelines (see Appendix F.6)
     *                     if false the class behavoiur is more strict
     *                     on input parameter
     *
     * in case the initial and the final arc end-points overlaps
     * a degenerate arc of zero length is generated
     *
     */
    SVGEllipticalArc( Point _initial_point, double _rx, double _ry,
                      double _rot_angle, bool _large_arc, bool _sweep,
                      Point _final_point,
                      bool _svg_compliant = true
                    )
        : m_initial_point(_initial_point), m_final_point(_final_point),
          m_rx(_rx), m_ry(_ry), m_rot_angle(_rot_angle),
          m_large_arc(_large_arc), m_sweep(_sweep),
          m_svg_compliant(_svg_compliant)
    {
            calculate_center_and_extreme_angles();
    }

    void set( Point _initial_point, double _rx, double _ry,
              double _rot_angle, bool _large_arc, bool _sweep,
              Point _final_point
             )
    {
        m_initial_point = _initial_point;
        m_final_point = _final_point;
        m_rx = _rx;
        m_ry = _ry;
        m_rot_angle = _rot_angle;
        m_large_arc = _large_arc;
        m_sweep = _sweep;
        calculate_center_and_extreme_angles();
    }

    Curve* duplicate() const
    {
        return new SVGEllipticalArc(*this);
    }

    double center(unsigned int i) const
    {
        return m_center[i];
    }

    Point center() const
    {
        return m_center;
    }

    Point initialPoint() const
    {
        return m_initial_point;
    }

    Point finalPoint() const
    {
        return m_final_point;
    }

    double start_angle() const
    {
        return m_start_angle;
    }

    double end_angle() const
    {
        return m_end_angle;
    }

    double ray(unsigned int i) const
    {
        return (i == 0) ? m_rx : m_ry;
    }

    bool large_arc_flag() const
    {
        return m_large_arc;
    }

    bool sweep_flag() const
    {
        return m_sweep;
    }

    double rotation_angle() const
    {
        return m_rot_angle;
    }

    void setInitial( const Point _point)
    {
        m_initial_point = _point;
        calculate_center_and_extreme_angles();
    }

    void setFinal( const Point _point)
    {
        m_final_point = _point;
        calculate_center_and_extreme_angles();
    }

    void setExtremes( const Point& _initial_point, const Point& _final_point )
    {
        m_initial_point = _initial_point;
        m_final_point = _final_point;
        calculate_center_and_extreme_angles();
    }

    bool isDegenerate() const
    {
        return ( are_near(ray(X), 0) || are_near(ray(Y), 0) );
    }

    bool is_svg_compliant() const
    {
        return m_svg_compliant;
    }

    Rect boundsFast() const
    {
        return boundsExact();
    }

    Rect boundsExact() const;

    // TODO: native implementation of the following methods
    Rect boundsLocal(Interval i, unsigned int deg) const
    {
        if (isDegenerate() && is_svg_compliant())
            return chord().boundsLocal(i, deg);
        else
            return SBasisCurve(toSBasis()).boundsLocal(i, deg);
    }

    std::vector<double> roots(double v, Dim2 d) const;

    /*
     * find all the points on the curve portion between "from" and "to"
     * at the same smallest distance from the point "p" the points are returned
     * as their parameter t value;
     */
    std::vector<double>
    allNearestPoints( Point const& p, double from = 0, double to = 1 ) const;

    /*
     * find a point on the curve portion between "from" and "to"
     * at the same smallest distance from the point "p";
     * the point is returned as its parameter t value;
     */
    double nearestPoint( Point const& p, double from = 0, double to = 1 ) const
    {
        if ( are_near(ray(X), ray(Y)) && are_near(center(), p) )
        {
            return from;
        }
        return allNearestPoints(p, from, to).front();
    }

    // TODO: native implementation of the following methods
    int winding(Point p) const
    {
        if (isDegenerate() && is_svg_compliant())
            return chord().winding(p);
        else
            return SBasisCurve(toSBasis()).winding(p);
    }

    Curve *derivative() const;

    Curve *transformed(Matrix const &m) const;

    std::vector<Point> pointAndDerivatives(Coord t, unsigned int n) const;

    D2<SBasis> toSBasis() const;

    /*
     * return true if the angle argument (in radiants) is contained
     * in the range [start_angle(), end_angle() ]
     */
    bool containsAngle(Coord angle) const;

    /*
     * return the value of the d-dimensional coordinate related to "t"
     * here t belongs to the [0,2PI] domain
     */
    double valueAtAngle(Coord t, Dim2 d) const;

    /*
     * return the point related to the parameter value "t"
     * here t belongs to the [0,2PI] domain
     */
    Point pointAtAngle(Coord t) const
    {
        double sin_rot_angle = std::sin(rotation_angle());
        double cos_rot_angle = std::cos(rotation_angle());
        Matrix m( ray(X) * cos_rot_angle, ray(X) * sin_rot_angle,
                 -ray(Y) * sin_rot_angle, ray(Y) * cos_rot_angle,
                  center(X),              center(Y) );
        Point p( std::cos(t), std::sin(t) );
        return p * m;
    }

    /*
     * return the value of the d-dimensional coordinate related to "t"
     * here t belongs to the [0,1] domain
     */
    double valueAt(Coord t, Dim2 d) const
    {
        if (isDegenerate() && is_svg_compliant())
            return chord().valueAt(t, d);

        Coord tt = map_to_02PI(t);
        return valueAtAngle(tt, d);
    }

    /*
     * return the point related to the parameter value "t"
     * here t belongs to the [0,1] domain
     */
    Point pointAt(Coord t) const
    {
        if (isDegenerate() && is_svg_compliant())
            return chord().pointAt(t);

        Coord tt = map_to_02PI(t);
        return pointAtAngle(tt);
    }

    std::pair<SVGEllipticalArc, SVGEllipticalArc>
    subdivide(Coord t) const
    {
        SVGEllipticalArc* arc1 = static_cast<SVGEllipticalArc*>(portion(0, t));
        SVGEllipticalArc* arc2 = static_cast<SVGEllipticalArc*>(portion(t, 1));
        assert( arc1 != NULL && arc2 != NULL);
        std::pair<SVGEllipticalArc, SVGEllipticalArc> arc_pair(*arc1, *arc2);
        delete arc1;
        delete arc2;
        return arc_pair;
    }

    Curve* portion(double f, double t) const;

    // the arc is the same but traversed in the opposite direction
    Curve* reverse() const
    {
        SVGEllipticalArc* rarc = new SVGEllipticalArc( *this );
        rarc->m_sweep = !m_sweep;
        rarc->m_initial_point = m_final_point;
        rarc->m_final_point = m_initial_point;
        rarc->m_start_angle = m_end_angle;
        rarc->m_end_angle = m_start_angle;
        return rarc;
    }


    double sweep_angle() const
    {
        Coord d = end_angle() - start_angle();
        if ( !sweep_flag() ) d = -d;
        if ( d < 0 )
            d += 2*M_PI;
        return d;
    }

    LineSegment chord() const
    {
        return LineSegment(initialPoint(), finalPoint());
    }

  private:
    Coord map_to_02PI(Coord t) const;
    Coord map_to_01(Coord angle) const;
    void calculate_center_and_extreme_angles();

  private:
    Point m_initial_point, m_final_point;
    double m_rx, m_ry, m_rot_angle;
    bool m_large_arc, m_sweep;
    bool m_svg_compliant;
    double m_start_angle, m_end_angle;
    Point m_center;

}; // end class SVGEllipticalArc


/*
 * useful for testing and debugging
 */
template< class charT >
inline
std::basic_ostream<charT> &
operator<< (std::basic_ostream<charT> & os, const SVGEllipticalArc & ea)
{
    os << "{ cx: " << ea.center(X) << ", cy: " <<  ea.center(Y)
       << ", rx: " << ea.ray(X) << ", ry: " << ea.ray(Y)
       << ", rot angle: " << decimal_round(rad_to_deg(ea.rotation_angle()),2)
       << ", start angle: " << decimal_round(rad_to_deg(ea.start_angle()),2)
       << ", end angle: " << decimal_round(rad_to_deg(ea.end_angle()),2)
       << " }";

    return os;
}




// forward declation
namespace detail
{
    struct ellipse_equation;
}

/*
 * make_elliptical_arc
 *
 * convert a parametric polynomial curve given in symmetric power basis form
 * into an SVGEllipticalArc type; in order to be successfull the input curve
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
     * _ea:         the output SVGEllipticalArc that will be generated;
     * _curve:      the input curve to be converted;
     * _total_samples: the amount of sample points to be taken
     *                 on the input curve for performing the conversion
     * _tolerance:     how much likelihood is required between the input curve
     *                 and the generated elliptical arc; the smaller it is the
     *                 the tolerance the higher it is the likelihood.
     */
    make_elliptical_arc( SVGEllipticalArc& _ea,
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

    /*
     * you can set a boolean parameter to tell the conversion routine
     * if the output elliptical arc has to be svg compliant or not;
     * the default value is true
     */
    bool svg_compliant_flag() const
    {
        return svg_compliant;
    }

    void svg_compliant_flag(bool _svg_compliant)
    {
        svg_compliant = _svg_compliant;
    }

  private:
      SVGEllipticalArc& ea;                 // output elliptical arc
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
      bool svg_compliant;
};


} // end namespace Geom




#endif /* _2GEOM_SVG_ELLIPTICAL_ARC_H_ */

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


/**
 * \file
 * \brief SVG 1.1-compliant elliptical arc curve
 *//*
 * Authors:
 *    MenTaLguY <mental@rydia.net>
 *    Marco Cecchetti <mrcekets at gmail.com>
 *    Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2007-2009 Authors
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


#ifndef LIB2GEOM_SEEN_SVG_ELLIPTICAL_ARC_H
#define LIB2GEOM_SEEN_SVG_ELLIPTICAL_ARC_H

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

class SVGEllipticalArc : public EllipticalArc {
public:
    SVGEllipticalArc()
        : EllipticalArc()
    {}
    SVGEllipticalArc( Point ip, double rx, double ry,
                      double rot_angle, bool large_arc, bool sweep,
                      Point fp
                    )
        : EllipticalArc()
    {
        _initial_point = ip;
        _final_point = fp;
        _rays[X] = rx; _rays[Y] = ry;
        _rot_angle = rot_angle;
        _large_arc = large_arc;
        _sweep = sweep;
        _updateCenterAndAngles(true);
    }

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    virtual Curve *duplicate() const {
        return new SVGEllipticalArc(*this);
    }
    virtual Coord valueAt(Coord t, Dim2 d) const {
        if (isDegenerate()) return chord().valueAt(t, d);
        return EllipticalArc::valueAt(t, d);
    }
    virtual Point pointAt(Coord t) const {
        if (isDegenerate()) return chord().pointAt(t);
        return EllipticalArc::pointAt(t);
    }
    virtual std::vector<Point> pointAndDerivatives(Coord t, unsigned int n) const {
        if (isDegenerate()) return chord().pointAndDerivatives(t, n);
        return EllipticalArc::pointAndDerivatives(t, n);
    }
    virtual Rect boundsExact() const {
        if (isDegenerate()) return chord().boundsExact();
        return EllipticalArc::boundsExact();
    }
    virtual OptRect boundsLocal(OptInterval const &i, unsigned int deg) const {
        if (isDegenerate()) return chord().boundsLocal(i, deg);
        return EllipticalArc::boundsLocal(i, deg);
    }

    virtual Curve *derivative() const {
        if (isDegenerate()) return chord().derivative();
        return EllipticalArc::derivative();
    }

    virtual std::vector<Coord> roots(Coord v, Dim2 d) const {
        if (isDegenerate()) return chord().roots(v, d);
        return EllipticalArc::roots(v, d);
    }
#ifdef HAVE_GSL
    virtual std::vector<Coord> allNearestPoints( Point const& p, double from = 0, double to = 1 ) const {
        if (isDegenerate()) {
            std::vector<Coord> result;
            result.push_back(chord().nearestPoint(p, from, to));
            return result;
        }
        return EllipticalArc::allNearestPoints(p, from, to);
    }
#endif
    virtual D2<SBasis> toSBasis() const {
        if (isDegenerate()) return chord().toSBasis();
        return EllipticalArc::toSBasis();
    }
    virtual bool isSVGCompliant() const { return true; }
    // TODO move SVG-specific behavior here.
//protected:
    //virtual void _updateCenterAndAngles();
#endif
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
       << ", rot angle: " << decimal_round(rad_to_deg(ea.rotationAngle()),2)
       << ", start angle: " << decimal_round(rad_to_deg(ea.initialAngle()),2)
       << ", end angle: " << decimal_round(rad_to_deg(ea.finalAngle()),2)
       << " }";

    return os;
}




// forward declation
namespace detail
{
    struct ellipse_equation;
}

// TODO this needs to be rewritten and moved to EllipticalArc header
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
      bool svg_compliant;
};

} // end namespace Geom

#endif // LIB2GEOM_SEEN_SVG_ELLIPTICAL_ARC_H

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


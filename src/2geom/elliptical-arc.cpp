/*
 * SVG Elliptical Arc Class
 *
 * Authors:
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * Copyright 2008-2009 Authors
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

#include <cfloat>
#include <limits>
#include <memory>

#include <2geom/elliptical-arc.h>
#include <2geom/ellipse.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-curve.h>
#include <2geom/poly.h>
#include <2geom/transforms.h>
#include <2geom/utils.h>

#include <2geom/numeric/vector.h>
#include <2geom/numeric/fitting-tool.h>
#include <2geom/numeric/fitting-model.h>


namespace Geom
{

/**
 * @class EllipticalArc
 * @brief Elliptical arc curve
 *
 * Elliptical arc is a curve taking the shape of a section of an ellipse.
 * 
 * The arc function has two forms: the regular one, mapping the unit interval to points
 * on 2D plane (the linear domain), and a second form that maps some interval
 * \f$A \subseteq [0,2\pi)\f$ to the same points (the angular domain). The interval \f$A\f$
 * determines which part of the ellipse forms the arc. The arc is said to contain an angle
 * if its angular domain includes that angle (and therefore it is defined for that angle).
 *
 * The angular domain considers each ellipse to be
 * a rotated, scaled and translated unit circle: 0 corresponds to \f$(1,0)\f$ on the unit circle,
 * \f$\pi/2\f$ corresponds to \f$(0,1)\f$, \f$\pi\f$ to \f$(-1,0)\f$ and \f$3\pi/2\f$
 * to \f$(0,-1)\f$. After the angle is mapped to a point from a unit circle, the point is
 * transformed using a matrix of this form
 * \f[ M = \left[ \begin{array}{ccc}
        r_X \cos(\theta) & -r_Y \sin(\theta) & 0 \\
        r_X \sin(\theta) & r_Y \cos(\theta) & 0 \\
        c_X & c_Y & 1 \end{array} \right] \f]
 * where \f$r_X, r_Y\f$ are the X and Y rays of the ellipse, \f$\theta\f$ is its angle of rotation,
 * and \f$c_X, c_Y\f$ the coordinates of the ellipse's center - thus mapping the angle
 * to some point on the ellipse. Note that for example the point at angluar coordinate 0,
 * the center and the point at angular coordinate \f$\pi/4\f$ do not necessarily
 * create an angle of \f$\pi/4\f$ radians; it is only the case if both axes of the ellipse
 * are of the same length (i.e. it is a circle).
 *
 * @image html ellipse-angular-coordinates.png "An illustration of the angular domain"
 *
 * Each arc is defined by five variables: The initial and final point, the ellipse's rays,
 * and the ellipse's rotation. Each set of those parameters corresponds to four different arcs,
 * with two of them larger than half an ellipse and two of them turning clockwise while traveling
 * from initial to final point. The two flags disambiguate between them: "large arc flag" selects
 * the bigger arc, while the "sweep flag" selects the clockwise arc.
 *
 * @image html elliptical-arc-flags.png "The four possible arcs and the meaning of flags"
 *
 * @ingroup Curves
 */

Rect EllipticalArc::boundsExact() const
{
    double extremes[4];
    double sinrot, cosrot;
    sincos(_rot_angle, sinrot, cosrot);

    extremes[0] = std::atan2( -ray(Y) * sinrot, ray(X) * cosrot );
    extremes[1] = extremes[0] + M_PI;
    if ( extremes[0] < 0 ) extremes[0] += 2*M_PI;
    extremes[2] = std::atan2( ray(Y) * cosrot, ray(X) * sinrot );
    extremes[3] = extremes[2] + M_PI;
    if ( extremes[2] < 0 ) extremes[2] += 2*M_PI;


    double arc_extremes[4];
    arc_extremes[0] = initialPoint()[X];
    arc_extremes[1] = finalPoint()[X];
    if ( arc_extremes[0] < arc_extremes[1] )
        std::swap(arc_extremes[0], arc_extremes[1]);
    arc_extremes[2] = initialPoint()[Y];
    arc_extremes[3] = finalPoint()[Y];
    if ( arc_extremes[2] < arc_extremes[3] )
        std::swap(arc_extremes[2], arc_extremes[3]);

    if ( !are_near(initialPoint(), finalPoint()) ) {
        for (unsigned i = 0; i < 4; ++i) {
            if (containsAngle(extremes[i])) {
                arc_extremes[i] = valueAtAngle(extremes[i], (i >> 1) ? Y : X);
            }
        }
    }

    return Rect( Point(arc_extremes[1], arc_extremes[3]) ,
                 Point(arc_extremes[0], arc_extremes[2]) );
}


Point EllipticalArc::pointAtAngle(Coord t) const
{
    Point ret = Point::polar(t) * unitCircleTransform();
    return ret;
}

Coord EllipticalArc::valueAtAngle(Coord t, Dim2 d) const
{
    Coord sinrot, cosrot, cost, sint;
    sincos(_rot_angle, sinrot, cosrot);
    sincos(t, sint, cost);

    if ( d == X ) {
        return    ray(X) * cosrot * cost
                - ray(Y) * sinrot * sint
                + center(X);
    } else {
        return    ray(X) * sinrot * cost
                + ray(Y) * cosrot * sint
                + center(Y);
    }
}

Affine EllipticalArc::unitCircleTransform() const
{
    Affine ret = Scale(ray(X), ray(Y)) * Rotate(_rot_angle);
    ret.setTranslation(center());
    return ret;
}

std::vector<Coord> EllipticalArc::roots(Coord v, Dim2 d) const
{
    std::vector<Coord> sol;

    if ( are_near(ray(X), 0) && are_near(ray(Y), 0) ) {
        if ( center(d) == v )
            sol.push_back(0);
        return sol;
    }

    static const char* msg[2][2] =
    {
        { "d == X; ray(X) == 0; "
          "s = (v - center(X)) / ( -ray(Y) * std::sin(_rot_angle) ); "
          "s should be contained in [-1,1]",
          "d == X; ray(Y) == 0; "
          "s = (v - center(X)) / ( ray(X) * std::cos(_rot_angle) ); "
          "s should be contained in [-1,1]"
        },
        { "d == Y; ray(X) == 0; "
          "s = (v - center(X)) / ( ray(Y) * std::cos(_rot_angle) ); "
          "s should be contained in [-1,1]",
          "d == Y; ray(Y) == 0; "
          "s = (v - center(X)) / ( ray(X) * std::sin(_rot_angle) ); "
          "s should be contained in [-1,1]"
        },
    };

    for ( unsigned int dim = 0; dim < 2; ++dim )
    {
        if ( are_near(ray((Dim2) dim), 0) )
        {
            if ( initialPoint()[d] == v && finalPoint()[d] == v )
            {
                THROW_INFINITESOLUTIONS(0);
            }
            if ( (initialPoint()[d] < finalPoint()[d])
                 && (initialPoint()[d] > v || finalPoint()[d] < v) )
            {
                return sol;
            }
            if ( (initialPoint()[d] > finalPoint()[d])
                 && (finalPoint()[d] > v || initialPoint()[d] < v) )
            {
                return sol;
            }
            double ray_prj = 0.0;
            switch(d)
            {
                case X:
                    switch(dim)
                    {
                        case X: ray_prj = -ray(Y) * std::sin(_rot_angle);
                                break;
                        case Y: ray_prj = ray(X) * std::cos(_rot_angle);
                                break;
                    }
                    break;
                case Y:
                    switch(dim)
                    {
                        case X: ray_prj = ray(Y) * std::cos(_rot_angle);
                                break;
                        case Y: ray_prj = ray(X) * std::sin(_rot_angle);
                                break;
                    }
                    break;
            }

            double s = (v - center(d)) / ray_prj;
            if ( s < -1 || s > 1 )
            {
                THROW_LOGICALERROR(msg[d][dim]);
            }
            switch(dim)
            {
                case X:
                    s = std::asin(s); // return a value in [-PI/2,PI/2]
                    if ( logical_xor( _sweep, are_near(initialAngle(), M_PI/2) )  )
                    {
                        if ( s < 0 ) s += 2*M_PI;
                    }
                    else
                    {
                        s = M_PI - s;
                        if (!(s < 2*M_PI) ) s -= 2*M_PI;
                    }
                    break;
                case Y:
                    s = std::acos(s); // return a value in [0,PI]
                    if ( logical_xor( _sweep, are_near(initialAngle(), 0) ) )
                    {
                        s = 2*M_PI - s;
                        if ( !(s < 2*M_PI) ) s -= 2*M_PI;
                    }
                    break;
            }

            //std::cerr << "s = " << rad_to_deg(s);
            s = map_to_01(s);
            //std::cerr << " -> t: " << s << std::endl;
            if ( !(s < 0 || s > 1) )
                sol.push_back(s);
            return sol;
        }
    }

    double rotx, roty;
    sincos(_rot_angle, roty, rotx); /// \todo sin and cos are calculated in many places in this function, optimize this a bit!
    if (d == X) roty = -roty;

    double rxrotx = ray(X) * rotx;
    double c_v = center(d) - v;

    double a = -rxrotx + c_v;
    double b = ray(Y) * roty;
    double c = rxrotx + c_v;
    //std::cerr << "a = " << a << std::endl;
    //std::cerr << "b = " << b << std::endl;
    //std::cerr << "c = " << c << std::endl;

    if ( are_near(a,0) )
    {
        sol.push_back(M_PI);
        if ( !are_near(b,0) )
        {
            double s = 2 * std::atan(-c/(2*b));
            if ( s < 0 ) s += 2*M_PI;
            sol.push_back(s);
        }
    }
    else
    {
        double delta = b * b - a * c;
        //std::cerr << "delta = " << delta << std::endl;
        if ( are_near(delta, 0) )
        {
            double s = 2 * std::atan(-b/a);
            if ( s < 0 ) s += 2*M_PI;
            sol.push_back(s);
        }
        else if ( delta > 0 )
        {
            double sq = std::sqrt(delta);
            double s = 2 * std::atan( (-b - sq) / a );
            if ( s < 0 ) s += 2*M_PI;
            sol.push_back(s);
            s = 2 * std::atan( (-b + sq) / a );
            if ( s < 0 ) s += 2*M_PI;
            sol.push_back(s);
        }
    }

    std::vector<double> arc_sol;
    for (unsigned int i = 0; i < sol.size(); ++i )
    {
        //std::cerr << "s = " << rad_to_deg(sol[i]);
        sol[i] = map_to_01(sol[i]);
        //std::cerr << " -> t: " << sol[i] << std::endl;
        if ( !(sol[i] < 0 || sol[i] > 1) )
            arc_sol.push_back(sol[i]);
    }
    return arc_sol;
}


// D(E(t,C),t) = E(t+PI/2,O), where C is the ellipse center
// the derivative doesn't rotate the ellipse but there is a translation
// of the parameter t by an angle of PI/2 so the ellipse points are shifted
// of such an angle in the cw direction
Curve *EllipticalArc::derivative() const
{
    EllipticalArc *result = static_cast<EllipticalArc*>(duplicate());
    result->_center[X] = result->_center[Y] = 0;
    result->_start_angle += M_PI/2;
    if( !( result->_start_angle < 2*M_PI ) )
    {
        result->_start_angle -= 2*M_PI;
    }
    result->_end_angle += M_PI/2;
    if( !( result->_end_angle < 2*M_PI ) )
    {
        result->_end_angle -= 2*M_PI;
    }
    result->_initial_point = result->pointAtAngle( result->initialAngle() );
    result->_final_point = result->pointAtAngle( result->finalAngle() );
    return result;
}


std::vector<Point>
EllipticalArc::pointAndDerivatives(Coord t, unsigned int n) const
{
    unsigned int nn = n+1; // nn represents the size of the result vector.
    std::vector<Point> result;
    result.reserve(nn);
    double angle = map_unit_interval_on_circular_arc(t, initialAngle(),
                                                     finalAngle(), _sweep);
    std::auto_ptr<EllipticalArc> ea( static_cast<EllipticalArc*>(duplicate()) );
    ea->_center = Point(0,0);
    unsigned int m = std::min(nn, 4u);
    for ( unsigned int i = 0; i < m; ++i )
    {
        result.push_back( ea->pointAtAngle(angle) );
        angle += (_sweep ? M_PI/2 : -M_PI/2);
        if ( !(angle < 2*M_PI) ) angle -= 2*M_PI;
    }
    m = nn / 4;
    for ( unsigned int i = 1; i < m; ++i )
    {
        for ( unsigned int j = 0; j < 4; ++j )
            result.push_back( result[j] );
    }
    m = nn - 4 * m;
    for ( unsigned int i = 0; i < m; ++i )
    {
        result.push_back( result[i] );
    }
    if ( !result.empty() ) // nn != 0
        result[0] = pointAtAngle(angle);
    return result;
}

bool EllipticalArc::containsAngle(Coord angle) const
{
    return AngleInterval::contains(angle);
}

Curve* EllipticalArc::portion(double f, double t) const
{
    // fix input arguments
    if (f < 0) f = 0;
    if (f > 1) f = 1;
    if (t < 0) t = 0;
    if (t > 1) t = 1;

    if ( are_near(f, t) )
    {
        EllipticalArc *arc = static_cast<EllipticalArc*>(duplicate());
        arc->_center = arc->_initial_point = arc->_final_point = pointAt(f);
        arc->_start_angle = arc->_end_angle = _start_angle;
        arc->_rot_angle = _rot_angle;
        arc->_sweep = _sweep;
        arc->_large_arc = _large_arc;
        return arc;
    }

    EllipticalArc *arc = static_cast<EllipticalArc*>(duplicate());
    arc->_initial_point = pointAt(f);
    arc->_final_point = pointAt(t);
    if ( f > t ) arc->_sweep = !_sweep;
    if ( _large_arc && fabs(sweepAngle() * (t-f)) < M_PI)
        arc->_large_arc = false;
    arc->_updateCenterAndAngles(arc->isSVGCompliant()); //TODO: be more clever
    return arc;
}

// the arc is the same but traversed in the opposite direction
Curve *EllipticalArc::reverse() const {
    EllipticalArc *rarc = static_cast<EllipticalArc*>(duplicate());
    rarc->_sweep = !_sweep;
    rarc->_initial_point = _final_point;
    rarc->_final_point = _initial_point;
    rarc->_start_angle = _end_angle;
    rarc->_end_angle = _start_angle;
    rarc->_updateCenterAndAngles(rarc->isSVGCompliant());
    return rarc;
}

#ifdef HAVE_GSL  // GSL is required for function "solve_reals"
std::vector<double> EllipticalArc::allNearestPoints( Point const& p, double from, double to ) const
{
    std::vector<double> result;

    if ( from > to ) std::swap(from, to);
    if ( from < 0 || to > 1 )
    {
        THROW_RANGEERROR("[from,to] interval out of range");
    }

    if ( ( are_near(ray(X), 0) && are_near(ray(Y), 0) )  || are_near(from, to) )
    {
        result.push_back(from);
        return result;
    }
    else if ( are_near(ray(X), 0) || are_near(ray(Y), 0) )
    {
        LineSegment seg(pointAt(from), pointAt(to));
        Point np = seg.pointAt( seg.nearestPoint(p) );
        if ( are_near(ray(Y), 0) )
        {
            if ( are_near(_rot_angle, M_PI/2)
                 || are_near(_rot_angle, 3*M_PI/2) )
            {
                result = roots(np[Y], Y);
            }
            else
            {
                result = roots(np[X], X);
            }
        }
        else
        {
            if ( are_near(_rot_angle, M_PI/2)
                 || are_near(_rot_angle, 3*M_PI/2) )
            {
                result = roots(np[X], X);
            }
            else
            {
                result = roots(np[Y], Y);
            }
        }
        return result;
    }
    else if ( are_near(ray(X), ray(Y)) )
    {
        Point r = p - center();
        if ( are_near(r, Point(0,0)) )
        {
            THROW_INFINITESOLUTIONS(0);
        }
        // TODO: implement case r != 0
//      Point np = ray(X) * unit_vector(r);
//      std::vector<double> solX = roots(np[X],X);
//      std::vector<double> solY = roots(np[Y],Y);
//      double t;
//      if ( are_near(solX[0], solY[0]) || are_near(solX[0], solY[1]))
//      {
//          t = solX[0];
//      }
//      else
//      {
//          t = solX[1];
//      }
//      if ( !(t < from || t > to) )
//      {
//          result.push_back(t);
//      }
//      else
//      {
//
//      }
    }

    // solve the equation <D(E(t),t)|E(t)-p> == 0
    // that provides min and max distance points
    // on the ellipse E wrt the point p
    // after the substitutions:
    // cos(t) = (1 - s^2) / (1 + s^2)
    // sin(t) = 2t / (1 + s^2)
    // where s = tan(t/2)
    // we get a 4th degree equation in s
    /*
     *  ry s^4 ((-cy + py) Cos[Phi] + (cx - px) Sin[Phi]) +
     *  ry ((cy - py) Cos[Phi] + (-cx + px) Sin[Phi]) +
     *  2 s^3 (rx^2 - ry^2 + (-cx + px) rx Cos[Phi] + (-cy + py) rx Sin[Phi]) +
     *  2 s (-rx^2 + ry^2 + (-cx + px) rx Cos[Phi] + (-cy + py) rx Sin[Phi])
     */

    Point p_c = p - center();
    double rx2_ry2 = (ray(X) - ray(Y)) * (ray(X) + ray(Y));
    double sinrot, cosrot;
    sincos(_rot_angle, sinrot, cosrot);
    double expr1 = ray(X) * (p_c[X] * cosrot + p_c[Y] * sinrot);
    Poly coeff;
    coeff.resize(5);
    coeff[4] = ray(Y) * ( p_c[Y] * cosrot - p_c[X] * sinrot );
    coeff[3] = 2 * ( rx2_ry2 + expr1 );
    coeff[2] = 0;
    coeff[1] = 2 * ( -rx2_ry2 + expr1 );
    coeff[0] = -coeff[4];

//  for ( unsigned int i = 0; i < 5; ++i )
//      std::cerr << "c[" << i << "] = " << coeff[i] << std::endl;

    std::vector<double> real_sol;
    // gsl_poly_complex_solve raises an error
    // if the leading coefficient is zero
    if ( are_near(coeff[4], 0) )
    {
        real_sol.push_back(0);
        if ( !are_near(coeff[3], 0) )
        {
            double sq = -coeff[1] / coeff[3];
            if ( sq > 0 )
            {
                double s = std::sqrt(sq);
                real_sol.push_back(s);
                real_sol.push_back(-s);
            }
        }
    }
    else
    {
        real_sol = solve_reals(coeff);
    }

    for ( unsigned int i = 0; i < real_sol.size(); ++i )
    {
        real_sol[i] = 2 * std::atan(real_sol[i]);
        if ( real_sol[i] < 0 ) real_sol[i] += 2*M_PI;
    }
    // when s -> Infinity then <D(E)|E-p> -> 0 iff coeff[4] == 0
    // so we add M_PI to the solutions being lim arctan(s) = PI when s->Infinity
    if ( (real_sol.size() % 2) != 0 )
    {
        real_sol.push_back(M_PI);
    }

    double mindistsq1 = std::numeric_limits<double>::max();
    double mindistsq2 = std::numeric_limits<double>::max();
    double dsq;
    unsigned int mi1 = 0, mi2 = 0;
    for ( unsigned int i = 0; i < real_sol.size(); ++i )
    {
        dsq = distanceSq(p, pointAtAngle(real_sol[i]));
        if ( mindistsq1 > dsq )
        {
            mindistsq2 = mindistsq1;
            mi2 = mi1;
            mindistsq1 = dsq;
            mi1 = i;
        }
        else if ( mindistsq2 > dsq )
        {
            mindistsq2 = dsq;
            mi2 = i;
        }
    }

    double t = map_to_01( real_sol[mi1] );
    if ( !(t < from || t > to) )
    {
        result.push_back(t);
    }

    bool second_sol = false;
    t = map_to_01( real_sol[mi2] );
    if ( real_sol.size() == 4 && !(t < from || t > to) )
    {
        if ( result.empty() || are_near(mindistsq1, mindistsq2) )
        {
            result.push_back(t);
            second_sol = true;
        }
    }

    // we need to test extreme points too
    double dsq1 = distanceSq(p, pointAt(from));
    double dsq2 = distanceSq(p, pointAt(to));
    if ( second_sol )
    {
        if ( mindistsq2 > dsq1 )
        {
            result.clear();
            result.push_back(from);
            mindistsq2 = dsq1;
        }
        else if ( are_near(mindistsq2, dsq) )
        {
            result.push_back(from);
        }
        if ( mindistsq2 > dsq2 )
        {
            result.clear();
            result.push_back(to);
        }
        else if ( are_near(mindistsq2, dsq2) )
        {
            result.push_back(to);
        }

    }
    else
    {
        if ( result.empty() )
        {
            if ( are_near(dsq1, dsq2) )
            {
                result.push_back(from);
                result.push_back(to);
            }
            else if ( dsq2 > dsq1 )
            {
                result.push_back(from);
            }
            else
            {
                result.push_back(to);
            }
        }
    }

    return result;
}
#endif

/*
 * NOTE: this implementation follows Standard SVG 1.1 implementation guidelines
 * for elliptical arc curves. See Appendix F.6.
 */
void EllipticalArc::_updateCenterAndAngles(bool svg)
{
    Point d = initialPoint() - finalPoint();

    // TODO move this to SVGElipticalArc?
    if (svg)
    {
        if ( initialPoint() == finalPoint() )
        {
            _rot_angle = _start_angle = _end_angle = 0;
            _center = initialPoint();
            _rays = Geom::Point(0,0);
            _large_arc = _sweep = false;
            return;
        }

        _rays[X] = std::fabs(_rays[X]);
        _rays[Y] = std::fabs(_rays[Y]);

        if ( are_near(ray(X), 0) || are_near(ray(Y), 0) )
        {
            _rays[X] = L2(d) / 2;
            _rays[Y] = 0;
            _rot_angle = std::atan2(d[Y], d[X]);
            _start_angle = 0;
            _end_angle = M_PI;
            _center = middle_point(initialPoint(), finalPoint());
            _large_arc = false;
            _sweep = false;
            return;
        }
    }
    else
    {
        if ( are_near(initialPoint(), finalPoint()) )
        {
            if ( are_near(ray(X), 0) && are_near(ray(Y), 0) )
            {
                _start_angle = _end_angle = 0;
                _center = initialPoint();
                return;
            }
            else
            {
                THROW_RANGEERROR("initial and final point are the same");
            }
        }
        if ( are_near(ray(X), 0) && are_near(ray(Y), 0) )
        { // but initialPoint != finalPoint
            THROW_RANGEERROR(
                "there is no ellipse that satisfies the given constraints: "
                "ray(X) == 0 && ray(Y) == 0 but initialPoint != finalPoint"
            );
        }
        if ( are_near(ray(Y), 0) )
        {
            Point v = initialPoint() - finalPoint();
            if ( are_near(L2sq(v), 4*ray(X)*ray(X)) )
            {
                Angle angle(v);
                if ( are_near( angle, _rot_angle ) )
                {
                    _start_angle = 0;
                    _end_angle = M_PI;
                    _center = v/2 + finalPoint();
                    return;
                }
                angle -= M_PI;
                if ( are_near( angle, _rot_angle ) )
                {
                    _start_angle = M_PI;
                    _end_angle = 0;
                    _center = v/2 + finalPoint();
                    return;
                }
                THROW_RANGEERROR(
                    "there is no ellipse that satisfies the given constraints: "
                    "ray(Y) == 0 "
                    "and slope(initialPoint - finalPoint) != rotation_angle "
                    "and != rotation_angle + PI"
                );
            }
            if ( L2sq(v) > 4*ray(X)*ray(X) )
            {
                THROW_RANGEERROR(
                    "there is no ellipse that satisfies the given constraints: "
                    "ray(Y) == 0 and distance(initialPoint, finalPoint) > 2*ray(X)"
                );
            }
            else
            {
                THROW_RANGEERROR(
                    "there is infinite ellipses that satisfy the given constraints: "
                    "ray(Y) == 0  and distance(initialPoint, finalPoint) < 2*ray(X)"
                );
            }

        }

        if ( are_near(ray(X), 0) )
        {
            Point v = initialPoint() - finalPoint();
            if ( are_near(L2sq(v), 4*ray(Y)*ray(Y)) )
            {
                double angle = std::atan2(v[Y], v[X]);
                if (angle < 0) angle += 2*M_PI;
                double rot_angle = _rot_angle.radians() + M_PI/2;
                if ( !(rot_angle < 2*M_PI) ) rot_angle -= 2*M_PI;
                if ( are_near( angle, rot_angle ) )
                {
                    _start_angle = M_PI/2;
                    _end_angle = 3*M_PI/2;
                    _center = v/2 + finalPoint();
                    return;
                }
                angle -= M_PI;
                if ( angle < 0 ) angle += 2*M_PI;
                if ( are_near( angle, rot_angle ) )
                {
                    _start_angle = 3*M_PI/2;
                    _end_angle = M_PI/2;
                    _center = v/2 + finalPoint();
                    return;
                }
                THROW_RANGEERROR(
                    "there is no ellipse that satisfies the given constraints: "
                    "ray(X) == 0 "
                    "and slope(initialPoint - finalPoint) != rotation_angle + PI/2 "
                    "and != rotation_angle + (3/2)*PI"
                );
            }
            if ( L2sq(v) > 4*ray(Y)*ray(Y) )
            {
                THROW_RANGEERROR(
                    "there is no ellipse that satisfies the given constraints: "
                    "ray(X) == 0 and distance(initialPoint, finalPoint) > 2*ray(Y)"
                );
            }
            else
            {
                THROW_RANGEERROR(
                    "there is infinite ellipses that satisfy the given constraints: "
                    "ray(X) == 0  and distance(initialPoint, finalPoint) < 2*ray(Y)"
                );
            }

        }

    }

    Rotate rm(_rot_angle);
    Affine m(rm);
    m[1] = -m[1];
    m[2] = -m[2];

    Point p = (d / 2) * m;
    double rx2 = _rays[X] * _rays[X];
    double ry2 = _rays[Y] * _rays[Y];
    double rxpy = _rays[X] * p[Y];
    double rypx = _rays[Y] * p[X];
    double rx2py2 = rxpy * rxpy;
    double ry2px2 = rypx * rypx;
    double num = rx2 * ry2;
    double den = rx2py2 + ry2px2;
    assert(den != 0);
    double rad = num / den;
    Point c(0,0);
    if (rad > 1)
    {
        rad -= 1;
        rad = std::sqrt(rad);

        if (_large_arc == _sweep) rad = -rad;
        c = rad * Point(rxpy / ray(Y), -rypx / ray(X));

        _center = c * rm + middle_point(initialPoint(), finalPoint());
    }
    else if (rad == 1 || svg)
    {
        double lamda = std::sqrt(1 / rad);
        _rays[X] *= lamda;
        _rays[Y] *= lamda;
        _center = middle_point(initialPoint(), finalPoint());
    }
    else
    {
        THROW_RANGEERROR(
            "there is no ellipse that satisfies the given constraints"
        );
    }

    Point sp((p[X] - c[X]) / ray(X), (p[Y] - c[Y]) / ray(Y));
    Point ep((-p[X] - c[X]) / ray(X), (-p[Y] - c[Y]) / ray(Y));
    Point v(1, 0);
    _start_angle = angle_between(v, sp);
    double sweep_angle = angle_between(sp, ep);
    if (!_sweep && sweep_angle > 0) sweep_angle -= 2*M_PI;
    if (_sweep && sweep_angle < 0) sweep_angle += 2*M_PI;

    _end_angle = _start_angle;
    _end_angle += sweep_angle;
}

D2<SBasis> EllipticalArc::toSBasis() const
{
    D2<SBasis> arc;
    // the interval of parametrization has to be [0,1]
    Coord et = initialAngle().radians() + ( _sweep ? sweepAngle() : -sweepAngle() );
    Linear param(initialAngle(), et);
    Coord cos_rot_angle, sin_rot_angle;
    sincos(_rot_angle, sin_rot_angle, cos_rot_angle);

    // order = 4 seems to be enough to get a perfect looking elliptical arc
    SBasis arc_x = ray(X) * cos(param,4);
    SBasis arc_y = ray(Y) * sin(param,4);
    arc[0] = arc_x * cos_rot_angle - arc_y * sin_rot_angle + Linear(center(X),center(X));
    arc[1] = arc_x * sin_rot_angle + arc_y * cos_rot_angle + Linear(center(Y),center(Y));

    // ensure that endpoints remain exact
    for ( int d = 0 ; d < 2 ; d++ ) {
        arc[d][0][0] = initialPoint()[d];
        arc[d][0][1] = finalPoint()[d];
    }

    return arc;
}


Curve *EllipticalArc::transformed(Affine const& m) const
{
    Ellipse e(center(X), center(Y), ray(X), ray(Y), _rot_angle);
    Ellipse et = e.transformed(m);
    Point inner_point = pointAt(0.5);
    return et.arc( initialPoint() * m,
                                  inner_point * m,
                                  finalPoint() * m,
                                  isSVGCompliant() );
}

Coord EllipticalArc::map_to_01(Coord angle) const
{
    return map_circular_arc_on_unit_interval(angle, initialAngle(),
                                             finalAngle(), _sweep);
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


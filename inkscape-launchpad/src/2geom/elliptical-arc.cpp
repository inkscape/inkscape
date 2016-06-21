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

#include <2geom/bezier-curve.h>
#include <2geom/ellipse.h>
#include <2geom/elliptical-arc.h>
#include <2geom/path-sink.h>
#include <2geom/sbasis-geometric.h>
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
 * the bigger arc, while the "sweep flag" selects the arc going in the direction of positive
 * angles. Angles always increase when going from the +X axis in the direction of the +Y axis,
 * so if Y grows downwards, this means clockwise.
 *
 * @image html elliptical-arc-flags.png "Meaning of arc flags (Y grows downwards)"
 *
 * @ingroup Curves
 */


/** @brief Compute bounds of an elliptical arc.
 * The bounds computation works as follows. The extreme X and Y points
 * are either the endpoints or local minima / maxima of the ellipse.
 * We already have endpoints, and we can find the local extremes
 * by computing a partial derivative with respect to the angle
 * and equating that to zero:
 * \f{align*}{
     x &= r_x \cos \varphi \cos \theta - r_y \sin \varphi \sin \theta + c_x \\ 
     \frac{\partial x}{\partial \theta} &= -r_x \cos \varphi \sin \theta - r_y \sin \varphi \cos \theta = 0 \\ 
     \frac{\sin \theta}{\cos \theta} &= \tan\theta = -\frac{r_y \sin \varphi}{r_x \cos \varphi} \\ 
     \theta &= \tan^{-1} \frac{-r_y \sin \varphi}{r_x \cos \varphi}
   \f}
 * The local extremes correspond to two angles separated by \f$\pi\f$.
 * Once we compute these angles, we check whether they belong to the arc,
 * and if they do, we evaluate the ellipse at these angles.
 * The bounding box of the arc is equal to the bounding box of the endpoints
 * and the local extrema that belong to the arc.
 */
Rect EllipticalArc::boundsExact() const
{
    if (isChord()) {
        return chord().boundsExact();
    }

    Coord coord[2][4] = {
        { _initial_point[X], _final_point[X], 0, 0 },
        { _initial_point[Y], _final_point[Y], 0, 0 }
    };
    int ncoord[2] = { 2, 2 };

    Angle extremes[2][2];
    double sinrot, cosrot;
    sincos(rotationAngle(), sinrot, cosrot);

    extremes[X][0] = std::atan2( -ray(Y) * sinrot, ray(X) * cosrot );
    extremes[X][1] = extremes[X][0] + M_PI;
    extremes[Y][0] = std::atan2( ray(Y) * cosrot, ray(X) * sinrot );
    extremes[Y][1] = extremes[Y][0] + M_PI;

    for (unsigned d = 0; d < 2; ++d) {
        for (unsigned i = 0; i < 2; ++i) {
            if (containsAngle(extremes[d][i])) {
                coord[d][ncoord[d]++] = valueAtAngle(extremes[d][i], d ? Y : X);
            }
        }
    }

    Interval xival = Interval::from_range(coord[X], coord[X] + ncoord[X]);
    Interval yival = Interval::from_range(coord[Y], coord[Y] + ncoord[Y]);
    Rect result(xival, yival);
    return result;
}


Point EllipticalArc::pointAtAngle(Coord t) const
{
    Point ret = _ellipse.pointAt(t);
    return ret;
}

Coord EllipticalArc::valueAtAngle(Coord t, Dim2 d) const
{
    return _ellipse.valueAt(t, d);
}

std::vector<Coord> EllipticalArc::roots(Coord v, Dim2 d) const
{
    std::vector<Coord> sol;

    if (isChord()) {
        sol = chord().roots(v, d);
        return sol;
    }

    Interval unit_interval(0, 1);

    double rotx, roty;
    if (d == X) {
        sincos(rotationAngle(), roty, rotx);
        roty = -roty;
    } else {
        sincos(rotationAngle(), rotx, roty);
    }

    double rxrotx = ray(X) * rotx;
    double c_v = center(d) - v;

    double a = -rxrotx + c_v;
    double b = ray(Y) * roty;
    double c = rxrotx + c_v;
    //std::cerr << "a = " << a << std::endl;
    //std::cerr << "b = " << b << std::endl;
    //std::cerr << "c = " << c << std::endl;

    if (a == 0)
    {
        sol.push_back(M_PI);
        if (b != 0)
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
        if (delta == 0) {
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
    for (unsigned int i = 0; i < sol.size(); ++i ) {
        //std::cerr << "s = " << deg_from_rad(sol[i]);
        sol[i] = timeAtAngle(sol[i]);
        //std::cerr << " -> t: " << sol[i] << std::endl;
        if (unit_interval.contains(sol[i])) {
            arc_sol.push_back(sol[i]);
        }
    }
    return arc_sol;
}


// D(E(t,C),t) = E(t+PI/2,O), where C is the ellipse center
// the derivative doesn't rotate the ellipse but there is a translation
// of the parameter t by an angle of PI/2 so the ellipse points are shifted
// of such an angle in the cw direction
Curve *EllipticalArc::derivative() const
{
    if (isChord()) {
        return chord().derivative();
    }

    EllipticalArc *result = static_cast<EllipticalArc*>(duplicate());
    result->_ellipse.setCenter(0, 0);
    result->_angles.setInitial(result->_angles.initialAngle() + M_PI/2);
    result->_angles.setFinal(result->_angles.finalAngle() + M_PI/2);
    result->_initial_point = result->pointAtAngle( result->initialAngle() );
    result->_final_point = result->pointAtAngle( result->finalAngle() );
    return result;
}


std::vector<Point>
EllipticalArc::pointAndDerivatives(Coord t, unsigned int n) const
{
    if (isChord()) {
        return chord().pointAndDerivatives(t, n);
    }

    unsigned int nn = n+1; // nn represents the size of the result vector.
    std::vector<Point> result;
    result.reserve(nn);
    double angle = angleAt(t);
    std::auto_ptr<EllipticalArc> ea( static_cast<EllipticalArc*>(duplicate()) );
    ea->_ellipse.setCenter(0, 0);
    unsigned int m = std::min(nn, 4u);
    for ( unsigned int i = 0; i < m; ++i )
    {
        result.push_back( ea->pointAtAngle(angle) );
        angle += (sweep() ? M_PI/2 : -M_PI/2);
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

Point EllipticalArc::pointAt(Coord t) const
{
    if (isChord()) return chord().pointAt(t);
    return _ellipse.pointAt(angleAt(t));
}

Coord EllipticalArc::valueAt(Coord t, Dim2 d) const
{
    if (isChord()) return chord().valueAt(t, d);
    return valueAtAngle(angleAt(t), d);
}

Curve* EllipticalArc::portion(double f, double t) const
{
    // fix input arguments
    if (f < 0) f = 0;
    if (f > 1) f = 1;
    if (t < 0) t = 0;
    if (t > 1) t = 1;

    if (f == t) {
        EllipticalArc *arc = new EllipticalArc();
        arc->_initial_point = arc->_final_point = pointAt(f);
        return arc;
    }

    EllipticalArc *arc = static_cast<EllipticalArc*>(duplicate());
    arc->_initial_point = pointAt(f);
    arc->_final_point = pointAt(t);
    arc->_angles.setAngles(angleAt(f), angleAt(t));
    if (f > t) arc->_angles.setSweep(!sweep());
    if ( _large_arc && fabs(angularExtent() * (t-f)) <= M_PI) {
        arc->_large_arc = false;
    }
    return arc;
}

// the arc is the same but traversed in the opposite direction
Curve *EllipticalArc::reverse() const
{
    using std::swap;
    EllipticalArc *rarc = static_cast<EllipticalArc*>(duplicate());
    rarc->_angles.reverse();
    swap(rarc->_initial_point, rarc->_final_point);
    return rarc;
}

#ifdef HAVE_GSL  // GSL is required for function "solve_reals"
std::vector<double> EllipticalArc::allNearestTimes( Point const& p, double from, double to ) const
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
        Point np = seg.pointAt( seg.nearestTime(p) );
        if ( are_near(ray(Y), 0) )
        {
            if ( are_near(rotationAngle(), M_PI/2)
                 || are_near(rotationAngle(), 3*M_PI/2) )
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
            if ( are_near(rotationAngle(), M_PI/2)
                 || are_near(rotationAngle(), 3*M_PI/2) )
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
    sincos(rotationAngle(), sinrot, cosrot);
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
    double dsq = 0;
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

    double t = timeAtAngle(real_sol[mi1]);
    if ( !(t < from || t > to) )
    {
        result.push_back(t);
    }

    bool second_sol = false;
    t = timeAtAngle(real_sol[mi2]);
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


void EllipticalArc::_filterIntersections(std::vector<ShapeIntersection> &xs, bool is_first) const
{
    Interval unit(0, 1);
    std::vector<ShapeIntersection>::reverse_iterator i = xs.rbegin(), last = xs.rend();
    while (i != last) {
        Coord &t = is_first ? i->first : i->second;
        assert(are_near(_ellipse.pointAt(t), i->point(), 1e-5));
        t = timeAtAngle(t);
        if (!unit.contains(t)) {
            xs.erase((++i).base());
            continue;
        } else {
            assert(are_near(pointAt(t), i->point(), 1e-5));
            ++i;
        }
    }
}

std::vector<CurveIntersection> EllipticalArc::intersect(Curve const &other, Coord eps) const
{
    if (isLineSegment()) {
        LineSegment ls(_initial_point, _final_point);
        return ls.intersect(other, eps);
    }

    std::vector<CurveIntersection> result;

    if (other.isLineSegment()) {
        LineSegment ls(other.initialPoint(), other.finalPoint());
        result = _ellipse.intersect(ls);
        _filterIntersections(result, true);
        return result;
    }

    BezierCurve const *bez = dynamic_cast<BezierCurve const *>(&other);
    if (bez) {
        result = _ellipse.intersect(bez->fragment());
        _filterIntersections(result, true);
        return result;
    }

    EllipticalArc const *arc = dynamic_cast<EllipticalArc const *>(&other);
    if (arc) {
        result = _ellipse.intersect(arc->_ellipse);
        _filterIntersections(result, true);
        arc->_filterIntersections(result, false);
        return result;
    }

    // in case someone wants to make a custom curve type
    result = other.intersect(*this, eps);
    transpose_in_place(result);
    return result;
}


void EllipticalArc::_updateCenterAndAngles()
{
    // See: http://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes
    Point d = initialPoint() - finalPoint();
    Point mid = middle_point(initialPoint(), finalPoint());

    // if ip = sp, the arc contains no other points
    if (initialPoint() == finalPoint()) {
        _ellipse = Ellipse();
        _ellipse.setCenter(initialPoint());
        _angles = AngleInterval();
        _large_arc = false;
        return;
    }

    // rays should be positive
    _ellipse.setRays(std::fabs(ray(X)), std::fabs(ray(Y)));

    if (isChord()) {
        _ellipse.setRays(L2(d) / 2, 0);
        _ellipse.setRotationAngle(atan2(d));
        _ellipse.setCenter(mid);
        _angles.setAngles(0, M_PI);
        _angles.setSweep(false);
        _large_arc = false;
        return;
    }

    Rotate rot(rotationAngle()); // the matrix in F.6.5.3
    Rotate invrot = rot.inverse(); // the matrix in F.6.5.1

    Point r = rays();
    Point p = (initialPoint() - mid) * invrot; // x', y' in F.6.5.1
    Point c(0,0); // cx', cy' in F.6.5.2

    // Correct out-of-range radii
    Coord lambda = hypot(p[X]/r[X], p[Y]/r[Y]);
    if (lambda > 1) {
        r *= lambda;
        _ellipse.setRays(r);
        _ellipse.setCenter(mid);
    } else {
        // evaluate F.6.5.2
        Coord rxry = r[X]*r[X] * r[Y]*r[Y];
        Coord pxry = p[X]*p[X] * r[Y]*r[Y];
        Coord rxpy = r[X]*r[X] * p[Y]*p[Y];
        Coord rad = (rxry - pxry - rxpy)/(rxpy + pxry);
        // normally rad should never be negative, but numerical inaccuracy may cause this
        if (rad > 0) {
            rad = std::sqrt(rad);
            if (sweep() == _large_arc) {
                rad = -rad;
            }
            c = rad * Point(r[X]*p[Y]/r[Y], -r[Y]*p[X]/r[X]);
            _ellipse.setCenter(c * rot + mid);
        } else {
            _ellipse.setCenter(mid);
        }
    }

    // Compute start and end angles.
    // If the ellipse was enlarged, c will be zero - this is correct.
    Point sp((p[X] - c[X]) / r[X], (p[Y] - c[Y]) / r[Y]);
    Point ep((-p[X] - c[X]) / r[X], (-p[Y] - c[Y]) / r[Y]);
    Point v(1, 0);

    _angles.setInitial(angle_between(v, sp));
    _angles.setFinal(angle_between(v, ep));

    /*double sweep_angle = angle_between(sp, ep);
    if (!sweep() && sweep_angle > 0) sweep_angle -= 2*M_PI;
    if (sweep() && sweep_angle < 0) sweep_angle += 2*M_PI;*/
}

D2<SBasis> EllipticalArc::toSBasis() const
{
    if (isChord()) {
        return chord().toSBasis();
    }

    D2<SBasis> arc;
    // the interval of parametrization has to be [0,1]
    Coord et = initialAngle().radians() + sweepAngle();
    Linear param(initialAngle().radians(), et);
    Coord cosrot, sinrot;
    sincos(rotationAngle(), sinrot, cosrot);

    // order = 4 seems to be enough to get a perfect looking elliptical arc
    SBasis arc_x = ray(X) * cos(param,4);
    SBasis arc_y = ray(Y) * sin(param,4);
    arc[0] = arc_x * cosrot - arc_y * sinrot + Linear(center(X), center(X));
    arc[1] = arc_x * sinrot + arc_y * cosrot + Linear(center(Y), center(Y));

    // ensure that endpoints remain exact
    for ( int d = 0 ; d < 2 ; d++ ) {
        arc[d][0][0] = initialPoint()[d];
        arc[d][0][1] = finalPoint()[d];
    }

    return arc;
}

// All operations that do not contain skew can be evaulated
// without passing through the implicit form of the ellipse,
// which preserves precision.

void EllipticalArc::operator*=(Translate const &tr)
{
    _initial_point *= tr;
    _final_point *= tr;
    _ellipse *= tr;
}

void EllipticalArc::operator*=(Scale const &s)
{
    _initial_point *= s;
    _final_point *= s;
    _ellipse *= s;
}

void EllipticalArc::operator*=(Rotate const &r)
{
    _initial_point *= r;
    _final_point *= r;
    _ellipse *= r;
}

void EllipticalArc::operator*=(Zoom const &z)
{
    _initial_point *= z;
    _final_point *= z;
    _ellipse *= z;
}

void EllipticalArc::operator*=(Affine const& m)
{
    if (isChord()) {
        _initial_point *= m;
        _final_point *= m;
        _ellipse.setCenter(middle_point(_initial_point, _final_point));
        _ellipse.setRays(0, 0);
        _ellipse.setRotationAngle(0);
        return;
    }

    _initial_point *= m;
    _final_point *= m;
    _ellipse *= m;
    if (m.det() < 0) {
        _angles.setSweep(!sweep());
    }

    // ellipse transformation does not preserve its functional form,
    // i.e. e.pointAt(0.5)*m and (e*m).pointAt(0.5) can be different.
    // We need to recompute start / end angles.
    _angles.setInitial(_ellipse.timeAt(_initial_point));
    _angles.setFinal(_ellipse.timeAt(_final_point));
}

bool EllipticalArc::operator==(Curve const &c) const
{
    EllipticalArc const *other = dynamic_cast<EllipticalArc const *>(&c);
    if (!other) return false;
    if (_initial_point != other->_initial_point) return false;
    if (_final_point != other->_final_point) return false;
    // TODO: all arcs with ellipse rays which are too small
    //       and fall back to a line should probably be equal
    if (rays() != other->rays()) return false;
    if (rotationAngle() != other->rotationAngle()) return false;
    if (_large_arc != other->_large_arc) return false;
    if (sweep() != other->sweep()) return false;
    return true;
}

bool EllipticalArc::isNear(Curve const &c, Coord precision) const
{
    EllipticalArc const *other = dynamic_cast<EllipticalArc const *>(&c);
    if (!other) {
        if (isChord()) {
            return c.isNear(chord(), precision);
        }
        return false;
    }

    if (!are_near(_initial_point, other->_initial_point, precision)) return false;
    if (!are_near(_final_point, other->_final_point, precision)) return false;
    if (isChord() && other->isChord()) return true;

    if (sweep() != other->sweep()) return false;
    if (!are_near(_ellipse, other->_ellipse, precision)) return false;
    return true;
}

void EllipticalArc::feed(PathSink &sink, bool moveto_initial) const
{
    if (moveto_initial) {
        sink.moveTo(_initial_point);
    }
    sink.arcTo(ray(X), ray(Y), rotationAngle(), _large_arc, sweep(), _final_point);
}

int EllipticalArc::winding(Point const &p) const
{
    using std::swap;

    double sinrot, cosrot;
    sincos(rotationAngle(), sinrot, cosrot);

    Angle ymin_a = std::atan2( ray(Y) * cosrot, ray(X) * sinrot );
    Angle ymax_a = ymin_a + M_PI;

    Point ymin = pointAtAngle(ymin_a);
    Point ymax = pointAtAngle(ymax_a);
    if (ymin[Y] > ymax[Y]) {
        swap(ymin, ymax);
        swap(ymin_a, ymax_a);
    }

    Interval yspan(ymin[Y], ymax[Y]);
    if (!yspan.lowerContains(p[Y])) return 0;

    bool left = cross(ymax - ymin, p - ymin) > 0;
    bool inside = _ellipse.contains(p);
    bool includes_ymin = _angles.contains(ymin_a);
    bool includes_ymax = _angles.contains(ymax_a);

    AngleInterval rarc(ymin_a, ymax_a, true),
                  larc(ymax_a, ymin_a, true);

    // we'll compute the result for an arc in the direction of increasing angles
    // and then negate if necessary
    Angle ia = initialAngle(), fa = finalAngle();
    Point ip = _initial_point, fp = _final_point;
    if (!sweep()) {
        swap(ia, fa);
        swap(ip, fp);
    }

    bool initial_left = larc.contains(ia);
    bool initial_right = !initial_left; //rarc.contains(ia);
    bool final_right = rarc.contains(fa);
    bool final_left = !final_right;//larc.contains(fa);

    int result = 0;
    if (inside || left) {
        if (includes_ymin && final_right) {
            Interval ival(ymin[Y], fp[Y]);
            if (ival.lowerContains(p[Y])) {
                ++result;
            }
        }
        if (initial_right && final_right && !largeArc()) {
            Interval ival(ip[Y], fp[Y]);
            if (ival.lowerContains(p[Y])) {
                ++result;
            }
        }
        if (initial_right && includes_ymax) {
            Interval ival(ip[Y], ymax[Y]);
            if (ival.lowerContains(p[Y])) {
                ++result;
            }
        }
        if (!initial_right && !final_right && includes_ymin && includes_ymax) {
            Interval ival(ymax[Y], ymin[Y]);
            if (ival.lowerContains(p[Y])) {
                ++result;
            }
        }
    }
    if (left && !inside) {
        if (includes_ymin && initial_left) {
            Interval ival(ymin[Y], ip[Y]);
            if (ival.lowerContains(p[Y])) {
                --result;
            }
        }
        if (initial_left && final_left && !largeArc()) {
            Interval ival(ip[Y], fp[Y]);
            if (ival.lowerContains(p[Y])) {
                --result;
            }
        }
        if (final_left && includes_ymax) {
            Interval ival(fp[Y], ymax[Y]);
            if (ival.lowerContains(p[Y])) {
                --result;
            }
        }
        if (!initial_left && !final_left && includes_ymin && includes_ymax) {
            Interval ival(ymax[Y], ymin[Y]);
            if (ival.lowerContains(p[Y])) {
                --result;
            }
        }
    }
    return sweep() ? result : -result;
}

std::ostream &operator<<(std::ostream &out, EllipticalArc const &ea)
{
    out << "EllipticalArc("
        << ea.initialPoint() << ", "
        << format_coord_nice(ea.ray(X)) << ", " << format_coord_nice(ea.ray(Y)) << ", "
        << format_coord_nice(ea.rotationAngle()) << ", "
        << "large_arc=" << (ea.largeArc() ? "true" : "false") << ", "
        << "sweep=" << (ea.sweep() ? "true" : "false") << ", "
        << ea.finalPoint() << ")";
    return out;
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


/**
 * \file
 * \brief  Infinite straight line
 *//*
 * Authors:
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * Copyright 2008-2011 Authors
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

#ifndef LIB2GEOM_SEEN_LINE_H
#define LIB2GEOM_SEEN_LINE_H

#include <cmath>
#include <iostream>
#include <boost/optional.hpp>
#include <2geom/bezier-curve.h> // for LineSegment
#include <2geom/rect.h>
#include <2geom/crossing.h>
#include <2geom/exception.h>
#include <2geom/ray.h>
#include <2geom/angle.h>

namespace Geom
{

class Line {
private:
    Point m_origin;
    Point m_versor;
public:
    /// @name Creating lines.
    /// @{
    /** @brief Create a default horizontal line. */
    Line()
        : m_origin(0,0), m_versor(1,0)
    {}
    /** @brief Create a line with the specified inclination.
     * @param _origin One of the points on the line
     * @param angle Angle of the line in mathematical convention */
    Line(Point const& _origin, Coord angle )
        : m_origin(_origin)
    {
        sincos(angle, m_versor[Y], m_versor[X]);
    }

    /** @brief Create a line going through two points.
     * @param A First point
     * @param B Second point */
    Line(Point const& A, Point const& B) {
        setPoints(A, B);
    }

    /** @brief Create a line based on the coefficients of its equation.
     @see Line::setCoefficients() */
    Line(double a, double b, double c) {
        setCoefficients(a, b, c);
    }

    /** @brief Create a line by extending a line segment. */
    explicit Line(LineSegment const& _segment) {
        setPoints(_segment.initialPoint(), _segment.finalPoint());
    }

    /** @brief Create a line by extending a ray. */
    explicit Line(Ray const& _ray)
        : m_origin(_ray.origin()), m_versor(_ray.versor())
    {}

    // huh?
    static Line from_normal_distance(Point n, double c) {
        Point P = n * c / dot(n,n);
        Line l(P, P+rot90(n));
        return l;
    }
    /** @brief Create a line from origin and unit vector.
     * Note that each line direction has two possible unit vectors.
     * @param o Point through which the line will pass
     * @param v Unit vector of the line's direction */
    static Line from_origin_and_versor(Point o, Point v) {
        Line l;
        l.m_origin = o;
        l.m_versor = v;
        return l;
    }

    Line* duplicate() const {
        return new Line(*this);
    }
    /// @}

    /// @name Retrieve and set the line's parameters.
    /// @{
    /** @brief Get the line's origin point. */
    Point origin() const { return m_origin; }
    /** @brief Get the line's direction unit vector. */
    Point versor() const { return m_versor; }
    // return the angle described by rotating the X-axis in cw direction
    // until it overlaps the line
    // the returned value is in the interval [0, PI[
    Coord angle() const {
        double a = std::atan2(m_versor[Y], m_versor[X]);
        if (a < 0) a += M_PI;
        if (a == M_PI) a = 0;
        return a;
    }

    void setOrigin(Point const& _point) {
        m_origin = _point;
    }
    void setVersor(Point const& _versor) {
        m_versor = _versor;
    }

    void setAngle(Coord _angle) {
        sincos(_angle, m_versor[Y], m_versor[X]);
    }

    /** @brief Set a line based on two points it should pass through. */
    void setPoints(Point const& A, Point const& B) {
        m_origin = A;
        if ( are_near(A, B) )
            m_versor = Point(0,0);
        else
            m_versor = B - A;
            m_versor.normalize();
    }

    void setCoefficients (double a, double b, double c);
    std::vector<double> coefficients() const;

    /** @brief Check if the line has any points.
     * A degenerate line can be created if the line is created from a line equation
     * that has no solutions.
     * @return True if the line has no points */
    bool isDegenerate() const {
        return ( m_versor[X] == 0 && m_versor[Y] == 0 );
    }
    /// @}

    /// @name Evaluate the line as a function.
    ///@{
    Point pointAt(Coord t) const {
        return m_origin + m_versor * t;
    }

    Coord valueAt(Coord t, Dim2 d) const {
        if (d < 0 || d > 1)
            THROW_RANGEERROR("Line::valueAt, dimension argument out of range");
        return m_origin[d] + m_versor[d] * t;
    }

    Coord timeAt(Point const &p) const;

    /** @brief Get a time value corresponding to a projection of a point on the line.
     * @param p Arbitrary point.
     * @return Time value corresponding to a point closest to @c p. */
    Coord timeAtProjection(Point const& p) const {
        if ( isDegenerate() ) return 0;
        return dot( p - m_origin, m_versor );
    }

    /** @brief Find a point on the line closest to the query point.
     * This is an alias for timeAtProjection(). */
    Coord nearestPoint(Point const& _point) const {
        return timeAtProjection(_point);
    }

    std::vector<Coord> roots(Coord v, Dim2 d) const;
    /// @}

    /// @name Create other objects based on this line.
    /// @{
    /** @brief Create a line containing the same points, but with negated time values.
     * @return Line \f$g\f$ such that \f$g(t) = f(-t)\f$ */
    Line reverse() const
    {
        Line result;
        result.setOrigin(m_origin);
        result.setVersor(-m_versor);
        return result;
    }

    /** @brief Same as segment(), but allocate the line segment dynamically. */
    // TODO remove this?
    Curve* portion(Coord  f, Coord t) const {
        LineSegment* seg = new LineSegment(pointAt(f), pointAt(t));
        return seg;
    }

    /** @brief Create a segment of this line.
     * @param f Time value for the initial point of the segment
     * @param t Time value for the final point of the segment
     * @return Created line segment */
    LineSegment segment(Coord  f, Coord t) const {
        return LineSegment(pointAt(f), pointAt(t));
    }

    /** @brief Create a ray starting at the specified time value.
     * The created ray will go in the direction of the line's versor (in the direction
     * of increasing time values).
     * @param t Time value where the ray should start
     * @return Ray starting at t and going in the direction of the versor */
    Ray ray(Coord t) {
        Ray result;
        result.setOrigin(pointAt(t));
        result.setVersor(m_versor);
        return result;
    }

    /** @brief Create a derivative of the line.
     * The new line will always be degenerate. Its origin will be equal to this
     * line's versor. */
    Line derivative() const {
        Line result;
        result.setOrigin(m_versor);
        result.setVersor(Point(0,0));
        return result;
    }

    /** @brief Create a line transformed by an affine transformation. */
    Line transformed(Affine const& m) const {
        return Line(m_origin * m, (m_origin + m_versor) * m);
    }

    /** @brief Get a vector normal to the line.
     * If Y grows upwards, then this is the left normal. If Y grows downwards,
     * then this is the right normal. */
    Point normal() const {
        return rot90(m_versor);
    }

    // what does this do?
    Point normalAndDist(double & dist) const {
        Point n = normal();
        dist = -dot(n, m_origin);
        return n;
    }

    friend inline std::ostream &operator<< (std::ostream &out_file, const Geom::Line &in_line);
/// @}
}; // end class Line

/** @brief Output operator for lines.
 * Prints out representation (point + versor)
 */
inline std::ostream &operator<< (std::ostream &out_file, const Geom::Line &in_line) {
    out_file << "X: "   << in_line.m_origin[X] << " Y: "  << in_line.m_origin[Y]
             << " dX: " << in_line.m_versor[X] << " dY: " << in_line.m_versor[Y];
    return out_file;
}

inline
double distance(Point const& _point, Line const& _line)
{
    if ( _line.isDegenerate() )
    {
        return ::Geom::distance( _point, _line.origin() );
    }
    else
    {
        return fabs( dot(_point - _line.origin(), _line.versor().ccw()) );
    }
}

inline
bool are_near(Point const& _point, Line const& _line, double eps = EPSILON)
{
    return are_near(distance(_point, _line), 0, eps);
}

inline
bool are_parallel(Line const& l1, Line const& l2, double eps = EPSILON)
{
    return ( are_near(l1.versor(), l2.versor(), eps)
             || are_near(l1.versor(), -l2.versor(), eps) );
}

inline
bool are_same(Line const& l1, Line const& l2, double eps = EPSILON)
{
    return are_parallel(l1, l2, eps) && are_near(l1.origin(), l2, eps);
}

inline
bool are_orthogonal(Line const& l1, Line const& l2, double eps = EPSILON)
{
    return ( are_near(l1.versor(), l2.versor().cw(), eps)
             || are_near(l1.versor(), l2.versor().ccw(), eps) );
}

inline
bool are_collinear(Point const& p1, Point const& p2, Point const& p3,
                   double eps = EPSILON)
{
    return are_near( cross(p3, p2) - cross(p3, p1) + cross(p2, p1), 0, eps);
}

// evaluate the angle between l1 and l2 rotating l1 in cw direction
// until it overlaps l2
// the returned value is an angle in the interval [0, PI[
inline
double angle_between(Line const& l1, Line const& l2)
{
    double angle = angle_between(l1.versor(), l2.versor());
    if (angle < 0) angle += M_PI;
    if (angle == M_PI) angle = 0;
    return angle;
}

inline
double distance(Point const& _point, LineSegment const& _segment)
{
    double t = _segment.nearestPoint(_point);
    return L2(_point - _segment.pointAt(t));
}

inline
bool are_near(Point const& _point, LineSegment const& _segment,
              double eps = EPSILON)
{
    return are_near(distance(_point, _segment), 0, eps);
}

// build a line passing by _point and orthogonal to _line
inline
Line make_orthogonal_line(Point const& _point, Line const& _line)
{
    Line l;
    l.setOrigin(_point);
    l.setVersor(_line.versor().cw());
    return l;
}

// build a line passing by _point and parallel to _line
inline
Line make_parallel_line(Point const& _point, Line const& _line)
{
    Line l(_line);
    l.setOrigin(_point);
    return l;
}

// build a line passing by the middle point of _segment and orthogonal to it.
inline
Line make_bisector_line(LineSegment const& _segment)
{
    return make_orthogonal_line( middle_point(_segment), Line(_segment) );
}

// build the bisector line of the angle between ray(O,A) and ray(O,B)
inline
Line make_angle_bisector_line(Point const& A, Point const& O, Point const& B)
{
    Point M = middle_point(A,B);
    if (are_near(O,M)) {
        Line l(A,B);
        M += (make_orthogonal_line(O,l)).versor();
    }
    return Line(O,M);
}

// prj(P) = rot(v, Point( rot(-v, P-O)[X], 0 )) + O
inline
Point projection(Point const& _point, Line const& _line)
{
    return _line.pointAt( _line.nearestPoint(_point) );
}

inline
LineSegment projection(LineSegment const& _segment, Line const& _line)
{
    return _line.segment( _line.nearestPoint(_segment.initialPoint()),
                          _line.nearestPoint(_segment.finalPoint()) );
}

boost::optional<LineSegment> clip (Line const& l, Rect const& r);


namespace detail
{

OptCrossing intersection_impl(Ray const& r1, Line const& l2, unsigned int i);
OptCrossing intersection_impl( LineSegment const& ls1,
                             Line const& l2,
                             unsigned int i );
OptCrossing intersection_impl( LineSegment const& ls1,
                             Ray const& r2,
                             unsigned int i );
}


inline
OptCrossing intersection(Ray const& r1, Line const& l2)
{
    return detail::intersection_impl(r1,  l2, 0);

}

inline
OptCrossing intersection(Line const& l1, Ray const& r2)
{
    return detail::intersection_impl(r2,  l1, 1);
}

inline
OptCrossing intersection(LineSegment const& ls1, Line const& l2)
{
    return detail::intersection_impl(ls1,  l2, 0);
}

inline
OptCrossing intersection(Line const& l1, LineSegment const& ls2)
{
    return detail::intersection_impl(ls2,  l1, 1);
}

inline
OptCrossing intersection(LineSegment const& ls1, Ray const& r2)
{
    return detail::intersection_impl(ls1,  r2, 0);

}

inline
OptCrossing intersection(Ray const& r1, LineSegment const& ls2)
{
    return detail::intersection_impl(ls2,  r1, 1);
}


OptCrossing intersection(Line const& l1, Line const& l2);

OptCrossing intersection(Ray const& r1, Ray const& r2);

OptCrossing intersection(LineSegment const& ls1, LineSegment const& ls2);


} // end namespace Geom


#endif // LIB2GEOM_SEEN_LINE_H


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

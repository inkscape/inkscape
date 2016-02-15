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
#include <boost/optional.hpp>
#include <2geom/bezier-curve.h> // for LineSegment
#include <2geom/rect.h>
#include <2geom/crossing.h>
#include <2geom/exception.h>
#include <2geom/ray.h>
#include <2geom/angle.h>
#include <2geom/intersection.h>

namespace Geom
{

// class docs in cpp file
class Line
    : boost::equality_comparable< Line >
{
private:
    Point _initial;
    Point _final;
public:
    /// @name Creating lines.
    /// @{
    /** @brief Create a default horizontal line.
     * Creates a line with unit speed going in +X direction. */
    Line()
        : _initial(0,0), _final(1,0)
    {}
    /** @brief Create a line with the specified inclination.
     * @param origin One of the points on the line
     * @param angle Angle of the line in mathematical convention */
    Line(Point const &origin, Coord angle)
        : _initial(origin)
    {
        Point v;
        sincos(angle, v[Y], v[X]);
        _final = _initial + v;
    }

    /** @brief Create a line going through two points.
     * The first point will be at time 0, while the second one
     * will be at time 1.
     * @param a Initial point
     * @param b First point */
    Line(Point const &a, Point const &b)
        : _initial(a)
        , _final(b)
    {}

    /** @brief Create a line based on the coefficients of its equation.
     @see Line::setCoefficients() */
    Line(double a, double b, double c) {
        setCoefficients(a, b, c);
    }

    /// Create a line by extending a line segment.
    explicit Line(LineSegment const &seg)
        : _initial(seg.initialPoint())
        , _final(seg.finalPoint())
    {}

    /// Create a line by extending a ray.
    explicit Line(Ray const &r)
        : _initial(r.origin())
        , _final(r.origin() + r.vector())
    {}

    /// Create a line normal to a vector at a specified distance from origin.
    static Line from_normal_distance(Point const &n, Coord c) {
        Point start = c * n.normalized();
        Line l(start, start + rot90(n));
        return l;
    }
    /** @brief Create a line from origin and unit vector.
     * Note that each line direction has two possible unit vectors.
     * @param o Point through which the line will pass
     * @param v Unit vector of the line's direction */
    static Line from_origin_and_vector(Point const &o, Point const &v) {
        Line l(o, o + v);
        return l;
    }

    Line* duplicate() const {
        return new Line(*this);
    }
    /// @}

    /// @name Retrieve and set the line's parameters.
    /// @{

    /// Get the line's origin point.
    Point origin() const { return _initial; }
    /** @brief Get the line's raw direction vector.
     * The retrieved vector is normalized to unit length. */
    Point vector() const { return _final - _initial; }
    /** @brief Get the line's normalized direction vector.
     * The retrieved vector is normalized to unit length. */
    Point versor() const { return (_final - _initial).normalized(); }
    /// Angle the line makes with the X axis, in mathematical convention.
    Coord angle() const {
        Point d = _final - _initial;
        double a = std::atan2(d[Y], d[X]);
        if (a < 0) a += M_PI;
        if (a == M_PI) a = 0;
        return a;
    }

    /** @brief Set the point at zero time.
     * The orientation remains unchanged, modulo numeric errors during addition. */
    void setOrigin(Point const &p) {
        Point d = p - _initial;
        _initial = p;
        _final += d;
    }
    /** @brief Set the speed of the line.
     * Origin remains unchanged. */
    void setVector(Point const &v) {
        _final = _initial + v;
    }

    /** @brief Set the angle the line makes with the X axis.
     * Origin remains unchanged. */
    void setAngle(Coord angle) {
        Point v;
        sincos(angle, v[Y], v[X]);
        v *= distance(_initial, _final);
        _final = _initial + v;
    }

    /// Set a line based on two points it should pass through.
    void setPoints(Point const &a, Point const &b) {
        _initial = a;
        _final = b;
    }

    /** @brief Set the coefficients of the line equation.
     * The line equation is: \f$ax + by = c\f$. Points that satisfy the equation
     * are on the line. */
    void setCoefficients(double a, double b, double c);

    /** @brief Get the coefficients of the line equation as a vector.
     * @return STL vector @a v such that @a v[0] contains \f$a\f$, @a v[1] contains \f$b\f$,
     * and @a v[2] contains \f$c\f$. */
    std::vector<double> coefficients() const;

    /// Get the coefficients of the line equation by reference.
    void coefficients(Coord &a, Coord &b, Coord &c) const;

    /** @brief Check if the line has more than one point.
     * A degenerate line can be created if the line is created from a line equation
     * that has no solutions.
     * @return True if the line has no points or exactly one point */
    bool isDegenerate() const {
        return _initial == _final;
    }
    /// Check if the line is horizontal (y is constant).
    bool isHorizontal() const {
        return _initial[Y] == _final[Y];
    }
    /// Check if the line is vertical (x is constant).
    bool isVertical() const {
        return _initial[X] == _final[X];
    }

    /** @brief Reparametrize the line so that it has unit speed.
     * Note that the direction of the line may also change. */
    void normalize() {
        // this helps with the nasty case of a line that starts somewhere far
        // and ends very close to the origin
        if (L2sq(_final) < L2sq(_initial)) {
            std::swap(_initial, _final);
        }
        Point v = _final - _initial;
        v.normalize();
        _final = _initial + v;
    }
    /** @brief Return a new line reparametrized for unit speed. */
    Line normalized() const {
        Point v = _final - _initial;
        v.normalize();
        Line ret(_initial, _initial + v);
        return ret;
    }
    /// @}

    /// @name Evaluate the line as a function.
    ///@{
    Point initialPoint() const {
        return _initial;
    }
    Point finalPoint() const {
        return _final;
    }
    Point pointAt(Coord t) const {
        return lerp(t, _initial, _final);;
    }

    Coord valueAt(Coord t, Dim2 d) const {
        return lerp(t, _initial[d], _final[d]);
    }

    Coord timeAt(Point const &p) const;

    /** @brief Get a time value corresponding to a projection of a point on the line.
     * @param p Arbitrary point.
     * @return Time value corresponding to a point closest to @c p. */
    Coord timeAtProjection(Point const& p) const {
        if ( isDegenerate() ) return 0;
        Point v = vector();
        return dot(p - _initial, v) / dot(v, v);
    }

    /** @brief Find a point on the line closest to the query point.
     * This is an alias for timeAtProjection(). */
    Coord nearestTime(Point const &p) const {
        return timeAtProjection(p);
    }

    std::vector<Coord> roots(Coord v, Dim2 d) const;
    Coord root(Coord v, Dim2 d) const;
    /// @}

    /// @name Create other objects based on this line.
    /// @{
    void reverse() {
        std::swap(_final, _initial);
    }
    /** @brief Create a line containing the same points, but in opposite direction.
     * @return Line \f$g\f$ such that \f$g(t) = f(1-t)\f$ */
    Line reversed() const {
        Line result(_final, _initial);
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

    /// Return the portion of the line that is inside the given rectangle
    boost::optional<LineSegment> clip(Rect const &r) const;

    /** @brief Create a ray starting at the specified time value.
     * The created ray will go in the direction of the line's vector (in the direction
     * of increasing time values).
     * @param t Time value where the ray should start
     * @return Ray starting at t and going in the direction of the vector */
    Ray ray(Coord t) {
        Ray result;
        result.setOrigin(pointAt(t));
        result.setVector(vector());
        return result;
    }

    /** @brief Create a derivative of the line.
     * The new line will always be degenerate. Its origin will be equal to this
     * line's vector. */
    Line derivative() const {
        Point v = vector();
        Line result(v, v);
        return result;
    }

    /// Create a line transformed by an affine transformation.
    Line transformed(Affine const& m) const {
        Line l(_initial * m, _final * m);
        return l;
    }

    /** @brief Get a unit vector normal to the line.
     * If Y grows upwards, then this is the left normal. If Y grows downwards,
     * then this is the right normal. */
    Point normal() const {
        return rot90(vector()).normalized();
    }

    // what does this do?
    Point normalAndDist(double & dist) const {
        Point n = normal();
        dist = -dot(n, _initial);
        return n;
    }

    /// Compute an affine matrix representing a reflection about the line.
    Affine reflection() const {
        Point v = versor();
        Coord x2 = v[X]*v[X], y2 = v[Y]*v[Y], xy = v[X]*v[Y];
        Affine m(x2-y2, 2.*xy,
                 2.*xy, y2-x2,
                 _initial[X], _initial[Y]);
        m = Translate(-_initial) * m;
        return m;
    }

    /** @brief Compute an affine which transforms all points on the line to zero X or Y coordinate.
     * This operation is useful in reducing intersection problems to root-finding problems.
     * There are many affines which do this transformation. This function returns one that
     * preserves angles, areas and distances - a rotation combined with a translation, and
     * additionaly moves the initial point of the line to (0,0). This way it works without
     * problems even for lines perpendicular to the target, though may in some cases have
     * lower precision than e.g. a shear transform.
     * @param d Which coordinate of points on the line should be zero after the transformation */
    Affine rotationToZero(Dim2 d) const {
        Point v = vector();
        if (d == X) {
            std::swap(v[X], v[Y]);
        } else {
            v[Y] = -v[Y];
        }
        Affine m = Translate(-_initial) * Rotate(v);
        return m;
    }
    /** @brief Compute a rotation affine which transforms the line to one of the axes.
     * @param d Which line should be the axis */
    Affine rotationToAxis(Dim2 d) const {
        Affine m = rotationToZero(other_dimension(d));
        return m;
    }

    Affine transformTo(Line const &other) const;
    /// @}

    std::vector<ShapeIntersection> intersect(Line const &other) const;
    std::vector<ShapeIntersection> intersect(Ray const &r) const;
    std::vector<ShapeIntersection> intersect(LineSegment const &ls) const;

    template <typename T>
    Line &operator*=(T const &tr) {
        BOOST_CONCEPT_ASSERT((TransformConcept<T>));
        _initial *= tr;
        _final *= tr;
        return *this;
    }

    bool operator==(Line const &other) const {
        if (distance(pointAt(nearestTime(other._initial)), other._initial) != 0) return false;
        if (distance(pointAt(nearestTime(other._final)), other._final) != 0) return false;
        return true;
    }

    template <typename T>
    friend Line operator*(Line const &l, T const &tr) {
        BOOST_CONCEPT_ASSERT((TransformConcept<T>));
        Line result(l);
        result *= tr;
        return result;
    }
}; // end class Line

/** @brief Removes intersections outside of the unit interval.
 * A helper used to implement line segment intersections.
 * @param xs Line intersections
 * @param a Whether the first time value has to be in the unit interval
 * @param b Whether the second time value has to be in the unit interval
 * @return Appropriately filtered intersections */
void filter_line_segment_intersections(std::vector<ShapeIntersection> &xs, bool a=false, bool b=true);
void filter_ray_intersections(std::vector<ShapeIntersection> &xs, bool a=false, bool b=true);

/// @brief Compute distance from point to line.
/// @relates Line
inline
double distance(Point const &p, Line const &line)
{
    if (line.isDegenerate()) {
        return ::Geom::distance(p, line.initialPoint());
    } else {
        Coord t = line.nearestTime(p);
        return ::Geom::distance(line.pointAt(t), p);
    }
}

inline
bool are_near(Point const &p, Line const &line, double eps = EPSILON)
{
    return are_near(distance(p, line), 0, eps);
}

inline
bool are_parallel(Line const &l1, Line const &l2, double eps = EPSILON)
{
    return are_near(cross(l1.vector(), l2.vector()), 0, eps);
}

/** @brief Test whether two lines are approximately the same.
 * This tests for being parallel and the origin of one line being close to the other,
 * so it tests whether the images of the lines are similar, not whether the same time values
 * correspond to similar points. For example a line from (1,1) to (2,2) and a line from
 * (-1,-1) to (0,0) will the the same, because their images match, even though there is
 * no time value for which the lines give similar points.
 * @relates Line */
inline
bool are_same(Line const &l1, Line const &l2, double eps = EPSILON)
{
    return are_parallel(l1, l2, eps) && are_near(l1.origin(), l2, eps);
}

/// Test whether two lines are perpendicular.
/// @relates Line
inline
bool are_orthogonal(Line const &l1, Line const &l2, double eps = EPSILON)
{
    return are_near(dot(l1.vector(), l2.vector()), 0, eps);
}

// evaluate the angle between l1 and l2 rotating l1 in cw direction
// until it overlaps l2
// the returned value is an angle in the interval [0, PI[
inline
double angle_between(Line const& l1, Line const& l2)
{
    double angle = angle_between(l1.vector(), l2.vector());
    if (angle < 0) angle += M_PI;
    if (angle == M_PI) angle = 0;
    return angle;
}

inline
double distance(Point const &p, LineSegment const &seg)
{
    double t = seg.nearestTime(p);
    return distance(p, seg.pointAt(t));
}

inline
bool are_near(Point const &p, LineSegment const &seg, double eps = EPSILON)
{
    return are_near(distance(p, seg), 0, eps);
}

// build a line passing by _point and orthogonal to _line
inline
Line make_orthogonal_line(Point const &p, Line const &line)
{
    Point d = line.vector().cw();
    Line l(p, p + d);
    return l;
}

// build a line passing by _point and parallel to _line
inline
Line make_parallel_line(Point const &p, Line const &line)
{
    Line result(line);
    result.setOrigin(p);
    return result;
}

// build a line passing by the middle point of _segment and orthogonal to it.
inline
Line make_bisector_line(LineSegment const& _segment)
{
    return make_orthogonal_line( middle_point(_segment), Line(_segment) );
}

// build the bisector line of the angle between ray(O,A) and ray(O,B)
inline
Line make_angle_bisector_line(Point const &A, Point const &O, Point const &B)
{
    AngleInterval ival(Angle(A-O), Angle(B-O));
    Angle bisect = ival.angleAt(0.5);
    return Line(O, bisect);
}

// prj(P) = rot(v, Point( rot(-v, P-O)[X], 0 )) + O
inline
Point projection(Point const &p, Line const &line)
{
    return line.pointAt(line.nearestTime(p));
}

inline
LineSegment projection(LineSegment const &seg, Line const &line)
{
    return line.segment(line.nearestTime(seg.initialPoint()),
                        line.nearestTime(seg.finalPoint()));
}

inline
boost::optional<LineSegment> clip(Line const &l, Rect const &r) {
    return l.clip(r);
}


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

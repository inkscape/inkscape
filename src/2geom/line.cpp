/*
 * Infinite Straight Line
 *
 * Copyright 2008  Marco Cecchetti <mrcekets at gmail.com>
 * Nathan Hurst
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

#include <algorithm>
#include <2geom/line.h>
#include <2geom/math-utils.h>

namespace Geom
{

/**
 * @class Line
 * @brief Infinite line on a plane.
 *
 * A line is specified as two points through which it passes. Lines can be interpreted as functions
 * \f$ f: (-\infty, \infty) \to \mathbb{R}^2\f$. Zero corresponds to the first (origin) point,
 * one corresponds to the second (final) point. All other points are computed as a linear
 * interpolation between those two: \f$p = (1-t) a + t b\f$. Many such functions have the same
 * image and therefore represent the same lines; for example, adding \f$b-a\f$ to both points
 * yields the same line.
 *
 * 2Geom can represent the same line in many ways by design: using a different representation
 * would lead to precision loss. For example, a line from (1e30, 1e30) to (10,0) would actually
 * evaluate to (0,0) at time 1 if it was stored as origin and normalized versor,
 * or origin and angle.
 *
 * @ingroup Primitives
 */

/** @brief Set the line by solving the line equation.
 * A line is a set of points that satisfies the line equation
 * \f$Ax + By + C = 0\f$. This function changes the line so that its points
 * satisfy the line equation with the given coefficients. */
void Line::setCoefficients (Coord a, Coord b, Coord c)
{
    // degenerate case
    if (a == 0 && b == 0) {
        if (c != 0) {
            THROW_LOGICALERROR("the passed coefficients give the empty set");
        }
        _initial = Point(0,0);
        _final = Point(0,0);
        return;
    }

    // The way final / initial points are set based on coefficients is somewhat unusual.
    // This is done to make sure that calling coefficients() will give back
    // (almost) the same values.

    // vertical case
    if (a == 0) {
        // b must be nonzero
        _initial = Point(-b/2, -c / b);
        _final = _initial;
        _final[X] = b/2;
        return;
    }

    // horizontal case
    if (b == 0) {
        _initial = Point(-c / a, a/2);
        _final = _initial;
        _final[Y] = -a/2;
        return;
    }

    // This gives reasonable results regardless of the magnitudes of a, b and c.
    _initial = Point(-b/2,a/2);
    _final = Point(b/2,-a/2);

    Point offset(-c/(2*a), -c/(2*b));

    _initial += offset;
    _final += offset;
}

void Line::coefficients(Coord &a, Coord &b, Coord &c) const
{
    Point v = vector().cw();
    a = v[X];
    b = v[Y];
    c = cross(_initial, _final);
}

/** @brief Get the implicit line equation coefficients.
 * Note that conversion to implicit form always causes loss of
 * precision when dealing with lines that start far from the origin
 * and end very close to it. It is recommended to normalize the line
 * before converting it to implicit form.
 * @return Vector with three values corresponding to the A, B and C
 *         coefficients of the line equation for this line. */
std::vector<Coord> Line::coefficients() const
{
    std::vector<Coord> c(3);
    coefficients(c[0], c[1], c[2]);
    return c;
}

/** @brief Find intersection with an axis-aligned line.
 * @param v Coordinate of the axis-aligned line
 * @param d Which axis the coordinate is on. X means a vertical line, Y means a horizontal line.
 * @return Time values at which this line intersects the query line. */
std::vector<Coord> Line::roots(Coord v, Dim2 d) const {
    std::vector<Coord> result;
    Coord r = root(v, d);
    if (IS_FINITE(r)) {
        result.push_back(r);
    }
    return result;
}

Coord Line::root(Coord v, Dim2 d) const
{
    assert(d == X || d == Y);
    Point vs = vector();
    if (vs[d] != 0) {
        return (v - _initial[d]) / vs[d];
    } else {
        return nan("");
    }
}

boost::optional<LineSegment> Line::clip(Rect const &r) const
{
    Point v = vector();
    // handle horizontal and vertical lines first,
    // since the root-based code below will break for them
    for (unsigned i = 0; i < 2; ++i) {
        Dim2 d = (Dim2) i;
        Dim2 o = other_dimension(d);
        if (v[d] != 0) continue;
        if (r[d].contains(_initial[d])) {
            Point a, b;
            a[o] = r[o].min();
            b[o] = r[o].max();
            a[d] = b[d] = _initial[d];
            if (v[o] > 0) {
                return LineSegment(a, b);
            } else {
                return LineSegment(b, a);
            }
        } else {
            return boost::none;
        }
    }

    Interval xpart(root(r[X].min(), X), root(r[X].max(), X));
    Interval ypart(root(r[Y].min(), Y), root(r[Y].max(), Y));
    if (!xpart.isFinite() || !ypart.isFinite()) {
        return boost::none;
    }

    OptInterval common = xpart & ypart;
    if (common) {
        Point p1 = pointAt(common->min()), p2 = pointAt(common->max());
        LineSegment result(r.clamp(p1), r.clamp(p2));
        return result;
    } else {
        return boost::none;
    }

    /* old implementation using coefficients:

    if (fabs(b) > fabs(a)) {
        p0 = Point(r[X].min(), (-c - a*r[X].min())/b);
        if (p0[Y] < r[Y].min())
            p0 = Point((-c - b*r[Y].min())/a, r[Y].min());
        if (p0[Y] > r[Y].max())
            p0 = Point((-c - b*r[Y].max())/a, r[Y].max());
        p1 = Point(r[X].max(), (-c - a*r[X].max())/b);
        if (p1[Y] < r[Y].min())
            p1 = Point((-c - b*r[Y].min())/a, r[Y].min());
        if (p1[Y] > r[Y].max())
            p1 = Point((-c - b*r[Y].max())/a, r[Y].max());
    } else {
        p0 = Point((-c - b*r[Y].min())/a, r[Y].min());
        if (p0[X] < r[X].min())
            p0 = Point(r[X].min(), (-c - a*r[X].min())/b);
        if (p0[X] > r[X].max())
            p0 = Point(r[X].max(), (-c - a*r[X].max())/b);
        p1 = Point((-c - b*r[Y].max())/a, r[Y].max());
        if (p1[X] < r[X].min())
            p1 = Point(r[X].min(), (-c - a*r[X].min())/b);
        if (p1[X] > r[X].max())
            p1 = Point(r[X].max(), (-c - a*r[X].max())/b);
    }
    return LineSegment(p0, p1); */
}

/** @brief Get a time value corresponding to a point.
 * @param p Point on the line. If the point is not on the line,
 *          the returned value will be meaningless.
 * @return Time value t such that \f$f(t) = p\f$.
 * @see timeAtProjection */
Coord Line::timeAt(Point const &p) const
{
    Point v = vector();
    // degenerate case
    if (v[X] == 0 && v[Y] == 0) {
        return 0;
    }

    // use the coordinate that will give better precision
    if (fabs(v[X]) > fabs(v[Y])) {
        return (p[X] - _initial[X]) / v[X];
    } else {
        return (p[Y] - _initial[Y]) / v[Y];
    }
}

/** @brief Create a transformation that maps one line to another.
 * This will return a transformation \f$A\f$ such that
 * \f$L_1(t) \cdot A = L_2(t)\f$, where \f$L_1\f$ is this line
 * and \f$L_2\f$ is the line passed as the parameter. The returned
 * transformation will preserve angles. */
Affine Line::transformTo(Line const &other) const
{
    Affine result = Translate(-_initial);
    result *= Rotate(angle_between(vector(), other.vector()));
    result *= Scale(other.vector().length() / vector().length());
    result *= Translate(other._initial);
    return result;
}

std::vector<ShapeIntersection> Line::intersect(Line const &other) const
{
    std::vector<ShapeIntersection> result;

    Point v1 = vector();
    Point v2 = other.vector();
    Coord cp = cross(v1, v2);
    if (cp == 0) return result;

    Point odiff = other.initialPoint() - initialPoint();
    Coord t1 = cross(odiff, v2) / cp;
    Coord t2 = cross(odiff, v1) / cp;
    result.push_back(ShapeIntersection(*this, other, t1, t2));
    return result;
}

std::vector<ShapeIntersection> Line::intersect(Ray const &r) const
{
    Line other(r);
    std::vector<ShapeIntersection> result = intersect(other);
    filter_ray_intersections(result, false, true);
    return result;
}

std::vector<ShapeIntersection> Line::intersect(LineSegment const &ls) const
{
    Line other(ls);
    std::vector<ShapeIntersection> result = intersect(other);
    filter_line_segment_intersections(result, false, true);
    return result;
}



void filter_line_segment_intersections(std::vector<ShapeIntersection> &xs, bool a, bool b)
{
    Interval unit(0, 1);
    std::vector<ShapeIntersection>::reverse_iterator i = xs.rbegin(), last = xs.rend();
    while (i != last) {
        if ((a && !unit.contains(i->first)) || (b && !unit.contains(i->second))) {
            xs.erase((++i).base());
        } else {
            ++i;
        }
    }
}

void filter_ray_intersections(std::vector<ShapeIntersection> &xs, bool a, bool b)
{
    Interval unit(0, 1);
    std::vector<ShapeIntersection>::reverse_iterator i = xs.rbegin(), last = xs.rend();
    while (i != last) {
        if ((a && i->first < 0) || (b && i->second < 0)) {
            xs.erase((++i).base());
        } else {
            ++i;
        }
    }
}

namespace detail
{

inline
OptCrossing intersection_impl(Point const &v1, Point const &o1,
                              Point const &v2, Point const &o2)
{
    Coord cp = cross(v1, v2);
    if (cp == 0) return OptCrossing();

    Point odiff = o2 - o1;

    Crossing c;
    c.ta = cross(odiff, v2) / cp;
    c.tb = cross(odiff, v1) / cp;
    return c;
}


OptCrossing intersection_impl(Ray const& r1, Line const& l2, unsigned int i)
{
    using std::swap;

    OptCrossing crossing =
        intersection_impl(r1.vector(), r1.origin(),
                          l2.vector(), l2.origin() );

    if (crossing) {
        if (crossing->ta < 0) {
            return OptCrossing();
        } else {
            if (i != 0) {
                swap(crossing->ta, crossing->tb);
            }
            return crossing;
        }
    }
    if (are_near(r1.origin(), l2)) {
        THROW_INFINITESOLUTIONS();
    } else {
        return OptCrossing();
    }
}


OptCrossing intersection_impl( LineSegment const& ls1,
                               Line const& l2,
                               unsigned int i )
{
    using std::swap;

    OptCrossing crossing =
        intersection_impl(ls1.finalPoint() - ls1.initialPoint(),
                          ls1.initialPoint(),
                          l2.vector(),
                          l2.origin() );

    if (crossing) {
        if ( crossing->getTime(0) < 0
             || crossing->getTime(0) > 1 )
        {
            return OptCrossing();
        } else {
            if (i != 0) {
                swap((*crossing).ta, (*crossing).tb);
            }
            return crossing;
        }
    }
    if (are_near(ls1.initialPoint(), l2)) {
        THROW_INFINITESOLUTIONS();
    } else {
        return OptCrossing();
    }
}


OptCrossing intersection_impl( LineSegment const& ls1,
                               Ray const& r2,
                               unsigned int i )
{
    using std::swap;

    Point direction = ls1.finalPoint() - ls1.initialPoint();
    OptCrossing crossing =
        intersection_impl( direction,
                           ls1.initialPoint(),
                           r2.vector(),
                           r2.origin() );

    if (crossing) {
        if ( (crossing->getTime(0) < 0)
             || (crossing->getTime(0) > 1)
             || (crossing->getTime(1) < 0) )
        {
            return OptCrossing();
        } else {
            if (i != 0) {
                swap(crossing->ta, crossing->tb);
            }
            return crossing;
        }
    }

    if ( are_near(r2.origin(), ls1) ) {
        bool eqvs = (dot(direction, r2.vector()) > 0);
        if ( are_near(ls1.initialPoint(), r2.origin()) && !eqvs)  {
            crossing->ta = crossing->tb = 0;
            return crossing;
        } else if ( are_near(ls1.finalPoint(), r2.origin()) && eqvs) {
            if (i == 0) {
                crossing->ta = 1;
                crossing->tb = 0;
            }  else {
                crossing->ta = 0;
                crossing->tb = 1;
            }
            return crossing;
        } else {
            THROW_INFINITESOLUTIONS();
        }
    } else if ( are_near(ls1.initialPoint(), r2) ) {
        THROW_INFINITESOLUTIONS();
    } else {
        OptCrossing no_crossing;
        return no_crossing;
    }
}

}  // end namespace detail



OptCrossing intersection(Line const& l1, Line const& l2)
{
    OptCrossing c = detail::intersection_impl(
        l1.vector(), l1.origin(),
        l2.vector(), l2.origin());

    if (!c && distance(l1.origin(), l2) == 0) {
        THROW_INFINITESOLUTIONS();
    }
    return c;
}

OptCrossing intersection(Ray const& r1, Ray const& r2)
{
    OptCrossing crossing =
    detail::intersection_impl( r1.vector(), r1.origin(),
                               r2.vector(), r2.origin() );

    if (crossing)
    {
        if ( crossing->ta < 0
             || crossing->tb < 0 )
        {
            OptCrossing no_crossing;
            return no_crossing;
        }
        else
        {
            return crossing;
        }
    }

    if ( are_near(r1.origin(), r2) || are_near(r2.origin(), r1) )
    {
        if ( are_near(r1.origin(), r2.origin())
             && !are_near(r1.vector(), r2.vector()) )
        {
            crossing->ta = crossing->tb = 0;
            return crossing;
        }
        else
        {
            THROW_INFINITESOLUTIONS();
        }
    }
    else
    {
        OptCrossing no_crossing;
        return no_crossing;
    }
}


OptCrossing intersection( LineSegment const& ls1, LineSegment const& ls2 )
{
    Point direction1 = ls1.finalPoint() - ls1.initialPoint();
    Point direction2 = ls2.finalPoint() - ls2.initialPoint();
    OptCrossing crossing =
        detail::intersection_impl( direction1,
                                   ls1.initialPoint(),
                                   direction2,
                                   ls2.initialPoint() );

    if (crossing)
    {
        if ( crossing->getTime(0) < 0
             || crossing->getTime(0) > 1
             || crossing->getTime(1) < 0
             || crossing->getTime(1) > 1 )
        {
            OptCrossing no_crossing;
            return no_crossing;
        }
        else
        {
            return crossing;
        }
    }

    bool eqvs = (dot(direction1, direction2) > 0);
    if ( are_near(ls2.initialPoint(), ls1) )
    {
        if ( are_near(ls1.initialPoint(), ls2.initialPoint()) && !eqvs )
        {
            crossing->ta = crossing->tb = 0;
            return crossing;
        }
        else if ( are_near(ls1.finalPoint(), ls2.initialPoint()) && eqvs )
        {
            crossing->ta = 1;
            crossing->tb = 0;
            return crossing;
        }
        else
        {
            THROW_INFINITESOLUTIONS();
        }
    }
    else if ( are_near(ls2.finalPoint(), ls1) )
    {
        if ( are_near(ls1.finalPoint(), ls2.finalPoint()) && !eqvs )
        {
            crossing->ta = crossing->tb = 1;
            return crossing;
        }
        else if ( are_near(ls1.initialPoint(), ls2.finalPoint()) && eqvs )
        {
            crossing->ta = 0;
            crossing->tb = 1;
            return crossing;
        }
        else
        {
            THROW_INFINITESOLUTIONS();
        }
    }
    else
    {
        OptCrossing no_crossing;
        return no_crossing;
    }
}

Line make_angle_bisector_line(Line const& l1, Line const& l2)
{
    OptCrossing crossing;
    try
    {
        crossing = intersection(l1, l2);
    }
    catch(InfiniteSolutions const &e)
    {
        return l1;
    }
    if (!crossing)
    {
        THROW_RANGEERROR("passed lines are parallel");
    }
    Point O = l1.pointAt(crossing->ta);
    Point A = l1.pointAt(crossing->ta + 1);
    double angle = angle_between(l1.vector(), l2.vector());
    Point B = (angle > 0) ? l2.pointAt(crossing->tb + 1)
        : l2.pointAt(crossing->tb - 1);

    return make_angle_bisector_line(A, O, B);
}




}  // end namespace Geom



/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/

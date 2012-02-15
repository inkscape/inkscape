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


#include <2geom/line.h>

#include <algorithm>


namespace Geom
{

/**
 * @class Line
 * @brief Infinite line on a plane.
 *
 * Every line in 2Geom has a special point on it, called the origin. The direction of the line
 * is stored as a unit vector (versor). This way a line can be interpreted as a function
 * \f$ f: (-\infty, \infty) \to \mathbb{R}^2\f$. Zero corresponds to the origin point,
 * positive values to the points in the direction of the unit vector, and negative values
 * to points in the opposite direction.
 * 
 * @ingroup Primitives
 */

/** @brief Set the line by solving the line equation.
 * A line is a set of points that satisfies the line equation
 * \f$Ax + By + C = 0\f$. This function changes the line so that its points
 * satisfy the line equation with the given coefficients. */
void Line::setCoefficients (double a, double b, double c) {
    if (a == 0 && b == 0) {
        if (c != 0) {
            THROW_LOGICALERROR("the passed coefficients gives the empty set");
        }
        m_versor = Point(0,0);
        m_origin = Point(0,0);
    } else {
        double l = hypot(a,b);
        a /= l;
        b /= l;
        c /= l;
        Point N(a, b);
        m_versor = N.ccw();
        m_origin = -c * N;
    }
}

/** @brief Get the line equation coefficients of this line.
 * @return Vector with three values corresponding to the A, B and C
 *         coefficients of the line equation for this line. */
std::vector<double> Line::coefficients() const {
    std::vector<double> coeff;
    coeff.reserve(3);
    Point N = versor().cw();
    coeff.push_back (N[X]);
    coeff.push_back (N[Y]);
    double d = - dot (N, origin());
    coeff.push_back (d);
    return coeff;
}

/** @brief Find intersection with an axis-aligned line.
 * @param v Coordinate of the axis-aligned line
 * @param d Which axis the coordinate is on. X means a vertical line, Y means a horizontal line.
 * @return Time values at which this line intersects the query line. */
std::vector<Coord> Line::roots(Coord v, Dim2 d) const {
    if (d < 0 || d > 1)
        THROW_RANGEERROR("Line::roots, dimension argument out of range");
    std::vector<Coord> result;
    if ( m_versor[d] != 0 )
    {
        result.push_back( (v - m_origin[d]) / m_versor[d] );
    }
    // TODO: else ?
    return result;
}

/** @brief Get a time value corresponding to a point.
 * @param p Point on the line. If the point is not on the line,
 *          the returned value will be meaningless.
 * @return Time value t such that \f$f(t) = p\f$.
 * @see timeAtProjection */
Coord Line::timeAt(Point const& _point) const {
    Coord t;
    if ( m_versor[X] != 0 ) {
        t = (_point[X] - m_origin[X]) / m_versor[X];
    }
    else if ( m_versor[Y] != 0 ) {
        t = (_point[Y] - m_origin[Y]) / m_versor[Y];
    }
    else { // degenerate case
        t = 0;
    }
    return t;
}

namespace detail
{

inline
OptCrossing intersection_impl(Point const& V1, Point const O1,
                        Point const& V2, Point const O2 )
{
    double detV1V2 = V1[X] * V2[Y] - V2[X] * V1[Y];
    if (are_near(detV1V2, 0)) return OptCrossing();

    Point B = O2 - O1;
    double detBV2 = B[X] * V2[Y] - V2[X] * B[Y];
    double detV1B =	B[X] * V1[Y] - V1[X] * B[Y];
    double inv_detV1V2 = 1 / detV1V2;

    Crossing c;
    c.ta = detBV2 * inv_detV1V2;
    c.tb = detV1B * inv_detV1V2;
//	std::cerr << "ta = " << c.ta << std::endl;
//	std::cerr << "tb = " << c.tb << std::endl;
    return OptCrossing(c);
}


OptCrossing intersection_impl(Ray const& r1, Line const& l2, unsigned int i)
{
    OptCrossing crossing =
        intersection_impl(r1.versor(), r1.origin(),
                          l2.versor(), l2.origin() );

    if (crossing)
    {
        if (crossing->ta < 0)
        {
            return OptCrossing();
        }
        else
        {
            if (i != 0)
            {
                std::swap(crossing->ta, crossing->tb);
            }
            return crossing;
        }
    }
    if (are_near(r1.origin(), l2))
    {
        THROW_INFINITESOLUTIONS();
    }
    else
    {
        return OptCrossing();
    }
}


OptCrossing intersection_impl( LineSegment const& ls1,
                               Line const& l2,
                               unsigned int i )
{
    OptCrossing crossing =
        intersection_impl(ls1.finalPoint() - ls1.initialPoint(),
                          ls1.initialPoint(),
                          l2.versor(),
                          l2.origin() );

    if (crossing)
    {
        if ( crossing->getTime(0) < 0
             || crossing->getTime(0) > 1 )
        {
            return OptCrossing();
        }
        else
        {
            if (i != 0)
            {
                std::swap((*crossing).ta, (*crossing).tb);
            }
            return crossing;
        }
    }
    if (are_near(ls1.initialPoint(), l2))
    {
        THROW_INFINITESOLUTIONS();
    }
    else
    {
        return OptCrossing();
    }
}


OptCrossing intersection_impl( LineSegment const& ls1,
                               Ray const& r2,
                               unsigned int i )
{
    Point direction = ls1.finalPoint() - ls1.initialPoint();
    OptCrossing crossing =
        intersection_impl( direction,
                           ls1.initialPoint(),
                           r2.versor(),
                           r2.origin() );

    if (crossing)
    {
        if ( (crossing->getTime(0) < 0)
             || (crossing->getTime(0) > 1)
             || (crossing->getTime(1) < 0) )
        {
            return OptCrossing();
        }
        else
        {
            if (i != 0)
            {
                std::swap(crossing->ta, crossing->tb);
            }
            return crossing;
        }
    }

    if ( are_near(r2.origin(), ls1) )
    {
        bool eqvs = (dot(direction, r2.versor()) > 0);
        if ( are_near(ls1.initialPoint(), r2.origin()) && !eqvs  )
        {
            crossing->ta = crossing->tb = 0;
            return crossing;
        }
        else if ( are_near(ls1.finalPoint(), r2.origin()) && eqvs )
        {
            if (i == 0)
            {
                crossing->ta = 1;
                crossing->tb = 0;
            }
            else
            {
                crossing->ta = 0;
                crossing->tb = 1;
            }
            return crossing;
        }
        else
        {
            THROW_INFINITESOLUTIONS();
        }
    }
    else if ( are_near(ls1.initialPoint(), r2) )
    {
        THROW_INFINITESOLUTIONS();
    }
    else
    {
        OptCrossing no_crossing;
        return no_crossing;
    }
}

}  // end namespace detail



OptCrossing intersection(Line const& l1, Line const& l2)
{
    OptCrossing crossing =
        detail::intersection_impl( l1.versor(), l1.origin(),
                                   l2.versor(), l2.origin() );
    if (crossing)
    {
        return crossing;
    }
    if (are_near(l1.origin(), l2))
    {
        THROW_INFINITESOLUTIONS();
    }
    else
    {
        return crossing;
    }
}


OptCrossing intersection(Ray const& r1, Ray const& r2)
{
    OptCrossing crossing =
    detail::intersection_impl( r1.versor(), r1.origin(),
                               r2.versor(), r2.origin() );

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
             && !are_near(r1.versor(), r2.versor()) )
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


boost::optional<LineSegment> clip (Line const& l, Rect const& r)
{
    typedef boost::optional<LineSegment> opt_linesegment;
    LineSegment result;
    //size_t index = 0;
    std::vector<Point> points;
    LineSegment ls (r.corner(0), r.corner(1));
    try
    {
        OptCrossing oc = intersection (ls, l);
        if (oc)
        {
            points.push_back (l.pointAt (oc->tb));
        }
    }
    catch (InfiniteSolutions const &e)
    {
        return opt_linesegment(ls);
    }

    for (size_t i = 2; i < 5; ++i)
    {
        ls.setInitial (ls[1]);
        ls.setFinal (r.corner(i));
        try
        {
            OptCrossing oc = intersection (ls, l);
            if (oc)
            {
                points.push_back (l.pointAt (oc->tb));
                if (points.size() > 1)
                {
                    size_t sz = points.size();
                    if (!are_near (points[sz - 2], points[sz - 1], 1e-10))
                    {
                        result.setInitial (points[sz - 2]);
                        result.setFinal (points[sz - 1]);
                        return opt_linesegment(result);
                    }
                }
            }
        }
        catch (InfiniteSolutions const &e)
        {
            return opt_linesegment(ls);
        }
    }
    if ( !points.empty() )
    {
        result.setInitial (points[0]);
        result.setFinal (points[0]);
        return opt_linesegment(result);
    }
    return opt_linesegment();
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
    double angle = angle_between(l1.versor(), l2.versor());
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

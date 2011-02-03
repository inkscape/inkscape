/**
 * \file
 * \brief Bezier curve
 *
 *//*
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
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

#include <2geom/bezier-curve.h>

namespace Geom 
{

/**
 * @class BezierCurve
 * @brief Two-dimensional Bezier curve of arbitrary order.
 *
 * Bezier curves are an expansion of the concept of linear interpolation to n points.
 * Linear segments in 2Geom are in fact Bezier curves of order 1.
 *
 * Let \f$\mathbf{B}_{\mathbf{p}_0\mathbf{p}_1\ldots\mathbf{p}_n}\f$ denote a Bezier curve
 * of order \f$n\f$ defined by the points \f$\mathbf{p}_0, \mathbf{p}_1, \ldots, \mathbf{p}_n\f$.
 * Bezier curve of order 1 is a linear interpolation curve between two points, defined as
 * \f[ \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1}(t) = (1-t)\mathbf{p}_0 + t\mathbf{p}_1 \f]
 * If we now substitute points \f$\mathbf{p_0}\f$ and \f$\mathbf{p_1}\f$ in this definition
 * by linear interpolations, we get the definition of a Bezier curve of order 2, also called
 * a quadratic Bezier curve.
 * \f{align*}{ \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1\mathbf{p}_2}(t)
       &= (1-t) \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1}(t) + t \mathbf{B}_{\mathbf{p}_1\mathbf{p}_2}(t) \\
     \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1\mathbf{p}_2}(t)
       &= (1-t)^2\mathbf{p}_0 + 2(1-t)t\mathbf{p}_1 + t^2\mathbf{p}_2 \f}
 * By substituting points for quadratic Bezier curves in the original definition,
 * we get a Bezier curve of order 3, called a cubic Bezier curve.
 * \f{align*}{ \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1\mathbf{p}_2\mathbf{p}_3}(t)
       &= (1-t) \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1\mathbf{p}_2}(t)
       + t \mathbf{B}_{\mathbf{p}_1\mathbf{p}_2\mathbf{p}_3}(t) \\
     \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1\mathbf{p}_2\mathbf{p}_3}(t)
       &= (1-t)^3\mathbf{p}_0+3(1-t)^2t\mathbf{p}_1+3(1-t)t^2\mathbf{p}_2+t^3\mathbf{p}_3 \f}
 * In general, a Bezier curve or order \f$n\f$ can be recursively defined as
 * \f[ \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1\ldots\mathbf{p}_n}(t)
     = (1-t) \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1\ldots\mathbf{p}_{n-1}}(t)
     + t \mathbf{B}_{\mathbf{p}_1\mathbf{p}_2\ldots\mathbf{p}_n}(t) \f]
 *
 * This substitution can be repeated an arbitrary number of times. To picture this, imagine
 * the evaluation of a point on the curve as follows: first, all control points are joined with
 * straight lines, and a point corresponding to the selected time value is marked on them.
 * Then, the marked points are joined with straight lines and the point corresponding to
 * the time value is marked. This is repeated until only one marked point remains, which is the
 * point at the selected time value.
 *
 * @image html bezier-curve-evaluation.png "Evaluation of the Bezier curve"
 *
 * An important property of the Bezier curves is that their parameters (control points)
 * have an intutive geometric interpretation. Because of this, they are frequently used
 * in vector graphics editors.
 *
 * Every bezier curve is contained in its control polygon (the convex polygon composed
 * of its control points). This fact is useful for sweepline algorithms and intersection.
 *
 * Bezier curves of order 1, 2 and 3 are common enough to have their own more specific subclasses.
 * Note that if you create a generic BezierCurve, you cannot use a dynamic cast to those more
 * specific types, and you might face a slight preformance penalty relative to the more specific
 * classes. To obtain an instance of the correct optimized type, use the optimize()
 * or duplicate() methods. The difference is that optimize() will not create a new object
 * if it can't be optimized or is already an instance of one of the specific types, while
 * duplicate() will always create a new object.
 *
 * @ingroup Curves
 */

/** @brief Create an optimized instance of the curve.
 * If the curve was created as a generic Bezier curve but has an order between 1 and 3,
 * this method will create a newly allocated curve that is a LineSegment, a QuadraticBezier,
 * or a CubicBezier. For other cases it returns the pointer to the current curve, to avoid
 * allocating extra memory. Be careful when you use this method, because whether you have
 * to delete the returned object or not depends on its return value! If easier deletion
 * semantics are required, you can use the duplicate() method instead, which returns
 * optimized objects by default.
 * @return Pointer to a curve that is an instance of LineSegment, QuadraticBezier
 *         or CubicBezier based on the order. If the object is already an instance
 *         of te more specific types or the order is above 3, a pointer to the original
 *         object is returned instead, and no memory is allocated. */
BezierCurve *BezierCurve::optimize() const
{
    switch(order()) {
    case 1:
        if (!dynamic_cast<LineSegment const *>(this))
            return new LineSegment((*this)[0], (*this)[1]);
        break;
    case 2:
        if (!dynamic_cast<QuadraticBezier const *>(this))
            return new QuadraticBezier((*this)[0], (*this)[1], (*this)[2]);
        break;
    case 3:
        if (!dynamic_cast<CubicBezier const *>(this))
            return new CubicBezier((*this)[0], (*this)[1], (*this)[2], (*this)[3]);
        break;
    }
    return const_cast<BezierCurve*>(this);
}
Curve *BezierCurve::duplicate() const
{
    switch(order()) {
    case 1:
        return new LineSegment((*this)[0], (*this)[1]);
    case 2:
        return new QuadraticBezier((*this)[0], (*this)[1], (*this)[2]);
    case 3:
        return new CubicBezier((*this)[0], (*this)[1], (*this)[2], (*this)[3]);
    }
    return new BezierCurve(*this);
}

Curve *BezierCurve::derivative() const
{
    if (order() == 1) {
        double dx = inner[X][1] - inner[X][0], dy = inner[Y][1] - inner[Y][0];
        return new LineSegment(Point(dx,dy),Point(dx,dy));
    }
    return new BezierCurve(Geom::derivative(inner[X]), Geom::derivative(inner[Y]));
}

Coord BezierCurve::length(Coord tolerance) const
{
    switch (order())
    {
    case 0:
        return 0.0;
    case 1:
        return distance(initialPoint(), finalPoint());
    case 2:
        {
            std::vector<Point> pts = points();
            return bezier_length(pts[0], pts[1], pts[2], tolerance);
        }
    case 3:
        {
            std::vector<Point> pts = points();
            return bezier_length(pts[0], pts[1], pts[2], pts[3], tolerance);
        }
    default:
        return bezier_length(points(), tolerance);
    }
}

/**
 * @class LineSegment 
 * @brief Linear segment, a special case of a Bezier curve.
 *
 * This class uses some optimizations for a linear segment (a Bezier curve of order 1).
 * Note that if you created a BezierCurve, you will not be able to cast it to a LineSegment
 * using a dynamic cast regardless of its order - use the optimize() method.
 *
 * @ingroup Curves */
Coord LineSegment::nearestPoint(Point const& p, Coord from, Coord to) const {
    if ( from > to ) std::swap(from, to);
    Point ip = pointAt(from);
    Point fp = pointAt(to);
    Point v = fp - ip;
    Coord l2v = L2sq(v);
    if (l2v == 0) return 0;
    Coord t = dot( p - ip, v ) / l2v;
    if ( t <= 0 )  		return from;
    else if ( t >= 1 )  return to;
    else return from + t*(to-from);
}


/**
 * @class QuadraticBezier
 * @brief Quadratic Bezier curve.
 * Note that if you created a BezierCurve, you will not be able to cast it to a QuadraticBezier
 * using a dynamic cast regardless of its order - use the optimize() method.
 * @ingroup Curves */

Coord QuadraticBezier::length(Coord tolerance) const
{
    std::vector<Point> pts = points();
    return bezier_length(pts[0], pts[1], pts[2], tolerance);
}

/**
 * @class CubicBezier
 * @brief Cubic Bezier curve.
 * Note that if you created a BezierCurve, you will not be able to cast it to a CubicBezier
 * using a dynamic cast regardless of its order - use the optimize() method.
 * @ingroup Curves */

Coord CubicBezier::length(Coord tolerance) const
{
    std::vector<Point> pts = points();
    return bezier_length(pts[0], pts[1], pts[2], pts[3], tolerance);
}

static Coord bezier_length_internal(std::vector<Point> &v1, Coord tolerance)
{
    /* The Bezier length algorithm used in 2Geom utilizes a simple fact:
     * the Bezier curve is longer than the distance between its endpoints
     * but shorter than the length of the polyline formed by its control
     * points. When the difference between the two values is smaller than the
     * error tolerance, we can be sure that the true value is no further than
     * 2*tolerance from their arithmetic mean. When it's larger, we recursively
     * subdivide the Bezier curve into two parts and add their lengths.
     */
    Coord lower = distance(v1.front(), v1.back());
    Coord upper = 0.0;
    for (size_t i = 0; i < v1.size() - 1; ++i) {
        upper += distance(v1[i], v1[i+1]);
    }
    if (upper - lower < 2*tolerance) {
        return (lower + upper) / 2;
    }
        

    std::vector<Point> v2 = v1;

    /* Compute the right subdivision directly in v1 and the left one in v2.
     * Explanation of the algorithm used:
     * We have to compute the left and right edges of this triangle in which
     * the top row are the control points of the Bezier curve, and each cell
     * is equal to the arithmetic mean of the cells directly above it
     * to the right and left. This corresponds to subdividing the Bezier curve
     * at time value 0.5: the left edge has the control points of the first
     * portion of the Bezier curve and the right edge - the second one.
     * In the example we subdivide a curve with 5 control points (order 4).
     *
     * Start:
     * 0 1 2 3 4
     *  ? ? ? ?
     *   ? ? ?
     *    ? ?
     *     ?
     * # means we have overwritten the value, ? means we don't know
     * the value yet. Numbers mean the value is at i-th position in the vector.
     *
     * After loop with i==1
     * # 1 2 3 4
     *  0 ? ? ? -> write 0 to v2[1]
     *   ? ? ?
     *    ? ?
     *     ?
     *
     * After loop with i==2
     * # # 2 3 4
     *  # 1 ? ?
     *   0 ? ? -> wirte 0 to v2[2]
     *    ? ?
     *     ?
     *
     * After loop with i==3
     * # # # 3 4
     *  # # 2 ?
     *   # 1 ?
     *    0 ? -> write 0 to v2[3]
     *     ?
     *
     * After loop with i==4, we have the right edge of the triangle in v1,
     * and we write the last value needed for the left edge in v2[4].
     */

    for (size_t i = 1; i < v1.size(); ++i) {
        for (size_t j = i; j > 0; --j) {
            v1[j-1] = 0.5 * (v1[j-1] + v1[j]);
        }
        v2[i] = v1[0];
    }

    return bezier_length_internal(v1, 0.5*tolerance) + bezier_length_internal(v2, 0.5*tolerance);
}

/** @brief Compute the length of a bezier curve given by a vector of its control points
 * @relates BezierCurve */
Coord bezier_length(std::vector<Point> const &points, Coord tolerance)
{
    if (points.size() < 2) return 0.0;
    std::vector<Point> v1 = points;
    return bezier_length_internal(v1, tolerance);
}

/** @brief Compute the length of a quadratic bezier curve given by its control points
 * @relates QuadraticBezier */
Coord bezier_length(Point a0, Point a1, Point a2, Coord tolerance)
{
    Coord lower = distance(a0, a2);
    Coord upper = distance(a0, a1) + distance(a1, a2);

    if (upper - lower < 2*tolerance) return (lower + upper)/2;

    Point // Casteljau subdivision
        // b0 = a0,
        // c0 = a2,
        b1 = 0.5*(a0 + a1),
        c1 = 0.5*(a1 + a2),
        b2 = 0.5*(b1 + c1); // == c2
    return bezier_length(a0, b1, b2, 0.5*tolerance) + bezier_length(b2, c1, a2, 0.5*tolerance);
}

/** @brief Compute the length of a cubic bezier curve given by its control points
 * @relates CubicBezier */
Coord bezier_length(Point a0, Point a1, Point a2, Point a3, Coord tolerance)
{
    Coord lower = distance(a0, a3);
    Coord upper = distance(a0, a1) + distance(a1, a2) + distance(a2, a3);

    if (upper - lower < 2*tolerance) return (lower + upper)/2;

    Point // Casteljau subdivision
        // b0 = a0,
        // c0 = a3,
        b1 = 0.5*(a0 + a1),
        t0 = 0.5*(a1 + a2),
        c1 = 0.5*(a2 + a3),
        b2 = 0.5*(b1 + t0),
        c2 = 0.5*(t0 + c1),
        b3 = 0.5*(b2 + c2); // == c3
    return bezier_length(a0, b1, b2, b3, 0.5*tolerance) + bezier_length(b3, c2, c1, a3, 0.5*tolerance);
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

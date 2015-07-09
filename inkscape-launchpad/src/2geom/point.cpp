/**
 *  \file
 *  \brief Cartesian point / 2D vector and related operations
 *//*
 *  Authors:
 *    Michael G. Sloan <mgsloan@gmail.com>
 *    Nathan Hurst <njh@njhurst.com>
 *    Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2006-2009 Authors
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

#include <assert.h>
#include <math.h>
#include <2geom/coord.h>
#include <2geom/point.h>
#include <2geom/transforms.h>

namespace Geom {

/**
 * @class Point
 * @brief Two-dimensional point that doubles as a vector.
 *
 * Points in 2Geom are represented in Cartesian coordinates, e.g. as a pair of numbers
 * that store the X and Y coordinates. Each point is also a vector in \f$\mathbb{R}^2\f$
 * from the origin (point at 0,0) to the stored coordinates,
 * and has methods implementing several vector operations (like length()).
 *
 * @section OpNotePoint Operator note
 *
 * Most operators are provided by Boost operator helpers, so they are not visible in this class.
 * If @a p, @a q, @a r denote points, @a s a floating-point scalar, and @a m a transformation matrix,
 * then the following operations are available:
 * @code
   p += q; p -= q; r = p + q; r = p - q;
   p *= s; p /= s; q = p * s; q = s * p; q = p / s;
   p *= m; q = p * m; q = m * p;
   @endcode
 * It is possible to left-multiply a point by a matrix, even though mathematically speaking
 * this is undefined. The result is a point identical to that obtained by right-multiplying.
 *
 * @ingroup Primitives */

/** @brief Normalize the vector representing the point.
 * After this method returns, the length of the vector will be 1 (unless both coordinates are
 * zero - the zero point will be returned then). The function tries to handle infinite
 * coordinates gracefully. If any of the coordinates are NaN, the function will do nothing.
 * @post \f$-\epsilon < \left|this\right| - 1 < \epsilon\f$
 * @see unit_vector(Geom::Point const &) */
void Point::normalize() {
    double len = hypot(_pt[0], _pt[1]);
    if(len == 0) return;
    if(IS_NAN(len)) return;
    static double const inf = HUGE_VAL;
    if(len != inf) {
        *this /= len;
    } else {
        unsigned n_inf_coords = 0;
        /* Delay updating pt in case neither coord is infinite. */
        Point tmp;
        for ( unsigned i = 0 ; i < 2 ; ++i ) {
            if ( _pt[i] == inf ) {
                ++n_inf_coords;
                tmp[i] = 1.0;
            } else if ( _pt[i] == -inf ) {
                ++n_inf_coords;
                tmp[i] = -1.0;
            } else {
                tmp[i] = 0.0;
            }
        }
        switch (n_inf_coords) {
            case 0: {
                /* Can happen if both coords are near +/-DBL_MAX. */
                *this /= 4.0;
                len = hypot(_pt[0], _pt[1]);
                assert(len != inf);
                *this /= len;
                break;
            }
            case 1: {
                *this = tmp;
                break;
            }
            case 2: {
                *this = tmp * sqrt(0.5);
                break;
            }
        }
    }
}

/** @brief Compute the first norm (Manhattan distance) of @a p.
 * This is equal to the sum of absolutes values of the coordinates.
 * @return \f$|p_X| + |p_Y|\f$
 * @relates Point */
Coord L1(Point const &p) {
    Coord d = 0;
    for ( int i = 0 ; i < 2 ; i++ ) {
        d += fabs(p[i]);
    }
    return d;
}

/** @brief Compute the infinity norm (maximum norm) of @a p.
 * @return \f$\max(|p_X|, |p_Y|)\f$
 * @relates Point */
Coord LInfty(Point const &p) {
    Coord const a(fabs(p[0]));
    Coord const b(fabs(p[1]));
    return ( a < b || IS_NAN(b)
             ? b
             : a );
}

/** @brief True if the point has both coordinates zero.
 * NaNs are treated as not equal to zero.
 * @relates Point */
bool is_zero(Point const &p) {
    return ( p[0] == 0 &&
             p[1] == 0   );
}

/** @brief True if the point has a length near 1. The are_near() function is used.
 * @relates Point */
bool is_unit_vector(Point const &p, Coord eps) {
    return are_near(L2(p), 1.0, eps);
}
/** @brief Return the angle between the point and the +X axis.
 * @return Angle in \f$(-\pi, \pi]\f$.
 * @relates Point */
Coord atan2(Point const &p) {
    return std::atan2(p[Y], p[X]);
}

/** @brief Compute the angle between a and b relative to the origin.
 * The computation is done by projecting b onto the basis defined by a, rot90(a).
 * @return Angle in \f$(-\pi, \pi]\f$.
 * @relates Point */
Coord angle_between(Point const &a, Point const &b) {
    return std::atan2(cross(a,b), dot(a,b));
}

/** @brief Create a normalized version of a point.
 * This is equivalent to copying the point and calling its normalize() method.
 * The returned point will be (0,0) if the argument has both coordinates equal to zero.
 * If any coordinate is NaN, this function will do nothing.
 * @param a Input point
 * @return Point on the unit circle in the same direction from origin as a, or the origin
 *         if a has both coordinates equal to zero
 * @relates Point */
Point unit_vector(Point const &a)
{
    Point ret(a);
    ret.normalize();
    return ret;
}
/** @brief Return the "absolute value" of the point's vector.
 * This is defined in terms of the default lexicographical ordering. If the point is "larger"
 * that the origin (0, 0), its negation is returned. You can check whether
 * the points' vectors have the same direction (e.g. lie
 * on the same line passing through the origin) using
 * @code abs(a).normalize() == abs(b).normalize() @endcode
 * To check with some margin of error, use
 * @code are_near(abs(a).normalize(), abs(b).normalize()) @endcode
 * Although naively this should take the absolute value of each coordinate, such an operation
 * is not very useful.
 * @relates Point */
Point abs(Point const &b)
{
    Point ret;
    if (b[Y] < 0.0) {
        ret = -b;
    } else if (b[Y] == 0.0) {
        ret = b[X] < 0.0 ? -b : b;
    } else {
        ret = b;
    }
    return ret;
}

/** @brief Transform the point by the specified matrix. */
Point &Point::operator*=(Affine const &m) {
    double x = _pt[X], y = _pt[Y];
    for(int i = 0; i < 2; i++) {
        _pt[i] = x * m[i] + y * m[i + 2] + m[i + 4];
    }
    return *this;
}

/** @brief Snap the angle B - A - dir to multiples of \f$2\pi/n\f$.
 * The 'dir' argument must be normalized (have unit length), otherwise the result
 * is undefined.
 * @return Point with the same distance from A as B, with a snapped angle.
 * @post distance(A, B) == distance(A, result)
 * @post angle_between(result - A, dir) == \f$2k\pi/n, k \in \mathbb{N}\f$
 * @relates Point */
Point constrain_angle(Point const &A, Point const &B, unsigned int n, Point const &dir)
{
    // for special cases we could perhaps use explicit testing (which might be faster)
    if (n == 0.0) {
        return B;
    }
    Point diff(B - A);
    double angle = -angle_between(diff, dir);
    double k = round(angle * (double)n / (2.0*M_PI));
    return A + dir * Rotate(k * 2.0 * M_PI / (double)n) * L2(diff);
}

std::ostream &operator<<(std::ostream &out, const Geom::Point &p)
{
    out << "(" << format_coord_nice(p[X]) << ", "
               << format_coord_nice(p[Y]) << ")";
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

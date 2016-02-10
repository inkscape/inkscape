/**
 *  \file
 *  \brief Cartesian point / 2D vector with integer coordinates
 *//*
 * Copyright 2011 Krzysztof Kosiński <tweenk.pl@gmail.com>
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

#ifndef LIB2GEOM_SEEN_INT_POINT_H
#define LIB2GEOM_SEEN_INT_POINT_H

#include <stdexcept>
#include <boost/operators.hpp>
#include <2geom/coord.h>

namespace Geom {

/**
 * @brief Two-dimensional point with integer coordinates.
 *
 * This class is an exact equivalent of Point, except it stores integer coordinates.
 * Integer points are useful in contexts related to rasterized graphics, for example
 * for bounding boxes when rendering SVG.
 *
 * @see Point
 * @ingroup Primitives */
class IntPoint
    : boost::additive< IntPoint
    , boost::totally_ordered< IntPoint
    > >
{
    IntCoord _pt[2];
public:
    /// @name Creating integer points
    /// @{
    IntPoint() { }
    IntPoint(IntCoord x, IntCoord y) {
        _pt[X] = x;
        _pt[Y] = y;
    }
    /// @}

    /// @name Access the coordinates of a point
    /// @{
    IntCoord operator[](unsigned i) const {
        if ( i > Y ) throw std::out_of_range("index out of range");
        return _pt[i];
    }
    IntCoord &operator[](unsigned i) {
        if ( i > Y ) throw std::out_of_range("index out of range");
        return _pt[i];
    }
    IntCoord operator[](Dim2 d) const { return _pt[d]; }
    IntCoord &operator[](Dim2 d) { return _pt[d]; }

    IntCoord x() const throw() { return _pt[X]; }
    IntCoord &x() throw() { return _pt[X]; }
    IntCoord y() const throw() { return _pt[Y]; }
    IntCoord &y() throw() { return _pt[Y]; }
    /// @}

    /// @name Vector-like arithmetic operations
    /// @{
    IntPoint operator-() const {
        IntPoint ret(-_pt[X], -_pt[Y]);
        return ret;
    }
    IntPoint &operator+=(IntPoint const &o) {
        _pt[X] += o._pt[X];
        _pt[Y] += o._pt[Y];
        return *this;
    }
    IntPoint &operator-=(IntPoint const &o) {
        _pt[X] -= o._pt[X];
        _pt[Y] -= o._pt[Y];
        return *this;
    }
    /// @}
    
    /// @name Various utilities
    /// @{
    /** @brief Equality operator. */
    bool operator==(IntPoint const &in_pnt) const {
        return ((_pt[X] == in_pnt[X]) && (_pt[Y] == in_pnt[Y]));
    }
    /** @brief Lexicographical ordering for points.
     * Y coordinate is regarded as more significant. When sorting according to this
     * ordering, the points will be sorted according to the Y coordinate, and within
     * points with the same Y coordinate according to the X coordinate. */
    bool operator<(IntPoint const &p) const {
        return ( ( _pt[Y] < p[Y] ) ||
             (( _pt[Y] == p[Y] ) && ( _pt[X] < p[X] )));
    }
    /// @}
    
    /** @brief Lexicographical ordering functor.
     * @param d The more significant dimension */
    template <Dim2 d> struct LexLess;
    /** @brief Lexicographical ordering functor.
     * @param d The more significant dimension */
    template <Dim2 d> struct LexGreater;
    /** @brief Lexicographical ordering functor with runtime dimension. */
    struct LexLessRt {
        LexLessRt(Dim2 d) : dim(d) {}
        inline bool operator()(IntPoint const &a, IntPoint const &b) const;
    private:
        Dim2 dim;
    };
    /** @brief Lexicographical ordering functor with runtime dimension. */
    struct LexGreaterRt {
        LexGreaterRt(Dim2 d) : dim(d) {}
        inline bool operator()(IntPoint const &a, IntPoint const &b) const;
    private:
        Dim2 dim;
    };
};

template<> struct IntPoint::LexLess<X> {
    bool operator()(IntPoint const &a, IntPoint const &b) const {
        return a[X] < b[X] || (a[X] == b[X] && a[Y] < b[Y]);
    }
};
template<> struct IntPoint::LexLess<Y> {
    bool operator()(IntPoint const &a, IntPoint const &b) const {
        return a[Y] < b[Y] || (a[Y] == b[Y] && a[X] < b[X]);
    }
};
template<> struct IntPoint::LexGreater<X> {
    bool operator()(IntPoint const &a, IntPoint const &b) const {
        return a[X] > b[X] || (a[X] == b[X] && a[Y] > b[Y]);
    }
};
template<> struct IntPoint::LexGreater<Y> {
    bool operator()(IntPoint const &a, IntPoint const &b) const {
        return a[Y] > b[Y] || (a[Y] == b[Y] && a[X] > b[X]);
    }
};
inline bool IntPoint::LexLessRt::operator()(IntPoint const &a, IntPoint const &b) const {
    return dim ? IntPoint::LexLess<Y>()(a, b) : IntPoint::LexLess<X>()(a, b);
}
inline bool IntPoint::LexGreaterRt::operator()(IntPoint const &a, IntPoint const &b) const {
    return dim ? IntPoint::LexGreater<Y>()(a, b) : IntPoint::LexGreater<X>()(a, b);
}

}  // namespace Geom

#endif // !SEEN_GEOM_INT_POINT_H

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

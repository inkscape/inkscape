/**
 * \file
 * \brief Axis-aligned rectangle
 *//*
 * Copyright 2007 Michael Sloan <mgsloan@gmail.com>
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
 * in the file COPYING-LGPL-2.1; if not, output to the Free Software
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
 *
 * Authors of original rect class:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *   bulia byak <buliabyak@users.sf.net>
 *   MenTaLguY <mental@rydia.net>
 */

#include <2geom/d2.h>

#ifndef LIB2GEOM_RECT_H
#define LIB2GEOM_RECT_H

#include <2geom/affine.h>
#include <boost/optional/optional.hpp>

namespace Geom {

/**
 * @brief Axis-aligned, non-empty rectangle - convenience typedef
 * @ingroup Primitives
 */
typedef D2<Interval> Rect;
class OptRect;

inline Rect unify(Rect const &, Rect const &);

/**
 * @brief Axis aligned, non-empty rectangle.
 * @ingroup Primitives
 */
template<>
class D2<Interval> {
private:
    Interval f[2];
public:
    /// @name Create rectangles.
    /// @{
    /** @brief Create a rectangle that contains only the point at (0,0). */
    D2<Interval>() { f[X] = f[Y] = Interval(); }
    /** @brief Create a rectangle from X and Y intervals. */
    D2<Interval>(Interval const &a, Interval const &b) {
        f[X] = a;
        f[Y] = b;
    }
    /** @brief Create a rectangle from two points. */
    D2<Interval>(Point const & a, Point const & b) {
        f[X] = Interval(a[X], b[X]);
        f[Y] = Interval(a[Y], b[Y]);
    }
    /** @brief Create a rectangle from a range of points.
     * The resulting rectangle will contain all ponts from the range.
     * The return type of iterators must be convertible to Point.
     * The range must not be empty. For possibly empty ranges, see OptRect.
     * @param start Beginning of the range
     * @param end   End of the range
     * @return Rectangle that contains all points from [start, end). */
    template <typename InputIterator>
    static Rect from_range(InputIterator start, InputIterator end) {
        assert(start != end);
        Point p1 = *start++;
        Rect result(p1, p1);
        for (; start != end; ++start) {
            result.expandTo(*start);
        }
        return result;
    }
    /** @brief Create a rectangle from a C-style array of points it should contain. */
    static Rect from_array(Point const *c, unsigned n) {
        Rect result = Rect::from_range(c, c+n);
        return result;
    }
    /// @}

    /// @name Inspect dimensions.
    /// @{
    Interval& operator[](unsigned i)              { return f[i]; }
    Interval const & operator[](unsigned i) const { return f[i]; }

    Point min() const { return Point(f[X].min(), f[Y].min()); }
    Point max() const { return Point(f[X].max(), f[Y].max()); }
    /** @brief Return the n-th corner of the rectangle.
     * If the Y axis grows upwards, this returns corners in clockwise order
     * starting from the lower left. If Y grows downwards, it returns the corners
     * in counter-clockwise order starting from the upper left. */
    Point corner(unsigned i) const {
        switch(i % 4) {
            case 0:  return Point(f[X].min(), f[Y].min());
            case 1:  return Point(f[X].max(), f[Y].min());
            case 2:  return Point(f[X].max(), f[Y].max());
            default: return Point(f[X].min(), f[Y].max());
        }
    }
        
    //We should probably remove these - they're coord sys gnostic
    /** @brief Return top coordinate of the rectangle (+Y is downwards). */
    Coord top() const { return f[Y].min(); }
    /** @brief Return bottom coordinate of the rectangle (+Y is downwards). */
    Coord bottom() const { return f[Y].max(); }
    /** @brief Return leftmost coordinate of the rectangle (+X is to the right). */
    Coord left() const { return f[X].min(); }
    /** @brief Return rightmost coordinate of the rectangle (+X is to the right). */
    Coord right() const { return f[X].max(); }

    Coord width() const { return f[X].extent(); }
    Coord height() const { return f[Y].extent(); }

    /** @brief Get rectangle's width and height as a point.
     * @return Point with X coordinate corresponding to the width and the Y coordinate
     *         corresponding to the height of the rectangle. */
    Point dimensions() const { return Point(f[X].extent(), f[Y].extent()); }
    Point midpoint() const { return Point(f[X].middle(), f[Y].middle()); }

/**
 * \brief Compute the area of this rectangle.
 *
 * Note that a zero area rectangle is not empty - just as the interval [0,0] contains one point, the rectangle [0,0] x [0,0] contains 1 point and no area.
 * \retval For a valid return value, the rect must be tested for emptyness first.
 */
    /** @brief Compute rectangle's area. */
    Coord area() const { return f[X].extent() * f[Y].extent(); }
    /** @brief Check whether the rectangle has zero area up to specified tolerance.
     * @param eps Maximum value of the area to consider empty
     * @return True if rectangle has an area smaller than tolerance, false otherwise */
    bool hasZeroArea(double eps = EPSILON) const { return (area() <= eps); }

    /** @brief Get the larger extent (width or height) of the rectangle. */
    Coord maxExtent() const { return std::max(f[X].extent(), f[Y].extent()); }
    /** @brief Get the smaller extent (width or height) of the rectangle. */
    Coord minExtent() const { return std::min(f[X].extent(), f[Y].extent()); }
    /// @}

    /// @name Test other rectangles and points for inclusion.
    /// @{
    /** @brief Check whether the rectangles have any common points. */
    bool intersects(Rect const &r) const { 
        return f[X].intersects(r[X]) && f[Y].intersects(r[Y]);
    }
    /** @brief Check whether the interiors of the rectangles have any common points. */
    bool interiorIntersects(Rect const &r) const {
        return f[X].interiorIntersects(r[X]) && f[Y].interiorIntersects(r[Y]);
    }
    /** @brief Check whether the rectangle includes all points in the given rectangle. */
    bool contains(Rect const &r) const { 
        return f[X].contains(r[X]) && f[Y].contains(r[Y]);
    }
    /** @brief Check whether the interior includes all points in the given rectangle.
     * Interior of the rectangle is the entire rectangle without its borders. */
    bool interiorContains(Rect const &r) const { 
        return f[X].interiorContains(r[X]) && f[Y].interiorContains(r[Y]);
    }

    /** @brief Check whether the rectangles have any common points.
     * A non-empty rectangle will not intersect empty rectangles. */
    inline bool intersects(OptRect const &r) const;
    /** @brief Check whether the rectangle includes all points in the given rectangle.
     * A non-empty rectangle will contain any empty rectangle. */
    inline bool contains(OptRect const &r) const;
    /** @brief Check whether the interior includes all points in the given rectangle.
     * The interior of a non-empty rectangle will contain any empty rectangle. */
    inline bool interiorContains(OptRect const &r) const;

    /** @brief Check whether the given point is within the rectangle. */
    bool contains(Point const &p) const {
        return f[X].contains(p[X]) && f[Y].contains(p[Y]);
    }
    /** @brief Check whether the given point is in the rectangle's interior.
     * This means the point must lie within the rectangle but not on its border. */
    bool interiorContains(Point const &p) const {
        return f[X].interiorContains(p[X]) && f[Y].interiorContains(p[Y]);
    }
    /// @}

    /// @name Modify the rectangle.
    /// @{
    /** @brief Enlarge the rectangle to contain the given point. */
    void expandTo(Point p)        { 
        f[X].expandTo(p[X]);  f[Y].expandTo(p[Y]);
    }
    /** @brief Enlarge the rectangle to contain the given rectangle. */
    void unionWith(Rect const &b) { 
        f[X].unionWith(b[X]); f[Y].unionWith(b[Y]);
    }
    /** @brief Enlarge the rectangle to contain the given rectangle.
     * Unioning with an empty rectangle results in no changes. */
    void unionWith(OptRect const &b);
    
    //TODO: figure out how these work with negative values and OptRect
    /** @brief Expand the rectangle in both directions by the specified amount.
     * Note that this is different from scaling. Negative values wil shrink the
     * rectangle. If <code>-amount</code> is larger than
     * half of the width, the X interval will contain only the X coordinate
     * of the midpoint; same for height. */
    void expandBy(Coord amount) {
        f[X].expandBy(amount);  f[Y].expandBy(amount);
    }
    /** @brief Expand the rectangle by the coordinates of the given point.
     * This will expand the width by the X coordinate of the point in both directions
     * and the height by Y coordinate of the point. Negative coordinate values will
     * shrink the rectangle. If <code>-p[X]</code> is larger than half of the width,
     * the X interval will contain only the X coordinate of the midpoint; same for height. */
    void expandBy(Point const p) { 
        f[X].expandBy(p[X]);  f[Y].expandBy(p[Y]);
    }
    /// @}
};

inline Rect unify(Rect const & a, Rect const & b) {
    return Rect(unify(a[X], b[X]), unify(a[Y], b[Y]));
}

inline Rect union_list(std::vector<Rect> const &r) {
    if(r.empty()) return Rect(Interval(0,0), Interval(0,0));
    Rect ret = r[0];
    for(unsigned i = 1; i < r.size(); i++)
        ret.unionWith(r[i]);
    return ret;
}

inline
Coord distanceSq( Point const& p, Rect const& rect )
{
    double dx = 0, dy = 0;
    if ( p[X] < rect.left() )
    {
        dx = p[X] - rect.left();
    }
    else if ( p[X] > rect.right() )
    {
        dx = rect.right() - p[X];
    }
    if ( p[Y] < rect.top() )
    {
        dy = rect.top() - p[Y];
    }
    else if (  p[Y] > rect.bottom() )
    {
        dy = p[Y] - rect.bottom();
    }
    return dx*dx + dy*dy;
}

/**
 * Returns the smallest distance between p and rect.
 */
inline 
Coord distance( Point const& p, Rect const& rect )
{
    return std::sqrt(distanceSq(p, rect));
}

/**
 * @brief Axis-aligned rectangle that can be empty.
 * @ingroup Primitives
 */
class OptRect : public boost::optional<Rect> {
public:
    OptRect() : boost::optional<Rect>() {};
    OptRect(Rect const &a) : boost::optional<Rect>(a) {};

    /**
     * Creates an empty OptRect when one of the argument intervals is empty.
     */
    OptRect(OptInterval const &x_int, OptInterval const &y_int) {
        if (x_int && y_int) {
            *this = Rect(*x_int, *y_int);
        }
        // else, stay empty.
    }

    /** @brief Check for emptiness. */
    inline bool isEmpty() const { return (*this == false); };

    bool intersects(Rect const &r) const { return r.intersects(*this); }
    bool contains(Rect const &r) const { return *this && (*this)->contains(r); }
    bool interiorContains(Rect const &r) const { return *this && (*this)->interiorContains(r); }

    bool intersects(OptRect const &r) const { return *this && (*this)->intersects(r); }
    bool contains(OptRect const &r) const { return *this && (*this)->contains(r); }
    bool interiorContains(OptRect const &r) const { return *this && (*this)->interiorContains(r); }

    bool contains(Point const &p) const { return *this && (*this)->contains(p); }
    bool interiorContains(Point const &p) const { return *this && (*this)->contains(p); }

    inline void unionWith(OptRect const &b) {
        if (*this) { // check that we are not empty
            (*this)->unionWith(b);
        } else {
            *this = b;
        }
    }
};


/** 
 * Returns the smallest rectangle that encloses both rectangles.
 * An empty argument is assumed to be an empty rectangle
 */
inline OptRect unify(OptRect const & a, OptRect const & b) {
    if (!a) {
        return b;
    } else if (!b) {
        return a;
    } else {
        return unify(*a, *b);
    }
}

inline OptRect intersect(Rect const & a, Rect const & b) {
    return OptRect(intersect(a[X], b[X]), intersect(a[Y], b[Y]));
}

inline void Rect::unionWith(OptRect const &b) { 
    if (b) {
        unionWith(*b);
    }
}
inline bool Rect::intersects(OptRect const &r) const {
    return r && intersects(*r);
}
inline bool Rect::contains(OptRect const &r) const {
    return !r || contains(*r);
}
inline bool Rect::interiorContains(OptRect const &r) const {
    return !r || interiorContains(*r);
}

} // end namespace Geom

#endif //_2GEOM_RECT

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

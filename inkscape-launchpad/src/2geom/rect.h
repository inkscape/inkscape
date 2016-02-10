/**
 * \file
 * \brief Axis-aligned rectangle
 *//*
 * Authors:
 *   Michael Sloan <mgsloan@gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * Copyright 2007-2011 Authors
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

#ifndef LIB2GEOM_SEEN_RECT_H
#define LIB2GEOM_SEEN_RECT_H

#include <boost/optional.hpp>
#include <2geom/affine.h>
#include <2geom/interval.h>
#include <2geom/int-rect.h>

namespace Geom {

/** Values for the <align> parameter of preserveAspectRatio.
 * See: http://www.w3.org/TR/SVG/coords.html#PreserveAspectRatioAttribute */
enum Align {
    ALIGN_NONE,
    ALIGN_XMIN_YMIN,
    ALIGN_XMID_YMIN,
    ALIGN_XMAX_YMIN,
    ALIGN_XMIN_YMID,
    ALIGN_XMID_YMID,
    ALIGN_XMAX_YMID,
    ALIGN_XMIN_YMAX,
    ALIGN_XMID_YMAX,
    ALIGN_XMAX_YMAX
};

/** Values for the <meetOrSlice> parameter of preserveAspectRatio.
 * See: http://www.w3.org/TR/SVG/coords.html#PreserveAspectRatioAttribute */
enum Expansion {
    EXPANSION_MEET,
    EXPANSION_SLICE
};

/// Convert an align specification to coordinate fractions.
Point align_factors(Align align);

/** @brief Structure that specifies placement of within a viewport.
 * Use this to create transformations that preserve aspect. */
struct Aspect {
    Align align;
    Expansion expansion;
    bool deferred; ///< for SVG compatibility

    Aspect(Align a = ALIGN_NONE, Expansion ex = EXPANSION_MEET)
        : align(a), expansion(ex), deferred(false)
    {}
};

/**
 * @brief Axis aligned, non-empty rectangle.
 * @ingroup Primitives
 */
class Rect
    : public GenericRect<Coord>
{
    typedef GenericRect<Coord> Base;
public:
    /// @name Create rectangles.
    /// @{
    /** @brief Create a rectangle that contains only the point at (0,0). */
    Rect() {}
    /** @brief Create a rectangle from X and Y intervals. */
    Rect(Interval const &a, Interval const &b) : Base(a,b) {}
    /** @brief Create a rectangle from two points. */
    Rect(Point const &a, Point const &b) : Base(a,b) {}
    Rect(Coord x0, Coord y0, Coord x1, Coord y1) : Base(x0, y0, x1, y1) {}
    Rect(Base const &b) : Base(b) {}
    Rect(IntRect const &ir) : Base(ir.min(), ir.max()) {}
    /// @}

    /// @name Inspect dimensions.
    /// @{
    /** @brief Check whether the rectangle has zero area up to specified tolerance.
     * @param eps Maximum value of the area to consider empty
     * @return True if rectangle has an area smaller than tolerance, false otherwise */
    bool hasZeroArea(Coord eps = EPSILON) const { return (area() <= eps); }
    /// Check whether the rectangle has finite area
    bool isFinite() const { return (*this)[X].isFinite() && (*this)[Y].isFinite(); }
    /// Calculate the diameter of the smallest circle that would contain the rectangle.
    Coord diameter() const { return distance(corner(0), corner(2)); }
    /// @}

    /// @name Test other rectangles and points for inclusion.
    /// @{
    /** @brief Check whether the interiors of the rectangles have any common points. */
    bool interiorIntersects(Rect const &r) const {
        return f[X].interiorIntersects(r[X]) && f[Y].interiorIntersects(r[Y]);
    }
    /** @brief Check whether the interior includes the given point. */
    bool interiorContains(Point const &p) const {
        return f[X].interiorContains(p[X]) && f[Y].interiorContains(p[Y]);
    }
    /** @brief Check whether the interior includes all points in the given rectangle.
     * Interior of the rectangle is the entire rectangle without its borders. */
    bool interiorContains(Rect const &r) const { 
        return f[X].interiorContains(r[X]) && f[Y].interiorContains(r[Y]);
    }
    inline bool interiorContains(OptRect const &r) const;
    /// @}

    /// @name Rounding to integer coordinates
    /// @{
    /** @brief Return the smallest integer rectangle which contains this one. */
    IntRect roundOutwards() const {
        IntRect ir(f[X].roundOutwards(), f[Y].roundOutwards());
        return ir;
    }
    /** @brief Return the largest integer rectangle which is contained in this one. */
    OptIntRect roundInwards() const {
        OptIntRect oir(f[X].roundInwards(), f[Y].roundInwards());
        return oir;
    }
    /// @}

    /// @name SVG viewbox functionality.
    /// @{
    /** @brief Transform contents to viewport.
     * Computes an affine that transforms the contents of this rectangle
     * to the specified viewport. The aspect parameter specifies how to
     * to the transformation (whether the aspect ratio of content
     * should be kept and where it should be placed in the viewport). */
    Affine transformTo(Rect const &viewport, Aspect const &aspect = Aspect()) const;
    /// @}

    /// @name Operators
    /// @{
    Rect &operator*=(Affine const &m);
    bool operator==(IntRect const &ir) const {
        return f[X] == ir[X] && f[Y] == ir[Y];
    }
    bool operator==(Rect const &other) const {
        return Base::operator==(other);
    }
    /// @}
};

/**
 * @brief Axis-aligned rectangle that can be empty.
 * @ingroup Primitives
 */
class OptRect
    : public GenericOptRect<Coord>
{
    typedef GenericOptRect<Coord> Base;
public:
    OptRect() : Base() {}
    OptRect(Rect const &a) : Base(a) {}
    OptRect(Point const &a, Point const &b) : Base(a, b) {}
    OptRect(Coord x0, Coord y0, Coord x1, Coord y1) : Base(x0, y0, x1, y1) {}
    OptRect(OptInterval const &x_int, OptInterval const &y_int) : Base(x_int, y_int) {}
    OptRect(Base const &b) : Base(b) {}

    OptRect(IntRect const &r) : Base(Rect(r)) {}
    OptRect(OptIntRect const &r) : Base() {
        if (r) *this = Rect(*r);
    }

    Affine transformTo(Rect const &viewport, Aspect const &aspect = Aspect()) {
        Affine ret = Affine::identity();
        if (empty()) return ret;
        ret = (*this)->transformTo(viewport, aspect);
        return ret;
    }

    bool operator==(OptRect const &other) const {
        return Base::operator==(other);
    }
    bool operator==(Rect const &other) const {
        return Base::operator==(other);
    }
};

Coord distanceSq(Point const &p, Rect const &rect);
Coord distance(Point const &p, Rect const &rect);
/// Minimum square of distance to rectangle, or infinity if empty.
Coord distanceSq(Point const &p, OptRect const &rect);
/// Minimum distance to rectangle, or infinity if empty.
Coord distance(Point const &p, OptRect const &rect);

inline bool Rect::interiorContains(OptRect const &r) const {
    return !r || interiorContains(static_cast<Rect const &>(*r));
}

// the functions below do not work when defined generically
inline OptRect operator&(Rect const &a, Rect const &b) {
    OptRect ret(a);
    ret.intersectWith(b);
    return ret;
}
inline OptRect intersect(Rect const &a, Rect const &b) {
    return a & b;
}
inline OptRect intersect(OptRect const &a, OptRect const &b) {
    return a & b;
}
inline Rect unify(Rect const &a, Rect const &b) {
    return a | b;
}
inline OptRect unify(OptRect const &a, OptRect const &b) {
    return a | b;
}

/** @brief Union a list of rectangles
 * @deprecated Use OptRect::from_range instead */
inline Rect union_list(std::vector<Rect> const &r) {
    if(r.empty()) return Rect(Interval(0,0), Interval(0,0));
    Rect ret = r[0];
    for(unsigned i = 1; i < r.size(); i++)
        ret.unionWith(r[i]);
    return ret;
}

} // end namespace Geom

#endif // LIB2GEOM_SEEN_RECT_H

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

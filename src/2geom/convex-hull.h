/** @file
 * @brief Convex hull data structures
 *//*
 * Copyright 2006 Nathan Hurst <njh@mail.csse.monash.edu.au>
 * Copyright 2006 Michael G. Sloan <mgsloan@gmail.com>
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
 *
 */

#ifndef LIB2GEOM_SEEN_CONVEX_HULL_H
#define LIB2GEOM_SEEN_CONVEX_HULL_H

#include <2geom/point.h>
#include <2geom/rect.h>
#include <vector>
#include <algorithm>
#include <boost/operators.hpp>
#include <boost/optional.hpp>
#include <boost/range/iterator_range.hpp>

namespace Geom {

namespace {

/** @brief Iterator for the lower convex hull.
 * This iterator allows us to avoid duplicating any points in the hull
 * boundary and still express most algorithms in a concise way. */
class ConvexHullLowerIterator
    : public boost::random_access_iterator_helper
        < ConvexHullLowerIterator
        , Point
        , std::ptrdiff_t
        , Point const *
        , Point const &
        >
{
public:
    typedef ConvexHullLowerIterator Self;
    ConvexHullLowerIterator()
        : _data(NULL)
        , _size(0)
        , _x(0)
    {}
    ConvexHullLowerIterator(std::vector<Point> const &pts, std::size_t x)
        : _data(&pts[0])
        , _size(pts.size())
        , _x(x)
    {}

    Self &operator++() {
        *this += 1;
        return *this;
    }
    Self &operator--() {
        *this -= 1;
        return *this;
    }
    Self &operator+=(std::ptrdiff_t d) {
        _x += d;
        return *this;
    }
    Self &operator-=(std::ptrdiff_t d) {
        _x -= d;
        return *this;
    }
    std::ptrdiff_t operator-(Self const &other) const {
        return _x - other._x;
    }
    Point const &operator*() const {
        if (_x < _size) {
            return _data[_x];
        } else {
            return *_data;
        }
    }
    bool operator==(Self const &other) const {
        return _data == other._data && _x == other._x;
    }
    bool operator<(Self const &other) const {
        return _data == other._data && _x < other._x;
    }

private:
    Point const *_data;
    std::size_t _size;
    std::size_t _x;
};

} // end anonymous namespace

/**
 * @brief Convex hull based on the Andrew's monotone chain algorithm.
 * @ingroup Shapes
 */
class ConvexHull {
public:
    typedef std::vector<Point>::const_iterator iterator;
    typedef std::vector<Point>::const_iterator const_iterator;
    typedef std::vector<Point>::const_iterator UpperIterator;
    typedef ConvexHullLowerIterator LowerIterator;

    /// @name Construct a convex hull.
    /// @{

    /// Create an empty convex hull.
    ConvexHull() {}
    /// Construct a singular convex hull.
    explicit ConvexHull(Point const &a)
        : _boundary(1, a)
        , _lower(1)
    {}
    /// Construct a convex hull of two points.
    ConvexHull(Point const &a, Point const &b);
    /// Construct a convex hull of three points.
    ConvexHull(Point const &a, Point const &b, Point const &c);
    /// Construct a convex hull of four points.
    ConvexHull(Point const &a, Point const &b, Point const &c, Point const &d);
    /// Create a convex hull of a vector of points.
    ConvexHull(std::vector<Point> const &pts);

    /// Create a convex hull of a range of points.
    template <typename Iter>
    ConvexHull(Iter first, Iter last)
        : _lower(0)
    {
        _prune(first, last, _boundary);
        _construct();
    }
    /// @}

    /// @name Inspect basic properties.
    /// @{

    /// Check for emptiness.
    bool empty() const { return _boundary.empty(); }
    /// Get the number of points in the hull.
    size_t size() const { return _boundary.size(); }
    /// Check whether the hull contains only one point.
    bool isSingular() const { return _boundary.size() == 1; }
    /// Check whether the hull is a line.
    bool isLinear() const { return _boundary.size() == 2; }
    /// Check whether the hull has zero area.
    bool isDegenerate() const { return _boundary.size() < 3; }
    /// Calculate the area of the convex hull.
    double area() const;
    //Point centroid() const;
    //double areaAndCentroid(Point &c);
    //FatLine maxDiameter() const;
    //FatLine minDiameter() const;
    /// @}

    /// @name Inspect bounds and extreme points.
    /// @{

    /// Compute the bounding rectangle of the convex hull.
    OptRect bounds() const;

    /// Get the leftmost (minimum X) coordinate of the hull.
    Coord left() const { return _boundary[0][X]; }
    /// Get the rightmost (maximum X) coordinate of the hull.
    Coord right() const { return _boundary[_lower-1][X]; }
    /// Get the topmost (minimum Y) coordinate of the hull.
    Coord top() const { return topPoint()[Y]; }
    /// Get the bottommost (maximum Y) coordinate of the hull.
    Coord bottom() const { return bottomPoint()[Y]; }

    /// Get the leftmost (minimum X) point of the hull.
    /// If the leftmost edge is vertical, the top point of the edge is returned.
    Point leftPoint() const { return _boundary[0]; }
    /// Get the rightmost (maximum X) point of the hull.
    /// If the rightmost edge is vertical, the bottom point edge is returned.
    Point rightPoint() const { return _boundary[_lower-1]; }
    /// Get the topmost (minimum Y) point of the hull.
    /// If the topmost edge is horizontal, the right point of the edge is returned.
    Point topPoint() const;
    /// Get the bottommost (maximum Y) point of the hull.
    /// If the bottommost edge is horizontal, the left point of the edge is returned.
    Point bottomPoint() const;
    ///@}

    /// @name Iterate over points.
    /// @{
    /** @brief Get the begin iterator to the points that form the hull.
     * Points are are returned beginning the the leftmost one, going along
     * the upper (minimum Y) side, and then along the bottom.
     * Thus the points are always ordered clockwise. No point is
     * repeated. */
    iterator begin() const { return _boundary.begin(); }
    /// Get the end iterator to the points that form the hull.
    iterator end() const { return _boundary.end(); }
    /// Get the first, leftmost point in the hull.
    Point const &front() const { return _boundary.front(); }
    /// Get the penultimate point of the lower hull.
    Point const &back() const { return _boundary.back(); }
    Point const &operator[](std::size_t i) const {
        return _boundary[i];
    }

    /** @brief Get an iterator range to the upper part of the hull.
     * This returns a range that includes the leftmost point,
     * all points of the upper hull, and the rightmost point. */
    boost::iterator_range<UpperIterator> upperHull() const {
        boost::iterator_range<UpperIterator> r(_boundary.begin(), _boundary.begin() + _lower);
        return r;
    }

    /** @brief Get an iterator range to the lower part of the hull.
     * This returns a range that includes the leftmost point,
     * all points of the lower hull, and the rightmost point. */
    boost::iterator_range<LowerIterator> lowerHull() const {
        if (_boundary.empty()) {
            boost::iterator_range<LowerIterator> r(LowerIterator(_boundary, 0),
                                                   LowerIterator(_boundary, 0));
            return r;
        }
        if (_boundary.size() == 1) {
            boost::iterator_range<LowerIterator> r(LowerIterator(_boundary, 0),
                                                   LowerIterator(_boundary, 1));
            return r;
        }
        boost::iterator_range<LowerIterator> r(LowerIterator(_boundary, _lower - 1),
                                               LowerIterator(_boundary, _boundary.size() + 1));
        return r;
    }
    /// @}

    /// @name Check for containment and intersection.
    /// @{
    /** @brief Check whether the given point is inside the hull.
     * This takes logarithmic time. */
    bool contains(Point const &p) const;
    /** @brief Check whether the given axis-aligned rectangle is inside the hull.
     * A rectangle is inside the hull if all of its corners are inside. */
    bool contains(Rect const &r) const;
    /// Check whether the given convex hull is completely contained in this one.
    bool contains(ConvexHull const &other) const;
    //bool interiorContains(Point const &p) const;
    //bool interiorContains(Rect const &r) const;
    //bool interiorContains(ConvexHull const &other) const;
    //bool intersects(Rect const &r) const;
    //bool intersects(ConvexHull const &other) const;

    //ConvexHull &operator|=(ConvexHull const &other);
    //ConvexHull &operator&=(ConvexHull const &other);
    //ConvexHull &operator*=(Affine const &m);

    //ConvexHull &expand(Point const &p);
    //void unifyWith(ConvexHull const &other);
    //void intersectWith(ConvexHull const &other);
    /// @}

    void swap(ConvexHull &other);
    void swap(std::vector<Point> &pts);

private:
    void _construct();
    static bool _is_clockwise_turn(Point const &a, Point const &b, Point const &c);

    /// Take a vector of points and produce a pruned sorted vector.
    template <typename Iter>
    static void _prune(Iter first, Iter last, std::vector<Point> &out) {
        boost::optional<Point> ymin, ymax, xmin, xmax;
        for (Iter i = first; i != last; ++i) {
            Point p = *i;
            if (!ymin || Point::LexLess<Y>()(p, *ymin)) {
                ymin = p;
            }
            if (!xmin || Point::LexLess<X>()(p, *xmin)) {
                xmin = p;
            }
            if (!ymax || Point::LexGreater<Y>()(p, *ymax)) {
                ymax = p;
            }
            if (!xmax || Point::LexGreater<X>()(p, *xmax)) {
                xmax = p;
            }
        }
        if (!ymin) return;

        ConvexHull qhull(*xmin, *xmax, *ymin, *ymax);
        for (Iter i = first; i != last; ++i) {
            if (qhull.contains(*i)) continue;
            out.push_back(*i);
        }

        out.push_back(*xmin);
        out.push_back(*xmax);
        out.push_back(*ymin);
        out.push_back(*ymax);
        std::sort(out.begin(), out.end(), Point::LexLess<X>());
        out.erase(std::unique(out.begin(), out.end()), out.end());
    }

    /// Sequence of points forming the convex hull polygon.
    std::vector<Point> _boundary;
    /// Index one past the rightmost point, where the lower part of the boundary starts.
    std::size_t _lower;
};

/** @brief Output operator for convex hulls.
 * Prints out all the coordinates. */
inline std::ostream &operator<< (std::ostream &out_file, const Geom::ConvexHull &in_cvx) {
    out_file << "ConvexHull(";
    for(unsigned i = 0; i < in_cvx.size(); i++) {
        out_file << in_cvx[i] << ", ";
    }
    out_file << ")";
    return out_file;
}

} // end namespace Geom

#endif // LIB2GEOM_SEEN_CONVEX_HULL_H

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

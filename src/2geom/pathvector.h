/** @file
 * @brief PathVector - a sequence of subpaths
 *//*
 * Authors:
 *   Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2008-2014 authors
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

#ifndef LIB2GEOM_SEEN_PATHVECTOR_H
#define LIB2GEOM_SEEN_PATHVECTOR_H

#include <boost/concept/requires.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/range/algorithm/equal.hpp>
#include <2geom/forward.h>
#include <2geom/path.h>
#include <2geom/transforms.h>

namespace Geom {

/** @brief Generalized time value in the path vector.
 *
 * This class exists because mapping the range of multiple curves onto the same interval
 * as the curve index, we lose some precision. For instance, a path with 16 curves will
 * have 4 bits less precision than a path with 1 curve. If you need high precision results
 * in long paths, use this class and related methods instead of the standard methods
 * pointAt(), nearestTime() and so on.
 * 
 * @ingroup Paths */
struct PathVectorTime
    : public PathTime
    , boost::totally_ordered<PathVectorTime>
{
    size_type path_index; ///< Index of the path in the vector

    PathVectorTime() : PathTime(0, 0), path_index(0) {}
    PathVectorTime(size_type _i, size_type _c, Coord _t)
        : PathTime(_c, _t), path_index(_i) {}
    PathVectorTime(size_type _i, PathTime const &pos)
        : PathTime(pos), path_index(_i) {}

    bool operator<(PathVectorTime const &other) const {
        if (path_index < other.path_index) return true;
        if (path_index == other.path_index) {
            return static_cast<PathTime const &>(*this) < static_cast<PathTime const &>(other);
        }
        return false;
    }
    bool operator==(PathVectorTime const &other) const {
        return path_index == other.path_index
            && static_cast<PathTime const &>(*this) == static_cast<PathTime const &>(other);
    }

    PathTime const &asPathTime() const {
        return *static_cast<PathTime const *>(this);
    }
};

inline std::ostream &operator<<(std::ostream &os, PathVectorTime const &pvt) {
    os << pvt.path_index << ": " << pvt.asPathTime();
    return os;
}

typedef Intersection<PathVectorTime> PathVectorIntersection;
typedef PathVectorIntersection PVIntersection; ///< Alias to save typing

template <>
struct ShapeTraits<PathVector> {
    typedef PathVectorTime TimeType;
    //typedef PathVectorInterval IntervalType;
    typedef PathVector AffineClosureType;
    typedef PathVectorIntersection IntersectionType;
};

/** @brief Sequence of subpaths.
 *
 * This class corresponds to the SVG notion of a path:
 * a sequence of any number of open or closed contiguous subpaths.
 * Unlike Path, this class is closed under boolean operations.
 *
 * If you want to represent an arbitrary shape, this is the best class to use.
 * Shapes with a boundary that is composed of only a single contiguous
 * component can be represented with Path instead.
 *
 * @ingroup Paths
 */
class PathVector
    : MultipliableNoncommutative< PathVector, Affine
    , MultipliableNoncommutative< PathVector, Translate
    , MultipliableNoncommutative< PathVector, Scale
    , MultipliableNoncommutative< PathVector, Rotate
    , MultipliableNoncommutative< PathVector, HShear
    , MultipliableNoncommutative< PathVector, VShear
    , MultipliableNoncommutative< PathVector, Zoom
    , boost::equality_comparable< PathVector
      > > > > > > > >
{
    typedef std::vector<Path> Sequence;
public:
    typedef PathVectorTime Position;
    typedef Sequence::iterator iterator;
    typedef Sequence::const_iterator const_iterator;
    typedef Sequence::size_type size_type;
    typedef Path value_type;
    typedef Path &reference;
    typedef Path const &const_reference;
    typedef Path *pointer;
    typedef std::ptrdiff_t difference_type;

    PathVector() {}
    PathVector(Path const &p)
        : _data(1, p)
    {}
    template <typename InputIter>
    PathVector(InputIter first, InputIter last)
        : _data(first, last)
    {}

    /// Check whether the vector contains any paths.
    bool empty() const { return _data.empty(); }
    /// Get the number of paths in the vector.
    size_type size() const { return _data.size(); }
    /// Get the total number of curves in the vector.
    size_type curveCount() const;

    iterator begin() { return _data.begin(); }
    iterator end() { return _data.end(); }
    const_iterator begin() const { return _data.begin(); }
    const_iterator end() const { return _data.end(); }
    Path &operator[](size_type index) {
        return _data[index];
    }
    Path const &operator[](size_type index) const {
        return _data[index];
    }
    Path &at(size_type index) {
        return _data.at(index);
    }
    Path const &at(size_type index) const {
        return _data.at(index);
    }
    Path &front() { return _data.front(); }
    Path const &front() const { return _data.front(); }
    Path &back() { return _data.back(); }
    Path const &back() const { return _data.back(); }
    /// Append a path at the end.
    void push_back(Path const &path) {
        _data.push_back(path);
    }
    /// Remove the last path.
    void pop_back() {
        _data.pop_back();
    }
    iterator insert(iterator pos, Path const &p) {
        return _data.insert(pos, p);
    }
    template <typename InputIter>
    void insert(iterator out, InputIter first, InputIter last) {
        _data.insert(out, first, last);
    }
    /// Remove a path from the vector.
    iterator erase(iterator i) {
        return _data.erase(i);
    }
    /// Remove a range of paths from the vector.
    iterator erase(iterator first, iterator last) {
        return _data.erase(first, last);
    }
    /// Remove all paths from the vector.
    void clear() { _data.clear(); }
    /** @brief Change the number of paths.
     * If the vector size increases, it is passed with paths that contain only
     * a degenerate closing segment at (0,0). */
    void resize(size_type n) { _data.resize(n); }
    /** @brief Reverse the direction of paths in the vector.
     * @param reverse_paths If this is true, the order of paths is reversed as well;
     *                      otherwise each path is reversed, but their order in the
     *                      PathVector stays the same */
    void reverse(bool reverse_paths = true);
    /** @brief Get a new vector with reversed direction of paths.
     * @param reverse_paths If this is true, the order of paths is reversed as well;
     *                      otherwise each path is reversed, but their order in the
     *                      PathVector stays the same */
    PathVector reversed(bool reverse_paths = true) const;

    /// Get the range of allowed time values.
    Interval timeRange() const {
        Interval ret(0, curveCount()); return ret;
    }
    /** @brief Get the first point in the first path of the vector.
     * This method will throw an exception if the vector doesn't contain any paths. */
    Point initialPoint() const {
        return _data.front().initialPoint();
    }
    /** @brief Get the last point in the last path of the vector.
     * This method will throw an exception if the vector doesn't contain any paths. */
    Point finalPoint() const {
        return _data.back().finalPoint();
    }
    Path &pathAt(Coord t, Coord *rest = NULL);
    Path const &pathAt(Coord t, Coord *rest = NULL) const;
    Curve const &curveAt(Coord t, Coord *rest = NULL) const;
    Coord valueAt(Coord t, Dim2 d) const;
    Point pointAt(Coord t) const;

    Path &pathAt(PathVectorTime const &pos) {
        return const_cast<Path &>(static_cast<PathVector const*>(this)->pathAt(pos));
    }
    Path const &pathAt(PathVectorTime const &pos) const {
        return at(pos.path_index);
    }
    Curve const &curveAt(PathVectorTime const &pos) const {
        return at(pos.path_index).at(pos.curve_index);
    }
    Point pointAt(PathVectorTime const &pos) const {
        return at(pos.path_index).at(pos.curve_index).pointAt(pos.t);
    }
    Coord valueAt(PathVectorTime const &pos, Dim2 d) const {
        return at(pos.path_index).at(pos.curve_index).valueAt(pos.t, d);
    }

    OptRect boundsFast() const;
    OptRect boundsExact() const;

    template <typename T>
    BOOST_CONCEPT_REQUIRES(((TransformConcept<T>)), (PathVector &))
    operator*=(T const &t) {
        if (empty()) return *this;
        for (iterator i = begin(); i != end(); ++i) {
            *i *= t;
        }
        return *this;
    }

    bool operator==(PathVector const &other) const {
        return boost::range::equal(_data, other._data);
    }

    void snapEnds(Coord precision = EPSILON);

    std::vector<PVIntersection> intersect(PathVector const &other, Coord precision = EPSILON) const;

    /** @brief Determine the winding number at the specified point.
     * This is simply the sum of winding numbers for constituent paths. */
    int winding(Point const &p) const;

    boost::optional<PathVectorTime> nearestTime(Point const &p, Coord *dist = NULL) const;
    std::vector<PathVectorTime> allNearestTimes(Point const &p, Coord *dist = NULL) const;

    std::vector<Point> nodes() const;

private:
    PathVectorTime _factorTime(Coord t) const;

    Sequence _data;
};

inline OptRect bounds_fast(PathVector const &pv) { return pv.boundsFast(); }
inline OptRect bounds_exact(PathVector const &pv) { return pv.boundsExact(); }

std::ostream &operator<<(std::ostream &out, PathVector const &pv);

} // end namespace Geom

#endif // LIB2GEOM_SEEN_PATHVECTOR_H

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

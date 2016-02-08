/** @file
 * @brief Path - a sequence of contiguous curves
 *//*
  * Authors:
  *   MenTaLguY <mental@rydia.net>
  *   Marco Cecchetti <mrcekets at gmail.com>
  *   Krzysztof Kosiński <tweenk.pl@gmail.com>
  *
  * Copyright 2007-2014 Authors
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

#ifndef LIB2GEOM_SEEN_PATH_H
#define LIB2GEOM_SEEN_PATH_H

#include <iterator>
#include <algorithm>
#include <iostream>
#include <boost/operators.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>
#include <2geom/intersection.h>
#include <2geom/curve.h>
#include <2geom/bezier-curve.h>
#include <2geom/transforms.h>

namespace Geom {

class Path;
class ConvexHull;

namespace PathInternal {

typedef boost::ptr_vector<Curve> Sequence;

struct PathData {
    Sequence curves;
    OptRect fast_bounds;
};

template <typename P>
class BaseIterator
    : public boost::random_access_iterator_helper
        < BaseIterator<P>
        , Curve const
        , std::ptrdiff_t
        , Curve const *
        , Curve const &
        >
{
  protected:
    BaseIterator(P &p, unsigned i) : path(&p), index(i) {}
    // default copy, default assign
    typedef BaseIterator<P> Self;

  public:
    BaseIterator() : path(NULL), index(0) {}

    bool operator<(BaseIterator const &other) const {
        return path == other.path && index < other.index;
    }
    bool operator==(BaseIterator const &other) const {
        return path == other.path && index == other.index;
    }
    Curve const &operator*() const {
        return (*path)[index];
    }

    Self &operator++() {
        ++index;
        return *this;
    }
    Self &operator--() {
        --index;
        return *this;
    }
    Self &operator+=(std::ptrdiff_t d) {
        index += d;
        return *this;
    }
    Self &operator-=(std::ptrdiff_t d) {
        index -= d;
        return *this;
    }

  private:
    P *path;
    unsigned index;

    friend class ::Geom::Path;
};

}

/** @brief Generalized time value in the path.
 *
 * This class exists because when mapping the range of multiple curves onto the same interval
 * as the curve index, we lose some precision. For instance, a path with 16 curves will
 * have 4 bits less precision than a path with 1 curve. If you need high precision results
 * in long paths, either use this class and related methods instead of the standard methods
 * pointAt(), nearestTime() and so on, or use curveAt() to first obtain the curve, then
 * call the method again to obtain a high precision result.
 * 
 * @ingroup Paths */
struct PathTime
    : boost::totally_ordered<PathTime>
{
    typedef PathInternal::Sequence::size_type size_type;

    Coord t; ///< Time value in the curve
    size_type curve_index; ///< Index of the curve in the path

    PathTime() : t(0), curve_index(0) {}
    PathTime(size_type idx, Coord tval) : t(tval), curve_index(idx) {}

    bool operator<(PathTime const &other) const {
        if (curve_index < other.curve_index) return true;
        if (curve_index == other.curve_index) {
            return t < other.t;
        }
        return false;
    }
    bool operator==(PathTime const &other) const {
        return curve_index == other.curve_index && t == other.t;
    }
    /// Convert times at or beyond 1 to 0 on the next curve.
    void normalizeForward(size_type path_size) {
        if (t >= 1) {
            curve_index = (curve_index + 1) % path_size;
            t = 0;
        }
    }
    /// Convert times at or before 0 to 1 on the previous curve.
    void normalizeBackward(size_type path_size) {
        if (t <= 0) {
            curve_index = (curve_index - 1) % path_size;
            t = 1;
        }
    }

    Coord asFlatTime() const { return curve_index + t; }
};

inline std::ostream &operator<<(std::ostream &os, PathTime const &pos) {
    os << pos.curve_index << ": " << format_coord_nice(pos.t);
    return os;
}


/** @brief Contiguous subset of the path's parameter domain.
 * This is a directed interval, which allows one to specify any contiguous subset
 * of the path's domain, including subsets that wrap around the initial point
 * of the path.
 * @ingroup Paths */
class PathInterval {
public:
    typedef PathInternal::Sequence::size_type size_type;

    /** @brief Default interval.
     * Default-constructed PathInterval includes only the initial point of the initial segment. */
    PathInterval();

    /** @brief Construct an interval in the path's parameter domain.
     * @param from Initial time
     * @param to Final time
     * @param cross_start If true, the interval will proceed from the initial to final
     *   time through the initial point of the path, wrapping around the closing segment;
     *   otherwise it will not wrap around the closing segment.
     * @param path_size Size of the path to which this interval applies, required
     *   to clean up degenerate cases */
    PathInterval(PathTime const &from, PathTime const &to, bool cross_start, size_type path_size);

    /// Get the time value of the initial point.
    PathTime const &initialTime() const { return _from; }
    /// Get the time value of the final point.
    PathTime const &finalTime() const { return _to; }

    PathTime const &from() const { return _from; }
    PathTime const &to() const { return _to; }

    /// Check whether the interval has only one point.
    bool isDegenerate() const { return _from == _to; }
    /// True if the interval goes in the direction of decreasing time values.
    bool reverse() const { return _reverse; }
    /// True if the interior of the interval contains the initial point of the path.
    bool crossesStart() const { return _cross_start; }

    /// Test a path time for inclusion.
    bool contains(PathTime const &pos) const;

    /// Get a time at least @a min_dist away in parameter space from the ends.
    /// If no such time exists, the middle point is returned.
    PathTime inside(Coord min_dist = EPSILON) const;

    /// Select one of two intervals with given endpoints by parameter direction.
    static PathInterval from_direction(PathTime const &from, PathTime const &to,
                                       bool reversed, size_type path_size);

    /// Select one of two intervals with given endpoints by whether it includes the initial point.
    static PathInterval from_start_crossing(PathTime const &from, PathTime const &to,
                                            bool cross_start, size_type path_size) {
        PathInterval result(from, to, cross_start, path_size);
        return result;
    }

    size_type pathSize() const { return _path_size; }
    size_type curveCount() const;

private:
    PathTime _from, _to;
    size_type _path_size;
    bool _cross_start, _reverse;
};

/// Create an interval in the direction of increasing time value.
/// @relates PathInterval
inline PathInterval forward_interval(PathTime const &from, PathTime const &to,
                                     PathInterval::size_type path_size)
{
    PathInterval result = PathInterval::from_direction(from, to, false, path_size);
    return result;
}

/// Create an interval in the direction of decreasing time value.
/// @relates PathInterval
inline PathInterval backward_interval(PathTime const &from, PathTime const &to,
                                      PathInterval::size_type path_size)
{
    PathInterval result = PathInterval::from_direction(from, to, true, path_size);
    return result;
}

/// Output an interval in the path's domain.
/// @relates PathInterval
inline std::ostream &operator<<(std::ostream &os, PathInterval const &ival) {
    os << "PathInterval[";
    if (ival.crossesStart()) {
        os << ival.from() << " -> 0: 0.0 -> " << ival.to();
    } else {
        os << ival.from() << " -> " << ival.to();
    }
    os << "]";
    return os;
}

typedef Intersection<PathTime> PathIntersection;

template <>
struct ShapeTraits<Path> {
    typedef PathTime TimeType;
    typedef PathInterval IntervalType;
    typedef Path AffineClosureType;
    typedef PathIntersection IntersectionType;
};

/** @brief Sequence of contiguous curves, aka spline.
 *
 * Path represents a sequence of contiguous curves, also known as a spline.
 * It corresponds to a "subpath" in SVG terminology. It can represent both
 * open and closed subpaths. The final point of each curve is exactly
 * equal to the initial point of the next curve.
 *
 * The path always contains a linear closing segment that connects
 * the final point of the last "real" curve to the initial point of the
 * first curve. This way the curves form a closed loop even for open paths.
 * If the closing segment has nonzero length and the path is closed, it is
 * considered a normal part of the path data. There are three distinct sets
 * of end iterators one can use to iterate over the segments:
 *
 * - Iterating between @a begin() and @a end() will iterate over segments
 *   which are part of the path.
 * - Iterating between @a begin() and @a end_closed()
 *   will always iterate over a closed loop of segments.
 * - Iterating between @a begin() and @a end_open() will always skip
 *   the final linear closing segment.
 *
 * If the final point of the last "real" segment coincides exactly with the initial
 * point of the first segment, the closing segment will be absent from both
 * [begin(), end_open()) and [begin(), end_closed()).
 *
 * Normally, an exception will be thrown when you try to insert a curve
 * that makes the path non-continuous. If you are working with unsanitized
 * curve data, you can call setStitching(true), which will insert line segments
 * to make the path continuous.
 *
 * Internally, Path uses copy-on-write data. This is done for two reasons: first,
 * copying a Curve requires calling a virtual function, so it's a little more expensive
 * that normal copying; and second, it reduces the memory cost of copying the path.
 * Therefore you can return Path and PathVector from functions without worrying
 * about temporary copies.
 *
 * Note that this class cannot represent arbitrary shapes, which may contain holes.
 * To do that, use PathVector, which is more generic.
 *
 * It's not very convenient to create a Path directly. To construct paths more easily,
 * use PathBuilder.
 *
 * @ingroup Paths */
class Path
    : boost::equality_comparable< Path >
{
public:
    typedef PathInternal::PathData PathData;
    typedef PathInternal::Sequence Sequence;
    typedef PathInternal::BaseIterator<Path> iterator;
    typedef PathInternal::BaseIterator<Path const> const_iterator;
    typedef Sequence::size_type size_type;
    typedef Sequence::difference_type difference_type;

    class ClosingSegment : public LineSegment {
      public:
        ClosingSegment() : LineSegment() {}
        ClosingSegment(Point const &p1, Point const &p2) : LineSegment(p1, p2) {}
        virtual Curve *duplicate() const { return new ClosingSegment(*this); }
        virtual Curve *reverse() const { return new ClosingSegment((*this)[1], (*this)[0]); }
    };

    class StitchSegment : public LineSegment {
      public:
        StitchSegment() : LineSegment() {}
        StitchSegment(Point const &p1, Point const &p2) : LineSegment(p1, p2) {}
        virtual Curve *duplicate() const { return new StitchSegment(*this); }
        virtual Curve *reverse() const { return new StitchSegment((*this)[1], (*this)[0]); }
    };

    // Path(Path const &other) - use default copy constructor

    /// Construct an empty path starting at the specified point.
    explicit Path(Point const &p = Point())
        : _data(new PathData())
        , _closing_seg(new ClosingSegment(p, p))
        , _closed(false)
        , _exception_on_stitch(true)
    {
        _data->curves.push_back(_closing_seg);
    }

    /// Construct a path containing a range of curves.
    template <typename Iter>
    Path(Iter first, Iter last, bool closed = false, bool stitch = false)
        : _data(new PathData())
        , _closed(closed)
        , _exception_on_stitch(!stitch)
    {
        for (Iter i = first; i != last; ++i) {
            _data->curves.push_back(i->duplicate());
        }
        if (!_data->curves.empty()) {
            _closing_seg = new ClosingSegment(_data->curves.back().finalPoint(),
                                              _data->curves.front().initialPoint());
        } else {
            _closing_seg = new ClosingSegment();
        }
        _data->curves.push_back(_closing_seg);
    }

    /// Create a path from a rectangle.
    explicit Path(Rect const &r);
    /// Create a path from a convex hull.
    explicit Path(ConvexHull const &);
    /// Create a path from a circle, using two elliptical arcs.
    explicit Path(Circle const &c);
    /// Create a path from an ellipse, using two elliptical arcs.
    explicit Path(Ellipse const &e);

    virtual ~Path() {}

    // Path &operator=(Path const &other) - use default assignment operator

    /** @brief Swap contents with another path
     * @todo Add noexcept specifiers for C++11 */
    void swap(Path &other) throw() {
        using std::swap;
        swap(other._data, _data);
        swap(other._closing_seg, _closing_seg);
        swap(other._closed, _closed);
        swap(other._exception_on_stitch, _exception_on_stitch);
    }
    /** @brief Swap contents of two paths.
     * @relates Path */
    friend inline void swap(Path &a, Path &b) throw() { a.swap(b); }

    /** @brief Access a curve by index */
    Curve const &operator[](size_type i) const { return _data->curves[i]; }
    /** @brief Access a curve by index */
    Curve const &at(size_type i) const { return _data->curves.at(i); }

    /** @brief Access the first curve in the path.
     * Since the curve always contains at least a degenerate closing segment,
     * it is always safe to use this method. */
    Curve const &front() const { return _data->curves.front(); }
    /// Alias for front().
    Curve const &initialCurve() const { return _data->curves.front(); }
    /** @brief Access the last curve in the path. */
    Curve const &back() const { return back_default(); }
    Curve const &back_open() const {
        if (empty()) return _data->curves.back();
        return _data->curves[_data->curves.size() - 2];
    }
    Curve const &back_closed() const {
        return _closing_seg->isDegenerate()
            ? _data->curves[_data->curves.size() - 2]
            : _data->curves[_data->curves.size() - 1];
    }
    Curve const &back_default() const {
        return _includesClosingSegment()
            ? back_closed()
            : back_open();
    }
    Curve const &finalCurve() const { return back_default(); }

    const_iterator begin() const { return const_iterator(*this, 0); }
    const_iterator end() const { return end_default(); }
    const_iterator end_default() const { return const_iterator(*this, size_default()); }
    const_iterator end_open() const { return const_iterator(*this, size_open()); }
    const_iterator end_closed() const { return const_iterator(*this, size_closed()); }
    iterator begin() { return iterator(*this, 0); }
    iterator end() { return end_default(); }
    iterator end_default() { return iterator(*this, size_default()); }
    iterator end_open() { return iterator(*this, size_open()); }
    iterator end_closed() { return iterator(*this, size_closed()); }

    /// Size without the closing segment, even if the path is closed.
    size_type size_open() const { return _data->curves.size() - 1; }

    /** @brief Size with the closing segment, if it makes a difference.
     * If the closing segment is degenerate, i.e. its initial and final points
     * are exactly equal, then it is not included in this size. */
    size_type size_closed() const {
        return _closing_seg->isDegenerate() ? _data->curves.size() - 1 : _data->curves.size();
    }

    /// Natural size of the path.
    size_type size_default() const {
        return _includesClosingSegment() ? size_closed() : size_open();
    }
    /// Natural size of the path.
    size_type size() const { return size_default(); }

    size_type max_size() const { return _data->curves.max_size() - 1; }

    /** @brief Check whether path is empty.
     * The path is empty if it contains only the closing segment, which according
     * to the continuity invariant must be degenerate. Note that unlike standard
     * containers, two empty paths are not necessarily identical, because the
     * degenerate closing segment may be at a different point, affecting the operation
     * of methods such as appendNew(). */
    bool empty() const { return (_data->curves.size() == 1); }

    /// Check whether the path is closed.
    bool closed() const { return _closed; }

    /** @brief Set whether the path is closed.
     * When closing a path where the last segment can be represented as a closing
     * segment, the last segment will be removed. When opening a path, the closing
     * segment will be erased. This means that closing and then opening a path
     * will not always give back the original path. */
    void close(bool closed = true);

    /** @brief Remove all curves from the path.
     * The initial and final points of the closing segment are set to (0,0).
     * The stitching flag remains unchanged. */
    void clear();

    /** @brief Get the approximate bounding box.
     * The rectangle returned by this method will contain all the curves, but it's not
     * guaranteed to be the smallest possible one */
    OptRect boundsFast() const;

    /** @brief Get a tight-fitting bounding box.
     * This will return the smallest possible axis-aligned rectangle containing
     * all the curves in the path. */
    OptRect boundsExact() const;

    Piecewise<D2<SBasis> > toPwSb() const;

    /// Test paths for exact equality.
    bool operator==(Path const &other) const;

    /// Apply a transform to each curve.
    template <typename T>
    Path &operator*=(T const &tr) {
        BOOST_CONCEPT_ASSERT((TransformConcept<T>));
        _unshare();
        for (std::size_t i = 0; i < _data->curves.size(); ++i) {
            _data->curves[i] *= tr;
        }
        return *this;
    }

    template <typename T>
    friend Path operator*(Path const &path, T const &tr) {
        BOOST_CONCEPT_ASSERT((TransformConcept<T>));
        Path result(path);
        result *= tr;
        return result;
    }

    /** @brief Get the allowed range of time values.
     * @return Values for which pointAt() and valueAt() yield valid results. */
    Interval timeRange() const;

    /** Get the curve at the specified time value.
     * @param t Time value
     * @param rest Optional storage for the corresponding time value in the curve */
    Curve const &curveAt(Coord t, Coord *rest = NULL) const;

    /// Get the closing segment of the path.
    LineSegment const &closingSegment() const { return *_closing_seg; }

    /** @brief Get the point at the specified time value.
     * Note that this method has reduced precision with respect to calling pointAt()
     * directly on the curve. If you want high precision results, use the version
     * that takes a PathTime parameter.
     * 
     * Allowed time values range from zero to the number of curves; you can retrieve
     * the allowed range of values with timeRange(). */
    Point pointAt(Coord t) const;

    /// Get one coordinate (X or Y) at the specified time value.
    Coord valueAt(Coord t, Dim2 d) const;

    /// Get the curve at the specified path time.
    Curve const &curveAt(PathTime const &pos) const;
    /// Get the point at the specified path time.
    Point pointAt(PathTime const &pos) const;
    /// Get one coordinate at the specified path time.
    Coord valueAt(PathTime const &pos, Dim2 d) const;

    Point operator()(Coord t) const { return pointAt(t); }

    /// Compute intersections with axis-aligned line.
    std::vector<PathTime> roots(Coord v, Dim2 d) const;

    /// Compute intersections with another path.
    std::vector<PathIntersection> intersect(Path const &other, Coord precision = EPSILON) const;

    /** @brief Determine the winding number at the specified point.
     * 
     * The winding number is the number of full turns made by a ray that connects the passed
     * point and the path's value (i.e. the result of the pointAt() method) as the time increases
     * from 0 to the maximum valid value. Positive numbers indicate turns in the direction
     * of increasing angles.
     *
     * Winding numbers are often used as the definition of what is considered "inside"
     * the shape. Typically points with either nonzero winding or odd winding are
     * considered to be inside the path. */
    int winding(Point const &p) const;

    std::vector<Coord> allNearestTimes(Point const &p, Coord from, Coord to) const;
    std::vector<Coord> allNearestTimes(Point const &p) const {
        return allNearestTimes(p, 0, size_default());
    }

    PathTime nearestTime(Point const &p, Coord *dist = NULL) const;
    std::vector<Coord> nearestTimePerCurve(Point const &p) const;

    std::vector<Point> nodes() const;

    void appendPortionTo(Path &p, Coord f, Coord t) const;

    /** @brief Append a subset of this path to another path.
     * An extra stitching segment will be inserted if the start point of the portion
     * and the final point of the target path do not match exactly.
     * The closing segment of the target path will be modified. */
    void appendPortionTo(Path &p, PathTime const &from, PathTime const &to, bool cross_start = false) const {
        PathInterval ival(from, to, cross_start, size_closed());
        appendPortionTo(p, ival, boost::none, boost::none);
    }

    /** @brief Append a subset of this path to another path.
     * This version allows you to explicitly pass a PathInterval. */
    void appendPortionTo(Path &p, PathInterval const &ival) const {
        appendPortionTo(p, ival, boost::none, boost::none);
    }

    /** @brief Append a subset of this path to another path, specifying endpoints.
     * This method is for use in situations where endpoints of the portion segments
     * have to be set exactly, for instance when computing Boolean operations. */
    void appendPortionTo(Path &p, PathInterval const &ival,
                         boost::optional<Point> const &p_from, boost::optional<Point> const &p_to) const;

    Path portion(Coord f, Coord t) const {
        Path ret;
        ret.close(false);
        appendPortionTo(ret, f, t);
        return ret;
    }

    Path portion(Interval const &i) const { return portion(i.min(), i.max()); }

    /** @brief Get a subset of the current path with full precision.
     * When @a from is larger (later in the path) than @a to, the returned portion
     * will be reversed. If @a cross_start is true, the portion will be reversed
     * and will cross the initial point of the path. Therefore, when @a from is larger
     * than @a to and @a cross_start is true, the returned portion will not be reversed,
     * but will "wrap around" the end of the path. */
    Path portion(PathTime const &from, PathTime const &to, bool cross_start = false) const {
        Path ret;
        ret.close(false);
        appendPortionTo(ret, from, to, cross_start);
        return ret;
    }

    /** @brief Get a subset of the current path with full precision.
     * This version allows you to explicitly pass a PathInterval. */
    Path portion(PathInterval const &ival) const {
        Path ret;
        ret.close(false);
        appendPortionTo(ret, ival);
        return ret;
    }

    /** @brief Obtain a reversed version of the current path.
     * The final point of the current path will become the initial point
     * of the reversed path, unless it is closed and has a non-degenerate
     * closing segment. In that case, the new initial point will be the final point
     * of the last "real" segment. */
    Path reversed() const;

    void insert(iterator pos, Curve const &curve);
    
    template <typename Iter>
    void insert(iterator pos, Iter first, Iter last) {
        _unshare();
        Sequence::iterator seq_pos(seq_iter(pos));
        Sequence source;
        for (; first != last; ++first) {
            source.push_back(first->duplicate());
        }
        do_update(seq_pos, seq_pos, source);
    }

    void erase(iterator pos);
    void erase(iterator first, iterator last);

    // erase last segment of path
    void erase_last() { erase(iterator(*this, size() - 1)); }

    void start(Point const &p);

    /** @brief Get the first point in the path. */
    Point initialPoint() const { return (*_closing_seg)[1]; }

    /** @brief Get the last point in the path.
     * If the path is closed, this is always the same as the initial point. */
    Point finalPoint() const { return (*_closing_seg)[_closed ? 1 : 0]; }

    void setInitial(Point const &p) {
        _unshare();
        _closed = false;
        _data->curves.front().setInitial(p);
        _closing_seg->setFinal(p);
    }
    void setFinal(Point const &p) {
        _unshare();
        _closed = false;
        _data->curves[size_open() - 1].setFinal(p);
        _closing_seg->setInitial(p);
    }

    /** @brief Add a new curve to the end of the path.
     * This inserts the new curve right before the closing segment.
     * The path takes ownership of the passed pointer, which should not be freed. */
    void append(Curve *curve) {
        _unshare();
        stitchTo(curve->initialPoint());
        do_append(curve);
    }

    void append(Curve const &curve) {
        _unshare();
        stitchTo(curve.initialPoint());
        do_append(curve.duplicate());
    }
    void append(D2<SBasis> const &curve) {
        _unshare();
        stitchTo(Point(curve[X][0][0], curve[Y][0][0]));
        do_append(new SBasisCurve(curve));
    }
    void append(Path const &other) {
        replace(end_open(), other.begin(), other.end());
    }

    void replace(iterator replaced, Curve const &curve);
    void replace(iterator first, iterator last, Curve const &curve);
    void replace(iterator replaced, Path const &path);
    void replace(iterator first, iterator last, Path const &path);

    template <typename Iter>
    void replace(iterator replaced, Iter first, Iter last) {
        replace(replaced, replaced + 1, first, last);
    }

    template <typename Iter>
    void replace(iterator first_replaced, iterator last_replaced, Iter first, Iter last) {
        _unshare();
        Sequence::iterator seq_first_replaced(seq_iter(first_replaced));
        Sequence::iterator seq_last_replaced(seq_iter(last_replaced));
        Sequence source;
        for (; first != last; ++first) {
            source.push_back(first->duplicate());
        }
        do_update(seq_first_replaced, seq_last_replaced, source);
    }

    /** @brief Append a new curve to the path.
     *
     * This family of methods will automaticaly use the current final point of the path
     * as the first argument of the new curve's constructor. To call this method,
     * you'll need to write e.g.:
     * @code
       path.template appendNew<CubicBezier>(control1, control2, end_point);
       @endcode
     * It is important to note that the coordinates passed to appendNew should be finite!
     * If one of the coordinates is infinite, 2geom will throw a ContinuityError exception.
     */
    template <typename CurveType, typename A>
    void appendNew(A a) {
        _unshare();
        do_append(new CurveType(finalPoint(), a));
    }

    template <typename CurveType, typename A, typename B>
    void appendNew(A a, B b) {
        _unshare();
        do_append(new CurveType(finalPoint(), a, b));
    }

    template <typename CurveType, typename A, typename B, typename C>
    void appendNew(A a, B b, C c) {
        _unshare();
        do_append(new CurveType(finalPoint(), a, b, c));
    }

    template <typename CurveType, typename A, typename B, typename C, typename D>
    void appendNew(A a, B b, C c, D d) {
        _unshare();
        do_append(new CurveType(finalPoint(), a, b, c, d));
    }

    template <typename CurveType, typename A, typename B, typename C, typename D, typename E>
    void appendNew(A a, B b, C c, D d, E e) {
        _unshare();
        do_append(new CurveType(finalPoint(), a, b, c, d, e));
    }

    template <typename CurveType, typename A, typename B, typename C, typename D, typename E, typename F>
    void appendNew(A a, B b, C c, D d, E e, F f) {
        _unshare();
        do_append(new CurveType(finalPoint(), a, b, c, d, e, f));
    }

    template <typename CurveType, typename A, typename B, typename C, typename D, typename E, typename F, typename G>
    void appendNew(A a, B b, C c, D d, E e, F f, G g) {
        _unshare();
        do_append(new CurveType(finalPoint(), a, b, c, d, e, f, g));
    }

    template <typename CurveType, typename A, typename B, typename C, typename D, typename E, typename F, typename G,
              typename H>
    void appendNew(A a, B b, C c, D d, E e, F f, G g, H h) {
        _unshare();
        do_append(new CurveType(finalPoint(), a, b, c, d, e, f, g, h));
    }

    template <typename CurveType, typename A, typename B, typename C, typename D, typename E, typename F, typename G,
              typename H, typename I>
    void appendNew(A a, B b, C c, D d, E e, F f, G g, H h, I i) {
        _unshare();
        do_append(new CurveType(finalPoint(), a, b, c, d, e, f, g, h, i));
    }

    /** @brief Reduce the closing segment to a point if it's shorter than precision.
     * Do this by moving the final point. */
    void snapEnds(Coord precision = EPSILON);

    /// Append a stitching segment ending at the specified point.
    void stitchTo(Point const &p);

    /** @brief Verify the continuity invariant.
     * If the path is not contiguous, this will throw a CountinuityError. */
    void checkContinuity() const;

    /** @brief Enable or disable the throwing of exceptions when stitching discontinuities.
     * Normally stitching will cause exceptions, but when you are working with unsanitized
     * curve data, you can disable these exceptions. */
    void setStitching(bool x) {
        _exception_on_stitch = !x;
    }

private:
    static Sequence::iterator seq_iter(iterator const &iter) {
        return iter.path->_data->curves.begin() + iter.index;
    }
    static Sequence::const_iterator seq_iter(const_iterator const &iter) {
        return iter.path->_data->curves.begin() + iter.index;
    }

    // whether the closing segment is part of the path
    bool _includesClosingSegment() const {
        return _closed && !_closing_seg->isDegenerate();
    }
    void _unshare() {
        // Called before every mutation.
        // Ensure we have our own copy of curve data and reset cached values
        if (!_data.unique()) {
            _data.reset(new PathData(*_data));
            _closing_seg = static_cast<ClosingSegment*>(&_data->curves.back());
        }
        _data->fast_bounds = OptRect();
    }
    PathTime _factorTime(Coord t) const;

    void stitch(Sequence::iterator first_replaced, Sequence::iterator last_replaced, Sequence &sequence);
    void do_update(Sequence::iterator first, Sequence::iterator last, Sequence &source);

    // n.b. takes ownership of curve object
    void do_append(Curve *curve);

    boost::shared_ptr<PathData> _data;
    ClosingSegment *_closing_seg;
    bool _closed;
    bool _exception_on_stitch;
}; // end class Path

Piecewise<D2<SBasis> > paths_to_pw(PathVector const &paths);

inline Coord nearest_time(Point const &p, Path const &c) {
    PathTime pt = c.nearestTime(p);
    return pt.curve_index + pt.t;
}

bool are_near(Path const &a, Path const &b, Coord precision = EPSILON);

std::ostream &operator<<(std::ostream &out, Path const &path);

} // end namespace Geom


#endif // LIB2GEOM_SEEN_PATH_H

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

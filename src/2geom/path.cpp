/** @file
 * @brief Path - a sequence of contiguous curves (implementation file)
 *//*
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright 2007-2014  authors
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

#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <2geom/transforms.h>
#include <2geom/circle.h>
#include <2geom/ellipse.h>
#include <2geom/convex-hull.h>
#include <2geom/svg-path-writer.h>
#include <2geom/sweeper.h>
#include <algorithm>
#include <limits>

using std::swap;
using namespace Geom::PathInternal;

namespace Geom {

// this represents an empty interval
PathInterval::PathInterval()
    : _from(0, 0.0)
    , _to(0, 0.0)
    , _path_size(1)
    , _cross_start(false)
    , _reverse(false)
{}

PathInterval::PathInterval(PathTime const &from, PathTime const &to, bool cross_start, size_type path_size)
    : _from(from)
    , _to(to)
    , _path_size(path_size)
    , _cross_start(cross_start)
    , _reverse(cross_start ? to >= from : to < from)
{
    if (_reverse) {
        _to.normalizeForward(_path_size);
        if (_from != _to) {
            _from.normalizeBackward(_path_size);
        }
    } else {
        _from.normalizeForward(_path_size);
        if (_from != _to) {
            _to.normalizeBackward(_path_size);
        }
    }

    if (_from == _to) {
        _reverse = false;
        _cross_start = false;
    }
}

bool PathInterval::contains(PathTime const &pos) const {
    if (_cross_start) {
        if (_reverse) {
            return pos >= _to || _from >= pos;
        } else {
            return pos >= _from || _to >= pos;
        }
    } else {
        if (_reverse) {
            return _to <= pos && pos <= _from;
        } else {
            return _from <= pos && pos <= _to;
        }
    }
}

PathInterval::size_type PathInterval::curveCount() const
{
    if (isDegenerate()) return 0;
    if (_cross_start) {
        if (_reverse) {
            return _path_size - _to.curve_index + _from.curve_index + 1;
        } else {
            return _path_size - _from.curve_index + _to.curve_index + 1;
        }
    } else {
        if (_reverse) {
            return _from.curve_index - _to.curve_index + 1;
        } else {
            return _to.curve_index - _from.curve_index + 1;
        }
    }
}

PathTime PathInterval::inside(Coord min_dist) const
{
    // If there is some node further than min_dist (in time coord) from the ends,
    // return that node. Otherwise, return the middle.
    PathTime result(0, 0.0);

    if (!_cross_start && _from.curve_index == _to.curve_index) {
        PathTime result(_from.curve_index, lerp(0.5, _from.t, _to.t));
        return result;
    }
    // If _cross_start, then we can be sure that at least one node is in the domain.
    // If dcurve == 0, it actually means that all curves are included in the domain

    if (_reverse) {
        size_type dcurve = (_path_size + _from.curve_index - _to.curve_index) % _path_size;
        bool from_close = _from.t < min_dist;
        bool to_close = _to.t > 1 - min_dist;

        if (dcurve == 0) {
            dcurve = _path_size;
        }

        if (dcurve == 1) {
            if (from_close || to_close) {
                result.curve_index = _from.curve_index;
                Coord tmid = _from.t - ((1 - _to.t) + _from.t) * 0.5;
                if (tmid < 0) {
                    result.curve_index = (_path_size + result.curve_index - 1) % _path_size;
                    tmid += 1;
                }
                result.t = tmid;
                return result;
            }

            result.curve_index = _from.curve_index;
            return result;
        }

        result.curve_index = (_to.curve_index + 1) % _path_size;
        if (to_close) {
            if (dcurve == 2) {
                result.t = 0.5;
            } else {
                result.curve_index = (result.curve_index + 1) % _path_size;
            }
        }
        return result;
    } else {
        size_type dcurve = (_path_size + _to.curve_index - _from.curve_index) % _path_size;
        bool from_close = _from.t > 1 - min_dist;
        bool to_close = _to.t < min_dist;

        if (dcurve == 0) {
            dcurve = _path_size;
        }

        if (dcurve == 1) {
            if (from_close || to_close) {
                result.curve_index = _from.curve_index;
                Coord tmid = ((1 - _from.t) + _to.t) * 0.5 + _from.t;
                if (tmid >= 1) {
                    result.curve_index = (result.curve_index + 1) % _path_size;
                    tmid -= 1;
                }
                result.t = tmid;
                return result;
            }

            result.curve_index = _to.curve_index;
            return result;
        }

        result.curve_index = (_from.curve_index + 1) % _path_size;
        if (from_close) {
            if (dcurve == 2) {
                result.t = 0.5;
            } else {
                result.curve_index = (result.curve_index + 1) % _path_size;
            }
        }
        return result;
    }

    result.curve_index = _reverse ? _from.curve_index : _to.curve_index;
    return result;
}

PathInterval PathInterval::from_direction(PathTime const &from, PathTime const &to, bool reversed, size_type path_size)
{
    PathInterval result;
    result._from = from;
    result._to = to;
    result._path_size = path_size;

    if (reversed) {
        result._to.normalizeForward(path_size);
        if (result._from != result._to) {
            result._from.normalizeBackward(path_size);
        }
    } else {
        result._from.normalizeForward(path_size);
        if (result._from != result._to) {
            result._to.normalizeBackward(path_size);
        }
    }

    if (result._from == result._to) {
        result._reverse = false;
        result._cross_start = false;
    } else {
        result._reverse = reversed;
        if (reversed) {
            result._cross_start = from < to;
        } else {
            result._cross_start = to < from;
        }
    }
    return result;
}


Path::Path(Rect const &r)
    : _data(new PathData())
    , _closing_seg(new ClosingSegment(r.corner(3), r.corner(0)))
    , _closed(true)
    , _exception_on_stitch(true)
{
    for (unsigned i = 0; i < 3; ++i) {
        _data->curves.push_back(new LineSegment(r.corner(i), r.corner(i+1)));
    }
    _data->curves.push_back(_closing_seg);
}

Path::Path(ConvexHull const &ch)
    : _data(new PathData())
    , _closing_seg(new ClosingSegment(Point(), Point()))
    , _closed(true)
    , _exception_on_stitch(true)
{
    if (ch.empty()) {
        _data->curves.push_back(_closing_seg);
        return;
    }

    _closing_seg->setInitial(ch.back());
    _closing_seg->setFinal(ch.front());

    Point last = ch.front();

    for (std::size_t i = 1; i < ch.size(); ++i) {
        _data->curves.push_back(new LineSegment(last, ch[i]));
        last = ch[i];
    }

    _data->curves.push_back(_closing_seg);
    _closed = true;
}

Path::Path(Circle const &c)
    : _data(new PathData())
    , _closing_seg(NULL)
    , _closed(true)
    , _exception_on_stitch(true)
{
    Point p1 = c.pointAt(0);
    Point p2 = c.pointAt(M_PI);
    _data->curves.push_back(new EllipticalArc(p1, c.radius(), c.radius(), 0, false, true, p2));
    _data->curves.push_back(new EllipticalArc(p2, c.radius(), c.radius(), 0, false, true, p1));
    _closing_seg = new ClosingSegment(p1, p1);
    _data->curves.push_back(_closing_seg);
}

Path::Path(Ellipse const &e)
    : _data(new PathData())
    , _closing_seg(NULL)
    , _closed(true)
    , _exception_on_stitch(true)
{
    Point p1 = e.pointAt(0);
    Point p2 = e.pointAt(M_PI);
    _data->curves.push_back(new EllipticalArc(p1, e.rays(), e.rotationAngle(), false, true, p2));
    _data->curves.push_back(new EllipticalArc(p2, e.rays(), e.rotationAngle(), false, true, p1));
    _closing_seg = new ClosingSegment(p1, p1);
    _data->curves.push_back(_closing_seg);
}

void Path::close(bool c)
{
    if (c == _closed) return;
    if (c && _data->curves.size() >= 2) {
        // when closing, if last segment is linear and ends at initial point,
        // replace it with the closing segment
        Sequence::iterator last = _data->curves.end() - 2;
        if (last->isLineSegment() && last->finalPoint() == initialPoint()) {
            _closing_seg->setInitial(last->initialPoint());
            _data->curves.erase(last);
        }
    }
    _closed = c;
}

void Path::clear()
{
    _unshare();
    _data->curves.pop_back().release();
    _data->curves.clear();
    _closing_seg->setInitial(Point(0, 0));
    _closing_seg->setFinal(Point(0, 0));
    _data->curves.push_back(_closing_seg);
    _closed = false;
}

OptRect Path::boundsFast() const
{
    OptRect bounds;
    if (empty()) {
        return bounds;
    }
    // if the path is not empty, we look for cached bounds
    if (_data->fast_bounds) {
        return _data->fast_bounds;
    }

    bounds = front().boundsFast();
    const_iterator iter = begin();
    // the closing path segment can be ignored, because it will always
    // lie within the bbox of the rest of the path
    if (iter != end()) {
        for (++iter; iter != end(); ++iter) {
            bounds.unionWith(iter->boundsFast());
        }
    }
    _data->fast_bounds = bounds;
    return bounds;
}

OptRect Path::boundsExact() const
{
    OptRect bounds;
    if (empty())
        return bounds;
    bounds = front().boundsExact();
    const_iterator iter = begin();
    // the closing path segment can be ignored, because it will always lie within the bbox of the rest of the path
    if (iter != end()) {
        for (++iter; iter != end(); ++iter) {
            bounds.unionWith(iter->boundsExact());
        }
    }
    return bounds;
}

Piecewise<D2<SBasis> > Path::toPwSb() const
{
    Piecewise<D2<SBasis> > ret;
    ret.push_cut(0);
    unsigned i = 1;
    bool degenerate = true;
    // pw<d2<>> is always open. so if path is closed, add closing segment as well to pwd2.
    for (const_iterator it = begin(); it != end_default(); ++it) {
        if (!it->isDegenerate()) {
            ret.push(it->toSBasis(), i++);
            degenerate = false;
        }
    }
    if (degenerate) {
        // if path only contains degenerate curves, no second cut is added
        // so we need to create at least one segment manually
        ret = Piecewise<D2<SBasis> >(initialPoint());
    }
    return ret;
}

template <typename iter>
iter inc(iter const &x, unsigned n) {
    iter ret = x;
    for (unsigned i = 0; i < n; i++)
        ret++;
    return ret;
}

bool Path::operator==(Path const &other) const
{
    if (this == &other)
        return true;
    if (_closed != other._closed)
        return false;
    return _data->curves == other._data->curves;
}

void Path::start(Point const &p) {
    if (_data->curves.size() > 1) {
        clear();
    }
    _closing_seg->setInitial(p);
    _closing_seg->setFinal(p);
}

Interval Path::timeRange() const
{
    Interval ret(0, size_default());
    return ret;
}

Curve const &Path::curveAt(Coord t, Coord *rest) const
{
    PathTime pos = _factorTime(t);
    if (rest) {
        *rest = pos.t;
    }
    return at(pos.curve_index);
}

Point Path::pointAt(Coord t) const
{
    return pointAt(_factorTime(t));
}

Coord Path::valueAt(Coord t, Dim2 d) const
{
    return valueAt(_factorTime(t), d);
}

Curve const &Path::curveAt(PathTime const &pos) const
{
    return at(pos.curve_index);
}
Point Path::pointAt(PathTime const &pos) const
{
    return at(pos.curve_index).pointAt(pos.t);
}
Coord Path::valueAt(PathTime const &pos, Dim2 d) const
{
    return at(pos.curve_index).valueAt(pos.t, d);
}

std::vector<PathTime> Path::roots(Coord v, Dim2 d) const
{
    std::vector<PathTime> res;
    for (unsigned i = 0; i <= size(); i++) {
        std::vector<Coord> temp = (*this)[i].roots(v, d);
        for (unsigned j = 0; j < temp.size(); j++)
            res.push_back(PathTime(i, temp[j]));
    }
    return res;
}


// The class below implements sweepline optimization for curve intersection in paths.
// Instead of O(N^2), this takes O(N + X), where X is the number of overlaps
// between the bounding boxes of curves.

struct CurveIntersectionSweepSet
{
public:
    struct CurveRecord {
        boost::intrusive::list_member_hook<> _hook;
        Curve const *curve;
        Rect bounds;
        std::size_t index;
        unsigned which;

        CurveRecord(Curve const *pc, std::size_t idx, unsigned w)
            : curve(pc)
            , bounds(curve->boundsFast())
            , index(idx)
            , which(w)
        {}
    };

    typedef std::vector<CurveRecord>::const_iterator ItemIterator;

    CurveIntersectionSweepSet(std::vector<PathIntersection> &result,
                              Path const &a, Path const &b, Coord precision)
        : _result(result)
        , _precision(precision)
        , _sweep_dir(X)
    {
        std::size_t asz = a.size(), bsz = b.size();
        _records.reserve(asz + bsz);

        for (std::size_t i = 0; i < asz; ++i) {
            _records.push_back(CurveRecord(&a[i], i, 0));
        }
        for (std::size_t i = 0; i < bsz; ++i) {
            _records.push_back(CurveRecord(&b[i], i, 1));
        }

        OptRect abb = a.boundsFast() | b.boundsFast();
        if (abb && abb->height() > abb->width()) {
            _sweep_dir = Y;
        }
    }

    std::vector<CurveRecord> const &items() { return _records; }
    Interval itemBounds(ItemIterator ii) {
        return ii->bounds[_sweep_dir];
    }

    void addActiveItem(ItemIterator ii) {
        unsigned w = ii->which;
        unsigned ow = (w+1) % 2;

        _active[w].push_back(const_cast<CurveRecord&>(*ii));

        for (ActiveCurveList::iterator i = _active[ow].begin(); i != _active[ow].end(); ++i) {
            if (!ii->bounds.intersects(i->bounds)) continue;
            std::vector<CurveIntersection> cx = ii->curve->intersect(*i->curve, _precision);
            for (std::size_t k = 0; k < cx.size(); ++k) {
                PathTime tw(ii->index, cx[k].first), tow(i->index, cx[k].second);
                _result.push_back(PathIntersection(
                    w == 0 ? tw : tow,
                    w == 0 ? tow : tw,
                    cx[k].point()));
            }
        }
    }
    void removeActiveItem(ItemIterator ii) {
        ActiveCurveList &acl = _active[ii->which];
        acl.erase(acl.iterator_to(*ii));
    }

private:
    typedef boost::intrusive::list
        < CurveRecord
        , boost::intrusive::member_hook
            < CurveRecord
            , boost::intrusive::list_member_hook<>
            , &CurveRecord::_hook
            >
        > ActiveCurveList;

    std::vector<CurveRecord> _records;
    std::vector<PathIntersection> &_result;
    ActiveCurveList _active[2];
    Coord _precision;
    Dim2 _sweep_dir;
};

std::vector<PathIntersection> Path::intersect(Path const &other, Coord precision) const
{
    std::vector<PathIntersection> result;

    CurveIntersectionSweepSet cisset(result, *this, other, precision);
    Sweeper<CurveIntersectionSweepSet> sweeper(cisset);
    sweeper.process();

    // preprocessing to remove duplicate intersections at endpoints
    std::size_t asz = size(), bsz = other.size();
    for (std::size_t i = 0; i < result.size(); ++i) {
        result[i].first.normalizeForward(asz);
        result[i].second.normalizeForward(bsz);
    }
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    return result;
}

int Path::winding(Point const &p) const {
    int wind = 0;

    /* To handle all the edge cases, we consider the maximum Y edge of the bounding box
     * as not included in box. This way paths that contain linear horizontal
     * segments will be treated correctly. */
    for (const_iterator i = begin(); i != end_closed(); ++i) {
        Rect bounds = i->boundsFast();

        if (bounds.height() == 0) continue;
        if (p[X] > bounds.right() || !bounds[Y].lowerContains(p[Y])) {
            // Ray doesn't intersect bbox, so we ignore this segment
            continue;
        }

        if (p[X] < bounds.left()) {
            /* Ray intersects the curve's bbox, but the point is outside it.
             * The winding contribution is exactly the same as that
             * of a linear segment with the same initial and final points. */
            Point ip = i->initialPoint();
            Point fp = i->finalPoint();
            Rect eqbox(ip, fp);

            if (eqbox[Y].lowerContains(p[Y])) {
                /* The ray intersects the equivalent linear segment.
                 * Determine winding contribution based on its derivative. */
                if (ip[Y] < fp[Y]) {
                    wind += 1;
                } else if (ip[Y] > fp[Y]) {
                    wind -= 1;
                } else {
                    // should never happen, because bounds.height() was not zero
                    assert(false);
                }
            }
        } else {
            // point is inside bbox
            wind += i->winding(p);
        }
    }
    return wind;
}

std::vector<double> Path::allNearestTimes(Point const &_point, double from, double to) const
{
    // TODO from and to are not used anywhere.
    // rewrite this to simplify.
    using std::swap;

    if (from > to)
        swap(from, to);
    const Path &_path = *this;
    unsigned int sz = _path.size();
    if (_path.closed())
        ++sz;
    if (from < 0 || to > sz) {
        THROW_RANGEERROR("[from,to] interval out of bounds");
    }
    double sif, st = modf(from, &sif);
    double eif, et = modf(to, &eif);
    unsigned int si = static_cast<unsigned int>(sif);
    unsigned int ei = static_cast<unsigned int>(eif);
    if (si == sz) {
        --si;
        st = 1;
    }
    if (ei == sz) {
        --ei;
        et = 1;
    }
    if (si == ei) {
        std::vector<double> all_nearest = _path[si].allNearestTimes(_point, st, et);
        for (unsigned int i = 0; i < all_nearest.size(); ++i) {
            all_nearest[i] = si + all_nearest[i];
        }
        return all_nearest;
    }
    std::vector<double> all_t;
    std::vector<std::vector<double> > all_np;
    all_np.push_back(_path[si].allNearestTimes(_point, st));
    std::vector<unsigned int> ni;
    ni.push_back(si);
    double dsq;
    double mindistsq = distanceSq(_point, _path[si].pointAt(all_np.front().front()));
    Rect bb(Geom::Point(0, 0), Geom::Point(0, 0));
    for (unsigned int i = si + 1; i < ei; ++i) {
        bb = (_path[i].boundsFast());
        dsq = distanceSq(_point, bb);
        if (mindistsq < dsq)
            continue;
        all_t = _path[i].allNearestTimes(_point);
        dsq = distanceSq(_point, _path[i].pointAt(all_t.front()));
        if (mindistsq > dsq) {
            all_np.clear();
            all_np.push_back(all_t);
            ni.clear();
            ni.push_back(i);
            mindistsq = dsq;
        } else if (mindistsq == dsq) {
            all_np.push_back(all_t);
            ni.push_back(i);
        }
    }
    bb = (_path[ei].boundsFast());
    dsq = distanceSq(_point, bb);
    if (mindistsq >= dsq) {
        all_t = _path[ei].allNearestTimes(_point, 0, et);
        dsq = distanceSq(_point, _path[ei].pointAt(all_t.front()));
        if (mindistsq > dsq) {
            for (unsigned int i = 0; i < all_t.size(); ++i) {
                all_t[i] = ei + all_t[i];
            }
            return all_t;
        } else if (mindistsq == dsq) {
            all_np.push_back(all_t);
            ni.push_back(ei);
        }
    }
    std::vector<double> all_nearest;
    for (unsigned int i = 0; i < all_np.size(); ++i) {
        for (unsigned int j = 0; j < all_np[i].size(); ++j) {
            all_nearest.push_back(ni[i] + all_np[i][j]);
        }
    }
    all_nearest.erase(std::unique(all_nearest.begin(), all_nearest.end()), all_nearest.end());
    return all_nearest;
}

std::vector<Coord> Path::nearestTimePerCurve(Point const &p) const
{
    // return a single nearest time for each curve in this path
    std::vector<Coord> np;
    for (const_iterator it = begin(); it != end_default(); ++it) {
        np.push_back(it->nearestTime(p));
    }
    return np;
}

PathTime Path::nearestTime(Point const &p, Coord *dist) const
{
    Coord mindist = std::numeric_limits<Coord>::max();
    PathTime ret;

    if (_data->curves.size() == 1) {
        // naked moveto
        ret.curve_index = 0;
        ret.t = 0;
        if (dist) {
            *dist = distance(_closing_seg->initialPoint(), p);
        }
        return ret;
    }

    for (size_type i = 0; i < size_default(); ++i) {
        Curve const &c = at(i);
        if (distance(p, c.boundsFast()) >= mindist) continue;

        Coord t = c.nearestTime(p);
        Coord d = distance(c.pointAt(t), p);
        if (d < mindist) {
            mindist = d;
            ret.curve_index = i;
            ret.t = t;
        }
    }
    if (dist) {
        *dist = mindist;
    }

    return ret;
}

std::vector<Point> Path::nodes() const
{
    std::vector<Point> result;
    size_type path_size = size_closed();
    for (size_type i = 0; i < path_size; ++i) {
        result.push_back(_data->curves[i].initialPoint());
    }
    return result;
}

void Path::appendPortionTo(Path &ret, double from, double to) const
{
    if (!(from >= 0 && to >= 0)) {
        THROW_RANGEERROR("from and to must be >=0 in Path::appendPortionTo");
    }
    if (to == 0)
        to = size() + 0.999999;
    if (from == to) {
        return;
    }
    double fi, ti;
    double ff = modf(from, &fi), tf = modf(to, &ti);
    if (tf == 0) {
        ti--;
        tf = 1;
    }
    const_iterator fromi = inc(begin(), (unsigned)fi);
    if (fi == ti && from < to) {
        ret.append(fromi->portion(ff, tf));
        return;
    }
    const_iterator toi = inc(begin(), (unsigned)ti);
    if (ff != 1.) {
        // fromv->setInitial(ret.finalPoint());
        ret.append(fromi->portion(ff, 1.));
    }
    if (from >= to) {
        const_iterator ender = end();
        if (ender->initialPoint() == ender->finalPoint())
            ++ender;
        ret.insert(ret.end(), ++fromi, ender);
        ret.insert(ret.end(), begin(), toi);
    } else {
        ret.insert(ret.end(), ++fromi, toi);
    }
    ret.append(toi->portion(0., tf));
}

void Path::appendPortionTo(Path &target, PathInterval const &ival,
                           boost::optional<Point> const &p_from, boost::optional<Point> const &p_to) const
{
    assert(ival.pathSize() == size_closed());

    if (ival.isDegenerate()) {
        Point stitch_to = p_from ? *p_from : pointAt(ival.from());
        target.stitchTo(stitch_to);
        return;
    }

    PathTime const &from = ival.from(), &to = ival.to();

    bool reverse = ival.reverse();
    int di = reverse ? -1 : 1;
    size_type s = size_closed();

    if (!ival.crossesStart() && from.curve_index == to.curve_index) {
        Curve *c = (*this)[from.curve_index].portion(from.t, to.t);
        if (p_from) {
            c->setInitial(*p_from);
        }
        if (p_to) {
            c->setFinal(*p_to);
        }
        target.append(c);
    } else {
        Curve *c_first = (*this)[from.curve_index].portion(from.t, reverse ? 0 : 1);
        if (p_from) {
            c_first->setInitial(*p_from);
        }
        target.append(c_first);

        for (size_type i = (from.curve_index + s + di) % s; i != to.curve_index;
             i = (i + s + di) % s)
        {
            if (reverse) {
                target.append((*this)[i].reverse());
            } else {
                target.append((*this)[i].duplicate());
            }
        }

        Curve *c_last = (*this)[to.curve_index].portion(reverse ? 1 : 0, to.t);
        if (p_to) {
            c_last->setFinal(*p_to);
        }
        target.append(c_last);
    }
}

Path Path::reversed() const
{
    typedef std::reverse_iterator<Sequence::const_iterator> RIter;

    Path ret(finalPoint());
    if (empty()) return ret;

    ret._data->curves.pop_back(); // this also deletes the closing segment from ret

    RIter iter(_includesClosingSegment() ? _data->curves.end() : _data->curves.end() - 1);
    RIter rend(_data->curves.begin());

    if (_closed) {
        // when the path is closed, there are two cases:
        if (front().isLineSegment()) {
            // 1. initial segment is linear: it becomes the new closing segment.
            rend = RIter(_data->curves.begin() + 1);
            ret._closing_seg = new ClosingSegment(front().finalPoint(), front().initialPoint());
        } else {
            // 2. initial segment is not linear: the closing segment becomes degenerate.
            // However, skip it if it's already degenerate.
            Point fp = finalPoint();
            ret._closing_seg = new ClosingSegment(fp, fp);
        }
    } else {
        // when the path is open, we reverse all real curves, and add a reversed closing segment.
        ret._closing_seg = static_cast<ClosingSegment *>(_closing_seg->reverse());
    }

    for (; iter != rend; ++iter) {
        ret._data->curves.push_back(iter->reverse());
    }
    ret._data->curves.push_back(ret._closing_seg);
    ret._closed = _closed;
    return ret;
}


void Path::insert(iterator pos, Curve const &curve)
{
    _unshare();
    Sequence::iterator seq_pos(seq_iter(pos));
    Sequence source;
    source.push_back(curve.duplicate());
    do_update(seq_pos, seq_pos, source);
}

void Path::erase(iterator pos)
{
    _unshare();
    Sequence::iterator seq_pos(seq_iter(pos));

    Sequence stitched;
    do_update(seq_pos, seq_pos + 1, stitched);
}

void Path::erase(iterator first, iterator last)
{
    _unshare();
    Sequence::iterator seq_first = seq_iter(first);
    Sequence::iterator seq_last = seq_iter(last);

    Sequence stitched;
    do_update(seq_first, seq_last, stitched);
}

void Path::stitchTo(Point const &p)
{
    if (!empty() && _closing_seg->initialPoint() != p) {
        if (_exception_on_stitch) {
            THROW_CONTINUITYERROR();
        }
        _unshare();
        do_append(new StitchSegment(_closing_seg->initialPoint(), p));
    }
}

void Path::replace(iterator replaced, Curve const &curve)
{
    replace(replaced, replaced + 1, curve);
}

void Path::replace(iterator first_replaced, iterator last_replaced, Curve const &curve)
{
    _unshare();
    Sequence::iterator seq_first_replaced(seq_iter(first_replaced));
    Sequence::iterator seq_last_replaced(seq_iter(last_replaced));
    Sequence source(1);
    source.push_back(curve.duplicate());

    do_update(seq_first_replaced, seq_last_replaced, source);
}

void Path::replace(iterator replaced, Path const &path)
{
    replace(replaced, path.begin(), path.end());
}

void Path::replace(iterator first, iterator last, Path const &path)
{
    replace(first, last, path.begin(), path.end());
}

void Path::snapEnds(Coord precision)
{
    if (!_closed) return;
    if (_data->curves.size() > 1 && are_near(_closing_seg->length(precision), 0, precision)) {
        _unshare();
        _closing_seg->setInitial(_closing_seg->finalPoint());
        (_data->curves.end() - 1)->setFinal(_closing_seg->finalPoint());
    }
}

// replace curves between first and last with contents of source,
// 
void Path::do_update(Sequence::iterator first, Sequence::iterator last, Sequence &source)
{
    // TODO: handle cases where first > last in closed paths?
    bool last_beyond_closing_segment = (last == _data->curves.end());

    // special case:
    // if do_update replaces the closing segment, we have to regenerate it
    if (source.empty()) {
        if (first == last) return; // nothing to do

        // only removing some segments
        if ((!_closed && first == _data->curves.begin()) || (!_closed && last == _data->curves.end() - 1) || last_beyond_closing_segment) {
            // just adjust the closing segment
            // do nothing
        } else if (first->initialPoint() != (last - 1)->finalPoint()) {
            if (_exception_on_stitch) {
                THROW_CONTINUITYERROR();
            }
            source.push_back(new StitchSegment(first->initialPoint(), (last - 1)->finalPoint()));
        }
    } else {
        // replacing
        if (first == _data->curves.begin() && last == _data->curves.end()) {
            // special case: replacing everything should work the same in open and closed curves
            _data->curves.erase(_data->curves.begin(), _data->curves.end() - 1);
            _closing_seg->setFinal(source.front().initialPoint());
            _closing_seg->setInitial(source.back().finalPoint());
            _data->curves.transfer(_data->curves.begin(), source.begin(), source.end(), source);
            return;
        }

        // stitch in front
        if (!_closed && first == _data->curves.begin()) {
            // not necessary to stitch in front
        } else if (first->initialPoint() != source.front().initialPoint()) {
            if (_exception_on_stitch) {
                THROW_CONTINUITYERROR();
            }
            source.insert(source.begin(), new StitchSegment(first->initialPoint(), source.front().initialPoint()));
        }

        // stitch at the end
        if ((!_closed && last == _data->curves.end() - 1) || last_beyond_closing_segment) {
            // repurpose the closing segment as the stitch segment
            // do nothing
        } else if (source.back().finalPoint() != (last - 1)->finalPoint()) {
            if (_exception_on_stitch) {
                THROW_CONTINUITYERROR();
            }
            source.push_back(new StitchSegment(source.back().finalPoint(), (last - 1)->finalPoint()));
        }
    }

    // do not erase the closing segment, adjust it instead
    if (last_beyond_closing_segment) {
        --last;
    }
    _data->curves.erase(first, last);
    _data->curves.transfer(first, source.begin(), source.end(), source);

    // adjust closing segment
    if (size_open() == 0) {
        _closing_seg->setFinal(_closing_seg->initialPoint());
    } else {
        _closing_seg->setInitial(back_open().finalPoint());
        _closing_seg->setFinal(front().initialPoint());
    }

    checkContinuity();
}

void Path::do_append(Curve *c)
{
    if (&_data->curves.front() == _closing_seg) {
        _closing_seg->setFinal(c->initialPoint());
    } else {
        // if we can't freely move the closing segment, we check whether
        // the new curve connects with the last non-closing curve
        if (c->initialPoint() != _closing_seg->initialPoint()) {
            THROW_CONTINUITYERROR();
        }
        if (_closed && c->isLineSegment() &&
            c->finalPoint() == _closing_seg->finalPoint())
        {
            // appending a curve that matches the closing segment has no effect
            delete c;
            return;
        }
    }
    _data->curves.insert(_data->curves.end() - 1, c);
    _closing_seg->setInitial(c->finalPoint());
}

void Path::checkContinuity() const
{
    Sequence::const_iterator i = _data->curves.begin(), j = _data->curves.begin();
    ++j;
    for (; j != _data->curves.end(); ++i, ++j) {
        if (i->finalPoint() != j->initialPoint()) {
            THROW_CONTINUITYERROR();
        }
    }
    if (_data->curves.front().initialPoint() != _data->curves.back().finalPoint()) {
        THROW_CONTINUITYERROR();
    }
}

// breaks time value into integral and fractional part
PathTime Path::_factorTime(Coord t) const
{
    size_type sz = size_default();
    if (t < 0 || t > sz) {
        THROW_RANGEERROR("parameter t out of bounds");
    }

    PathTime ret;
    Coord k;
    ret.t = modf(t, &k);
    ret.curve_index = k;
    if (ret.curve_index == sz) {
        --ret.curve_index;
        ret.t = 1;
    }
    return ret;
}

Piecewise<D2<SBasis> > paths_to_pw(PathVector const &paths)
{
    Piecewise<D2<SBasis> > ret = paths[0].toPwSb();
    for (unsigned i = 1; i < paths.size(); i++) {
        ret.concat(paths[i].toPwSb());
    }
    return ret;
}

bool are_near(Path const &a, Path const &b, Coord precision)
{
    if (a.size() != b.size()) return false;

    for (unsigned i = 0; i < a.size(); ++i) {
        if (!a[i].isNear(b[i], precision)) return false;
    }
    return true;
}

std::ostream &operator<<(std::ostream &out, Path const &path)
{
    SVGPathWriter pw;
    pw.feed(path);
    out << pw.str();
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

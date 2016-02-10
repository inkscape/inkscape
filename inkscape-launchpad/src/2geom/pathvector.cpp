/** @file
 * @brief PathVector - a sequence of subpaths
 *//*
 * Authors:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2008-2014 Authors
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

#include <2geom/affine.h>
#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <2geom/svg-path-writer.h>
#include <2geom/sweeper.h>

namespace Geom {

//PathVector &PathVector::operator+=(PathVector const &other);

PathVector::size_type PathVector::curveCount() const
{
    size_type n = 0;
    for (const_iterator it = begin(); it != end(); ++it) {
        n += it->size_default();
    }
    return n;
}

void PathVector::reverse(bool reverse_paths)
{
    if (reverse_paths) {
        std::reverse(begin(), end());
    }
    for (iterator i = begin(); i != end(); ++i) {
        *i = i->reversed();
    }
}

PathVector PathVector::reversed(bool reverse_paths) const
{
    PathVector ret;
    for (const_iterator i = begin(); i != end(); ++i) {
        ret.push_back(i->reversed());
    }
    if (reverse_paths) {
        std::reverse(ret.begin(), ret.end());
    }
    return ret;
}

Path &PathVector::pathAt(Coord t, Coord *rest)
{
    return const_cast<Path &>(static_cast<PathVector const*>(this)->pathAt(t, rest));
}
Path const &PathVector::pathAt(Coord t, Coord *rest) const
{
    PathVectorTime pos = _factorTime(t);
    if (rest) {
        *rest = Coord(pos.curve_index) + pos.t;
    }
    return at(pos.path_index);
}
Curve const &PathVector::curveAt(Coord t, Coord *rest) const
{
    PathVectorTime pos = _factorTime(t);
    if (rest) {
        *rest = pos.t;
    }
    return at(pos.path_index).at(pos.curve_index);
}
Coord PathVector::valueAt(Coord t, Dim2 d) const
{
    PathVectorTime pos = _factorTime(t);
    return at(pos.path_index).at(pos.curve_index).valueAt(pos.t, d);
}
Point PathVector::pointAt(Coord t) const
{
    PathVectorTime pos = _factorTime(t);
    return at(pos.path_index).at(pos.curve_index).pointAt(pos.t);
}

OptRect PathVector::boundsFast() const
{
    OptRect bound;
    if (empty()) return bound;

    bound = front().boundsFast();
    for (const_iterator it = ++begin(); it != end(); ++it) {
        bound.unionWith(it->boundsFast());
    }
    return bound;
}

OptRect PathVector::boundsExact() const
{
    OptRect bound;
    if (empty()) return bound;

    bound = front().boundsExact();
    for (const_iterator it = ++begin(); it != end(); ++it) {
        bound.unionWith(it->boundsExact());
    }
    return bound;
}

void PathVector::snapEnds(Coord precision)
{
    for (std::size_t i = 0; i < size(); ++i) {
        (*this)[i].snapEnds(precision);
    }
}

// sweepline optimization
// this is very similar to CurveIntersectionSweepSet in path.cpp
// should probably be merged
class PathIntersectionSweepSet {
public:
    struct PathRecord {
        boost::intrusive::list_member_hook<> _hook;
        Path const *path;
        std::size_t index;
        unsigned which;

        PathRecord(Path const &p, std::size_t i, unsigned w)
            : path(&p)
            , index(i)
            , which(w)
        {}
    };

    typedef std::vector<PathRecord>::iterator ItemIterator;

    PathIntersectionSweepSet(std::vector<PVIntersection> &result,
                             PathVector const &a, PathVector const &b, Coord precision)
        : _result(result)
        , _precision(precision)
    {
        _records.reserve(a.size() + b.size());
        for (std::size_t i = 0; i < a.size(); ++i) {
            _records.push_back(PathRecord(a[i], i, 0));
        }
        for (std::size_t i = 0; i < b.size(); ++i) {
            _records.push_back(PathRecord(b[i], i, 1));
        }
    }

    std::vector<PathRecord> &items() { return _records; }

    Interval itemBounds(ItemIterator ii) {
        OptRect r = ii->path->boundsFast();
        return (*r)[X];
    }

    void addActiveItem(ItemIterator ii) {
        unsigned w = ii->which;
        unsigned ow = (ii->which + 1) % 2;

        for (ActivePathList::iterator i = _active[ow].begin(); i != _active[ow].end(); ++i) {
            if (!ii->path->boundsFast().intersects(i->path->boundsFast())) continue;
            std::vector<PathIntersection> px = ii->path->intersect(*i->path, _precision);
            for (std::size_t k = 0; k < px.size(); ++k) {
                PathVectorTime tw(ii->index, px[k].first), tow(i->index, px[k].second);
                _result.push_back(PVIntersection(
                    w == 0 ? tw : tow,
                    w == 0 ? tow : tw,
                    px[k].point()));
            }
        }
        _active[w].push_back(*ii);
    }

    void removeActiveItem(ItemIterator ii) {
        ActivePathList &apl = _active[ii->which];
        apl.erase(apl.iterator_to(*ii));
    }

private:
    typedef boost::intrusive::list
        < PathRecord
        , boost::intrusive::member_hook
            < PathRecord
            , boost::intrusive::list_member_hook<>
            , &PathRecord::_hook
            >
        > ActivePathList;

    std::vector<PVIntersection> &_result;
    std::vector<PathRecord> _records;
    ActivePathList _active[2];
    Coord _precision;
};

std::vector<PVIntersection> PathVector::intersect(PathVector const &other, Coord precision) const
{
    std::vector<PVIntersection> result;

    PathIntersectionSweepSet pisset(result, *this, other, precision);
    Sweeper<PathIntersectionSweepSet> sweeper(pisset);
    sweeper.process();

    std::sort(result.begin(), result.end());

    return result;
}

int PathVector::winding(Point const &p) const
{
    int wind = 0;
    for (const_iterator i = begin(); i != end(); ++i) {
        if (!i->boundsFast().contains(p)) continue;
        wind += i->winding(p);
    }
    return wind;
}

boost::optional<PathVectorTime> PathVector::nearestTime(Point const &p, Coord *dist) const
{
    boost::optional<PathVectorTime> retval;

    Coord mindist = infinity();
    for (size_type i = 0; i < size(); ++i) {
        Coord d;
        PathTime pos = (*this)[i].nearestTime(p, &d);
        if (d < mindist) {
            mindist = d;
            retval = PathVectorTime(i, pos.curve_index, pos.t);
        }
    }

    if (dist) {
        *dist = mindist;
    }
    return retval;
}

std::vector<PathVectorTime> PathVector::allNearestTimes(Point const &p, Coord *dist) const
{
    std::vector<PathVectorTime> retval;

    Coord mindist = infinity();
    for (size_type i = 0; i < size(); ++i) {
        Coord d;
        PathTime pos = (*this)[i].nearestTime(p, &d);
        if (d < mindist) {
            mindist = d;
            retval.clear();
        }
        if (d <= mindist) {
            retval.push_back(PathVectorTime(i, pos.curve_index, pos.t));
        }
    }

    if (dist) {
        *dist = mindist;
    }
    return retval;
}

std::vector<Point> PathVector::nodes() const
{
    std::vector<Point> result;
    for (size_type i = 0; i < size(); ++i) {
        size_type path_size = (*this)[i].size_closed();
        for (size_type j = 0; j < path_size; ++j) {
            result.push_back((*this)[i][j].initialPoint());
        }
    }
    return result;
}

PathVectorTime PathVector::_factorTime(Coord t) const
{
    PathVectorTime ret;
    Coord rest = 0;
    ret.t = modf(t, &rest);
    ret.curve_index = rest;
    for (; ret.path_index < size(); ++ret.path_index) {
        unsigned s = _data.at(ret.path_index).size_default();
        if (s > ret.curve_index) break;
        // special case for the last point
        if (s == ret.curve_index && ret.path_index + 1 == size()) {
            --ret.curve_index;
            ret.t = 1;
            break;
        }
        ret.curve_index -= s;
    }
    return ret;
}

std::ostream &operator<<(std::ostream &out, PathVector const &pv)
{
    SVGPathWriter wr;
    wr.feed(pv);
    out << wr.str();
    return out;
}

} // namespace Geom

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

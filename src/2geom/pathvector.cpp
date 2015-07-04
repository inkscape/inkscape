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

std::vector<PVIntersection> PathVector::intersect(PathVector const &other, Coord precision) const
{
    typedef PathVectorTime PVPos;
    std::vector<PVIntersection> result;
    for (std::size_t i = 0; i < size(); ++i) {
        for (std::size_t j = 0; j < other.size(); ++j) {
            std::vector<PathIntersection> xs = (*this)[i].intersect(other[j], precision);
            for (std::size_t k = 0; k < xs.size(); ++k) {
                PVIntersection pvx(PVPos(i, xs[k].first), PVPos(j, xs[k].second), xs[k].point());
                result.push_back(pvx);
            }
        }
    }
    return result;
}

int PathVector::winding(Point const &p) const
{
    int wind = 0;
    for (const_iterator i = begin(); i != end(); ++i) {
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

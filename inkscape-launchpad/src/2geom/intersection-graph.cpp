/**
 * \file
 * \brief Intersection graph for Boolean operations
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2015 Authors
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

#include <2geom/intersection-graph.h>
#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <iostream>
#include <iterator>

namespace Geom {

struct PathIntersectionGraph::IntersectionVertexLess {
    bool operator()(IntersectionVertex const &a, IntersectionVertex const &b) const {
        return a.pos < b.pos;
    }
};

/** @class PathIntersectionGraph
 * @brief Intermediate data for computing Boolean operations on paths.
 *
 * This class implements the Greiner-Hormann clipping algorithm,
 * with improvements inspired by Foster and Overfelt as well as some
 * original contributions.
 *
 * @ingroup Paths
 */

PathIntersectionGraph::PathIntersectionGraph(PathVector const &a, PathVector const &b, Coord precision)
    : _a(a)
    , _b(b)
{
    if (a.empty() || b.empty()) return;

    // all paths must be closed, otherwise we will miss some intersections
    for (std::size_t i = 0; i < a.size(); ++i) {
        _a[i].close();
    }
    for (std::size_t i = 0; i < b.size(); ++i) {
        _b[i].close();
    }

    std::vector<PVIntersection> pxs = _a.intersect(_b, precision);
    // NOTE: this early return means that the path data structures will not be created
    // if there are no intersections at all!
    if (pxs.empty()) return;

    // prepare intersection lists for each path component
    for (std::size_t i = 0; i < _a.size(); ++i) {
        _apaths.push_back(new PathData());
    }
    for (std::size_t i = 0; i < _b.size(); ++i) {
        _bpaths.push_back(new PathData());
    }

    for (std::size_t i = 0; i < pxs.size(); ++i) {
        IntersectionVertex *xa, *xb;
        xa = new IntersectionVertex();
        xb = new IntersectionVertex();
        xa->processed = xb->processed = false;
        xa->pos = pxs[i].first;
        xb->pos = pxs[i].second;
        xa->p = xb->p = pxs[i].point();
        xa->neighbor = xb;
        xb->neighbor = xa;
        _xs.push_back(xa);
        _xs.push_back(xb);
        _apaths[xa->pos.path_index].xlist.push_back(*xa);
        _bpaths[xb->pos.path_index].xlist.push_back(*xb);
    }

    for (std::size_t i = 0; i < _apaths.size(); ++i) {
        _apaths[i].xlist.sort(IntersectionVertexLess());
    }
    for (std::size_t i = 0; i < _bpaths.size(); ++i) {
        _bpaths[i].xlist.sort(IntersectionVertexLess());
    }

    typedef IntersectionList::iterator Iter;

    // determine in/out/on flags using winding
    for (unsigned npv = 0; npv < 2; ++npv) {
        boost::ptr_vector<PathData> &ls = npv ? _bpaths : _apaths;
        boost::ptr_vector<PathData> &ols = npv ? _apaths : _bpaths;
        PathVector const &pv = npv ? b : a;
        PathVector const &other = npv ? a : b;

        for (unsigned li = 0; li < ls.size(); ++li) {
            IntersectionList &xl = ls[li].xlist;
            for (Iter i = xl.begin(); i != xl.end(); ++i) {
                Iter n = boost::next(i);
                if (n == xl.end()) {
                    n = xl.begin();
                }
                std::size_t pi = i->pos.path_index;

                PathInterval ival = forward_interval(i->pos, n->pos, pv[pi].size());
                PathTime mid = ival.inside(precision);

                // TODO check for degenerate cases
                int w = other.winding(pv[pi].pointAt(mid));
                if (w % 2) {
                    i->next = POINT_INSIDE;
                    n->previous = POINT_INSIDE;
                } else {
                    i->next = POINT_OUTSIDE;
                    n->previous = POINT_OUTSIDE;
                }
            }

            // remove intersections that don't change between in/out
            // and assign exit / entry flags
            for (Iter i = xl.begin(); i != xl.end();) {
                if (i->previous == i->next) {
                    IntersectionList &oxl = ols[i->neighbor->pos.path_index].xlist;
                    oxl.erase(oxl.iterator_to(*i->neighbor));
                    xl.erase(i++);
                    if (i->next == POINT_INSIDE) {
                        ++ls[li].removed_in;
                    } else {
                        ++ls[li].removed_out;
                    }
                } else {
                    i->entry = ((i->next == POINT_INSIDE) && (i->previous == POINT_OUTSIDE));
                    ++i;
                }
            }
        }
    }
}

PathVector PathIntersectionGraph::getUnion()
{
    PathVector result = _getResult(false, false);
    _handleNonintersectingPaths(result, 0, false);
    _handleNonintersectingPaths(result, 1, false);
    return result;
}

PathVector PathIntersectionGraph::getIntersection()
{
    PathVector result = _getResult(true, true);
    _handleNonintersectingPaths(result, 0, true);
    _handleNonintersectingPaths(result, 1, true);
    return result;
}

PathVector PathIntersectionGraph::getAminusB()
{
    PathVector result = _getResult(false, true);
    _handleNonintersectingPaths(result, 0, false);
    _handleNonintersectingPaths(result, 1, true);
    return result;
}

PathVector PathIntersectionGraph::getBminusA()
{
    PathVector result = _getResult(true, false);
    _handleNonintersectingPaths(result, 1, false);
    _handleNonintersectingPaths(result, 0, true);
    return result;
}

PathVector PathIntersectionGraph::getXOR()
{
    PathVector r1 = getAminusB();
    PathVector r2 = getBminusA();
    std::copy(r2.begin(), r2.end(), std::back_inserter(r1));
    return r1;
}

std::size_t PathIntersectionGraph::size() const
{
    std::size_t result = 0;
    for (std::size_t i = 0; i < _apaths.size(); ++i) {
        result += _apaths[i].xlist.size();
    }
    return result;
}

std::vector<Point> PathIntersectionGraph::intersectionPoints() const
{
    std::vector<Point> result;

    typedef IntersectionList::const_iterator Iter;
    for (std::size_t i = 0; i < _apaths.size(); ++i) {
        for (Iter j = _apaths[i].xlist.begin(); j != _apaths[i].xlist.end(); ++j) {
            result.push_back(j->p);
        }
    }
    return result;
}

PathVector PathIntersectionGraph::_getResult(bool enter_a, bool enter_b)
{
    typedef IntersectionList::iterator Iter;
    PathVector result;
    if (_xs.empty()) return result;

    // reset processed status
    for (unsigned npv = 0; npv < 2; ++npv) {
        boost::ptr_vector<PathData> &ls = npv ? _bpaths : _apaths;
        for (std::size_t li = 0; li < ls.size(); ++li) {
            for (Iter k = ls[li].xlist.begin(); k != ls[li].xlist.end(); ++k) {
                k->processed = false;
            }
        }
    }

    unsigned n_processed = 0;

    while (true) {
        PathVector const *cur = &_a, *other = &_b;
        boost::ptr_vector<PathData> *lscur = &_apaths, *lsother = &_bpaths;

        // find unprocessed intersection
        Iter i;
        if (!_findUnprocessed(i)) break;

        result.push_back(Path(i->p));
        result.back().setStitching(true);

        while (!i->processed) {
            Iter prev = i;
            std::size_t pi = i->pos.path_index;
            // determine which direction to go
            // union: always go outside
            // intersection: always go inside
            // a minus b: go inside in b, outside in a
            // b minus a: go inside in a, outside in b
            bool reverse = false;
            if (cur == &_a) {
                reverse = i->entry ^ enter_a;
            } else {
                reverse = i->entry ^ enter_b;
            }

            // get next intersection
            if (reverse) {
                if (i == (*lscur)[pi].xlist.begin()) {
                    i = (*lscur)[pi].xlist.end();
                }
                --i;
            } else {
                ++i;
                if (i == (*lscur)[pi].xlist.end()) {
                    i = (*lscur)[pi].xlist.begin();
                }
            }

            // append portion of path
            PathInterval ival = PathInterval::from_direction(
                prev->pos.asPathTime(), i->pos.asPathTime(),
                reverse, (*cur)[pi].size());

            (*cur)[pi].appendPortionTo(result.back(), ival, prev->p, i->p);

            // mark both vertices as processed
            prev->processed = true;
            i->processed = true;
            n_processed += 2;

            // switch to the other path
            i = (*lsother)[i->neighbor->pos.path_index].xlist.iterator_to(*i->neighbor);
            std::swap(lscur, lsother);
            std::swap(cur, other);
        }
        result.back().close(true);

        assert(!result.back().empty());
    }

    assert(n_processed == size() * 2);

    return result;
}

void PathIntersectionGraph::_handleNonintersectingPaths(PathVector &result, int which, bool inside)
{
    /* Every component that has any intersections will be processed by _getResult.
     * Here we take care of paths that don't have any intersections. They are either
     * completely inside or completely outside the other pathvector. We test this by
     * evaluating the winding rule at the initial point. If inside is true and
     * the path is inside, we add it to the result.
     */
    boost::ptr_vector<PathData> const &ls = which ? _bpaths : _apaths;
    PathVector const &cur = which ? _b : _a;
    PathVector const &other = which ? _a : _b;

    for (std::size_t i = 0; i < cur.size(); ++i) {
        // the path data vector might have been left empty if there were no intersections at all
        bool has_path_data = !ls.empty();
        // Skip if the path has intersections
        if (has_path_data && !ls[i].xlist.empty()) continue;
        bool path_inside = false;

        // If the path had any intersections removed, use the result of that,
        // since one of those might have been at the initial point.
        // Also, it saves time.
        if (has_path_data && ls[i].removed_in != 0) {
            path_inside = true;
        } else if (has_path_data && ls[i].removed_out != 0) {
            path_inside = false;
        } else {
            int w = other.winding(cur[i].initialPoint());
            path_inside = w % 2 != 0;
        }

        if (path_inside == inside) {
            result.push_back(cur[i]);
        }
    }
}

bool PathIntersectionGraph::_findUnprocessed(IntersectionList::iterator &result)
{
    typedef IntersectionList::iterator Iter;

    Iter it, last;

    for (std::size_t k = 0; k < _apaths.size(); ++k) {
        it = _apaths[k].xlist.begin();
        last = _apaths[k].xlist.end();
        for (; it != last; ++it) {
            if (!it->_hook.is_linked()) {
                // this intersection was removed since it did not change inside/outside status
                continue;
            }
            if (!it->processed) {
                result = it;
                return true;
            }
        }
    }

    return false;
}


std::ostream &operator<<(std::ostream &os, PathIntersectionGraph const &pig)
{
    os << "Intersection graph:\n"
       << pig._xs.size()/2 << " total intersections\n"
       << pig.size() << " considered intersections\n";
    for (std::size_t i = 0; i < pig._apaths.size(); ++i) {
        typedef PathIntersectionGraph::IntersectionList::const_iterator Iter;
        for (Iter j = pig._apaths[i].xlist.begin(); j != pig._apaths[i].xlist.end(); ++j) {
            os << j->pos << " - " << j->neighbor->pos << " @ " << j->p << "\n";
        }
    }
    return os;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

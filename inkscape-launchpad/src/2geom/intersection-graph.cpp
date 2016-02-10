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
#include <2geom/utils.h>
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
    : _graph_valid(true)
{
    if (a.empty() || b.empty()) return;

    _pv[0] = a;
    _pv[1] = b;

    _prepareArguments();
    bool has_intersections = _prepareIntersectionLists(precision);
    if (!has_intersections) return;

    _assignEdgeWindingParities(precision);
    _assignComponentStatusFromDegenerateIntersections();
    _removeDegenerateIntersections();
    if (_graph_valid) {
        _verify();
    }
}

void PathIntersectionGraph::_prepareArguments()
{
    // all paths must be closed, otherwise we will miss some intersections
    for (int w = 0; w < 2; ++w) {
        for (std::size_t i = 0; i < _pv[w].size(); ++i) {
            _pv[w][i].close();
        }
    }
    // remove degenerate segments
    for (int w = 0; w < 2; ++w) {
        for (std::size_t i = _pv[w].size(); i > 0; --i) {
            if (_pv[w][i-1].empty()) {
                _pv[w].erase(_pv[w].begin() + (i-1));
            }
            for (std::size_t j = _pv[w][i-1].size(); j > 0; --j) {
                if (_pv[w][i-1][j-1].isDegenerate()) {
                    _pv[w][i-1].erase(_pv[w][i-1].begin() + (j-1));
                }
            }
        }
    }
}

bool PathIntersectionGraph::_prepareIntersectionLists(Coord precision)
{
    std::vector<PVIntersection> pxs = _pv[0].intersect(_pv[1], precision);
    // NOTE: this early return means that the path data structures will not be created
    // if there are no intersections at all!
    if (pxs.empty()) return false;

    // prepare intersection lists for each path component
    for (unsigned w = 0; w < 2; ++w) {
        for (std::size_t i = 0; i < _pv[w].size(); ++i) {
            _components[w].push_back(new PathData(w, i));
        }
    }

    // create intersection vertices
    for (std::size_t i = 0; i < pxs.size(); ++i) {
        IntersectionVertex *xa, *xb;
        xa = new IntersectionVertex();
        xb = new IntersectionVertex();
        //xa->processed = xb->processed = false;
        xa->which = 0; xb->which = 1;
        xa->pos = pxs[i].first;
        xb->pos = pxs[i].second;
        xa->p = xb->p = pxs[i].point();
        xa->neighbor = xb;
        xb->neighbor = xa;
        xa->next_edge = xb->next_edge = OUTSIDE;
        xa->defective = xb->defective = false;
        _xs.push_back(xa);
        _xs.push_back(xb);
        _components[0][xa->pos.path_index].xlist.push_back(*xa);
        _components[1][xb->pos.path_index].xlist.push_back(*xb);
    }

    // sort components according to time value of intersections
    for (unsigned w = 0; w < 2; ++w) {
        for (std::size_t i = 0; i < _components[w].size(); ++i) {
            _components[w][i].xlist.sort(IntersectionVertexLess());
        }
    }

    return true;
}

void PathIntersectionGraph::_assignEdgeWindingParities(Coord precision)
{
    // determine the winding numbers of path portions between intersections
    for (unsigned w = 0; w < 2; ++w) {
        unsigned ow = (w+1) % 2;

        for (unsigned li = 0; li < _components[w].size(); ++li) {
            IntersectionList &xl = _components[w][li].xlist;
            for (ILIter i = xl.begin(); i != xl.end(); ++i) {
                ILIter n = cyclic_next(i, xl);
                std::size_t pi = i->pos.path_index;

                PathInterval ival = forward_interval(i->pos, n->pos, _pv[w][pi].size());
                PathTime mid = ival.inside(precision);

                Point wpoint = _pv[w][pi].pointAt(mid);
                _winding_points.push_back(wpoint);
                int wdg = _pv[ow].winding(wpoint);
                if (wdg % 2) {
                    i->next_edge = INSIDE;
                } else {
                    i->next_edge = OUTSIDE;
                }
            }
        }
    }
}

void PathIntersectionGraph::_assignComponentStatusFromDegenerateIntersections()
{
    // If a path has only degenerate intersections, assign its status now.
    // This protects against later accidentaly picking a point for winding
    // determination that is exactly at a removed intersection.
    for (unsigned w = 0; w < 2; ++w) {
        for (unsigned li = 0; li < _components[w].size(); ++li) {
            IntersectionList &xl = _components[w][li].xlist;
            bool has_in = false;
            bool has_out = false;
            for (ILIter i = xl.begin(); i != xl.end(); ++i) {
                has_in |= (i->next_edge == INSIDE);
                has_out |= (i->next_edge == OUTSIDE);
            }
            if (has_in && !has_out) {
                _components[w][li].status = INSIDE;
            }
            if (!has_in && has_out) {
                _components[w][li].status = OUTSIDE;
            }
        }
    }
}

void PathIntersectionGraph::_removeDegenerateIntersections()
{
    // remove intersections that don't change between in/out
    for (unsigned w = 0; w < 2; ++w) {
        for (unsigned li = 0; li < _components[w].size(); ++li) {
            IntersectionList &xl = _components[w][li].xlist;
            for (ILIter i = xl.begin(); i != xl.end();) {
                ILIter n = cyclic_next(i, xl);
                if (i->next_edge == n->next_edge) {
                    bool last_node = (i == n);
                    ILIter nn = _getNeighbor(n);
                    IntersectionList &oxl = _getPathData(nn).xlist;

                    // When exactly 3 out of 4 edges adjacent to an intersection
                    // have the same winding, we have a defective intersection,
                    // which is neither degenerate nor normal. Those can occur in paths
                    // that contain overlapping segments. We cannot handle that case
                    // for now, so throw an exception.
                    if (cyclic_prior(nn, oxl)->next_edge != nn->next_edge) {
                        _graph_valid = false;
                        n->defective = true;
                        nn->defective = true;
                        ++i;
                        continue;
                    }

                    oxl.erase(nn);
                    xl.erase(n);
                    if (last_node) break;
                } else {
                    ++i;
                }
            }
        }
    }
}

void PathIntersectionGraph::_verify()
{
    for (unsigned w = 0; w < 2; ++w) {
        for (unsigned li = 0; li < _components[w].size(); ++li) {
            IntersectionList &xl = _components[w][li].xlist;
            assert(xl.size() % 2 == 0);
            for (ILIter i = xl.begin(); i != xl.end(); ++i) {
                ILIter j = cyclic_next(i, xl);
                assert(i->next_edge != j->next_edge);
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
    PathVector r1, r2;
    r1 = getAminusB();
    r2 = getBminusA();
    std::copy(r2.begin(), r2.end(), std::back_inserter(r1));
    return r1;
}

std::size_t PathIntersectionGraph::size() const
{
    std::size_t result = 0;
    for (std::size_t i = 0; i < _components[0].size(); ++i) {
        result += _components[0][i].xlist.size();
    }
    return result;
}

std::vector<Point> PathIntersectionGraph::intersectionPoints(bool defective) const
{
    std::vector<Point> result;

    typedef IntersectionList::const_iterator CILIter;
    for (std::size_t i = 0; i < _components[0].size(); ++i) {
        for (CILIter j = _components[0][i].xlist.begin(); j != _components[0][i].xlist.end(); ++j) {
            if (j->defective == defective) {
                result.push_back(j->p);
            }
        }
    }
    return result;
}

void PathIntersectionGraph::fragments(PathVector &in, PathVector &out) const
{
    typedef boost::ptr_vector<PathData>::const_iterator PIter;
    for (unsigned w = 0; w < 2; ++w) {
        for (PIter li = _components[w].begin(); li != _components[w].end(); ++li) {
            for (CILIter k = li->xlist.begin(); k != li->xlist.end(); ++k) {
                CILIter n = cyclic_next(k, li->xlist);
                // TODO: investigate why non-contiguous paths are sometimes generated here
                Path frag(k->p);
                frag.setStitching(true);
                PathInterval ival = forward_interval(k->pos, n->pos, _pv[w][k->pos.path_index].size());
                _pv[w][k->pos.path_index].appendPortionTo(frag, ival, k->p, n->p);
                if (k->next_edge == INSIDE) {
                    in.push_back(frag);
                } else {
                    out.push_back(frag);
                }
            }
        }
    }
}

PathVector PathIntersectionGraph::_getResult(bool enter_a, bool enter_b)
{
    typedef boost::ptr_vector<PathData>::iterator PIter;
    PathVector result;
    if (_xs.empty()) return result;

    // reset processed status
    _ulist.clear();
    for (unsigned w = 0; w < 2; ++w) {
        for (PIter li = _components[w].begin(); li != _components[w].end(); ++li) {
            for (ILIter k = li->xlist.begin(); k != li->xlist.end(); ++k) {
                _ulist.push_back(*k);
            }
        }
    }

    unsigned n_processed = 0;

    while (true) {
        // get unprocessed intersection
        if (_ulist.empty()) break;
        IntersectionVertex &iv = _ulist.front();
        unsigned w = iv.which;
        ILIter i = _components[w][iv.pos.path_index].xlist.iterator_to(iv);

        result.push_back(Path(i->p));
        result.back().setStitching(true);

        while (i->_proc_hook.is_linked()) {
            ILIter prev = i;
            std::size_t pi = i->pos.path_index;
            // determine which direction to go
            // union: always go outside
            // intersection: always go inside
            // a minus b: go inside in b, outside in a
            // b minus a: go inside in a, outside in b
            bool reverse = false;
            if (w == 0) {
                reverse = (i->next_edge == INSIDE) ^ enter_a;
            } else {
                reverse = (i->next_edge == INSIDE) ^ enter_b;
            }

            // get next intersection
            if (reverse) {
                i = cyclic_prior(i, _components[w][pi].xlist);
            } else {
                i = cyclic_next(i, _components[w][pi].xlist);
            }

            // append portion of path
            PathInterval ival = PathInterval::from_direction(
                prev->pos.asPathTime(), i->pos.asPathTime(),
                reverse, _pv[i->which][pi].size());

            _pv[i->which][pi].appendPortionTo(result.back(), ival, prev->p, i->p);

            // mark both vertices as processed
            //prev->processed = true;
            //i->processed = true;
            n_processed += 2;
            if (prev->_proc_hook.is_linked()) {
                _ulist.erase(_ulist.iterator_to(*prev));
            }
            if (i->_proc_hook.is_linked()) {
                _ulist.erase(_ulist.iterator_to(*i));
            }

            // switch to the other path
            i = _getNeighbor(i);
            w = i->which;
        }
        result.back().close(true);

        assert(!result.back().empty());
    }

    /*if (n_processed != size() * 2) {
        std::cerr << "Processed " << n_processed << " intersections, expected " << (size() * 2) << std::endl;
    }*/
    assert(n_processed == size() * 2);

    return result;
}

void PathIntersectionGraph::_handleNonintersectingPaths(PathVector &result, unsigned which, bool inside)
{
    /* Every component that has any intersections will be processed by _getResult.
     * Here we take care of paths that don't have any intersections. They are either
     * completely inside or completely outside the other pathvector. We test this by
     * evaluating the winding rule at the initial point. If inside is true and
     * the path is inside, we add it to the result.
     */
    unsigned w = which;
    unsigned ow = (w+1) % 2;

    for (std::size_t i = 0; i < _pv[w].size(); ++i) {
        // the path data vector might have been left empty if there were no intersections at all
        bool has_path_data = !_components[w].empty();
        // Skip if the path has intersections
        if (has_path_data && !_components[w][i].xlist.empty()) continue;
        bool path_inside = false;

        // Use the in/out determination from constructor, if available
        if (has_path_data && _components[w][i].status == INSIDE) {
            path_inside = true;
        } else if (has_path_data && _components[w][i].status == OUTSIDE) {
            path_inside = false;
        } else {
            int wdg = _pv[ow].winding(_pv[w][i].initialPoint());
            path_inside = wdg % 2 != 0;
        }

        if (path_inside == inside) {
            result.push_back(_pv[w][i]);
        }
    }
}

PathIntersectionGraph::ILIter PathIntersectionGraph::_getNeighbor(ILIter iter)
{
    unsigned ow = (iter->which + 1) % 2;
    return _components[ow][iter->neighbor->pos.path_index].xlist.iterator_to(*iter->neighbor);
}

PathIntersectionGraph::PathData &
PathIntersectionGraph::_getPathData(ILIter iter)
{
    return _components[iter->which][iter->pos.path_index];
}

std::ostream &operator<<(std::ostream &os, PathIntersectionGraph const &pig)
{
    typedef PathIntersectionGraph::IntersectionList::const_iterator CILIter;
    os << "Intersection graph:\n"
       << pig._xs.size()/2 << " total intersections\n"
       << pig.size() << " considered intersections\n";
    for (std::size_t i = 0; i < pig._components[0].size(); ++i) {
        PathIntersectionGraph::IntersectionList const &xl = pig._components[0][i].xlist;
        for (CILIter j = xl.begin(); j != xl.end(); ++j) {
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

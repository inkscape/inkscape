/**
 * \file
 * \brief Path intersection graph
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

#ifndef SEEN_LIB2GEOM_INTERSECTION_GRAPH_H
#define SEEN_LIB2GEOM_INTERSECTION_GRAPH_H

#include <set>
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/intrusive/list.hpp>
#include <2geom/forward.h>
#include <2geom/pathvector.h>

namespace Geom {

class PathIntersectionGraph
{
    // this is called PathIntersectionGraph so that we can also have a class for polygons,
    // e.g. PolygonIntersectionGraph, which is going to be significantly faster
public:
    PathIntersectionGraph(PathVector const &a, PathVector const &b, Coord precision = EPSILON);

    PathVector getUnion();
    PathVector getIntersection();
    PathVector getAminusB();
    PathVector getBminusA();
    PathVector getXOR();

    /// Returns the number of intersections used when computing Boolean operations.
    std::size_t size() const;
    std::vector<Point> intersectionPoints(bool defective = false) const;
    std::vector<Point> windingPoints() const {
        return _winding_points;
    }
    void fragments(PathVector &in, PathVector &out) const;
    bool valid() const { return _graph_valid; }

private:
    enum InOutFlag {
        INSIDE,
        OUTSIDE,
        BOTH
    };

    struct IntersectionVertex {
        boost::intrusive::list_member_hook<> _hook;
        boost::intrusive::list_member_hook<> _proc_hook;
        PathVectorTime pos;
        Point p; // guarantees that endpoints are exact
        IntersectionVertex *neighbor;
        InOutFlag next_edge;
        unsigned which;
        bool defective;
    };

    typedef boost::intrusive::list
        < IntersectionVertex
        , boost::intrusive::member_hook
            < IntersectionVertex
            , boost::intrusive::list_member_hook<>
            , &IntersectionVertex::_hook
            >
        > IntersectionList;

    typedef boost::intrusive::list
        < IntersectionVertex
        , boost::intrusive::member_hook
            < IntersectionVertex
            , boost::intrusive::list_member_hook<>
            , &IntersectionVertex::_proc_hook
            >
        > UnprocessedList;

    struct PathData {
        IntersectionList xlist;
        std::size_t path_index;
        int which;
        InOutFlag status;

        PathData(int w, std::size_t pi)
            : path_index(pi)
            , which(w)
            , status(BOTH)
        {}
    };

    struct IntersectionVertexLess;
    typedef IntersectionList::iterator ILIter;
    typedef IntersectionList::const_iterator CILIter;

    PathVector _getResult(bool enter_a, bool enter_b);
    void _handleNonintersectingPaths(PathVector &result, unsigned which, bool inside);
    void _prepareArguments();
    bool _prepareIntersectionLists(Coord precision);
    void _assignEdgeWindingParities(Coord precision);
    void _assignComponentStatusFromDegenerateIntersections();
    void _removeDegenerateIntersections();
    void _verify();

    ILIter _getNeighbor(ILIter iter);
    PathData &_getPathData(ILIter iter);

    PathVector _pv[2];
    boost::ptr_vector<IntersectionVertex> _xs;
    boost::ptr_vector<PathData> _components[2];
    UnprocessedList _ulist;
    bool _graph_valid;
    std::vector<Point> _winding_points;

    friend std::ostream &operator<<(std::ostream &, PathIntersectionGraph const &);
};

std::ostream &operator<<(std::ostream &os, PathIntersectionGraph const &pig);

} // namespace Geom

#endif // SEEN_LIB2GEOM_PATH_GRAPH_H
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

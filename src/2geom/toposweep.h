/**
 * \file
 * \brief  TopoSweep - topology / graph representation of a PathVector, for boolean operations and related tasks
 *//*
 * Authors:
 * 		Michael Sloan <mgsloan at gmail.com>
 * 		Nathan Hurst <njhurst at njhurst.com>
 * 
 * Copyright 2007-2009  authors
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

#ifndef LIB2GEOM_SEEN_TOPOSWEEP_H
#define LIB2GEOM_SEEN_TOPOSWEEP_H

#include <2geom/coord.h>
#include <2geom/point.h>
#include <2geom/pathvector.h>
#include <2geom/rect.h>
#include <2geom/path.h>
#include <2geom/curve.h>

#include <boost/shared_ptr.hpp>

namespace Geom {

// indicates a particular curve in a pathvector
struct CurveIx {
    unsigned path, ix;
    CurveIx(unsigned p, unsigned i) : path(p), ix(i) {}
    // retrieves the indicated curve from the pathvector
    Curve const &get(PathVector const &ps) const {
        return ps[path][ix];
    }
    bool operator==(CurveIx const &other) const {
        return other.path == path && other.ix == ix;
    }
};

// represents a monotonic section of a path
struct Section {
    CurveIx curve;
    double f, t;
    Point fp, tp;
    std::vector<int> windings;
    Section(CurveIx cix, double fd, double td, Point fdp, Point tdp) : curve(cix), f(fd), t(td), fp(fdp), tp(tdp) { }
    Section(CurveIx cix, double fd, double td, PathVector ps, Dim2 d) : curve(cix), f(fd), t(td) {
        fp = curve.get(ps).pointAt(f), tp = curve.get(ps).pointAt(t);
        if (Point::LexLessRt(d)(tp, fp)) {
            //swap from and to, since tp is left or above fp
            using std::swap;
            swap(f, t);
            swap(fp, tp);
        }
    }
    Rect bbox() const { return Rect(fp, tp); }
    bool operator==(Section const &other) const {
        return (curve == other.curve) && (f == other.f) && (t == other.t);
    }
};

class TopoGraph {
  public:

    // Represents an e    double tol;dge on a vertex
    class Edge {
      public:
        boost::shared_ptr<Section> section;    // section associated with this edge
        unsigned other; // index of the vertex this edge points to
        Edge(boost::shared_ptr<Section> s, unsigned o) : section(s), other(o) {}
    };
    
    // Represents a vertex in the graph, in terms of a point and edges which enter and exit.
    // A vertex has an "avg" point, which is a representative point for the vertex.  All
    // edges have an endpoint tol away.
    class Vertex {
      public:
        std::vector<Edge> enters, exits; // indexes of the enter / exit edges
        Point avg;
        Vertex(Point p) : avg(p) {}
        inline unsigned degree() const { return enters.size() + exits.size(); }
        Edge operator[](unsigned ix) const;
        Edge &operator[](unsigned ix);
        void erase(unsigned ix);
        void insert(unsigned ix, Edge e);
        unsigned find_section(boost::shared_ptr<Section> section) const;
    };
    
    TopoGraph(PathVector const &ps, Dim2 d, double t);
    
    unsigned size() const { return vertices.size(); }
    
    Vertex       &operator[](unsigned ix)       { return vertices[ix]; }
    Vertex const &operator[](unsigned ix) const { return vertices[ix]; }
    
    //removes both edges, and returns the vertices[ix][jx] one
    Edge remove_edge(unsigned ix, unsigned jx);
    
    //returns a graph with all zero degree vertices and unused edges removed
    void cannonize();
    
    //checks invariants
    void assert_invariants() const;
    
    std::vector<Vertex> vertices;
    Dim2 dim;
    double tol;
};

//TODO: convert to classes
typedef std::vector<boost::shared_ptr<Section> > Area;
typedef std::vector<Area> Areas;

//TopoGraph sweep_graph(PathVector const &ps, Dim2 d = X, double tol = 0.00001);

void trim_whiskers(TopoGraph &g);
void double_whiskers(TopoGraph &g);
//void remove_degenerate(TopoGraph &g);
//void remove_vestigial(TopoGraph &g);
//Areas traverse_areas(TopoGraph const &g);


void remove_area_whiskers(Areas &areas);
PathVector areas_to_paths(PathVector const &ps, Areas const &areas);

class SectionSorter {
    const PathVector &ps;
    Dim2 dim;
    double tol;
    bool section_order(Section const &a, double at, Section const &b, double bt) const;
  public:
    typedef Section first_argument_type;
    typedef Section second_argument_type;
    typedef bool result_type;
  
    SectionSorter(const PathVector &rs, Dim2 d, double t = 0.00001) : ps(rs), dim(d), tol(t) {}
    bool operator()(Section const &a, Section const &b) const;
};

//sorter used to create the initial sweep of sections, such that they are dealt with in order
struct SweepSorter {
    typedef Section first_argument_type;
    typedef Section second_argument_type;
    typedef bool result_type;
    Dim2 dim;
    SweepSorter(Dim2 d) : dim(d) {}
    bool operator()(const Section &a, const Section &b) const {
        return Point::LexLessRt(dim)(a.fp, b.fp);
    }
};

struct UnionOp {
    unsigned ix;
    bool nz1, nz2;
    UnionOp(unsigned i, bool a, bool b) : ix(i), nz1(a), nz2(b) {}
    bool operator()(std::vector<int> const &windings) const {
        int w1 = 0, w2 = 0;
        for(unsigned j = 0; j < ix; j++) w1 += windings[j];
        for(unsigned j = ix; j < windings.size(); j++) w2 += windings[j];
        return (nz1 ? w1 : w1 % 2) != 0 || (nz2 ? w2 : w2 % 2) != 0;
    }
};

//returns all areas for which the winding -> bool function yields true
template<class Z>
Areas filter_areas(PathVector const &ps, Areas const & areas, Z const &z) {
    Areas ret;
    SweepSorter sorty = SweepSorter(Y);
    SectionSorter sortx = SectionSorter(ps, X);
    for(unsigned i = 0; i < areas.size(); i++) {
        if(areas[i].size() < 2) continue;
        //find a representative section
        unsigned rj = 0;
        //bool rev = are_near(areas[i][0]->fp, areas[i][1]->tp);
        for(unsigned j = 1; j < areas[i].size(); j++)
            if(sorty(*areas[i][rj], *areas[i][j])) rj = j;
        if(sortx(*areas[i][rj], *areas[i][(rj+areas[i].size() - 1) % areas[i].size()])) {
            rj = 0;
            for(unsigned j = 1; j < areas[i].size(); j++)
                if(sorty(*areas[i][j], *areas[i][rj])) rj = j;
        }
        if(z(areas[i][rj]->windings)) ret.push_back(areas[i]);
    }
    return ret;
}

} // end namespace Geom

#endif // LIB2GEOM_SEEN_TOPOSWEEP_H

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

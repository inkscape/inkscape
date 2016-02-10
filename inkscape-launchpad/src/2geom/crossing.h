/**
 * @file
 * @brief Structure representing the intersection of two curves
 *//*
 * Authors:
 *   Michael Sloan <mgsloan@gmail.com>
 *   Marco Cecchetti <mrcekets at gmail.com>
 * 
 * Copyright 2006-2008  authors
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

#ifndef LIB2GEOM_SEEN_CROSSING_H
#define LIB2GEOM_SEEN_CROSSING_H

#include <vector>
#include <2geom/rect.h>
#include <2geom/sweep-bounds.h>
#include <boost/optional/optional.hpp>
#include <2geom/pathvector.h>

namespace Geom {

//Crossing between one or two paths
struct Crossing {
    bool dir; //True: along a, a becomes outside.
    double ta, tb;  //time on a and b of crossing
    unsigned a, b;  //storage of indices
    Crossing() : dir(false), ta(0), tb(1), a(0), b(1) {}
    Crossing(double t_a, double t_b, bool direction) : dir(direction), ta(t_a), tb(t_b), a(0), b(1) {}
    Crossing(double t_a, double t_b, unsigned ai, unsigned bi, bool direction) : dir(direction), ta(t_a), tb(t_b), a(ai), b(bi) {}
    bool operator==(const Crossing & other) const { return a == other.a && b == other.b && dir == other.dir && ta == other.ta && tb == other.tb; }
    bool operator!=(const Crossing & other) const { return !(*this == other); }

    unsigned getOther(unsigned cur) const { return a == cur ? b : a; }
    double getTime(unsigned cur) const { return a == cur ? ta : tb; }
    double getOtherTime(unsigned cur) const { return a == cur ? tb : ta; }
    bool onIx(unsigned ix) const { return a == ix || b == ix; }
};

typedef boost::optional<Crossing> OptCrossing;


/*
struct Edge {
    unsigned node, path;
    double time;
    bool reverse;
    Edge(unsigned p, double t, bool r) : path(p), time(t), reverse(r) {}
    bool operator==(Edge const &other) const { return other.path == path && other.time == time && other.reverse == reverse; }
};

struct CrossingNode {
    std::vector<Edge> edges;
    CrossingNode() : edges(std::vector<Edge>()) {}
    explicit CrossingNode(std::vector<Edge> es) : edges(es) {}
    void add_edge(Edge const &e) {
    	if(std::find(edges.begin(), edges.end(), e) == edges.end())
    	    edges.push_back(e);
    }
    double time_on(unsigned p) {
    	for(unsigned i = 0; i < edges.size(); i++)
    	    if(edges[i].path == p) return edges[i].time;
    	std::cout << "CrossingNode time_on failed\n";
    	return 0;
    }
};


typedef std::vector<CrossingNode> CrossingGraph;

struct TimeOrder {
    bool operator()(Edge a, Edge b) {
        return a.time < b.time;
    }
};

class Path;
CrossingGraph create_crossing_graph(PathVector const &p, Crossings const &crs);
*/

/*inline bool are_near(Crossing a, Crossing b) {
    return are_near(a.ta, b.ta) && are_near(a.tb, b.tb);
}

struct NearF { bool operator()(Crossing a, Crossing b) { return are_near(a, b); } };
*/

struct CrossingOrder {
    unsigned ix;
    bool rev;
    CrossingOrder(unsigned i, bool r = false) : ix(i), rev(r) {}
    bool operator()(Crossing a, Crossing b) {
        if(rev) 
            return (ix == a.a ? a.ta : a.tb) <
                   (ix == b.a ? b.ta : b.tb);
        else
            return (ix == a.a ? a.ta : a.tb) >
                   (ix == b.a ? b.ta : b.tb);
    }
};

typedef std::vector<Crossing> Crossings;

typedef std::vector<Crossings> CrossingSet;

template<typename C>
std::vector<Rect> bounds(C const &a) {
    std::vector<Rect> rs;
    for (unsigned i = 0; i < a.size(); i++) {
        OptRect bb = a[i].boundsFast();
        if (bb) {
            rs.push_back(*bb);
        }
    }
    return rs;
}
// provide specific method for Paths because paths can be closed or open. Path::size() is named somewhat wrong...
std::vector<Rect> bounds(Path const &a);

inline void sort_crossings(Crossings &cr, unsigned ix) { std::sort(cr.begin(), cr.end(), CrossingOrder(ix)); }

template <typename T>
struct CrossingTraits {
    typedef std::vector<T> VectorT;
    static inline VectorT init(T const &x) { return VectorT(1, x); }
};
template <>
struct CrossingTraits<Path> {
    typedef PathVector VectorT;
    static inline VectorT vector_one(Path const &x) { return VectorT(x); }
};

template<typename T>
struct Crosser {
    typedef typename CrossingTraits<T>::VectorT VectorT;
    virtual ~Crosser() {}
    virtual Crossings crossings(T const &a, T const &b) {
        return crossings(CrossingTraits<T>::vector_one(a), CrossingTraits<T>::vector_one(b))[0]; }
    virtual CrossingSet crossings(VectorT const &a, VectorT const &b) {
        CrossingSet results(a.size() + b.size(), Crossings());
    
        std::vector<std::vector<unsigned> > cull = sweep_bounds(bounds(a), bounds(b));
        for(unsigned i = 0; i < cull.size(); i++) {
            for(unsigned jx = 0; jx < cull[i].size(); jx++) {
                unsigned j = cull[i][jx];
                unsigned jc = j + a.size();
                Crossings cr = crossings(a[i], b[j]);
                for(unsigned k = 0; k < cr.size(); k++) { cr[k].a = i; cr[k].b = jc; }
                
                //Sort & add A-sorted crossings
                sort_crossings(cr, i);
                Crossings n(results[i].size() + cr.size());
                std::merge(results[i].begin(), results[i].end(), cr.begin(), cr.end(), n.begin(), CrossingOrder(i));
                results[i] = n;
                
                //Sort & add B-sorted crossings
                sort_crossings(cr, jc);
                n.resize(results[jc].size() + cr.size());
                std::merge(results[jc].begin(), results[jc].end(), cr.begin(), cr.end(), n.begin(), CrossingOrder(jc));
                results[jc] = n;
            }
        }
        return results;
    }
};
void merge_crossings(Crossings &a, Crossings &b, unsigned i);
void offset_crossings(Crossings &cr, double a, double b);

Crossings reverse_ta(Crossings const &cr, std::vector<double> max);
Crossings reverse_tb(Crossings const &cr, unsigned split, std::vector<double> max);
CrossingSet reverse_ta(CrossingSet const &cr, unsigned split, std::vector<double> max);
CrossingSet reverse_tb(CrossingSet const &cr, unsigned split, std::vector<double> max);

void clean(Crossings &cr_a, Crossings &cr_b);
void delete_duplicates(Crossings &crs);

} // end namespace Geom

#endif
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

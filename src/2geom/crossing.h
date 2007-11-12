#ifndef __GEOM_CROSSING_H
#define __GEOM_CROSSING_H

#include <vector>
#include "rect.h"
#include "sweep.h"

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

typedef std::vector<Crossing> Crossings;

typedef std::vector<CrossingNode> CrossingGraph;

struct TimeOrder {
    bool operator()(Edge a, Edge b) {
        return a.time < b.time;
    }
};

class Path;
CrossingGraph create_crossing_graph(std::vector<Path> const &p, Crossings const &crs);

/*inline bool are_near(Crossing a, Crossing b) {
    return are_near(a.ta, b.ta) && are_near(a.tb, b.tb);
}

struct NearF { bool operator()(Crossing a, Crossing b) { return are_near(a, b); } };
*/

struct CrossingOrder {
    unsigned ix;
    CrossingOrder(unsigned i) : ix(i) {}
    bool operator()(Crossing a, Crossing b) {
        return (ix == a.a ? a.ta : a.tb) <
               (ix == b.a ? b.ta : b.tb);
    }
};


typedef std::vector<Crossings> CrossingSet;

template<typename C>
std::vector<Rect> bounds(C const &a) {
    std::vector<Rect> rs;
    for(unsigned i = 0; i < a.size(); i++) rs.push_back(a[i].boundsFast());
    return rs;
}

inline void sort_crossings(Crossings &cr, unsigned ix) { std::sort(cr.begin(), cr.end(), CrossingOrder(ix)); }

template<typename T>
struct Crosser {
    virtual ~Crosser() {}
    virtual Crossings crossings(T const &a, T const &b) { return crossings(std::vector<T>(1,a), std::vector<T>(1,b))[0]; }
    virtual CrossingSet crossings(std::vector<T> const &a, std::vector<T> const &b) {
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

}

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

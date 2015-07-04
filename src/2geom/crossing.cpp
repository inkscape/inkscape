#include <2geom/crossing.h>
#include <2geom/path.h>

namespace Geom {

//bool edge_involved_in(Edge const &e, Crossing const &c) {
//    if(e.path == c.a) {
//        if(e.time == c.ta) return true;
//    } else if(e.path == c.b) {
//        if(e.time == c.tb) return true;
//    }
//    return false;
//}

double wrap_dist(double from, double to, double size, bool rev) {
    if(rev) {
        if(to > from) {
            return from + (size - to);
        } else {
            return from - to;
        }
    } else {
        if(to < from) {
            return to + (size - from);
        } else {
            return to - from;
        }
    }
}
/*
CrossingGraph create_crossing_graph(PathVector const &p, Crossings const &crs) {
    std::vector<Point> locs;
    CrossingGraph ret;
    for(unsigned i = 0; i < crs.size(); i++) {
        Point pnt = p[crs[i].a].pointAt(crs[i].ta);
        unsigned j = 0;
        for(; j < locs.size(); j++) {
            if(are_near(pnt, locs[j])) break;
        }
        if(j == locs.size()) {
            ret.push_back(CrossingNode());
            locs.push_back(pnt);
        }
        ret[j].add_edge(Edge(crs[i].a, crs[i].ta, false));
        ret[j].add_edge(Edge(crs[i].a, crs[i].ta, true));
        ret[j].add_edge(Edge(crs[i].b, crs[i].tb, false));
        ret[j].add_edge(Edge(crs[i].b, crs[i].tb, true));
    }
    
    for(unsigned i = 0; i < ret.size(); i++) {
        for(unsigned j = 0; j < ret[i].edges.size(); j++) {
            unsigned pth = ret[i].edges[j].path;
            double t = ret[i].edges[j].time;
            bool rev = ret[i].edges[j].reverse;
            double size = p[pth].size()+1;
            double best = size;
            unsigned bix = ret.size();
            for(unsigned k = 0; k < ret.size(); k++) {
                for(unsigned l = 0; l < ret[k].edges.size(); l++) {
            	    if(ret[i].edges[j].path == ret[k].edges[l].path && (k != i || l != j)) {
                        double d = wrap_dist(t, ret[i].edges[j].time, size, rev);
                        if(d < best) {
                            best = d;
                            bix = k;
                        }
                    }
                }
            }
            if(bix == ret.size()) {
                std::cout << "couldn't find an adequate next-crossing node";
                bix = i;
            }
            ret[i].edges[j].node = bix;
        }
    }
    
    return ret;
 */
 /*  Various incoherent code bits   
    // list of sets of edges, each set corresponding to those emanating from the path
    CrossingGraph ret;
    std::vector<Edge> edges(crs.size());
    
    std::vector<std::vector<bool> > used;
    unsigned i, j;
    do {
        first_false(used, i, j);
        CrossingNode cn;
        do {
            unsigned di = i, dj = j;
            crossing_dual(di, dj);
            if(!used[di,dj]) {
                
            }
        }
        
    } while(!used[i,j])
    
    
    for(unsigned j = 0; j < crs[i].size(); j++) {
        
        edges.push_back(Edge(i, crs[i][j].getOtherTime(i), false));
        edges.push_back(Edge(i, crs[i][j].getOtherTime(i), true));
    }
    std::sort(edges.begin(), edges.end(), TimeOrder());
    for(unsigned j = 0; j < edges.size(); ) {
        CrossingNode cn;
        double t = edges[j].time;
        while(j < edges.size() && are_near(edges[j].time, t)) {
            cn.edges.push_back(edges[j]);
        }
    }
*/
//}

// provide specific method for Paths because paths can be closed or open. Path::size() is named somewhat wrong...
std::vector<Rect> bounds(Path const &a) {
    std::vector<Rect> rs;
    for (unsigned i = 0; i < a.size_default(); i++) {
        OptRect bb = a[i].boundsFast();
        if (bb) {
            rs.push_back(*bb);
        }
    }
    return rs;
}

void merge_crossings(Crossings &a, Crossings &b, unsigned i) {
    Crossings n;
    sort_crossings(b, i);
    n.resize(a.size() + b.size());
    std::merge(a.begin(), a.end(), b.begin(), b.end(), n.begin(), CrossingOrder(i));
    a = n;
}

void offset_crossings(Crossings &cr, double a, double b) {
    for(unsigned i = 0; i < cr.size(); i++) {
        cr[i].ta += a;
        cr[i].tb += b;
    }
}

Crossings reverse_ta(Crossings const &cr, std::vector<double> max) {
    Crossings ret;
    for(Crossings::const_iterator i = cr.begin(); i != cr.end(); ++i) {
        double mx = max[i->a];
        ret.push_back(Crossing(i->ta > mx+0.01 ? (1 - (i->ta - mx) + mx) : mx - i->ta,
                               i->tb, !i->dir));
    }
    return ret;
}

Crossings reverse_tb(Crossings const &cr, unsigned split, std::vector<double> max) {
    Crossings ret;
    for(Crossings::const_iterator i = cr.begin(); i != cr.end(); ++i) {
        double mx = max[i->b - split];
        ret.push_back(Crossing(i->ta, i->tb > mx+0.01 ? (1 - (i->tb - mx) + mx) : mx - i->tb,
                               !i->dir));
    }
    return ret;
}

CrossingSet reverse_ta(CrossingSet const &cr, unsigned split, std::vector<double> max) {
    CrossingSet ret;
    for(unsigned i = 0; i < cr.size(); i++) {
        Crossings res = reverse_ta(cr[i], max);
        if(i < split) std::reverse(res.begin(), res.end());
        ret.push_back(res);
    }
    return ret;
}

CrossingSet reverse_tb(CrossingSet const &cr, unsigned split, std::vector<double> max) {
    CrossingSet ret;
    for(unsigned i = 0; i < cr.size(); i++) {
        Crossings res = reverse_tb(cr[i], split, max);
        if(i >= split) std::reverse(res.begin(), res.end());
        ret.push_back(res);
    }
    return ret;
}

// Delete any duplicates in a vector of crossings
// A crossing is considered to be a duplicate when it has both t_a and t_b near to another crossing's t_a and t_b
// For example, duplicates will be found when calculating the intersections of a linesegment with a polygon, if the
// endpoint of that line coincides with a cusp node of the polygon. In that case, an intersection will be found of
// the linesegment with each of the polygon's linesegments extending from the cusp node (i.e. two intersections)
void delete_duplicates(Crossings &crs) {
    Crossings::reverse_iterator rit = crs.rbegin();

    for (rit = crs.rbegin(); rit!= crs.rend(); ++rit) {
        Crossings::reverse_iterator rit2 = rit;
        while (++rit2 != crs.rend()) {
            if (Geom::are_near((*rit).ta, (*rit2).ta) && Geom::are_near((*rit).tb, (*rit2).tb)) {
                crs.erase((rit + 1).base()); // This +1 and .base() construction is needed to convert to a regular iterator
                break; // out of while loop, and continue with next iteration of for loop
            }
        }
    }
}

void clean(Crossings &/*cr_a*/, Crossings &/*cr_b*/) {
/*    if(cr_a.empty()) return;
    
    //Remove anything with dupes
    
    for(Eraser<Crossings> i(&cr_a); !i.ended(); i++) {
        const Crossing cur = *i;
        Eraser<Crossings> next(i);
        next++;
        if(are_near(cur, *next)) {
            cr_b.erase(std::find(cr_b.begin(), cr_b.end(), cur));
            for(i = next; near(*i, cur); i++) {
                cr_b.erase(std::find(cr_b.begin(), cr_b.end(), *i));
            }
            continue;
        }
    }
*/
}

}

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

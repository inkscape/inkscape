#include "shape.h"
#include "utils.h"
#include "sweep.h"

#include <iostream>
#include <algorithm>

namespace Geom {

// Utility funcs

// Yes, xor is !=, but I'm pretty sure this is safer in the event of strange bools
bool logical_xor (bool a, bool b) { return (a || b) && !(a && b); }

// A little sugar for appending a list to another
template<typename T>
void append(T &a, T const &b) {
    a.insert(a.end(), b.begin(), b.end());
}

/* Used within shape_boolean and related functions, as the name describes, finds the
 * first false within the list of lists of booleans.
 */
void first_false(std::vector<std::vector<bool> > visited, unsigned &i, unsigned &j) {
    for(i = 0, j = 0; i < visited.size(); i++) {
        std::vector<bool>::iterator unvisited = std::find(visited[i].begin(), visited[i].end(), false);
        if(unvisited != visited[i].end()) {
            j = unvisited - visited[i].begin();
            break;
        }
    }
}

// Finds a crossing in a list of them, given the sorting index.
unsigned find_crossing(Crossings const &cr, Crossing x, unsigned i) {
    return std::lower_bound(cr.begin(), cr.end(), x, CrossingOrder(i)) - cr.begin();
}

/* This function handles boolean ops on shapes.  The first parameter is a bool
 * which determines its behavior in each combination of cases.  For proper
 * fill information and noncrossing behavior, the fill data of the regions
 * must be correct.  The boolean parameter determines whether the operation
 * is a union or a subtraction.  Reversed paths represent inverse regions,
 * where everything is included in the fill except for the insides.
 *
 * Here is a chart of the behavior under various circumstances:
 * 
 * rev = false (union)
 *            A
 *        F         H
 * F  A+B -> F  A-B -> H
 *B
 * H  B-A -> H  AxB -> H
 *
 * rev = true (intersect)
 *            A
 *        F         H
 * F  AxB -> F  B-A -> F
 *B
 * H  A-B -> F  A+B -> H
 *
 * F/H = Fill outer / Hole outer
 * A/B specify operands
 * + = union, - = subtraction, x = intersection
 * -> read as "produces"
 *
 * This is the main function of boolops, yet its operation isn't very complicated.
 * It traverses the crossings, and uses the crossing direction to decide whether
 * the next segment should be taken from A or from B.  The second half of the
 * function deals with figuring out what to do with bits that have no intersection.
 */
Shape shape_boolean(bool rev, Shape const & a, Shape const & b, CrossingSet const & crs) {
    const Regions ac = a.content, bc = b.content;

    //Keep track of which crossings we've hit.
    std::vector<std::vector<bool> > visited;
    for(unsigned i = 0; i < crs.size(); i++)
        visited.push_back(std::vector<bool>(crs[i].size(), false));
    
    //bool const exception = 
    
    //Traverse the crossings, creating chunks
    Regions chunks;
    while(true) {
        unsigned i, j;
        first_false(visited, i, j);
        if(i == visited.size()) break;
        
        Path res;
        do {
            Crossing cur = crs[i][j];
            visited[i][j] = true;
            
            //get indices of the dual:
            unsigned io = cur.getOther(i), jo = find_crossing(crs[io], cur, io);
            if(jo < visited[io].size()) visited[io][jo] = true;
            
            //main driving logic
            if(logical_xor(cur.dir, rev)) {
                if(i >= ac.size()) { i = io; j = jo; }
                j++;
                if(j >= crs[i].size()) j = 0;
                Crossing next = crs[i][j];
                ac[next.a].boundary.appendPortionTo(res, cur.ta, next.ta);
            } else {
                if(i < ac.size()) { i = io; j = jo; }
                j++;
                if(j >= crs[i].size()) j = 0;
                Crossing next = crs[i][j];
                bc[next.b - ac.size()].boundary.appendPortionTo(res, cur.tb, next.tb);
            }
        } while (!visited[i][j]);
        if(res.size() > 0) chunks.push_back(Region(res));
    }
    
    //If true, then we are on the 'subtraction diagonal'
    bool const on_sub = logical_xor(a.fill, b.fill);
    //If true, then the hole must be inside the other to be included
    bool const a_mode = logical_xor(logical_xor(!rev, a.fill), on_sub),
               b_mode = logical_xor(logical_xor(!rev, b.fill), on_sub);
    
    //Handle unintersecting portions
    for(unsigned i = 0; i < crs.size(); i++) {
        if(crs[i].size() == 0) {
            Region    r(i < ac.size() ? ac[i] : bc[i - ac.size()]);
            bool   mode(i < ac.size() ? a_mode : b_mode);
            
            if(logical_xor(r.fill, i < ac.size() ? a.fill : b.fill)) {
                //is an inner (fill is opposite the outside fill)
                Point exemplar = r.boundary.initialPoint();
                Regions const & others = i < ac.size() ? bc : ac;
                for(unsigned j = 0; j < others.size(); j++) {
                    if(others[j].contains(exemplar)) {
                        //contained in another
                        if(mode) chunks.push_back(r);
                        goto skip;
                    }
                }
            }
            //disjoint
            if(!mode) chunks.push_back(r);
            skip: (void)0;
        }
    }
    
    return Shape(chunks);
}

// Just a convenience wrapper for shape_boolean, which handles the crossings
Shape shape_boolean(bool rev, Shape const & a, Shape const & b) {
    CrossingSet crs = crossings_between(a, b);
    
    return shape_boolean(rev, a, b, crs);
}


// Some utility functions for boolop:

std::vector<double> region_sizes(Shape const &a) {
    std::vector<double> ret;
    for(unsigned i = 0; i < a.size(); i++) {
        ret.push_back(double(a[i].size()));
    }
    return ret;
}

Shape shape_boolean_ra(bool rev, Shape const &a, Shape const &b, CrossingSet const &crs) {
    return shape_boolean(rev, a.inverse(), b, reverse_ta(crs, a.size(), region_sizes(a)));
}

Shape shape_boolean_rb(bool rev, Shape const &a, Shape const &b, CrossingSet const &crs) {
    return shape_boolean(rev, a, b.inverse(), reverse_tb(crs, a.size(), region_sizes(b)));
}

/* This is a function based on shape_boolean which allows boolean operations
 * to be specified as a logic table.  This logic table is 4 bit-flags, which
 * correspond to the elements of the 'truth table' for a particular operation.
 * These flags are defined with the enums starting with BOOLOP_ .
 */
Shape boolop(Shape const &a, Shape const &b, unsigned flags, CrossingSet const &crs) {
    flags &= 15;
    if(flags <= BOOLOP_UNION) {
        switch(flags) {
            case BOOLOP_INTERSECT:    return shape_boolean(true, a, b, crs);
            case BOOLOP_SUBTRACT_A_B: return shape_boolean_rb(true, a, b, crs);
            case BOOLOP_IDENTITY_A:   return a;
            case BOOLOP_SUBTRACT_B_A: return shape_boolean_ra(true, a, b, crs);
            case BOOLOP_IDENTITY_B:   return b;
            case BOOLOP_EXCLUSION: {
                Shape res = shape_boolean_rb(true, a, b, crs);
                append(res.content, shape_boolean_ra(true, a, b, crs).content);
                return res;
            }
            case BOOLOP_UNION:        return shape_boolean(false, a, b);
        }
    } else {
        switch(flags - BOOLOP_NEITHER) {
            case BOOLOP_SUBTRACT_A_B: return shape_boolean_ra(false, a, b, crs);
            case BOOLOP_SUBTRACT_B_A: return shape_boolean_rb(false, a, b, crs);
            case BOOLOP_EXCLUSION: {
                Shape res = shape_boolean_ra(false, a, b, CrossingSet(crs));
                append(res.content, shape_boolean_rb(false, a, b, CrossingSet(crs)).content);
                return res;
            }
        }
        return boolop(a, b, ~flags, crs).inverse();
    }
    return Shape();
}

/* This version of the boolop function doesn't require a set of crossings, as
 * it computes them for you.  This is more efficient in some cases, as the
 * shape can be inverted before finding crossings.  In the special case of
 * exclusion it uses the other version of boolop.
 */
Shape boolop(Shape const &a, Shape const &b, unsigned flags) {
    flags &= 15;
    if(flags <= BOOLOP_UNION) {
        switch(flags) {
            case BOOLOP_INTERSECT:    return shape_boolean(true, a, b);
            case BOOLOP_SUBTRACT_A_B: return shape_boolean(true, a, b.inverse());
            case BOOLOP_IDENTITY_A:   return a;
            case BOOLOP_SUBTRACT_B_A: return shape_boolean(true, b, a.inverse());
            case BOOLOP_IDENTITY_B:   return b;
            case BOOLOP_EXCLUSION: {
                Shape res = shape_boolean(true, a, b.inverse());
                append(res.content, shape_boolean(true, b, a.inverse()).content);
                return res;
            } //return boolop(a, b, flags,  crossings_between(a, b));
            case BOOLOP_UNION:        return shape_boolean(false, a, b);
        }
    } else {
        switch(flags - BOOLOP_NEITHER) {
            case BOOLOP_SUBTRACT_A_B: return shape_boolean(false, b, a.inverse());
            case BOOLOP_SUBTRACT_B_A: return shape_boolean(false, a, b.inverse());
            case BOOLOP_EXCLUSION: {
                Shape res = shape_boolean(false, a, b.inverse());
                append(res.content, shape_boolean(false, b, a.inverse()).content);
                return res;
            } //return boolop(a, b, flags, crossings_between(a, b));
        }
        return boolop(a, b, ~flags).inverse();
    }
    return Shape();
}


int paths_winding(std::vector<Path> const &ps, Point p) {
    int ret;
    for(unsigned i = 0; i < ps.size(); i++)
        ret += winding(ps[i], p);
    return ret;
}

std::vector<double> y_of_roots(std::vector<Path> const & ps, double x) {
    std::vector<double> res;
    for(unsigned i = 0; i < ps.size(); i++) {
        std::vector<double> temp = ps[i].roots(x, X);
        for(unsigned i = 0; i < temp.size(); i++)
            res.push_back(ps[i].valueAt(temp[i], Y));
    }
    std::sort(res.begin(), res.end());
    return res;
}

struct Edge {
    unsigned ix;
    double from, to;
    bool rev;
    int wind;
    Edge(unsigned i, double ft, double tt, bool r, unsigned w) : ix(i), from(ft), to(tt), rev(r), wind(w) {}
    Edge(unsigned i, double ft, double tt, bool r, std::vector<Path> const &ps) : ix(i), from(ft), to(tt), rev(r) {
        //TODO: get the edge wind data some other way
        Point p = ps[i].pointAt(ft);
        std::vector<double> rs = y_of_roots(ps, p[X]);
        unsigned interv = std::lower_bound(rs.begin(), rs.end(), p[Y]) - rs.begin();
        wind = interv % 2;
    }
    double initial() { return rev ? to : from; }
    double final() { return rev ? from : to; }
    void addTo(Path &res, std::vector<Path> const &ps) {
        if(rev) {
            Path p = ps[ix].portion(to, from).reverse();
            for(unsigned i = 0; i < p.size(); i++)
                res.append(p[i]);
        } else {
            ps[ix].appendPortionTo(res, from, to);
        }
    }
};

typedef std::vector<Edge> Edges;

double point_cosine(Point a, Point b, Point c) {
    Point db = b - a, dc = c - a;
    return cross(db, dc) / (db.length() * dc.length());
}

//sanitize
Regions regionize_paths(std::vector<Path> const &ps, bool evenodd) {
    CrossingSet crs = crossings_among(ps);
    
    Edges es;
    
    for(unsigned i = 0; i < crs.size(); i++) {
        for(unsigned j = 0; j < crs[i].size(); j++) {
            Crossing cur = crs[i][j];
            int io = i, jo = j;
            
            jo++;
            if(jo >= crs[io].size()) jo = 0;
            //std::cout << io << ", " << jo << "\n";
            if(cur.a == i)
                es.push_back(Edge(i, cur.ta, crs[io][jo].ta, false, ps));
            else
                es.push_back(Edge(i, cur.tb, crs[io][jo].tb, false, ps));
            
            jo-=2;
            if(jo < 0) jo += crs[io].size();
            // std::cout << io << ", " << jo << "\n";
            if(cur.a == i)
                es.push_back(Edge(i, crs[io][jo].ta, cur.ta, true, ps));
            else
                es.push_back(Edge(i, crs[io][jo].tb, cur.tb, true, ps));
        }
    }
    for(unsigned i = 0; i<crs.size(); i++) {
        if(crs[i].empty()) {
            es.push_back(Edge(i, 0, ps[i].size(), false, ps));
            es.push_back(Edge(i, ps[i].size(), 0, true, ps));
        }
    }
    
    Edges es2;
    //filter
    for(unsigned i = 0; i < es.size(); i++) {
        if(true) //(evenodd && es[i].wind % 2 == 0) || (!evenodd && es[i].wind == 0))
            es2.push_back(es[i]);
    }
    es = es2;
    
    std::cout << es.size() << " edges\n";
    
    Regions chunks;
    for(unsigned i = 0; i < es.size(); i++) {
        Edge cur = es[i];
        if(cur.rev)
            chunks.push_back(Region(ps[cur.ix].portion(cur.from, cur.to).reverse(), cur.wind % 2 != 0));
        else
            chunks.push_back(Region(ps[cur.ix].portion(cur.from, cur.to), cur.wind % 2 != 0));
    }
    return chunks;
    
    //Regions chunks;
    std::vector<bool> used(es2.size(), false);
    while(true) {
        unsigned i = std::find(used.begin(), used.end(), false) - used.begin();
        if(i == used.size()) break;
        Path res;
        do {
            es2[i].addTo(res, ps);
            Point pnt = res.finalPoint();
            std::vector<unsigned> poss;
            for(unsigned j = 0; j < es2.size(); j++)
                if(near(pnt, ps[es2[j].ix].pointAt(es2[j].initial()))) poss.push_back(j);
            if(poss.empty()) break;
            unsigned best = 0;
            if(poss.size() > 1) {
                double crossval = 10;
                Point along = ps[i].pointAt(es2[i].final()+0.1);
                for(unsigned j = 0; j < poss.size(); j++) {
                    unsigned ix = poss[j];
                    double val = point_cosine(pnt, along, ps[ix].pointAt(es2[ix].initial()+.01));
                    if(val < crossval) {
                        crossval = val;
                        best = j;
                    }
                }
            }
            i = poss[best];
        } while(!used[i]);
        chunks.push_back(Region(res));
    }
    return chunks;
}

/* This transforms a shape by a matrix.  In the case that the matrix flips
 * the shape, it reverses the paths in order to preserve the fill.
 */
Shape Shape::operator*(Matrix const &m) const {
    Shape ret;
    for(unsigned i = 0; i < size(); i++)
        ret.content.push_back(content[i] * m);
    ret.fill = fill;
    return ret;
}

// Inverse is a boolean not, and simply reverses all the paths & fill flags
Shape Shape::inverse() const {
    Shape ret;
    for(unsigned i = 0; i < size(); i++)
        ret.content.push_back(content[i].inverse());
    ret.fill = !fill;
    return ret;
}

struct ContainmentOrder {
    std::vector<Region> const *rs;
    explicit ContainmentOrder(std::vector<Region> const *r) : rs(r) {}
    bool operator()(unsigned a, unsigned b) const { return (*rs)[b].contains((*rs)[a]); }
};

bool Shape::contains(Point const &p) const {
    std::vector<Rect> pnt;
    pnt.push_back(Rect(p, p));
    std::vector<std::vector<unsigned> > cull = sweep_bounds(pnt, bounds(*this));
    if(cull[0].size() == 0) return !fill;
    return content[*min_element(cull[0].begin(), cull[0].end(), ContainmentOrder(&content))].isFill();
}

bool Shape::inside_invariants() const {  //semi-slow & easy to violate
    for(unsigned i = 0; i < size(); i++)
        if( logical_xor(content[i].isFill(), contains(content[i].boundary.initialPoint())) ) return false;
    return true;
}
bool Shape::region_invariants() const { //semi-slow
    for(unsigned i = 0; i < size(); i++)
        if(!content[i].invariants()) return false;
    return true;
}
bool Shape::cross_invariants() const { //slow
    CrossingSet crs; // = crossings_among(paths_from_regions(content));
    for(unsigned i = 0; i < crs.size(); i++)
        if(!crs[i].empty()) return false;
    return true;
}

bool Shape::invariants() const {
    return inside_invariants() && region_invariants() && cross_invariants();
}

}

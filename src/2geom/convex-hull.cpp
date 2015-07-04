/** @file
 * @brief Convex hull of a set of points
 *//*
 * Authors:
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *   Michael G. Sloan <mgsloan@gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * Copyright 2006-2015 Authors
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

#include <2geom/convex-hull.h>
#include <2geom/exception.h>
#include <algorithm>
#include <map>
#include <iostream>
#include <cassert>
#include <boost/array.hpp>

/** Todo:
    + modify graham scan to work top to bottom, rather than around angles
    + intersection
    + minimum distance between convex hulls
    + maximum distance between convex hulls
    + hausdorf metric?
    + check all degenerate cases carefully
    + check all algorithms meet all invariants
    + generalise rotating caliper algorithm (iterator/circulator?)
*/

using std::vector;
using std::map;
using std::pair;
using std::make_pair;
using std::swap;

namespace Geom {

ConvexHull::ConvexHull(Point const &a, Point const &b)
    : _boundary(2)
    , _lower(0)
{
    _boundary[0] = a;
    _boundary[1] = b;
    std::sort(_boundary.begin(), _boundary.end(), Point::LexLess<X>());
    _construct();
}

ConvexHull::ConvexHull(Point const &a, Point const &b, Point const &c)
    : _boundary(3)
    , _lower(0)
{
    _boundary[0] = a;
    _boundary[1] = b;
    _boundary[2] = c;
    std::sort(_boundary.begin(), _boundary.end(), Point::LexLess<X>());
    _construct();
}

ConvexHull::ConvexHull(Point const &a, Point const &b, Point const &c, Point const &d)
    : _boundary(4)
    , _lower(0)
{
    _boundary[0] = a;
    _boundary[1] = b;
    _boundary[2] = c;
    _boundary[3] = d;
    std::sort(_boundary.begin(), _boundary.end(), Point::LexLess<X>());
    _construct();
}

ConvexHull::ConvexHull(std::vector<Point> const &pts)
    : _lower(0)
{
    //if (pts.size() > 16) { // arbitrary threshold
    //    _prune(pts.begin(), pts.end(), _boundary);
    //} else {
        _boundary = pts;
        std::sort(_boundary.begin(), _boundary.end(), Point::LexLess<X>());
    //}
    _construct();
}

bool ConvexHull::_is_clockwise_turn(Point const &a, Point const &b, Point const &c)
{
    if (b == c) return false;
    return cross(b-a, c-a) > 0;
}

void ConvexHull::_construct()
{
    // _boundary must already be sorted in LexLess<X> order
    if (_boundary.empty()) {
        _lower = 0;
        return;
    }
    if (_boundary.size() == 1 || (_boundary.size() == 2 && _boundary[0] == _boundary[1])) {
        _boundary.resize(1);
        _lower = 1;
        return;
    }
    if (_boundary.size() == 2) {
        _lower = 2;
        return;
    }

    std::size_t k = 2;
    for (std::size_t i = 2; i < _boundary.size(); ++i) {
        while (k >= 2 && !_is_clockwise_turn(_boundary[k-2], _boundary[k-1], _boundary[i])) {
            --k;
        }
        std::swap(_boundary[k++], _boundary[i]);
    }

    _lower = k;
    std::sort(_boundary.begin() + k, _boundary.end(), Point::LexGreater<X>());
    _boundary.push_back(_boundary.front());
    for (std::size_t i = _lower; i < _boundary.size(); ++i) {
        while (k > _lower && !_is_clockwise_turn(_boundary[k-2], _boundary[k-1], _boundary[i])) {
            --k;
        }
        std::swap(_boundary[k++], _boundary[i]);
    }

    _boundary.resize(k-1);
}

double ConvexHull::area() const
{
    if (size() <= 2) return 0;

    double a = 0;
    for (std::size_t i = 0; i < size()-1; ++i) {
        a += cross(_boundary[i], _boundary[i+1]);
    }
    a += cross(_boundary.back(), _boundary.front());
    return fabs(a * 0.5);
}

OptRect ConvexHull::bounds() const
{
    OptRect ret;
    if (empty()) return ret;
    ret = Rect(left(), top(), right(), bottom());
    return ret;
}

Point ConvexHull::topPoint() const
{
    Point ret;
    ret[Y] = std::numeric_limits<Coord>::infinity();

    for (UpperIterator i = upperHull().begin(); i != upperHull().end(); ++i) {
        if (ret[Y] >= i->y()) {
            ret = *i;
        } else {
            break;
        }
    }

    return ret;
}

Point ConvexHull::bottomPoint() const
{
    Point ret;
    ret[Y] = -std::numeric_limits<Coord>::infinity();

    for (LowerIterator j = lowerHull().begin(); j != lowerHull().end(); ++j) {
        if (ret[Y] <= j->y()) {
            ret = *j;
        } else {
            break;
        }
    }

    return ret;
}

template <typename Iter, typename Lex>
bool below_x_monotonic_polyline(Point const &p, Iter first, Iter last, Lex lex)
{
    typename Lex::Secondary above;
    Iter f = std::lower_bound(first, last, p, lex);
    if (f == last) return false;
    if (f == first) {
        if (p == *f) return true;
        return false;
    }

    Point a = *(f-1), b = *f;
    if (a[X] == b[X]) {
        if (above(p[Y], a[Y]) || above(b[Y], p[Y])) return false;
    } else {
        // TODO: maybe there is a more numerically stable method
        Coord y = lerp((p[X] - a[X]) / (b[X] - a[X]), a[Y], b[Y]);
        if (above(p[Y], y)) return false;
    }
    return true;
}

bool ConvexHull::contains(Point const &p) const
{
    if (_boundary.empty()) return false;
    if (_boundary.size() == 1) {
        if (_boundary[0] == p) return true;
        return false;
    }

    // 1. verify that the point is in the relevant X range
    if (p[X] < _boundary[0][X] || p[X] > _boundary[_lower-1][X]) return false;

    // 2. check whether it is below the upper hull
    UpperIterator ub = upperHull().begin(), ue = upperHull().end();
    if (!below_x_monotonic_polyline(p, ub, ue, Point::LexLess<X>())) return false;

    // 3. check whether it is above the lower hull
    LowerIterator lb = lowerHull().begin(), le = lowerHull().end();
    if (!below_x_monotonic_polyline(p, lb, le, Point::LexGreater<X>())) return false;
    
    return true;
}

bool ConvexHull::contains(Rect const &r) const
{
    for (unsigned i = 0; i < 4; ++i) {
        if (!contains(r.corner(i))) return false;
    }
    return true;
}

bool ConvexHull::contains(ConvexHull const &ch) const
{
    // TODO: requires interiorContains.
    // We have to check all points of ch, and each point takes logarithmic time.
    // If there are more points in ch that here, it is faster to make the check
    // the other way around.
    /*if (ch.size() > size()) {
        for (iterator i = begin(); i != end(); ++i) {
            if (ch.interiorContains(*i)) return false;
        }
        return true;
    }*/

    for (iterator i = ch.begin(); i != ch.end(); ++i) {
        if (!contains(*i)) return false;
    }
    return true;
}

void ConvexHull::swap(ConvexHull &other)
{
    _boundary.swap(other._boundary);
    std::swap(_lower, other._lower);
}

void ConvexHull::swap(std::vector<Point> &pts)
{
    _boundary.swap(pts);
    _lower = 0;
    std::sort(_boundary.begin(), _boundary.end(), Point::LexLess<X>());
    _construct();
}

#if 0
/*** SignedTriangleArea
 * returns the area of the triangle defined by p0, p1, p2.  A clockwise triangle has positive area.
 */
double
SignedTriangleArea(Point p0, Point p1, Point p2) {
    return cross((p1 - p0), (p2 - p0));
}

class angle_cmp{
public:
    Point o;
    angle_cmp(Point o) : o(o) {}

#if 0
    bool
    operator()(Point a, Point b) {
        // not remove this check or std::sort could crash
        if (a == b) return false;
        Point da = a - o;
        Point db = b - o;
        if (da == -db) return false;

#if 1
        double aa = da[0];
        double ab = db[0];
        if((da[1] == 0) && (db[1] == 0))
            return da[0] < db[0];
        if(da[1] == 0)
            return true; // infinite tangent
        if(db[1] == 0)
            return false; // infinite tangent
        aa = da[0] / da[1];
        ab = db[0] / db[1];
        if(aa > ab)
            return true;
#else
        //assert((ata > atb) == (aa < ab));
        double aa = atan2(da);
        double ab = atan2(db);
        if(aa < ab)
            return true;
#endif
        if(aa == ab)
            return L2sq(da) < L2sq(db);
        return false;
    }
#else
    bool operator() (Point const& a, Point const&  b)
    {
        // not remove this check or std::sort could generate
        // a segmentation fault because it needs a strict '<'
        // but due to round errors a == b doesn't mean dxy == dyx
        if (a == b) return false;
        Point da = a - o;
        Point db = b - o;
        if (da == -db) return false;
        double dxy = da[X] * db[Y];
        double dyx = da[Y] * db[X];
        if (dxy > dyx) return true;
        else if (dxy < dyx) return false;
        return L2sq(da) < L2sq(db);
    }
#endif
};

//Mathematically incorrect mod, but more useful.
int mod(int i, int l) {
    return i >= 0 ?
           i % l : (i % l) + l;
}
//OPT: usages can often be replaced by conditions

/*** ConvexHull::add_point
 * to add a point we need to find whether the new point extends the boundary, and if so, what it
 * obscures.  Tarjan?  Jarvis?*/
void
ConvexHull::merge(Point p) {
    std::vector<Point> out;

    int len = boundary.size();

    if(len < 2) {
        if(boundary.empty() || boundary[0] != p)
            boundary.push_back(p);
        return;
    }

    bool pushed = false;
 
    bool pre = is_left(p, -1);
    for(int i = 0; i < len; i++) {
        bool cur = is_left(p, i);
        if(pre) {
            if(cur) {
                if(!pushed) {
                    out.push_back(p);
                    pushed = true;
                }
                continue;
            }
            else if(!pushed) {
                out.push_back(p);
                pushed = true;
            }
        }
        out.push_back(boundary[i]);
        pre = cur;
    }

    boundary = out;
}
//OPT: quickly find an obscured point and find the bounds by extending from there.  then push all points not within the bounds in order.
  //OPT: use binary searches to find the actual starts/ends, use known rights as boundaries.  may require cooperation of find_left algo.

/*** ConvexHull::is_clockwise
 * We require that successive pairs of edges always turn right.
 * We return false on collinear points
 * proposed algorithm: walk successive edges and require triangle area is positive.
 */
bool
ConvexHull::is_clockwise() const {
    if(is_degenerate())
        return true;
    Point first = boundary[0];
    Point second = boundary[1];
    for(std::vector<Point>::const_iterator it(boundary.begin()+2), e(boundary.end());
        it != e;) {
        if(SignedTriangleArea(first, second, *it) > 0)
            return false;
        first = second;
        second = *it;
        ++it;
    }
    return true;
}

/*** ConvexHull::top_point_first
 * We require that the first point in the convex hull has the least y coord, and that off all such points on the hull, it has the least x coord.
 * proposed algorithm: track lexicographic minimum while walking the list.
 */
bool
ConvexHull::top_point_first() const {
    if(size() <= 1) return true;
    std::vector<Point>::const_iterator pivot = boundary.begin();
    for(std::vector<Point>::const_iterator it(boundary.begin()+1),
            e(boundary.end());
        it != e; it++) {
        if((*it)[1] < (*pivot)[1])
            pivot = it;
        else if(((*it)[1] == (*pivot)[1]) &&
                ((*it)[0] < (*pivot)[0]))
            pivot = it;
    }
    return pivot == boundary.begin();
}
//OPT: since the Y values are orderly there should be something like a binary search to do this.

bool
ConvexHull::meets_invariants() const {
    return is_clockwise() && top_point_first();
}

/*** ConvexHull::is_degenerate
 * We allow three degenerate cases: empty, 1 point and 2 points.  In many cases these should be handled explicitly.
 */
bool
ConvexHull::is_degenerate() const {
    return boundary.size() < 3;
}


int sgn(double x) {
    if(x == 0) return 0;
    return (x<0)?-1:1;
}

bool same_side(Point L[2], Point  xs[4]) {
    int side = 0;
    for(int i = 0; i < 4; i++) {
        int sn = sgn(SignedTriangleArea(L[0], L[1], xs[i]));
        if(sn &&  !side)
            side = sn;
        else if(sn != side) return false;
    }
    return true;
}

/** find bridging pairs between two convex hulls.
 *   this code is based on Hormoz Pirzadeh's masters thesis.  There is room for optimisation:
 * 1. reduce recomputation
 * 2. use more efficient angle code
 * 3. write as iterator
 */
std::vector<pair<int, int> > bridges(ConvexHull a, ConvexHull b) {
    vector<pair<int, int> > ret;
    
    // 1. find maximal points on a and b
    int ai = 0, bi = 0;
    // 2. find first copodal pair
    double ap_angle = atan2(a[ai+1] - a[ai]);
    double bp_angle = atan2(b[bi+1] - b[bi]);
    Point L[2] = {a[ai], b[bi]};
    while(ai < int(a.size()) || bi < int(b.size())) {
        if(ap_angle == bp_angle) {
            // In the case of parallel support lines, we must consider all four pairs of copodal points
            {
                assert(0); // untested
                Point xs[4] = {a[ai-1], a[ai+1], b[bi-1], b[bi+1]};
                if(same_side(L, xs)) ret.push_back(make_pair(ai, bi));
                xs[2] = b[bi];
                xs[3] = b[bi+2];
                if(same_side(L, xs)) ret.push_back(make_pair(ai, bi));
                xs[0] = a[ai];
                xs[1] = a[ai+2];
                if(same_side(L, xs)) ret.push_back(make_pair(ai, bi));
                xs[2] = b[bi-1];
                xs[3] = b[bi+1];
                if(same_side(L, xs)) ret.push_back(make_pair(ai, bi));
            }
            ai++;
            ap_angle += angle_between(a[ai] - a[ai-1], a[ai+1] - a[ai]);
            L[0] = a[ai];
            bi++;
            bp_angle += angle_between(b[bi] - b[bi-1], b[bi+1] - b[bi]);
            L[1] = b[bi];
            std::cout << "parallel\n";
        } else if(ap_angle < bp_angle) {
            ai++;
            ap_angle += angle_between(a[ai] - a[ai-1], a[ai+1] - a[ai]);
            L[0] = a[ai];
            Point xs[4] = {a[ai-1], a[ai+1], b[bi-1], b[bi+1]};
            if(same_side(L, xs)) ret.push_back(make_pair(ai, bi));
        } else {
            bi++;
            bp_angle += angle_between(b[bi] - b[bi-1], b[bi+1] - b[bi]);
            L[1] = b[bi];
            Point xs[4] = {a[ai-1], a[ai+1], b[bi-1], b[bi+1]};
            if(same_side(L, xs)) ret.push_back(make_pair(ai, bi));
        }
    }
    return ret;
}

unsigned find_bottom_right(ConvexHull const &a) {
    unsigned it = 1;
    while(it < a.boundary.size() &&
          a.boundary[it][Y] > a.boundary[it-1][Y])
        it++;
    return it-1;
}

/*** ConvexHull sweepline_intersection(ConvexHull a, ConvexHull b);
 * find the intersection between two convex hulls.  The intersection is also a convex hull.
 * (Proof: take any two points both in a and in b.  Any point between them is in a by convexity,
 * and in b by convexity, thus in both.  Need to prove still finite bounds.)
 * This algorithm works by sweeping a line down both convex hulls in parallel, working out the left and right edges of the new hull.
 */
ConvexHull sweepline_intersection(ConvexHull const &a, ConvexHull const &b) {
    ConvexHull ret;

    unsigned al = 0;
    unsigned bl = 0;

    while(al+1 < a.boundary.size() &&
          (a.boundary[al+1][Y] > b.boundary[bl][Y])) {
        al++;
    }
    while(bl+1 < b.boundary.size() &&
          (b.boundary[bl+1][Y] > a.boundary[al][Y])) {
        bl++;
    }
    // al and bl now point to the top of the first pair of edges that overlap in y value
    //double sweep_y = std::min(a.boundary[al][Y],
    //                          b.boundary[bl][Y]);
    return ret;
}

/*** ConvexHull intersection(ConvexHull a, ConvexHull b);
 * find the intersection between two convex hulls.  The intersection is also a convex hull.
 * (Proof: take any two points both in a and in b.  Any point between them is in a by convexity,
 * and in b by convexity, thus in both.  Need to prove still finite bounds.)
 */
ConvexHull intersection(ConvexHull /*a*/, ConvexHull /*b*/) {
    ConvexHull ret;
    /*
    int ai = 0, bi = 0;
    int aj = a.boundary.size() - 1;
    int bj = b.boundary.size() - 1;
    */
    /*while (true) {
        if(a[ai]
    }*/
    return ret;
}

template <typename T>
T idx_to_pair(pair<T, T> p, int idx) {
    return idx?p.second:p.first;
}

/*** ConvexHull merge(ConvexHull a, ConvexHull b);
 * find the smallest convex hull that surrounds a and b.
 */
ConvexHull merge(ConvexHull a, ConvexHull b) {
    ConvexHull ret;

    std::cout << "---\n";
    std::vector<pair<int, int> > bpair = bridges(a, b);
    
    // Given our list of bridges {(pb1, qb1), ..., (pbk, qbk)}
    // we start with the highest point in p0, q0, say it is p0.
    // then the merged hull is p0, ..., pb1, qb1, ..., qb2, pb2, ...
    // In other words, either of the two polygons vertices are added in order until the vertex coincides with a bridge point, at which point we swap.

    unsigned state = (a[0][Y] < b[0][Y])?0:1;
    ret.boundary.reserve(a.size() + b.size());
    ConvexHull chs[2] = {a, b};
    unsigned idx = 0;
    
    for(unsigned k = 0; k < bpair.size(); k++) {
        unsigned limit = idx_to_pair(bpair[k], state);
        std::cout << bpair[k].first << " , " << bpair[k].second << "; "
                  << idx << ", " << limit << ", s: "
                  << state
                  << " \n";
        while(idx <= limit) {
            ret.boundary.push_back(chs[state][idx++]);
        }
        state = 1-state;
        idx = idx_to_pair(bpair[k], state);
    }
    while(idx < chs[state].size()) {
        ret.boundary.push_back(chs[state][idx++]);
    }
    return ret;
}

ConvexHull graham_merge(ConvexHull a, ConvexHull b) {
    ConvexHull result;

    // we can avoid the find pivot step because of top_point_first
    if(b.boundary[0] <= a.boundary[0])
        swap(a, b);

    result.boundary = a.boundary;
    result.boundary.insert(result.boundary.end(),
                           b.boundary.begin(), b.boundary.end());

/** if we modified graham scan to work top to bottom as proposed in lect754.pdf we could replace the
 angle sort with a simple merge sort type algorithm. furthermore, we could do the graham scan
 online, avoiding a bunch of memory copies.  That would probably be linear. -- njh*/
    result.angle_sort();
    result.graham_scan();

    return result;
}

ConvexHull andrew_merge(ConvexHull a, ConvexHull b) {
    ConvexHull result;

    // we can avoid the find pivot step because of top_point_first
    if(b.boundary[0] <= a.boundary[0])
        swap(a, b);

    result.boundary = a.boundary;
    result.boundary.insert(result.boundary.end(),
                           b.boundary.begin(), b.boundary.end());

/** if we modified graham scan to work top to bottom as proposed in lect754.pdf we could replace the
 angle sort with a simple merge sort type algorithm. furthermore, we could do the graham scan
 online, avoiding a bunch of memory copies.  That would probably be linear. -- njh*/
    result.andrew_scan();

    return result;
}

//TODO: reinstate
/*ConvexCover::ConvexCover(Path const &sp) : path(&sp) {
    cc.reserve(sp.size());
    for(Geom::Path::const_iterator it(sp.begin()), end(sp.end()); it != end; ++it) {
        cc.push_back(ConvexHull((*it).begin(), (*it).end()));
    }
}*/

double ConvexHull::centroid_and_area(Geom::Point& centroid) const {
    const unsigned n = boundary.size();
    if (n < 2)
        return 0;
    if(n < 3) {
        centroid = (boundary[0] + boundary[1])/2;
        return 0;
    }
    Geom::Point centroid_tmp(0,0);
    double atmp = 0;
    for (unsigned i = n-1, j = 0; j < n; i = j, j++) {
        const double ai = cross(boundary[j], boundary[i]);
        atmp += ai;
        centroid_tmp += (boundary[j] + boundary[i])*ai; // first moment.
    }
    if (atmp != 0) {
        centroid = centroid_tmp / (3 * atmp);
    }
    return atmp / 2;
}

// TODO: This can be made lg(n) using golden section/fibonacci search three starting points, say 0,
// n/2, n-1 construct a new point, say (n/2 + n)/2 throw away the furthest boundary point iterate
// until interval is a single value
Point const * ConvexHull::furthest(Point direction) const {
    Point const * p = &boundary[0];
    double d = dot(*p, direction);
    for(unsigned i = 1; i < boundary.size(); i++) {
        double dd = dot(boundary[i], direction);
        if(d < dd) {
            p = &boundary[i];
            d = dd;
        }
    }
    return p;
}


// returns (a, (b,c)), three points which define the narrowest diameter of the hull as the pair of
// lines going through b,c, and through a, parallel to b,c TODO: This can be made linear time by
// moving point tc incrementally from the previous value (it can only move in one direction).  It
// is currently n*O(furthest)
double ConvexHull::narrowest_diameter(Point &a, Point &b, Point &c) {
    Point tb = boundary.back();
    double d = std::numeric_limits<double>::max();
    for(unsigned i = 0; i < boundary.size(); i++) {
        Point tc = boundary[i];
        Point n = -rot90(tb-tc);
        Point ta = *furthest(n);
        double td = dot(n, ta-tb)/dot(n,n);
        if(td < d) {
            a = ta;
            b = tb;
            c = tc;
            d = td;
        }
        tb = tc;
    }
    return d;
}
#endif

};

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

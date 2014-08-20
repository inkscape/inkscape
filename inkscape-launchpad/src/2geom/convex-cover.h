/**
 * \file
 * \brief Dynamic convex hull structure
 *
 * Copyright 2006 Nathan Hurst <njh@mail.csse.monash.edu.au>
 * Copyright 2006 Michael G. Sloan <mgsloan@gmail.com>
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

#ifndef GEOM_CONVEX_COVER_H
#define GEOM_CONVEX_COVER_H

#include <2geom/point.h>
#include <vector>

namespace Geom{

/* A convex cover is a sequence of convex polygons that completely cover the path.  For now a
 * convex hull class is included here (the convex-hull header is wrong)
 */

/** ConvexHull
 * A convexhull is a convex region - every point between two points in the convex hull is also in
 * the convex hull.  It is defined by a set of points travelling in a clockwise direction.  We require the first point to be top most, and of the topmost, leftmost.

 * An empty hull has no points, we allow a single point or two points degenerate cases.

 * We could provide the centroid as a member for efficient direction determination.  We can update the
 * centroid with all operations with the same time complexity as the operation.
 */

class ConvexHull{
public: // XXX: should be private :)
    // extracts the convex hull of boundary. internal use only
    void find_pivot();
    void angle_sort();
    void graham_scan();
    void andrew_scan();
    void graham();
public:
    std::vector<Point> boundary;
    //Point centroid;

    void merge(Point p);
    bool contains_point(Point p);
    bool strict_contains_point(Point p);

    inline size_t size() const { return boundary.size();}
    inline Point operator[](int i) const {

        int l = boundary.size();
        if(l == 0) return Point();
        return boundary[i >= 0 ? i % l : (i % l) + l];
    }

    /*inline Point &operator[](unsigned i) {
        int l = boundary.size();
        if(l == 0) return Point();
        return boundary[i >= 0 ? i % l : i % l + l];
    }*/

public:
    ConvexHull() {}
    ConvexHull(std::vector<Point> const & points) :
        boundary (points)
    {
        graham();
    }

    template <typename T>
    ConvexHull(T b, T e) :boundary(b,e) {}

    ~ConvexHull()
    {
    }

public:
    /** Is the convex hull clockwise?  We use the definition of clockwise from point.h
    **/
    bool is_clockwise() const;
    bool top_point_first() const;
    bool meets_invariants() const;

    // contains no points
    bool empty() const { return boundary.empty();}

    // contains exactly one point
    bool singular() const { return boundary.size() == 1;}

    //  all points are on a line
    bool linear() const { return boundary.size() == 2;}
    bool is_degenerate() const;

    // area of the convex hull
    double centroid_and_area(Geom::Point& centroid) const;
    double area() const {
        Point tmp;
        return centroid_and_area(tmp);
    }

    // furthest point in a direction (lg time)
    Point const * furthest(Point direction) const;

    bool is_left(Point p, int n);
    bool is_strict_left(Point p, int n);
    int find_left(Point p);
    int find_strict_left(Point p);
    double narrowest_diameter(Point &a, Point &b, Point &c);

};
/** @brief Output operator for points.
 * Prints out all the coordinates. */
inline std::ostream &operator<< (std::ostream &out_file, const Geom::ConvexHull &in_cvx) {
    out_file << "ConvexHull(";
    for(unsigned i = 0; i < in_cvx.size(); i++) {
        out_file << in_cvx.boundary[i] << ", ";
    }
    out_file << ")";
    return out_file;
}

// do two convex hulls intersect?
bool intersectp(ConvexHull a, ConvexHull b);

std::vector<std::pair<int, int> > bridges(ConvexHull a, ConvexHull b);

// find the convex hull intersection
ConvexHull intersection(ConvexHull a, ConvexHull b);
ConvexHull sweepline_intersection(ConvexHull const &a, ConvexHull const &b);

// find the convex hull of a set of convex hulls
ConvexHull merge(ConvexHull a, ConvexHull b);

// naive approach
ConvexHull graham_merge(ConvexHull a, ConvexHull b);

// naive approach
ConvexHull andrew_merge(ConvexHull a, ConvexHull b);

unsigned find_bottom_right(ConvexHull const &a);

/*** Arbitrary transform operator.
 * Take a convex hull and apply an arbitrary convexity preserving transform.
 *  we should be concerned about singular tranforms here.
 */
template <class T> ConvexHull operator*(ConvexHull const &p, T const &m) {
    ConvexHull pr;

    pr.boundary.reserve(p.boundary.size());

    for(unsigned i = 0; i < p.boundary.size(); i++) {
        pr.boundary.push_back(p.boundary[i]*m);
    }
    return pr;
}

ConvexHull clip(ConvexHull const & ch, Point n, double d);

//TODO: reinstate
/*class ConvexCover{
public:
    Path const* path;
    std::vector<ConvexHull> cc;

    ConvexCover(Path const &sp);
};*/

};

#endif //2GEOM_CONVEX_COVER_H

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

/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2004-2009  Monash University
 *
 * --------------------------------------------------------------------
 * Much of the code in this module is based on code published with
 * and/or described in "Computational Geometry in C" (Second Edition),
 * Copyright (C) 1998  Joseph O'Rourke <orourke@cs.smith.edu>
 * --------------------------------------------------------------------
 * The segmentIntersectPoint function is based on code published and
 * described in Franklin Antonio, Faster Line Segment Intersection,
 * Graphics Gems III, p. 199-202, code: p. 500-501.
 * --------------------------------------------------------------------
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * Licensees holding a valid commercial license may use this file in
 * accordance with the commercial license agreement provided with the 
 * library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * Author(s):   Michael Wybrow <mjwybrow@users.sourceforge.net>
*/


#include <cmath>

#include "libavoid/graph.h"
#include "libavoid/geometry.h"
#include "libavoid/assertions.h"

namespace Avoid {



// Returns true iff the point c lies on the closed segment ab.
// To be used when the points are known to be collinear.
//
// Based on the code of 'Between'.
//
bool inBetween(const Point& a, const Point& b, const Point& c)
{
    // We only call this when we know the points are collinear,
    // otherwise we should be checking this here.
    COLA_ASSERT(vecDir(a, b, c, 0.0001) == 0);

    if ((fabs(a.x - b.x) > 1) && (a.x != b.x))
    {
        // not vertical
        return (((a.x < c.x) && (c.x < b.x)) ||
                ((b.x < c.x) && (c.x < a.x)));
    }
    else
    {
        return (((a.y < c.y) && (c.y < b.y)) ||
                ((b.y < c.y) && (c.y < a.y)));
    }
}


// Returns true iff the point c lies on the closed segment ab.
//
bool pointOnLine(const Point& a, const Point& b, const Point& c, 
        const double tolerance)
{
    return (vecDir(a, b, c, tolerance) == 0) && inBetween(a, b, c);
}


// Returns true if the segment cd intersects the segment ab, blocking
// visibility.
//
// Based on the code of 'IntersectProp' and 'Intersect'.
//
bool segmentIntersect(const Point& a, const Point& b, const Point& c,
        const Point& d)
{
    int ab_c = vecDir(a, b, c);
    if (ab_c == 0)
    {
        return false;
    }

    int ab_d = vecDir(a, b, d);
    if (ab_d == 0)
    {
        return false;
    }

    // It's ok for either of the points a or b to be on the line cd,
    // so we don't have to check the other two cases.

    int cd_a = vecDir(c, d, a);
    int cd_b = vecDir(c, d, b);

    // Is an intersection if a and b are on opposite sides of cd,
    // and c and d are on opposite sides of the line ab.
    //
    // Note: this is safe even though the textbook warns about it
    // since, unlike them, our vecDir is equivilent to 'AreaSign'
    // rather than 'Area2'.
    return (((ab_c * ab_d) < 0) && ((cd_a * cd_b) < 0));
}


// Returns true if the segment e1-e2 intersects the shape boundary 
// segment s1-s2, blocking visibility.
//
bool segmentShapeIntersect(const Point& e1, const Point& e2, const Point& s1,
        const Point& s2, bool& seenIntersectionAtEndpoint)
{
    if (segmentIntersect(e1, e2, s1, s2))
    {
        // Basic intersection of segments.
        return true;
    }
    else if ( (((s2 == e1) || pointOnLine(s1, s2, e1)) && 
               (vecDir(s1, s2, e2) != 0)) 
              ||
              (((s2 == e2) || pointOnLine(s1, s2, e2)) &&
               (vecDir(s1, s2, e1) != 0)) )
    {
        // Segments intersect at the endpoint of one of the segments.  We
        // allow this once, but the second one blocks visibility.  Otherwise
        // shapes butted up against each other could have visibility through
        // shapes.
        if (seenIntersectionAtEndpoint)
        {
            return true;
        }
        seenIntersectionAtEndpoint = true;
    }
    return false;
}


// Returns true iff the point p in a valid region that can contain
// shortest paths.  a0, a1, a2 are ordered vertices of a shape.
//
// Based on the code of 'InCone'.
//
bool inValidRegion(bool IgnoreRegions, const Point& a0, const Point& a1,
        const Point& a2, const Point& b)
{
    // r is a0--a1
    // s is a1--a2

    int rSide = vecDir(b, a0, a1);
    int sSide = vecDir(b, a1, a2);

    bool rOutOn = (rSide <= 0);
    bool sOutOn = (sSide <= 0);

    bool rOut = (rSide < 0);
    bool sOut = (sSide < 0);

    if (vecDir(a0, a1, a2) > 0)
    {
        // Convex at a1:
        //
        //   !rO      rO
        //    sO      sO
        //
        // ---s---+
        //        |
        //   !rO  r   rO
        //   !sO  |  !sO
        //
        //
        if (IgnoreRegions)
        {
            return (rOutOn && !sOut) || (!rOut && sOutOn);
        }
        return (rOutOn || sOutOn);
    }
    else
    {
        // Concave at a1:
        //
        //   !rO      rO
        //   !sO     !sO
        //
        //        +---s---
        //        |
        //   !rO  r   rO
        //    sO  |   sO
        //
        //
        return (IgnoreRegions ? false : (rOutOn && sOutOn));
    }
}


// Gives the side of a corner that a point lies on:
//      1   anticlockwise
//     -1   clockwise
// e.g.                     /|s2
//       /s3          -1   / |
//      /                 /  |
//  1  |s2  -1           / 1 |  -1
//     |                /    |
//     |s1           s3/     |s1
//     
int cornerSide(const Point &c1, const Point &c2, const Point &c3,
        const Point& p)
{
    int s123 = vecDir(c1, c2, c3);
    int s12p = vecDir(c1, c2, p);
    int s23p = vecDir(c2, c3, p);

    if (s123 == 1)
    {
        if ((s12p >= 0) && (s23p >= 0))
        {
            return 1;
        }
        return -1;
    }
    else if (s123 == -1)
    {
        if ((s12p <= 0) && (s23p <= 0))
        {
            return -1;
        }
        return 1;
    }

    // c1-c2-c3 are collinear, so just return vecDir from c1-c2
    return s12p;
}


// Returns the Euclidean distance between points a and b.
//
double euclideanDist(const Point& a, const Point& b)
{
    double xdiff = a.x - b.x;
    double ydiff = a.y - b.y;

    return sqrt((xdiff * xdiff) + (ydiff * ydiff));
}

// Returns the Manhattan distance between points a and b.
//
double manhattanDist(const Point& a, const Point& b)
{
    return fabs(a.x - b.x) + fabs(a.y - b.y);
}


// Returns the Euclidean distance between points a and b.
//
double dist(const Point& a, const Point& b)
{
    double xdiff = a.x - b.x;
    double ydiff = a.y - b.y;

    return sqrt((xdiff * xdiff) + (ydiff * ydiff));
}

// Returns the total length of all line segments in the polygon
double totalLength(const Polygon& poly)
{
    double l = 0;
    for (size_t i = 1; i < poly.size(); ++i) 
    {
        l += dist(poly.ps[i-1], poly.ps[i]);
    }
    return l;
}

// Uses the dot-product rule to find the angle (radians) between ab and bc
double angle(const Point& a, const Point& b, const Point& c)
{
    double ux = b.x - a.x,
           uy = b.y - a.y,
           vx = c.x - b.x,
           vy = c.y - b.y,
           lu = sqrt(ux*ux+uy*uy),
           lv = sqrt(vx*vx+vy*vy),
           udotv = ux * vx + uy * vy,
           costheta = udotv / (lu * lv);
    return acos(costheta);
}

// Returns true iff the point q is inside (or on the edge of) the
// polygon argpoly.
//
// This is a fast version that only works for convex shapes.  The
// other version (inPolyGen) is more general.
//
bool inPoly(const Polygon& poly, const Point& q, bool countBorder)
{
    size_t n = poly.size();
    const std::vector<Point>& P = poly.ps;
    bool onBorder = false;
    for (size_t i = 0; i < n; i++)
    {
        // point index; i1 = i-1 mod n
        size_t prev = (i + n - 1) % n;
        int dir = vecDir(P[prev], P[i], q);
        if (dir == -1)
        {
            // Point is outside
            return false;
        }
        // Record if point was on a boundary.
        onBorder |= (dir == 0);
    }
    if (!countBorder && onBorder)
    {
        return false;
    }
    return true;
}


// Returns true iff the point q is inside (or on the edge of) the
// polygon argpoly.
//
// Based on the code of 'InPoly'.
//
bool inPolyGen(const PolygonInterface& argpoly, const Point& q)
{
    // Numbers of right and left edge/ray crossings.
    int Rcross = 0;
    int Lcross = 0;

    // Copy the argument polygon
    Polygon poly = argpoly;
    std::vector<Point>& P = poly.ps;
    size_t    n = poly.size();

    // Shift so that q is the origin. This is done for pedogical clarity.
    for (size_t i = 0; i < n; ++i)
    {
        P[i].x = P[i].x - q.x;
        P[i].y = P[i].y - q.y;
    }

    // For each edge e=(i-1,i), see if crosses ray.
    for (size_t i = 0; i < n; ++i)
    {
        // First see if q=(0,0) is a vertex.
        if ((P[i].x == 0) && (P[i].y == 0))
        {
            // We count a vertex as inside.
            return true;
        }

        // point index; i1 = i-1 mod n
        size_t i1 = ( i + n - 1 ) % n;

        // if e "straddles" the x-axis...
        // The commented-out statement is logically equivalent to the one
        // following.
        // if( ((P[i].y > 0) && (P[i1].y <= 0)) ||
        //         ((P[i1].y > 0) && (P[i].y <= 0)) )

        if ((P[i].y > 0) != (P[i1].y > 0))
        {
            // e straddles ray, so compute intersection with ray.
            double x = (P[i].x * P[i1].y - P[i1].x * P[i].y)
                    / (P[i1].y - P[i].y);

            // crosses ray if strictly positive intersection.
            if (x > 0)
            {
                Rcross++;
            }
        }

        // if e straddles the x-axis when reversed...
        // if( ((P[i].y < 0) && (P[i1].y >= 0)) ||
        //         ((P[i1].y < 0) && (P[i].y >= 0)) )

        if ((P[i].y < 0) != (P[i1].y < 0))
        {
            // e straddles ray, so compute intersection with ray.
            double x = (P[i].x * P[i1].y - P[i1].x * P[i].y)
                    / (P[i1].y - P[i].y);

            // crosses ray if strictly positive intersection.
            if (x < 0)
            {
                Lcross++;
            }
        }
    }

    // q on the edge if left and right cross are not the same parity.
    if ( (Rcross % 2) != (Lcross % 2) )
    {
        // We count the edge as inside.
        return true;
    }

    // Inside iff an odd number of crossings.
    if ((Rcross % 2) == 1)
    {
        return true;
    }

    // Outside.
    return false;
}



// Line Segment Intersection
// Original code by Franklin Antonio 
// 
// The SAME_SIGNS macro assumes arithmetic where the exclusive-or
// operation will work on sign bits.  This works for twos-complement,
// and most other machine arithmetic.
#define SAME_SIGNS( a, b ) \
        (((long) ((unsigned long) a ^ (unsigned long) b)) >= 0 )
// 
int segmentIntersectPoint(const Point& a1, const Point& a2,
        const Point& b1, const Point& b2, double *x, double *y) 
{
    double Ax,Bx,Cx,Ay,By,Cy,d,e,f,num;
    double x1lo,x1hi,y1lo,y1hi;

    Ax = a2.x - a1.x;
    Bx = b1.x - b2.x;

    // X bound box test:
    if (Ax < 0)
    {
        x1lo = a2.x;
        x1hi = a1.x;
    }
    else
    {
        x1hi = a2.x;
        x1lo = a1.x;
    }
    if (Bx > 0)
    {
        if (x1hi < b2.x || b1.x < x1lo) return DONT_INTERSECT;
    }
    else
    {
        if (x1hi < b1.x || b2.x < x1lo) return DONT_INTERSECT;
    }

    Ay = a2.y - a1.y;
    By = b1.y - b2.y;

    // Y bound box test:
    if (Ay < 0)
    {
        y1lo = a2.y;
        y1hi = a1.y;
    }
    else
    {
        y1hi = a2.y;
        y1lo = a1.y;
    }
    if (By > 0)
    {
        if (y1hi < b2.y || b1.y < y1lo) return DONT_INTERSECT;
    }
    else
    {
        if (y1hi < b1.y || b2.y < y1lo) return DONT_INTERSECT;
    }

    Cx = a1.x - b1.x;
    Cy = a1.y - b1.y;
    // alpha numerator:
    d = By*Cx - Bx*Cy;
    // Both denominator:
    f = Ay*Bx - Ax*By;
    // alpha tests:
    if (f > 0)
    {
        if (d < 0 || d > f) return DONT_INTERSECT;
    }
    else
    {
        if (d > 0 || d < f) return DONT_INTERSECT;
    }

    // beta numerator:
    e = Ax*Cy - Ay*Cx;       
    // beta tests:
    if (f > 0)
    {
        if (e < 0 || e > f) return DONT_INTERSECT;
    }
    else
    {
        if (e > 0 || e < f) return DONT_INTERSECT;
    }

    // compute intersection coordinates:

    if (f == 0) return PARALLEL;
    
    // Numerator:
    num = d*Ax;
    // Intersection X:
    *x = a1.x + (num) / f;

    num = d*Ay;
    // Intersection Y:
    *y = a1.y + (num) / f;

    return DO_INTERSECT;
}


// Line Segment Intersection
// Original code by Franklin Antonio 
//
int rayIntersectPoint(const Point& a1, const Point& a2,
        const Point& b1, const Point& b2, double *x, double *y) 
{
    double Ax,Bx,Cx,Ay,By,Cy,d,f,num;

    Ay = a2.y - a1.y;
    By = b1.y - b2.y;
    Ax = a2.x - a1.x;
    Bx = b1.x - b2.x;

    Cx = a1.x - b1.x;
    Cy = a1.y - b1.y;
    // alpha numerator:
    d = By*Cx - Bx*Cy;
    // Both denominator:
    f = Ay*Bx - Ax*By;

    // compute intersection coordinates:

    if (f == 0) return PARALLEL;
    
    // Numerator:
    num = d*Ax;
    // Intersection X:
    *x = a1.x + (num) / f;

    num = d*Ay;
    // Intersection Y:
    *y = a1.y + (num) / f;

    return DO_INTERSECT;
}


}


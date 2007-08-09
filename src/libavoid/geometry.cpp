/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 * Copyright (C) 2004-2006  Michael Wybrow <mjwybrow@users.sourceforge.net>
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
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/

#include "libavoid/graph.h"
#include "libavoid/geometry.h"
#include "libavoid/polyutil.h"

#include <math.h>

namespace Avoid {


Point::Point()
{
}


Point::Point(const double xv, const double yv)
    : x(xv)
    , y(yv)
{
}


bool Point::operator==(const Point& rhs) const
{
    if ((x == rhs.x) && (y == rhs.y))
    {
        return true;
    }
    return false;
}


bool Point::operator!=(const Point& rhs) const
{
    if ((x != rhs.x) || (y != rhs.y))
    {
        return true;
    }
    return false;
}


// Returns true iff the point c lies on the closed segment ab.
//
// Based on the code of 'Between'.
//
static const bool inBetween(const Point& a, const Point& b, const Point& c)
{
    // We only call this when we know the points are collinear,
    // otherwise we should be checking this here.
    assert(vecDir(a, b, c) == 0);

    if (a.x != b.x)
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


// Returns true if the segment cd intersects the segment ab, blocking
// visibility.
//
// Based on the code of 'IntersectProp' and 'Intersect'.
//
bool segmentIntersect(const Point& a, const Point& b, const Point& c,
        const Point& d)
{
    int ab_c = vecDir(a, b, c);
    if ((ab_c == 0) && inBetween(a, b, c))
    {
        return true;
    }

    int ab_d = vecDir(a, b, d);
    if ((ab_d == 0) && inBetween(a, b, d))
    {
        return true;
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

    if (s12p == 0)
    {
        // Case of p being somewhere on c1-c2.
        return s23p;
    }
    if (s23p == 0)
    {
        // Case of p being somewhere on c2-c3.
        return s12p;
    }

    if (s123 == 1)
    {
        if ((s12p == 1) && (s23p == 1))
        {
            return 1;
        }
        return -1;
    }
    else if (s123 == -1)
    {
        if ((s12p == -1) && (s23p == -1))
        {
            return -1;
        }
        return 1;
    }
    // Case of c3 being somewhere on c1-c2.
    return s12p;
}


// Returns the distance between points a and b.
//
double dist(const Point& a, const Point& b)
{
    double xdiff = a.x - b.x;
    double ydiff = a.y - b.y;

    return sqrt((xdiff * xdiff) + (ydiff * ydiff));
}

// Returns the total length of all line segments in the polygon
double totalLength(const Polygn& poly)
{
    double l = 0;
    for (int i = 0; i < poly.pn-1; ++i) {
        l += dist(poly.ps[i], poly.ps[i+1]);
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
bool inPoly(const Polygn& poly, const Point& q)
{
    int n = poly.pn;
    Point *P = poly.ps;
    for (int i = 0; i < n; i++)
    {
        // point index; i1 = i-1 mod n
        int prev = (i + n - 1) % n;
        if (vecDir(P[prev], P[i], q) == -1)
        {
            return false;
        }
    }
    return true;
}


// Returns true iff the point q is inside (or on the edge of) the
// polygon argpoly.
//
// Based on the code of 'InPoly'.
//
bool inPolyGen(const Polygn& argpoly, const Point& q)
{
    // Numbers of right and left edge/ray crossings.
    int Rcross = 0;
    int Lcross = 0;

    // Copy the argument polygon
    Polygn poly = copyPoly(argpoly);
    Point *P = poly.ps;
    int    n = poly.pn;

    // Shift so that q is the origin. This is done for pedogical clarity.
    for (int i = 0; i < n; ++i)
    {
        P[i].x = P[i].x - q.x;
        P[i].y = P[i].y - q.y;
    }

    // For each edge e=(i-1,i), see if crosses ray.
    for (int i = 0; i < n; ++i)
    {
        // First see if q=(0,0) is a vertex.
        if ((P[i].x == 0) && (P[i].y == 0))
        {
            // We count a vertex as inside.
            freePoly(poly);
            return true;
        }

        // point index; i1 = i-1 mod n
        int i1 = ( i + n - 1 ) % n;

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
    freePoly(poly);

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

    double Ax,Bx,Cx,Ay,By,Cy,d,e,f,num,offset;
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
    // aplha tests:
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
    // Round direction:
    offset = SAME_SIGNS(num,f) ? f/2 : -f/2;
    // Intersection X:
    *x = a1.x + (num+offset) / f;

    num = d*Ay;
    offset = SAME_SIGNS(num,f) ? f/2 : -f/2;
    // Intersection Y:
    *y = a1.y + (num+offset) / f;

    return DO_INTERSECT;
}


}


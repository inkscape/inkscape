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
    // since, unlike them, out vecDir is equivilent to 'AreaSign'
    // rather than 'Area2'.
    return (((ab_c * ab_d) < 0) && ((cd_a * cd_b) < 0));
}


// Returns true iff the point p in a valid region that can contain
// shortest paths.  a0, a1, a2 are ordered vertices of a shape.
// This function may seem 'backwards' to the user due to some of
// the code being reversed due to screen cooridinated being the
// opposite of graph paper coords.
// TODO: Rewrite this after checking whether it works for Inkscape.
//
// Based on the code of 'InCone'.
//
bool inValidRegion(bool IgnoreRegions, const Point& a0, const Point& a1,
        const Point& a2, const Point& b)
{
    int rSide = vecDir(b, a0, a1);
    int sSide = vecDir(b, a1, a2);

    bool rOutOn = (rSide >= 0);
    bool sOutOn = (sSide >= 0);

    bool rOut = (rSide > 0);
    bool sOut = (sSide > 0);

    if (vecDir(a0, a1, a2) > 0)
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
    else
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
}


// Returns the distance between points a and b.
//
double dist(const Point& a, const Point& b)
{
    double xdiff = a.x - b.x;
    double ydiff = a.y - b.y;

    return sqrt((xdiff * xdiff) + (ydiff * ydiff));
}


// Returns true iff the point q is inside (or on the edge of) the
// polygon argpoly.
//
// Based on the code of 'InPoly'.
//
bool inPoly(const Polygn& argpoly, const Point& q)
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


}


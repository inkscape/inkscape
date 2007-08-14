/* circle_circle_intersection() *
 * Determine the points where 2 circles in a common plane intersect.
 *
 * int circle_circle_intersection(
 *                                // center and radius of 1st circle
 *                                double x0, double y0, double r0,
 *                                // center and radius of 2nd circle
 *                                double x1, double y1, double r1,
 *                                // 1st intersection point
 *                                double *xi, double *yi,              
 *                                // 2nd intersection point
 *                                double *xi_prime, double *yi_prime)
 *
 * This is a public domain work. 3/26/2005 Tim Voght
 * Ported to lib2geom, 2006 Nathan Hurst
 *
 * Copyright 2006 Nathan Hurst <njh@mail.csse.monash.edu.au>
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
#include <stdio.h>
#include <math.h>
#include "point.h"

namespace Geom{

int circle_circle_intersection(Point X0, double r0,
                               Point X1, double r1,
                               Point & p0, Point & p1)
{
  /* dx and dy are the vertical and horizontal distances between
   * the circle centers.
   */
  Point D = X1 - X0;

  /* Determine the straight-line distance between the centers. */
  double d = L2(D);

  /* Check for solvability. */
  if (d > (r0 + r1))
  {
    /* no solution. circles do not intersect. */
    return 0;
  }
  if (d <= fabs(r0 - r1))
  {
    /* no solution. one circle is contained in the other */
    return 1;
  }
  
  /* 'point 2' is the point where the line through the circle
   * intersection points crosses the line between the circle
   * centers.  
   */

  /* Determine the distance from point 0 to point 2. */
  double a = ((r0*r0) - (r1*r1) + (d*d)) / (2.0 * d) ;

  /* Determine the coordinates of point 2. */
  Point p2 = X0 + D * (a/d);

  /* Determine the distance from point 2 to either of the
   * intersection points.
   */
  double h = sqrt((r0*r0) - (a*a));

  /* Now determine the offsets of the intersection points from
   * point 2.
   */
  Point r = (h/d)*rot90(D);

  /* Determine the absolute intersection points. */
  p0 = p2 + r;
  p1 = p2 - r;

  return 2;
}

};


#ifdef TEST

void run_test(double x0, double y0, double r0,
              double x1, double y1, double r1)
{
  double x3, y3, x3_prime, y3_prime;

  printf("x0=%F, y0=%F, r0=%F, x1=%F, y1=%F, r1=%F :\n",
          x0, y0, r0, x1, y1, r1);
  Geom::Point p0, p1;
  Geom::circle_circle_intersection(Geom::Point(x0, y0), r0, 
				   Geom::Point(x1, y1), r1,
				   p0, p1);
  printf("  x3=%F, y3=%F, x3_prime=%F, y3_prime=%F\n",
            p0[0], p0[1], p1[0], p1[1]);
}

int main(void)
{
  /* Add more! */    
  run_test(-1.0, -1.0, 1.5, 1.0, 1.0, 2.0);
  run_test(1.0, -1.0, 1.5, -1.0, 1.0, 2.0);
  run_test(-1.0, 1.0, 1.5, 1.0, -1.0, 2.0);
  run_test(1.0, 1.0, 1.5, -1.0, -1.0, 2.0);
  exit(0);
}
#endif


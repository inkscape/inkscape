/* Copyright (C) 2001-2015 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "render.h"
#include "greymap.h"
#include "auxiliary.h"

/* ---------------------------------------------------------------------- */
/* routines for anti-aliased rendering of curves */

/* we use the following method. Given a point (x,y) (with real-valued
   coordinates) in the plane, let (xi,yi) be the integer part of the
   coordinates, i.e., xi=floor(x), yi=floor(y). Define a path from
   (x,y) to infinity as follows: path(x,y) =
   (x,y)--(xi+1,y)--(xi+1,yi)--(+infty,yi).  Now as the point (x,y)
   moves smoothly across the plane, the path path(x,y) sweeps
   (non-smoothly) across a certain area. We proportionately blacken
   the area as the path moves "downward", and we whiten the area as
   the path moves "upward". This way, after the point has traversed a
   closed curve, the interior of the curve has been darkened
   (counterclockwise movement) or lightened (clockwise movement). (The
   "grey shift" is actually proportional to the winding number). By
   choosing the above path with mostly integer coordinates, we achieve
   that only pixels close to (x,y) receive grey values and are subject
   to round-off errors. The grey value of pixels far away from (x,y)
   is always in "integer" (where 0=black, 1=white).  As a special
   trick, we keep an accumulator rm->a1, which holds a double value to
   be added to the grey value to be added to the current pixel
   (xi,yi).  Only when changing "current" pixels, we convert this
   double value to an integer. This way we avoid round-off errors at
   the meeting points of line segments. Another speedup measure is
   that we sometimes use the rm->incrow_buf array to postpone
   incrementing or decrementing an entire row. If incrow_buf[y]=x+1!=0,
   then all the pixels (x,y),(x+1,y),(x+2,y),... are scheduled to be
   incremented/decremented (which one is the case will be clear from 
   context). This keeps the greymap operations reasonably local. */

/* allocate a new rendering state */
render_t *render_new(greymap_t *gm) {
  render_t *rm;

  rm = (render_t *) malloc(sizeof(render_t));
  if (!rm) {
    return NULL;
  }
  memset(rm, 0, sizeof(render_t));
  rm->gm = gm;
  rm->incrow_buf = (int *) calloc(gm->h, sizeof(int));
  if (!rm->incrow_buf) {
    free(rm);
    return NULL;
  }
  memset(rm->incrow_buf, 0, gm->h * sizeof(int));
  return rm;
}

/* free a given rendering state. Note: this does not free the
   underlying greymap. */
void render_free(render_t *rm) {
  free(rm->incrow_buf);
  free(rm);
}

/* close path */
void render_close(render_t *rm) {
  if (rm->x0 != rm->x1 || rm->y0 != rm->y1) {
    render_lineto(rm, rm->x0, rm->y0);
  }
  GM_INC(rm->gm, rm->x0i, rm->y0i, (rm->a0+rm->a1)*255);

  /* assert (rm->x0i != rm->x1i || rm->y0i != rm->y1i); */
  
  /* the persistent state is now undefined */
}

/* move point */
void render_moveto(render_t *rm, double x, double y) {
  /* close the previous path */
  render_close(rm);

  rm->x0 = rm->x1 = x;
  rm->y0 = rm->y1 = y;
  rm->x0i = (int)floor(rm->x0);
  rm->x1i = (int)floor(rm->x1);
  rm->y0i = (int)floor(rm->y0);
  rm->y1i = (int)floor(rm->y1);
  rm->a0 = rm->a1 = 0;
}

/* add b to pixels (x,y) and all pixels to the right of it. However,
   use rm->incrow_buf as a buffer to economize on multiple calls */
static void incrow(render_t *rm, int x, int y, int b) {
  int i, x0;

  if (y < 0 || y >= rm->gm->h) {
    return;
  }

  if (x < 0) {
    x = 0;
  } else if (x > rm->gm->w) {
    x = rm->gm->w;
  }
  if (rm->incrow_buf[y] == 0) {
    rm->incrow_buf[y] = x+1; /* store x+1 so that we can use 0 for "vacant" */
    return;
  }
  x0 = rm->incrow_buf[y]-1;
  rm->incrow_buf[y] = 0;
  if (x0 < x) {
    for (i=x0; i<x; i++) {
      GM_INC(rm->gm, i, y, -b);
    }
  } else {
    for (i=x; i<x0; i++) {
      GM_INC(rm->gm, i, y, b);
    }
  }    
}

/* render a straight line */
void render_lineto(render_t *rm, double x2, double y2) {
  int x2i, y2i;
  double t0=2, s0=2;
  int sn, tn;
  double ss=2, ts=2;
  double r0, r1;
  int i, j;
  int rxi, ryi;
  int s;

  x2i = (int)floor(x2);
  y2i = (int)floor(y2);

  sn = abs(x2i - rm->x1i);
  tn = abs(y2i - rm->y1i);

  if (sn) {
    s0 = ((x2>rm->x1 ? rm->x1i+1 : rm->x1i) - rm->x1)/(x2-rm->x1);
    ss = fabs(1.0/(x2-rm->x1));
  }
  if (tn) {
    t0 = ((y2>rm->y1 ? rm->y1i+1 : rm->y1i) - rm->y1)/(y2-rm->y1);
    ts = fabs(1.0/(y2-rm->y1));
  }

  r0 = 0;

  i = 0;
  j = 0;

  rxi = rm->x1i;
  ryi = rm->y1i;

  while (i<sn || j<tn) {
    if (j>=tn || (i<sn && s0+i*ss < t0+j*ts)) {
      r1 = s0+i*ss;
      i++;
      s = 1;
    } else {
      r1 = t0+j*ts;
      j++;
      s = 0;
    }
    /* render line from r0 to r1 segment of (rm->x1,rm->y1)..(x2,y2) */
    
    /* move point to r1 */
    rm->a1 += (r1-r0)*(y2-rm->y1)*(rxi+1-((r0+r1)/2.0*(x2-rm->x1)+rm->x1));

    /* move point across pixel boundary */
    if (s && x2>rm->x1) {
      GM_INC(rm->gm, rxi, ryi, rm->a1*255);
      rm->a1 = 0;
      rxi++;
      rm->a1 += rm->y1+r1*(y2-rm->y1)-ryi;
    } else if (!s && y2>rm->y1) {
      GM_INC(rm->gm, rxi, ryi, rm->a1*255);
      rm->a1 = 0;
      incrow(rm, rxi+1, ryi, 255);
      ryi++;
    } else if (s && x2<=rm->x1) {
      rm->a1 -= rm->y1+r1*(y2-rm->y1)-ryi;
      GM_INC(rm->gm, rxi, ryi, rm->a1*255);
      rm->a1 = 0;
      rxi--;
    } else if (!s && y2<=rm->y1) {
      GM_INC(rm->gm, rxi, ryi, rm->a1*255);
      rm->a1 = 0;
      ryi--;
      incrow(rm, rxi+1, ryi, -255);
    }

    r0 = r1;
  }
  
  /* move point to (x2,y2) */
  
  r1 = 1;
  rm->a1 += (r1-r0)*(y2-rm->y1)*(rxi+1-((r0+r1)/2.0*(x2-rm->x1)+rm->x1));

  rm->x1i = x2i;
  rm->y1i = y2i;
  rm->x1 = x2;
  rm->y1 = y2;

  /* assert (rxi != rm->x1i || ryi != rm->y1i); */
}

/* render a Bezier curve. */
void render_curveto(render_t *rm, double x2, double y2, double x3, double y3, double x4, double y4) {
  double x1, y1, dd0, dd1, dd, delta, e2, epsilon, t;

  x1 = rm->x1;  /* starting point */
  y1 = rm->y1;

  /* we approximate the curve by small line segments. The interval
     size, epsilon, is determined on the fly so that the distance
     between the true curve and its approximation does not exceed the
     desired accuracy delta. */

  delta = .1;  /* desired accuracy, in pixels */

  /* let dd = maximal value of 2nd derivative over curve - this must
     occur at an endpoint. */
  dd0 = sq(x1-2*x2+x3) + sq(y1-2*y2+y3);
  dd1 = sq(x2-2*x3+x4) + sq(y2-2*y3+y4);
  dd = 6*sqrt(max(dd0, dd1));
  e2 = 8*delta <= dd ? 8*delta/dd : 1;
  epsilon = sqrt(e2);  /* necessary interval size */

  for (t=epsilon; t<1; t+=epsilon) {
    render_lineto(rm, x1*cu(1-t)+3*x2*sq(1-t)*t+3*x3*(1-t)*sq(t)+x4*cu(t),
		  y1*cu(1-t)+3*y2*sq(1-t)*t+3*y3*(1-t)*sq(t)+y4*cu(t));
  }
  render_lineto(rm, x4, y4);
}

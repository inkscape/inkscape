/* Copyright (C) 2001-2005 Peter Selinger.
   This file is part of potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

/* This header file collects some general-purpose macros (and static
   inline functions) that are used in various places. */

#ifndef AUXILIARY_H
#define AUXILIARY_H

/* ---------------------------------------------------------------------- */
/* point arithmetic */

#include "potracelib.h"

struct point_s {
  long x;
  long y;
};
typedef struct point_s point_t;

typedef potrace_dpoint_t dpoint_t;

/* convert point_t to dpoint_t */
static inline dpoint_t dpoint(point_t p) {
  dpoint_t res;
  res.x = p.x;
  res.y = p.y;
  return res;
}

/* range over the straight line segment [a,b] when lambda ranges over [0,1] */
static inline dpoint_t interval(double lambda, dpoint_t a, dpoint_t b) {
  dpoint_t res;

  res.x = a.x + lambda * (b.x - a.x);
  res.y = a.y + lambda * (b.y - a.y);
  return res;
}

/* ---------------------------------------------------------------------- */
/* integer arithmetic */

static inline int mod(int a, int n) {
  return a>=n ? a%n : a>=0 ? a : n-1-(-1-a)%n;
}

static inline int floordiv(int a, int n) {
  return a>=0 ? a/n : -1-(-1-a)/n;
}

/* Note: the following work for integers and other numeric types. */
#define sign(x) ((x)>0 ? 1 : (x)<0 ? -1 : 0)
#define abs(a) ((a)>0 ? (a) : -(a))
#define min(a,b) ((a)<(b) ? (a) : (b))
#define max(a,b) ((a)>(b) ? (a) : (b))
#define sq(a) ((a)*(a))
#define cu(a) ((a)*(a)*(a))

#endif /* AUXILIARY_H */

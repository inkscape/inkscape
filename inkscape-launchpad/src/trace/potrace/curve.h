/* Copyright (C) 2001-2011 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

#ifndef CURVE_H
#define CURVE_H

#include "auxiliary.h"

/* vertex is c[1] for tag=POTRACE_CORNER, and the intersection of
   .c[-1][2]..c[0] and c[1]..c[2] for tag=POTRACE_CURVETO. alpha is only
   defined for tag=POTRACE_CURVETO and is the alpha parameter of the curve:
   .c[-1][2]..c[0] = alpha*(.c[-1][2]..vertex), and
   c[2]..c[1] = alpha*(c[2]..vertex).
   Beta is so that (.beta[i])[.vertex[i],.vertex[i+1]] = .c[i][2].
*/

struct privcurve_s {
  int n;            /* number of segments */
  int *tag;         /* tag[n]: POTRACE_CORNER or POTRACE_CURVETO */
  dpoint_t (*c)[3]; /* c[n][i]: control points. 
		       c[n][0] is unused for tag[n]=POTRACE_CORNER */
  /* the remainder of this structure is special to privcurve, and is
     used in EPS debug output and special EPS "short coding". These
     fields are valid only if "alphacurve" is set. */
  int alphacurve;   /* have the following fields been initialized? */
  dpoint_t *vertex; /* for POTRACE_CORNER, this equals c[1] */
  double *alpha;    /* only for POTRACE_CURVETO */
  double *alpha0;   /* "uncropped" alpha parameter - for debug output only */
  double *beta;
};
typedef struct privcurve_s privcurve_t;

struct sums_s {
  double x;
  double y;
  double x2;
  double xy;
  double y2;
};
typedef struct sums_s sums_t;

/* the path structure is filled in with information about a given path
   as it is accumulated and passed through the different stages of the
   Potrace algorithm. Backends only need to read the fcurve and fm
   fields of this data structure, but debugging backends may read
   other fields. */
struct potrace_privpath_s {
  int len;
  point_t *pt;     /* pt[len]: path as extracted from bitmap */
  int *lon;        /* lon[len]: (i,lon[i]) = longest straight line from i */

  int x0, y0;      /* origin for sums */
  sums_t *sums;    /* sums[len+1]: cache for fast summing */

  int m;           /* length of optimal polygon */
  int *po;         /* po[m]: optimal polygon */

  privcurve_t curve;   /* curve[m]: array of curve elements */
  privcurve_t ocurve;  /* ocurve[om]: array of curve elements */
  privcurve_t *fcurve;  /* final curve: this points to either curve or
		       ocurve. Do not free this separately. */
};
typedef struct potrace_privpath_s potrace_privpath_t;

/* shorter names */
typedef potrace_privpath_t privpath_t;
typedef potrace_path_t path_t;

path_t *path_new(void);
void path_free(path_t *p);
void pathlist_free(path_t *plist);
int privcurve_init(privcurve_t *curve, int n);
void privcurve_to_curve(privcurve_t *pc, potrace_curve_t *c);

#endif /* CURVE_H */


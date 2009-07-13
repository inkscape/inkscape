#ifndef __NR_MACROS_H__
#define __NR_MACROS_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <math.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <assert.h>

#ifndef TRUE
#define TRUE (!0)
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef MAX
#define MAX(a,b) (((a) < (b)) ? (b) : (a))
#endif
#ifndef MIN
#define MIN(a,b) (((a) > (b)) ? (b) : (a))
#endif

/** Returns v bounded to within [a, b].  If v is NaN then returns a. 
 *
 *  \pre \a a \<= \a b.
 */
#define NR_CLAMP(v,a,b)	\
	(assert (a <= b),	\
	 ((v) >= (a))	\
	 ? (((v) > (b))	\
	    ? (b)	\
	    : (v))	\
	 : (a))

#undef CLAMP  /* get rid of glib's version, which doesn't handle NaN correctly */
#define CLAMP(v,a,b) NR_CLAMP(v,a,b)

#define NR_DF_TEST_CLOSE(a,b,e) (fabs ((a) - (b)) <= (e))

// Todo: move these into nr-matrix.h
#define NR_RECT_DFLS_TEST_EMPTY(a) (((a)->x0 >= (a)->x1) || ((a)->y0 >= (a)->y1))
#define NR_RECT_DFLS_TEST_EMPTY_REF(a) (((a).x0 >= (a).x1) || ((a).y0 >= (a).y1))
#define NR_RECT_DFLS_TEST_INTERSECT(a,b) (((a)->x0 < (b)->x1) && ((a)->x1 > (b)->x0) && ((a)->y0 < (b)->y1) && ((a)->y1 > (b)->y0))
#define NR_RECT_DFLS_TEST_INTERSECT_REF(a,b) (((a).x0 < (b).x1) && ((a).x1 > (b).x0) && ((a).y0 < (b).y1) && ((a).y1 > (b).y0))
#define NR_RECT_DF_POINT_DF_TEST_INSIDE(r,p) (((p)->x >= (r)->x0) && ((p)->x < (r)->x1) && ((p)->y >= (r)->y0) && ((p)->y < (r)->y1))
#define NR_RECT_LS_POINT_LS_TEST_INSIDE(r,p) (((p)->x >= (r)->x0) && ((p)->x < (r)->x1) && ((p)->y >= (r)->y0) && ((p)->y < (r)->y1))
#define NR_RECT_LS_TEST_INSIDE(r,x,y) ((x >= (r)->x0) && (x < (r)->x1) && (y >= (r)->y0) && (y < (r)->y1))

#define NR_MATRIX_D_FROM_DOUBLE(d) ((NR::Matrix *) &(d)[0])

#endif

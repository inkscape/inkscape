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

#ifndef CLAMP
/** Returns v bounded to within [a, b].  If v is NaN then returns a. 
 *
 *  \pre \a a \<= \a b.
 */
# define CLAMP(v,a,b)	\
	(assert (a <= b),	\
	 ((v) >= (a))	\
	 ? (((v) > (b))	\
	    ? (b)	\
	    : (v))	\
	 : (a))
#endif

#define NR_DF_TEST_CLOSE(a,b,e) (fabs ((a) - (b)) <= (e))

// Todo: move these into nr-matrix.h
#define NR_MATRIX_DF_TEST_TRANSFORM_CLOSE(a,b,e) (NR_DF_TEST_CLOSE ((*(a))[0], (*(b))[0], e) && \
				        NR_DF_TEST_CLOSE ((*(a))[1], (*(b))[1], e) && \
					NR_DF_TEST_CLOSE ((*(a))[2], (*(b))[2], e) && \
					NR_DF_TEST_CLOSE ((*(a))[3], (*(b))[3], e))
#define NR_MATRIX_DF_TEST_TRANSLATE_CLOSE(a,b,e) (NR_DF_TEST_CLOSE ((*(a))[4], (*(b))[4], e) && \
					NR_DF_TEST_CLOSE ((*(a))[5], (*(b))[5], e))
#define NR_MATRIX_DF_TEST_CLOSE(a,b,e) (NR_MATRIX_DF_TEST_TRANSLATE_CLOSE (a, b, e) && \
					NR_MATRIX_DF_TEST_TRANSFORM_CLOSE (a, b, e))

#define NR_RECT_DFLS_TEST_EMPTY(a) (((a)->x0 >= (a)->x1) || ((a)->y0 >= (a)->y1))
#define NR_RECT_DFLS_TEST_INTERSECT(a,b) (((a)->x0 < (b)->x1) && ((a)->x1 > (b)->x0) && ((a)->y0 < (b)->y1) && ((a)->y1 > (b)->y0))
#define NR_RECT_DF_POINT_DF_TEST_INSIDE(r,p) (((p)->x >= (r)->x0) && ((p)->x < (r)->x1) && ((p)->y >= (r)->y0) && ((p)->y < (r)->y1))
#define NR_RECT_LS_POINT_LS_TEST_INSIDE(r,p) (((p)->x >= (r)->x0) && ((p)->x < (r)->x1) && ((p)->y >= (r)->y0) && ((p)->y < (r)->y1))
#define NR_RECT_LS_TEST_INSIDE(r,x,y) ((x >= (r)->x0) && (x < (r)->x1) && (y >= (r)->y0) && (y < (r)->y1))

#define NR_MATRIX_D_TO_DOUBLE(m) ((m)->c)
#define NR_MATRIX_D_FROM_DOUBLE(d) ((NRMatrix *) &(d)[0])

#endif

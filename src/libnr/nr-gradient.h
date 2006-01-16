#ifndef __NR_GRADIENT_H__
#define __NR_GRADIENT_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-matrix.h>
#include <libnr/nr-render.h>

#define NR_GRADIENT_VECTOR_LENGTH 1024

enum {
	NR_GRADIENT_SPREAD_PAD,
	NR_GRADIENT_SPREAD_REFLECT,
	NR_GRADIENT_SPREAD_REPEAT
};

/* Radial */

struct NRRGradientRenderer {
	NRRenderer renderer;
	const unsigned char *vector;
	unsigned int spread;
	NRMatrix px2gs;
	float cx, cy;
	float fx, fy;
	float r;
	float C;
};

NRRenderer *nr_rgradient_renderer_setup (NRRGradientRenderer *rgr,
					 const unsigned char *cv,
					 unsigned int spread,
					 const NRMatrix *gs2px,
					 float cx, float cy,
					 float fx, float fy,
					 float r);



#endif

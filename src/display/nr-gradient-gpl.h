#ifndef __NR_GRADIENT_GPL_H__
#define __NR_GRADIENT_GPL_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
 * Here is GPL code, unlike other libnr wihich is public domain
 */

#include <libnr/nr-gradient.h>

/* Linear */

struct NRLGradientRenderer {
	NRRenderer renderer;
	const unsigned char *vector;
	unsigned int spread;
	double x0, y0;
	double dx, dy;
};

NRRenderer *nr_lgradient_renderer_setup (NRLGradientRenderer *lgr,
					 const unsigned char *cv, 
					 unsigned int spread, 
					 const NRMatrix *gs2px,
					 float x0, float y0,
					 float x1, float y1);



#endif

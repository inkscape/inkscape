#ifndef __NR_GRADIENT_H__
#define __NR_GRADIENT_H__

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
 * Derived in part from public domain code by Lauris Kaplinski
 */

#include <libnr/nr-matrix.h>
#include <libnr/nr-render.h>

#define NR_GRADIENT_VECTOR_BITS 10
#define NR_GRADIENT_VECTOR_LENGTH (1<<NR_GRADIENT_VECTOR_BITS)

enum NRGradientSpread {
    NR_GRADIENT_SPREAD_PAD,
    NR_GRADIENT_SPREAD_REFLECT,
    NR_GRADIENT_SPREAD_REPEAT
};

struct NRGradientRenderer : public NRRenderer {
    const unsigned char *vector;
    unsigned int spread;
};

/* Linear */

struct NRLGradientRenderer : public NRGradientRenderer {
    double x0, y0;
    double dx, dy;
};

NRRenderer *nr_lgradient_renderer_setup (NRLGradientRenderer *lgr,
					 const unsigned char *cv, 
					 unsigned int spread, 
					 const NR::Matrix *gs2px,
					 float x0, float y0,
					 float x1, float y1);

/* Radial */

struct NRRGradientRenderer : public NRGradientRenderer {
    NR::Matrix px2gs;
    float cx, cy;
    float fx, fy;
    float r;
    float C;
};

NRRenderer *nr_rgradient_renderer_setup (NRRGradientRenderer *rgr,
					 const unsigned char *cv,
					 unsigned int spread,
					 const NR::Matrix *gs2px,
					 float cx, float cy,
					 float fx, float fy,
					 float r);



#endif
/*
 * Local Variables:
 * mode:c++
 * c-file-style:"stroustrup"
 * c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
 * indent-tabs-mode:nil
 * fill-column:99
 * End:
 */
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

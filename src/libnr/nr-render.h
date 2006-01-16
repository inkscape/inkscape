#ifndef __NR_RENDER_H__
#define __NR_RENDER_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-pixblock.h>

struct NRRenderer;

typedef void (* NRRenderFunc) (NRRenderer *r, NRPixBlock *pb, NRPixBlock *m);

struct NRRenderer {
	NRRenderFunc render;
};

#define nr_render(r,pb,m) ((NRRenderer *) (r))->render ((NRRenderer *) (r), (pb), (m))

#endif

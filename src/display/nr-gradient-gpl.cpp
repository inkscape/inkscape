#define __NR_GRADIENT_C__

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

#include <libnr/nr-pixops.h>
#include <libnr/nr-pixblock-pixel.h>

#include "nr-gradient-gpl.h"

#define noNR_USE_GENERIC_RENDERER

#define NRG_MASK (NR_GRADIENT_VECTOR_LENGTH - 1)
#define NRG_2MASK ((NR_GRADIENT_VECTOR_LENGTH << 1) - 1)

static void nr_lgradient_render_block (NRRenderer *r, NRPixBlock *pb, NRPixBlock *m);
static void nr_lgradient_render_R8G8B8A8N_EMPTY (NRLGradientRenderer *lgr, unsigned char *px, int x0, int y0, int width, int height, int rs);
static void nr_lgradient_render_R8G8B8A8N (NRLGradientRenderer *lgr, unsigned char *px, int x0, int y0, int width, int height, int rs);
static void nr_lgradient_render_R8G8B8 (NRLGradientRenderer *lgr, unsigned char *px, int x0, int y0, int width, int height, int rs);
static void nr_lgradient_render_generic (NRLGradientRenderer *lgr, NRPixBlock *pb);

NRRenderer *
nr_lgradient_renderer_setup (NRLGradientRenderer *lgr,
			     const unsigned char *cv, 
			     unsigned int spread, 
			     const NRMatrix *gs2px,
			     float x0, float y0,
			     float x1, float y1)
{
	NRMatrix n2gs, n2px, px2n;

	lgr->renderer.render = nr_lgradient_render_block;

	lgr->vector = cv;
	lgr->spread = spread;

	n2gs.c[0] = x1 - x0;
	n2gs.c[1] = y1 - y0;
	n2gs.c[2] = y1 - y0;
	n2gs.c[3] = x0 - x1;
	n2gs.c[4] = x0;
	n2gs.c[5] = y0;

	nr_matrix_multiply (&n2px, &n2gs, gs2px);
	nr_matrix_invert (&px2n, &n2px);

	lgr->x0 = n2px.c[4] - 0.5;
	lgr->y0 = n2px.c[5] - 0.5;
	lgr->dx = px2n.c[0] * NR_GRADIENT_VECTOR_LENGTH;
	lgr->dy = px2n.c[2] * NR_GRADIENT_VECTOR_LENGTH;

	return (NRRenderer *) lgr;
}

static void
nr_lgradient_render_block (NRRenderer *r, NRPixBlock *pb, NRPixBlock *m)
{
	NRLGradientRenderer *lgr;
	int width, height;

	lgr = (NRLGradientRenderer *) r;

	width = pb->area.x1 - pb->area.x0;
	height = pb->area.y1 - pb->area.y0;

#ifdef NR_USE_GENERIC_RENDERER
	nr_lgradient_render_generic (lgr, pb);
#else
	if (pb->empty) {
		switch (pb->mode) {
		case NR_PIXBLOCK_MODE_A8:
			nr_lgradient_render_generic (lgr, pb);
			break;
		case NR_PIXBLOCK_MODE_R8G8B8:
			nr_lgradient_render_generic (lgr, pb);
			break;
		case NR_PIXBLOCK_MODE_R8G8B8A8N:
			nr_lgradient_render_R8G8B8A8N_EMPTY (lgr, NR_PIXBLOCK_PX (pb), pb->area.x0, pb->area.y0, width, height, pb->rs);
			break;
		case NR_PIXBLOCK_MODE_R8G8B8A8P:
			nr_lgradient_render_generic (lgr, pb);
			break;
		default:
			break;
		}
	} else {
		switch (pb->mode) {
		case NR_PIXBLOCK_MODE_A8:
			nr_lgradient_render_generic (lgr, pb);
			break;
		case NR_PIXBLOCK_MODE_R8G8B8:
			nr_lgradient_render_R8G8B8 (lgr, NR_PIXBLOCK_PX (pb), pb->area.x0, pb->area.y0, width, height, pb->rs);
			break;
		case NR_PIXBLOCK_MODE_R8G8B8A8N:
			nr_lgradient_render_R8G8B8A8N (lgr, NR_PIXBLOCK_PX (pb), pb->area.x0, pb->area.y0, width, height, pb->rs);
			break;
		case NR_PIXBLOCK_MODE_R8G8B8A8P:
			nr_lgradient_render_generic (lgr, pb);
			break;
		default:
			break;
		}
	}
#endif
}

static void
nr_lgradient_render_R8G8B8A8N_EMPTY (NRLGradientRenderer *lgr, unsigned char *px, int x0, int y0, int width, int height, int rs)
{
	int x, y;
	double pos;

	for (y = 0; y < height; y++) {
		const unsigned char *s;
		unsigned char *d;
		int idx;
		d = px + y * rs;
		pos = (y + y0 - lgr->y0) * lgr->dy + (0 + x0 - lgr->x0) * lgr->dx;
		if (lgr->spread == NR_GRADIENT_SPREAD_PAD) {
			for (x = 0; x < width; x++) {
				idx = (int) CLAMP (pos, 0, (double) NRG_MASK);
				s = lgr->vector + 4 * idx;
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = s[3];
				d += 4;
				pos += lgr->dx;
			}
		} else if (lgr->spread == NR_GRADIENT_SPREAD_REFLECT) {
			for (x = 0; x < width; x++) {
				idx = (int) ((long long) pos & NRG_2MASK);
				if (idx > NRG_MASK) idx = NRG_2MASK - idx;
				s = lgr->vector + 4 * idx;
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = s[3];
				d += 4;
				pos += lgr->dx;
			}
		} else {
			for (x = 0; x < width; x++) {
				idx = (int) ((long long) pos & NRG_MASK);
				s = lgr->vector + 4 * idx;
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = s[3];
				d += 4;
				pos += lgr->dx;
			}
		}
	}
}

static void
nr_lgradient_render_R8G8B8A8N (NRLGradientRenderer *lgr, unsigned char *px, int x0, int y0, int width, int height, int rs)
{
	int x, y;
	unsigned char *d;
	double pos;

	for (y = 0; y < height; y++) {
		d = px + y * rs;
		pos = (y + y0 - lgr->y0) * lgr->dy + (0 + x0 - lgr->x0) * lgr->dx;
		for (x = 0; x < width; x++) {
			int idx;
			unsigned int ca;
			const unsigned char *s;
			switch (lgr->spread) {
			case NR_GRADIENT_SPREAD_PAD:
				idx = (int) CLAMP (pos, 0, (double) NRG_MASK);
				break;
			case NR_GRADIENT_SPREAD_REFLECT:
				idx = (int) ((long long) pos & NRG_2MASK);
				if (idx > NRG_MASK) idx = NRG_2MASK - idx;
				break;
			case NR_GRADIENT_SPREAD_REPEAT:
				idx = (int) ((long long) pos & NRG_MASK);
				break;
			default:
				idx = 0;
				break;
			}
			/* Full composition */
			s = lgr->vector + 4 * idx;
			if (s[3] == 255) {
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = 255;
			} else if (s[3] != 0) {
				ca = NR_COMPOSEA_112(s[3],d[3]);
				d[0] = NR_COMPOSENNN_111121 (s[0], s[3], d[0], d[3], ca);
				d[1] = NR_COMPOSENNN_111121 (s[1], s[3], d[1], d[3], ca);
				d[2] = NR_COMPOSENNN_111121 (s[2], s[3], d[2], d[3], ca);
				d[3] = NR_NORMALIZE_21(ca);
			}
			d += 4;
			pos += lgr->dx;
		}
	}
}

static void
nr_lgradient_render_R8G8B8 (NRLGradientRenderer *lgr, unsigned char *px, int x0, int y0, int width, int height, int rs)
{
	int x, y;
	unsigned char *d;
	double pos;

	for (y = 0; y < height; y++) {
		d = px + y * rs;
		pos = (y + y0 - lgr->y0) * lgr->dy + (0 + x0 - lgr->x0) * lgr->dx;
		for (x = 0; x < width; x++) {
			int idx;
			const unsigned char *s;
			switch (lgr->spread) {
			case NR_GRADIENT_SPREAD_PAD:
				idx = (int) CLAMP (pos, 0, (double) NRG_MASK);
				break;
			case NR_GRADIENT_SPREAD_REFLECT:
				idx = (int) ((long long) pos & NRG_2MASK);
				if (idx > NRG_MASK) idx = NRG_2MASK - idx;
				break;
			case NR_GRADIENT_SPREAD_REPEAT:
				idx = (int) ((long long) pos & NRG_MASK);
				break;
			default:
				idx = 0;
				break;
			}
			/* Full composition */
			s = lgr->vector + 4 * idx;
			d[0] = NR_COMPOSEN11_1111 (s[0], s[3], d[0]);
			d[1] = NR_COMPOSEN11_1111 (s[1], s[3], d[1]);
			d[2] = NR_COMPOSEN11_1111 (s[2], s[3], d[2]);
			d += 3;
			pos += lgr->dx;
		}
	}
}

static void
nr_lgradient_render_generic (NRLGradientRenderer *lgr, NRPixBlock *pb)
{
	int x, y;
	unsigned char *d;
	double pos;
	int bpp;
	NRPixBlock spb;
	int x0, y0, width, height, rs;

	x0 = pb->area.x0;
	y0 = pb->area.y0;
	width = pb->area.x1 - pb->area.x0;
	height = pb->area.y1 - pb->area.y0;
	rs = pb->rs;

	nr_pixblock_setup_extern (&spb, NR_PIXBLOCK_MODE_R8G8B8A8N, 0, 0, NR_GRADIENT_VECTOR_LENGTH, 1,
				  (unsigned char *) lgr->vector,
				  4 * NR_GRADIENT_VECTOR_LENGTH,
				  0, 0);
    spb.visible_area = pb->visible_area; 
	bpp = (pb->mode == NR_PIXBLOCK_MODE_A8) ? 1 : (pb->mode == NR_PIXBLOCK_MODE_R8G8B8) ? 3 : 4;

	for (y = 0; y < height; y++) {
		d = NR_PIXBLOCK_PX (pb) + y * rs;
		pos = (y + y0 - lgr->y0) * lgr->dy + (0 + x0 - lgr->x0) * lgr->dx;
		for (x = 0; x < width; x++) {
			int idx;
			const unsigned char *s;
			switch (lgr->spread) {
			case NR_GRADIENT_SPREAD_PAD:
				idx = (int) CLAMP (pos, 0, (double) NRG_MASK);
				break;
			case NR_GRADIENT_SPREAD_REFLECT:
				idx = (int) ((long long) pos & NRG_2MASK);
				if (idx > NRG_MASK) idx = NRG_2MASK - idx;
				break;
			case NR_GRADIENT_SPREAD_REPEAT:
				idx = (int) ((long long) pos & NRG_MASK);
				break;
			default:
				idx = 0;
				break;
			}
			s = lgr->vector + 4 * idx;
			nr_compose_pixblock_pixblock_pixel (pb, d, &spb, s);
			d += bpp;
			pos += lgr->dx;
		}
	}

	nr_pixblock_release (&spb);
}


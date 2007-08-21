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

/*
 * Derived in part from public domain code by Lauris Kaplinski
 */

#include <libnr/nr-pixops.h>
#include <libnr/nr-pixblock-pixel.h>
#include <libnr/nr-blit.h>
#include <libnr/nr-gradient.h>
#include <glib/gtypes.h>
#include <stdio.h>

/* Common */

#define noNR_USE_GENERIC_RENDERER

#define NRG_MASK (NR_GRADIENT_VECTOR_LENGTH - 1)
#define NRG_2MASK ((long long) ((NR_GRADIENT_VECTOR_LENGTH << 1) - 1))

static guint32 mls_state=0xa37e5375;

inline NR::Coord noise() {
  guint32 lsb = mls_state & 1;
  guint32 msb = ( lsb << 31 ) ^ ( ( mls_state & 2) << 30 );
  mls_state = ( mls_state >> 1 ) | msb;
  return lsb * 1.75;
}

inline unsigned char const *index_to_pointer(int idx,
                                             unsigned char const *vector)
{
  return vector + 4 * idx;
}

inline unsigned char const *r_to_pointer_pad(NR::Coord r,
                                             unsigned char const *vector)
{
  r += noise();
  return index_to_pointer((int)CLAMP(r, 0, (double)(NR_GRADIENT_VECTOR_LENGTH - 1)), vector);
}

inline unsigned char const *r_to_pointer_repeat(NR::Coord r,
                                                unsigned char const *vector)
{
  r += noise();
  return index_to_pointer((int)((long long)r & NRG_MASK), vector);
}

inline unsigned char const *r_to_pointer_reflect(NR::Coord r,
                                                 unsigned char const *vector)
{
  r += noise();
  int idx = (int) ((long long)r & NRG_2MASK);
  if (idx > NRG_MASK) idx = NRG_2MASK - idx;
  return index_to_pointer(idx, vector);
}

/* Linear */

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
		d = px + y * rs;
		pos = (y + y0 - lgr->y0) * lgr->dy + (0 + x0 - lgr->x0) * lgr->dx;
		if (lgr->spread == NR_GRADIENT_SPREAD_PAD) {
			for (x = 0; x < width; x++) {
                                s = r_to_pointer_pad(pos, lgr->vector);
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = s[3];
				d += 4;
				pos += lgr->dx;
			}
		} else if (lgr->spread == NR_GRADIENT_SPREAD_REFLECT) {
			for (x = 0; x < width; x++) {
                                s = r_to_pointer_reflect(pos, lgr->vector);
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = s[3];
				d += 4;
				pos += lgr->dx;
			}
		} else {
			for (x = 0; x < width; x++) {
                                s = r_to_pointer_repeat(pos, lgr->vector);
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
			unsigned int ca;
			const unsigned char *s;
			switch (lgr->spread) {
			case NR_GRADIENT_SPREAD_PAD:
                                s = r_to_pointer_pad(pos, lgr->vector);
				break;
			case NR_GRADIENT_SPREAD_REFLECT:
                                s = r_to_pointer_reflect(pos, lgr->vector);
				break;
			case NR_GRADIENT_SPREAD_REPEAT:
                                s = r_to_pointer_repeat(pos, lgr->vector);
				break;
			default:
				s = lgr->vector;
				break;
			}
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
			const unsigned char *s;
			switch (lgr->spread) {
			case NR_GRADIENT_SPREAD_PAD:
                                s = r_to_pointer_reflect(pos, lgr->vector);
				break;
			case NR_GRADIENT_SPREAD_REFLECT:
                                s = r_to_pointer_reflect(pos, lgr->vector);
				break;
			case NR_GRADIENT_SPREAD_REPEAT:
                                s = r_to_pointer_repeat(pos, lgr->vector);
				break;
			default:
                                s = lgr->vector;
				break;
			}
			/* Full composition */
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
			const unsigned char *s;
			switch (lgr->spread) {
			case NR_GRADIENT_SPREAD_PAD:
                                s = r_to_pointer_pad(pos, lgr->vector);
				break;
			case NR_GRADIENT_SPREAD_REFLECT:
                                s = r_to_pointer_reflect(pos, lgr->vector);
				break;
			case NR_GRADIENT_SPREAD_REPEAT:
                                s = r_to_pointer_repeat(pos, lgr->vector);
				break;
			default:
                                s = lgr->vector;
				break;
			}
			nr_compose_pixblock_pixblock_pixel (pb, d, &spb, s);
			d += bpp;
			pos += lgr->dx;
		}
	}

	nr_pixblock_release (&spb);
}

/* Radial */

static void nr_rgradient_render_block_symmetric(NRRenderer *r, NRPixBlock *pb, NRPixBlock *m);
static void nr_rgradient_render_block_optimized(NRRenderer *r, NRPixBlock *pb, NRPixBlock *m);
static void nr_rgradient_render_block_end(NRRenderer *r, NRPixBlock *pb, NRPixBlock *m);
static void nr_rgradient_render_generic_symmetric(NRRGradientRenderer *rgr, NRPixBlock *pb);
static void nr_rgradient_render_generic_optimized(NRRGradientRenderer *rgr, NRPixBlock *pb);

NRRenderer *
nr_rgradient_renderer_setup(NRRGradientRenderer *rgr,
                            unsigned char const *cv,
                            unsigned spread,
                            NRMatrix const *gs2px,
                            float cx, float cy,
                            float fx, float fy,
                            float r)
{
    rgr->vector = cv;
    rgr->spread = spread;

    if (r < NR_EPSILON) {
        rgr->renderer.render = nr_rgradient_render_block_end;
    } else if (NR_DF_TEST_CLOSE(cx, fx, NR_EPSILON) &&
               NR_DF_TEST_CLOSE(cy, fy, NR_EPSILON)) {
        rgr->renderer.render = nr_rgradient_render_block_symmetric;

        nr_matrix_invert(&rgr->px2gs, gs2px);
        rgr->px2gs.c[0] *= (NR_GRADIENT_VECTOR_LENGTH / r);
        rgr->px2gs.c[1] *= (NR_GRADIENT_VECTOR_LENGTH / r);
        rgr->px2gs.c[2] *= (NR_GRADIENT_VECTOR_LENGTH / r);
        rgr->px2gs.c[3] *= (NR_GRADIENT_VECTOR_LENGTH / r);
        rgr->px2gs.c[4] -= cx;
        rgr->px2gs.c[5] -= cy;
        rgr->px2gs.c[4] *= (NR_GRADIENT_VECTOR_LENGTH / r);
        rgr->px2gs.c[5] *= (NR_GRADIENT_VECTOR_LENGTH / r);

        rgr->cx = 0.0;
        rgr->cy = 0.0;
        rgr->fx = rgr->cx;
        rgr->fy = rgr->cy;
        rgr->r = 1.0;
    } else {
        rgr->renderer.render = nr_rgradient_render_block_optimized;

        NR::Coord const df = hypot(fx - cx, fy - cy);
        if (df >= r) {
            fx = cx + (fx - cx ) * r / (float) df;
            fy = cy + (fy - cy ) * r / (float) df;
        }

        NRMatrix n2gs;
        n2gs.c[0] = cx - fx;
        n2gs.c[1] = cy - fy;
        n2gs.c[2] = cy - fy;
        n2gs.c[3] = fx - cx;
        n2gs.c[4] = fx;
        n2gs.c[5] = fy;

        NRMatrix n2px;
        nr_matrix_multiply(&n2px, &n2gs, gs2px);
        nr_matrix_invert(&rgr->px2gs, &n2px);

        rgr->cx = 1.0;
        rgr->cy = 0.0;
        rgr->fx = 0.0;
        rgr->fy = 0.0;
        rgr->r = r / (float) hypot(fx - cx, fy - cy);
        rgr->C = 1.0F - rgr->r * rgr->r;
        /* INVARIANT: C < 0 */
        rgr->C = MIN(rgr->C, -NR_EPSILON);
    }

    return (NRRenderer *) rgr;
}

static void
nr_rgradient_render_block_symmetric(NRRenderer *r, NRPixBlock *pb, NRPixBlock *m)
{
    NRRGradientRenderer *rgr = (NRRGradientRenderer *) r;
    nr_rgradient_render_generic_symmetric(rgr, pb);
}

static void
nr_rgradient_render_block_optimized(NRRenderer *r, NRPixBlock *pb, NRPixBlock *m)
{
    NRRGradientRenderer *rgr = (NRRGradientRenderer *) r;
    nr_rgradient_render_generic_optimized(rgr, pb);
}

static void
nr_rgradient_render_block_end(NRRenderer *r, NRPixBlock *pb, NRPixBlock *m)
{
    unsigned char const *c = ((NRRGradientRenderer *) r)->vector + 4 * (NR_GRADIENT_VECTOR_LENGTH - 1);

    nr_blit_pixblock_mask_rgba32(pb, m, (c[0] << 24) | (c[1] << 16) | (c[2] << 8) | c[3]);
}

/*
 * The archetype is following
 *
 * gx gy - pixel coordinates
 * Px Py - coordinates, where Fx Fy - gx gy line intersects with circle
 *
 * (1)  (gx - fx) * (Py - fy) = (gy - fy) * (Px - fx)
 * (2)  (Px - cx) * (Px - cx) + (Py - cy) * (Py - cy) = r * r
 *
 * (3)   Py = (Px - fx) * (gy - fy) / (gx - fx) + fy
 * (4)  (gy - fy) / (gx - fx) = D
 * (5)   Py = D * Px - D * fx + fy
 *
 * (6)   D * fx - fy + cy = N
 * (7)   Px * Px - 2 * Px * cx + cx * cx + (D * Px) * (D * Px) - 2 * (D * Px) * N + N * N = r * r
 * (8)  (D * D + 1) * (Px * Px) - 2 * (cx + D * N) * Px + cx * cx + N * N = r * r
 *
 * (9)   A = D * D + 1
 * (10)  B = -2 * (cx + D * N)
 * (11)  C = cx * cx + N * N - r * r
 *
 * (12)  Px = (-B +- SQRT(B * B - 4 * A * C)) / 2 * A
 */

static void
nr_rgradient_render_generic_symmetric(NRRGradientRenderer *rgr, NRPixBlock *pb)
{
    NR::Coord const dx = rgr->px2gs.c[0];
    NR::Coord const dy = rgr->px2gs.c[1];

    if (pb->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
        for (int y = pb->area.y0; y < pb->area.y1; y++) {
            unsigned char *d = NR_PIXBLOCK_PX(pb) + (y - pb->area.y0) * pb->rs;
            NR::Coord gx = rgr->px2gs.c[0] * pb->area.x0 + rgr->px2gs.c[2] * y + rgr->px2gs.c[4];
            NR::Coord gy = rgr->px2gs.c[1] * pb->area.x0 + rgr->px2gs.c[3] * y + rgr->px2gs.c[5];
            for (int x = pb->area.x0; x < pb->area.x1; x++) {
                NR::Coord const pos = sqrt(((gx*gx) + (gy*gy)));
                unsigned char const *s;
                if (rgr->spread == NR_GRADIENT_SPREAD_REFLECT) {
                    s = r_to_pointer_reflect(pos, rgr->vector);
                } else if (rgr->spread == NR_GRADIENT_SPREAD_REPEAT) {
                    s = r_to_pointer_repeat(pos, rgr->vector);
                } else {
                    s = r_to_pointer_pad(pos, rgr->vector);
                }
                d[0] = NR_COMPOSENPP_1111(s[0], s[3], d[0]);
                d[1] = NR_COMPOSENPP_1111(s[1], s[3], d[1]);
                d[2] = NR_COMPOSENPP_1111(s[2], s[3], d[2]);
                d[3] = NR_COMPOSEA_111(s[3], d[3]);
                d += 4;
                gx += dx;
                gy += dy;
            }
        }
    } else if (pb->mode == NR_PIXBLOCK_MODE_R8G8B8A8N) {
        for (int y = pb->area.y0; y < pb->area.y1; y++) {
            unsigned char *d = NR_PIXBLOCK_PX(pb) + (y - pb->area.y0) * pb->rs;
            NR::Coord gx = rgr->px2gs.c[0] * pb->area.x0 + rgr->px2gs.c[2] * y + rgr->px2gs.c[4];
            NR::Coord gy = rgr->px2gs.c[1] * pb->area.x0 + rgr->px2gs.c[3] * y + rgr->px2gs.c[5];
            for (int x = pb->area.x0; x < pb->area.x1; x++) {
                NR::Coord const pos = sqrt(((gx*gx) + (gy*gy)));
                unsigned char const *s;
                if (rgr->spread == NR_GRADIENT_SPREAD_REFLECT) {
                    s = r_to_pointer_reflect(pos, rgr->vector);
                } else if (rgr->spread == NR_GRADIENT_SPREAD_REPEAT) {
                    s = r_to_pointer_repeat(pos, rgr->vector);
                } else {
                    s = r_to_pointer_pad(pos, rgr->vector);
                }
                if (s[3] == 255) {
                    d[0] = s[0];
                    d[1] = s[1];
                    d[2] = s[2];
                    d[3] = 255;
                } else if (s[3] != 0) {
                    unsigned ca = NR_COMPOSEA_112(s[3], d[3]);
                    d[0] = NR_COMPOSENNN_111121(s[0], s[3], d[0], d[3], ca);
                    d[1] = NR_COMPOSENNN_111121(s[1], s[3], d[1], d[3], ca);
                    d[2] = NR_COMPOSENNN_111121(s[2], s[3], d[2], d[3], ca);
                    d[3] = NR_NORMALIZE_21(ca);
                }
                d += 4;
                gx += dx;
                gy += dy;
            }
        }
    } else {
        NRPixBlock spb;
        nr_pixblock_setup_extern(&spb, NR_PIXBLOCK_MODE_R8G8B8A8N, 0, 0, NR_GRADIENT_VECTOR_LENGTH, 1,
                                 (unsigned char *) rgr->vector,
                                 4 * NR_GRADIENT_VECTOR_LENGTH,
                                 0, 0);
        int const bpp = ( pb->mode == NR_PIXBLOCK_MODE_A8
                          ? 1
                          : pb->mode == NR_PIXBLOCK_MODE_R8G8B8
                          ? 3
                          : 4 );

        for (int y = pb->area.y0; y < pb->area.y1; y++) {
            unsigned char *d = NR_PIXBLOCK_PX(pb) + (y - pb->area.y0) * pb->rs;
            NR::Coord gx = rgr->px2gs.c[0] * pb->area.x0 + rgr->px2gs.c[2] * y + rgr->px2gs.c[4];
            NR::Coord gy = rgr->px2gs.c[1] * pb->area.x0 + rgr->px2gs.c[3] * y + rgr->px2gs.c[5];
            for (int x = pb->area.x0; x < pb->area.x1; x++) {
                NR::Coord const pos = sqrt(((gx*gx) + (gy*gy)));
                unsigned char const *s;
                if (rgr->spread == NR_GRADIENT_SPREAD_REFLECT) {
                    s = r_to_pointer_reflect(pos, rgr->vector);
                } else if (rgr->spread == NR_GRADIENT_SPREAD_REPEAT) {
                    s = r_to_pointer_repeat(pos, rgr->vector);
                } else {
                    s = r_to_pointer_pad(pos, rgr->vector);
                }
                nr_compose_pixblock_pixblock_pixel(pb, d, &spb, s);
                d += bpp;
                gx += dx;
                gy += dy;
            }
        }

        nr_pixblock_release(&spb);
    }
}

static void
nr_rgradient_render_generic_optimized(NRRGradientRenderer *rgr, NRPixBlock *pb)
{
    int const x0 = pb->area.x0;
    int const y0 = pb->area.y0;
    int const x1 = pb->area.x1;
    int const y1 = pb->area.y1;
    int const rs = pb->rs;

    NRPixBlock spb;
    nr_pixblock_setup_extern(&spb, NR_PIXBLOCK_MODE_R8G8B8A8N, 0, 0, NR_GRADIENT_VECTOR_LENGTH, 1,
                             (unsigned char *) rgr->vector,
                             4 * NR_GRADIENT_VECTOR_LENGTH,
                             0, 0);
    int const bpp = ( pb->mode == NR_PIXBLOCK_MODE_A8
                      ? 1
                      : pb->mode == NR_PIXBLOCK_MODE_R8G8B8
                      ? 3
                      : 4 );

    for (int y = y0; y < y1; y++) {
        unsigned char *d = NR_PIXBLOCK_PX(pb) + (y - y0) * rs;
        NR::Coord gx = rgr->px2gs.c[0] * x0 + rgr->px2gs.c[2] * y + rgr->px2gs.c[4];
        NR::Coord gy = rgr->px2gs.c[1] * x0 + rgr->px2gs.c[3] * y + rgr->px2gs.c[5];
        NR::Coord const dx = rgr->px2gs.c[0];
        NR::Coord const dy = rgr->px2gs.c[1];
        for (int x = x0; x < x1; x++) {
            NR::Coord const gx2 = gx * gx;
            NR::Coord const gxy2 = gx2 + gy * gy;
            NR::Coord const qgx2_4 = gx2 - rgr->C * gxy2;
            /* INVARIANT: qgx2_4 >= 0.0 */
            /* qgx2_4 = MAX(qgx2_4, 0.0); */
            NR::Coord const pxgx = gx + sqrt(qgx2_4);
            /* We can safely divide by 0 here */
            /* If we are sure pxgx cannot be -0 */
            NR::Coord const pos = gxy2 / pxgx * NR_GRADIENT_VECTOR_LENGTH;

            unsigned char const *s;
            if (pos < (1U << 31)) {
                if (rgr->spread == NR_GRADIENT_SPREAD_REFLECT) {
                    s = r_to_pointer_reflect(pos, rgr->vector);
                } else if (rgr->spread == NR_GRADIENT_SPREAD_REPEAT) {
                    s = r_to_pointer_repeat(pos, rgr->vector);
                } else {
                    s = r_to_pointer_pad(pos, rgr->vector);
                }
            } else {
                s = index_to_pointer(NR_GRADIENT_VECTOR_LENGTH - 1, rgr->vector);
            }
            nr_compose_pixblock_pixblock_pixel(pb, d, &spb, s);
            d += bpp;

            gx += dx;
            gy += dy;
        }
    }

    nr_pixblock_release(&spb);
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

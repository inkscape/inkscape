#define __NR_BLIT_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include "nr-pixops.h"
#include "nr-compose.h"
#include "nr-blit.h"

void
nr_blit_pixblock_pixblock_alpha (NRPixBlock *d, NRPixBlock *s, unsigned int alpha)
{
	NRRectL clip;
	unsigned char *dpx, *spx;
	int dbpp, sbpp;
	int w, h;

	if (alpha == 0) return;
	if (s->empty) return;
	/* fixme: */
	if (s->mode == NR_PIXBLOCK_MODE_A8) return;
	/* fixme: */
	if (s->mode == NR_PIXBLOCK_MODE_R8G8B8) return;

	/*
	 * Possible variants as of now:
	 *
	 * 0. SRC EP - DST EP *
	 * 1. SRC EP - DST EN *
	 * 2. SRC EP - DST  P *
	 * 3. SRC EP - DST  N *
	 * 4. SRC EN - DST EP *
	 * 5. SRC EN - DST EN *
	 * 6. SRC EN - DST  P *
	 * 7. SRC EN - DST  N *
	 * 8. SRC  P - DST EP *
	 * 9. SRC  P - DST EN *
	 * A. SRC  P - DST  P *
	 * B. SRC  P - DST  N *
	 * C. SRC  N - DST EP *
	 * D. SRC  N - DST EN *
	 * E. SRC  N - DST  P *
	 * F. SRC  N - DST  N *
	 *
	 */

	nr_rect_l_intersect (&clip, &d->area, &s->area);

	if (nr_rect_l_test_empty(clip)) return;

	/* Pointers */
	dbpp = NR_PIXBLOCK_BPP (d);
	dpx = NR_PIXBLOCK_PX (d) + (clip.y0 - d->area.y0) * d->rs + dbpp * (clip.x0 - d->area.x0);
	sbpp = NR_PIXBLOCK_BPP (s);
	spx = NR_PIXBLOCK_PX (s) + (clip.y0 - s->area.y0) * s->rs + sbpp * (clip.x0 - s->area.x0);
	w = clip.x1 - clip.x0;
	h = clip.y1 - clip.y0;

	switch (d->mode) {
	case NR_PIXBLOCK_MODE_A8:
		/* No rendering into alpha at moment */
		break;
	case NR_PIXBLOCK_MODE_R8G8B8:
		if (s->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
			nr_R8G8B8_R8G8B8_R8G8B8A8_P (dpx, w, h, d->rs, spx, s->rs, alpha);
		} else {
			nr_R8G8B8_R8G8B8_R8G8B8A8_N (dpx, w, h, d->rs, spx, s->rs, alpha);
		}
		break;
	case NR_PIXBLOCK_MODE_R8G8B8A8P:
		if (d->empty) {
			if (s->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
				/* Case 8 */
				nr_R8G8B8A8_P_EMPTY_R8G8B8A8_P (dpx, w, h, d->rs, spx, s->rs, alpha);
			} else {
				/* Case C */
				nr_R8G8B8A8_P_EMPTY_R8G8B8A8_N (dpx, w, h, d->rs, spx, s->rs, alpha);
			}
		} else {
			if (s->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
				/* case A */
				nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P (dpx, w, h, d->rs, spx, s->rs, alpha);
			} else {
				/* case E */
				nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N (dpx, w, h, d->rs, spx, s->rs, alpha);
			}
		}
		break;
	case NR_PIXBLOCK_MODE_R8G8B8A8N:
		if (d->empty) {
			if (s->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
				/* Case 9 */
				nr_R8G8B8A8_N_EMPTY_R8G8B8A8_P (dpx, w, h, d->rs, spx, s->rs, alpha);
			} else {
				/* Case D */
				nr_R8G8B8A8_N_EMPTY_R8G8B8A8_N (dpx, w, h, d->rs, spx, s->rs, alpha);
			}
		} else {
			if (s->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
				/* case B */
				nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P (dpx, w, h, d->rs, spx, s->rs, alpha);
			} else {
				/* case F */
				nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N (dpx, w, h, d->rs, spx, s->rs, alpha);
			}
		}
		break;
	}
}

void
nr_blit_pixblock_pixblock_mask (NRPixBlock *d, NRPixBlock *s, NRPixBlock *m)
{
	NRRectL clip;
	unsigned char *dpx, *spx, *mpx;
	int dbpp, sbpp;
	int w, h;

	if (s->empty) return;
	/* fixme: */
	if (s->mode == NR_PIXBLOCK_MODE_A8) return;
	/* fixme: */
	if (s->mode == NR_PIXBLOCK_MODE_R8G8B8) return;

	/*
	 * Possible variants as of now:
	 *
	 * 0. SRC EP - DST EP *
	 * 1. SRC EP - DST EN *
	 * 2. SRC EP - DST  P *
	 * 3. SRC EP - DST  N *
	 * 4. SRC EN - DST EP *
	 * 5. SRC EN - DST EN *
	 * 6. SRC EN - DST  P *
	 * 7. SRC EN - DST  N *
	 * 8. SRC  P - DST EP *
	 * 9. SRC  P - DST EN *
	 * A. SRC  P - DST  P *
	 * B. SRC  P - DST  N *
	 * C. SRC  N - DST EP *
	 * D. SRC  N - DST EN *
	 * E. SRC  N - DST  P *
	 * F. SRC  N - DST  N *
	 *
	 */

	nr_rect_l_intersect (&clip, &d->area, &s->area);
	nr_rect_l_intersect (&clip, &clip, &m->area);

	if (nr_rect_l_test_empty(clip)) return;

	/* Pointers */
	dbpp = NR_PIXBLOCK_BPP (d);
	dpx = NR_PIXBLOCK_PX (d) + (clip.y0 - d->area.y0) * d->rs + dbpp * (clip.x0 - d->area.x0);
	sbpp = NR_PIXBLOCK_BPP (s);
	spx = NR_PIXBLOCK_PX (s) + (clip.y0 - s->area.y0) * s->rs + sbpp * (clip.x0 - s->area.x0);
	mpx = NR_PIXBLOCK_PX (m) + (clip.y0 - m->area.y0) * m->rs + 1 * (clip.x0 - m->area.x0);
	w = clip.x1 - clip.x0;
	h = clip.y1 - clip.y0;

	switch (d->mode) {
	case NR_PIXBLOCK_MODE_A8:
		/* No rendering into alpha at moment */
		break;
	case NR_PIXBLOCK_MODE_R8G8B8:
		if (s->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
			nr_R8G8B8_R8G8B8_R8G8B8A8_P_A8 (dpx, w, h, d->rs, spx, s->rs, mpx, m->rs);
		} else {
			nr_R8G8B8_R8G8B8_R8G8B8A8_N_A8 (dpx, w, h, d->rs, spx, s->rs, mpx, m->rs);
		}
		break;
	case NR_PIXBLOCK_MODE_R8G8B8A8P:
		if (d->empty) {
			if (s->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
				/* Case 8 */
				nr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_A8 (dpx, w, h, d->rs, spx, s->rs, mpx, m->rs);
			} else {
				/* Case C */
				nr_R8G8B8A8_P_EMPTY_R8G8B8A8_N_A8 (dpx, w, h, d->rs, spx, s->rs, mpx, m->rs);
			}
		} else {
			if (s->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
				/* case A */
				nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P_A8 (dpx, w, h, d->rs, spx, s->rs, mpx, m->rs);
			} else {
				/* case E */
				nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_A8 (dpx, w, h, d->rs, spx, s->rs, mpx, m->rs);
			}
		}
		break;
	case NR_PIXBLOCK_MODE_R8G8B8A8N:
		if (d->empty) {
			if (s->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
				/* Case 9 */
				nr_R8G8B8A8_N_EMPTY_R8G8B8A8_P_A8 (dpx, w, h, d->rs, spx, s->rs, mpx, m->rs);
			} else {
				/* Case D */
				nr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_A8 (dpx, w, h, d->rs, spx, s->rs, mpx, m->rs);
			}
		} else {
			if (s->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
				/* case B */
				nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P_A8 (dpx, w, h, d->rs, spx, s->rs, mpx, m->rs);
			} else {
				/* case F */
				nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_A8 (dpx, w, h, d->rs, spx, s->rs, mpx, m->rs);
			}
		}
		break;
	}
}

void
nr_blit_pixblock_mask_rgba32 (NRPixBlock *d, NRPixBlock *m, unsigned long rgba)
{
	if (!(rgba & 0xff)) return;

	if (m) {
		NRRectL clip;
		unsigned char *dpx, *mpx;
		int w, h;

		if (m->mode != NR_PIXBLOCK_MODE_A8) return;

		if (!nr_rect_l_test_intersect(d->area, m->area)) return;

		nr_rect_l_intersect (&clip, &d->area, &m->area);

		/* Pointers */
		dpx = NR_PIXBLOCK_PX (d) + (clip.y0 - d->area.y0) * d->rs + NR_PIXBLOCK_BPP (d) * (clip.x0 - d->area.x0);
		mpx = NR_PIXBLOCK_PX (m) + (clip.y0 - m->area.y0) * m->rs + (clip.x0 - m->area.x0);
		w = clip.x1 - clip.x0;
		h = clip.y1 - clip.y0;

		if (d->empty) {
			if (d->mode == NR_PIXBLOCK_MODE_R8G8B8) {
				nr_R8G8B8_R8G8B8_A8_RGBA32 (dpx, w, h, d->rs, mpx, m->rs, rgba);
			} else if (d->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
				nr_R8G8B8A8_P_EMPTY_A8_RGBA32 (dpx, w, h, d->rs, mpx, m->rs, rgba);
			} else {
				nr_R8G8B8A8_N_EMPTY_A8_RGBA32 (dpx, w, h, d->rs, mpx, m->rs, rgba);
			}
			d->empty = 0;
		} else {
			if (d->mode == NR_PIXBLOCK_MODE_R8G8B8) {
				nr_R8G8B8_R8G8B8_A8_RGBA32 (dpx, w, h, d->rs, mpx, m->rs, rgba);
			} else if (d->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
				nr_R8G8B8A8_P_R8G8B8A8_P_A8_RGBA32 (dpx, w, h, d->rs, mpx, m->rs, rgba);
			} else {
				nr_R8G8B8A8_N_R8G8B8A8_N_A8_RGBA32 (dpx, w, h, d->rs, mpx, m->rs, rgba);
			}
		}
	} else {
		unsigned int r, g, b, a;
		int x, y;
		r = NR_RGBA32_R (rgba);
		g = NR_RGBA32_G (rgba);
		b = NR_RGBA32_B (rgba);
		a = NR_RGBA32_A (rgba);
		for (y = d->area.y0; y < d->area.y1; y++) {
			unsigned char *p;
			p = NR_PIXBLOCK_PX (d) + (y - d->area.y0) * d->rs;
			for (x = d->area.x0; x < d->area.x1; x++) {
				unsigned int da;
				switch (d->mode) {
				case NR_PIXBLOCK_MODE_R8G8B8:
					p[0] = NR_COMPOSEN11_1111 (r, a, p[0]);
					p[1] = NR_COMPOSEN11_1111 (g, a, p[1]);
					p[2] = NR_COMPOSEN11_1111 (b, a, p[2]);
					p += 3;
					break;
				case NR_PIXBLOCK_MODE_R8G8B8A8P:
					p[0] = NR_COMPOSENPP_1111 (r, a, p[0]);
					p[1] = NR_COMPOSENPP_1111 (g, a, p[1]);
					p[2] = NR_COMPOSENPP_1111 (b, a, p[2]);
					p[3] = NR_COMPOSEA_111(a, p[3]);
					p += 4;
					break;
				case NR_PIXBLOCK_MODE_R8G8B8A8N:
					da = NR_COMPOSEA_112(a, p[3]);
					p[0] = NR_COMPOSENNN_111121 (r, a, p[0], p[3], da);
					p[1] = NR_COMPOSENNN_111121 (g, a, p[1], p[3], da);
					p[2] = NR_COMPOSENNN_111121 (b, a, p[2], p[3], da);
					p[3] = NR_NORMALIZE_21(da);
					p += 4;
					break;
				default:
					break;
				}
			}
		}
	}
}


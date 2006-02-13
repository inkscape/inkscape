#define __NR_COMPOSE_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>
#include "nr-pixops.h"

#ifdef WITH_MMX
/* fixme: */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
int nr_have_mmx (void);
void nr_mmx_R8G8B8A8_P_EMPTY_A8_RGBAP (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned char *c);
void nr_mmx_R8G8B8A8_P_R8G8B8A8_P_A8_RGBAP (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned char *c);
void nr_mmx_R8G8B8_R8G8B8_R8G8B8A8_P (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha);
#define NR_PIXOPS_MMX nr_have_mmx ()
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif

void
nr_R8G8B8A8_N_EMPTY_R8G8B8A8_N (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	int r, c;

	for (r = 0; r < h; r++) {
		if (alpha == 0) {
			memset (px, 0x0, 4 * w);
		} else if (alpha == 255) {
			memcpy (px, spx, 4 * w);
		} else {
			const unsigned char *s;
			unsigned char *d;
			d = px;
			s = spx;
			for (c = 0; c < w; c++) {
				*d++ = *s++;
				*d++ = *s++;
				*d++ = *s++;
				*d++ = NR_PREMUL (*s, alpha);
				s++;
			}
		}
		px += rs;
		spx += srs;
	}
}

void
nr_R8G8B8A8_N_EMPTY_R8G8B8A8_P (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	int r, c;

	for (r = 0; r < h; r++) {
		if (alpha == 0) {
			memset (px, 0x0, 4 * w);
		} else {
			const unsigned char *s;
			unsigned char *d;
			s = spx;
			d = px;
			for (c = 0; c < w; c++) {
				unsigned int a;
				a = NR_PREMUL (s[3], alpha);
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = a;
				d += 4;
				s += 4;
			}
			px += rs;
			spx += srs;
		}
	}
}

void
nr_R8G8B8A8_P_EMPTY_R8G8B8A8_N (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	int r, c;

	for (r = 0; r < h; r++) {
		unsigned char *d, *s;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		for (c = 0; c < w; c++) {
			unsigned int a;
			a = (s[3] * alpha + 127) / 255;
			d[0] = (s[0] * a + 127) / 255;
			d[1] = (s[1] * a + 127) / 255;
			d[2] = (s[2] * a + 127) / 255;
			d[3] = a;
			d += 4;
			s += 4;
		}
		px += rs;
		spx += srs;
	}
}

void
nr_R8G8B8A8_P_EMPTY_R8G8B8A8_P (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	int r, c;

	for (r = 0; r < h; r++) {
		unsigned char *d, *s;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		for (c = 0; c < w; c++) {
			if (alpha == 255) {
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = s[3];
			} else {
				d[0] = NR_PREMUL (s[0], alpha);
				d[1] = NR_PREMUL (s[1], alpha);
				d[2] = NR_PREMUL (s[2], alpha);
				d[3] = NR_PREMUL (s[3], alpha);
			}
			d += 4;
			s += 4;
		}
		px += rs;
		spx += srs;
	}
}

void
nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	int r, c;

	for (r = 0; r < h; r++) {
		unsigned char *d, *s;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		for (c = 0; c < w; c++) {
			unsigned int a;
			a = NR_PREMUL (s[3], alpha);
			if (a == 0) {
				/* Transparent FG, NOP */
			} else if ((a == 255) || (d[3] == 0)) {
				/* Full coverage, COPY */
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = a;
			} else {
				unsigned int ca;
				/* Full composition */
				ca = 65025 - (255 - a) * (255 - d[3]);
				d[0] = NR_COMPOSENNN_A7 (s[0], a, d[0], d[3], ca);
				d[1] = NR_COMPOSENNN_A7 (s[1], a, d[1], d[3], ca);
				d[2] = NR_COMPOSENNN_A7 (s[2], a, d[2], d[3], ca);
				d[3] = (ca + 127) / 255;
			}
			d += 4;
			s += 4;
		}
		px += rs;
		spx += srs;
	}
}

void
nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	int r, c;

	for (r = 0; r < h; r++) {
		unsigned char *d, *s;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		for (c = 0; c < w; c++) {
			unsigned int a;
			a = NR_PREMUL (s[3], alpha);
			if (a == 0) {
				/* Transparent FG, NOP */
			} else if ((a == 255) || (d[3] == 0)) {
				/* Full coverage, demul src */
				d[0] = (s[0] * 255 + (s[3] >> 1)) / s[3];
				d[1] = (s[1] * 255 + (s[3] >> 1)) / s[3];
				d[2] = (s[2] * 255 + (s[3] >> 1)) / s[3];
				d[3] = a;
			} else {
				if (alpha == 255) {
					unsigned int ca;
					/* Full composition */
					ca = 65025 - (255 - s[3]) * (255 - d[3]);
					d[0] = NR_COMPOSEPNN_A7 (s[0], s[3], d[0], d[3], ca);
					d[1] = NR_COMPOSEPNN_A7 (s[1], s[3], d[1], d[3], ca);
					d[2] = NR_COMPOSEPNN_A7 (s[2], s[3], d[2], d[3], ca);
					d[3] = (65025 - (255 - s[3]) * (255 - d[3]) + 127) / 255;
				} else {
					// calculate premultiplied from two premultiplieds:
					d[0] = NR_COMPOSEPPP(NR_PREMUL (s[0], alpha), a, NR_PREMUL (d[0], d[3]), 0); // last parameter not used
					d[1] = NR_COMPOSEPPP(NR_PREMUL (s[1], alpha), a, NR_PREMUL (d[1], d[3]), 0);
					d[2] = NR_COMPOSEPPP(NR_PREMUL (s[2], alpha), a, NR_PREMUL (d[2], d[3]), 0);
					// total opacity:
					d[3] = (65025 - (255 - a) * (255 - d[3]) + 127) / 255;
					// un-premultiply channels:
					d[0] = d[0]*255/d[3];
					d[1] = d[1]*255/d[3];
					d[2] = d[2]*255/d[3];
				}
			}
			d += 4;
			s += 4;
		}
		px += rs;
		spx += srs;
	}
}

void
nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	int r, c;

	for (r = 0; r < h; r++) {
		unsigned char *d, *s;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		for (c = 0; c < w; c++) {
			unsigned int a;
			a = NR_PREMUL (s[3], alpha);
			if (a == 0) {
				/* Transparent FG, NOP */
			} else if ((a == 255) || (d[3] == 0)) {
				/* Transparent BG, premul src */
				d[0] = NR_PREMUL (s[0], a);
				d[1] = NR_PREMUL (s[1], a);
				d[2] = NR_PREMUL (s[2], a);
				d[3] = a;
			} else {
				d[0] = NR_COMPOSENPP (s[0], a, d[0], d[3]);
				d[1] = NR_COMPOSENPP (s[1], a, d[1], d[3]);
				d[2] = NR_COMPOSENPP (s[2], a, d[2], d[3]);
				d[3] = (65025 - (255 - a) * (255 - d[3]) + 127) / 255;
			}
			d += 4;
			s += 4;
		}
		px += rs;
		spx += srs;
	}
}

void
nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	int r, c;

	for (r = 0; r < h; r++) {
		unsigned char *d, *s;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		for (c = 0; c < w; c++) {
			unsigned int a;
			a = NR_PREMUL (s[3], alpha);
			if (a == 0) {
				/* Transparent FG, NOP */
			} else if ((a == 255) || (d[3] == 0)) {
				/* Transparent BG, COPY */
				d[0] = NR_PREMUL (s[0], alpha);
				d[1] = NR_PREMUL (s[1], alpha);
				d[2] = NR_PREMUL (s[2], alpha);
				d[3] = NR_PREMUL (s[3], alpha);
			} else {
				if (alpha == 255) {
					/* Simple */
					d[0] = NR_COMPOSEPPP (s[0], s[3], d[0], d[3]);
					d[1] = NR_COMPOSEPPP (s[1], s[3], d[1], d[3]);
					d[2] = NR_COMPOSEPPP (s[2], s[3], d[2], d[3]);
					d[3] = (65025 - (255 - s[3]) * (255 - d[3]) + 127) / 255;
				} else {
					unsigned int c;
					c = NR_PREMUL (s[0], alpha);
					d[0] = NR_COMPOSEPPP (c, a, d[0], d[3]);
					c = NR_PREMUL (s[1], alpha);
					d[1] = NR_COMPOSEPPP (c, a, d[1], d[3]);
					c = NR_PREMUL (s[2], alpha);
					d[2] = NR_COMPOSEPPP (c, a, d[2], d[3]);
					d[3] = (65025 - (255 - a) * (255 - d[3]) + 127) / 255;
				}
			}
			d += 4;
			s += 4;
		}
		px += rs;
		spx += srs;
	}
}

/* Masked operations */

void
nr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_A8 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs)
{
	int x, y;

	for (y = 0; y < h; y++) {
		unsigned char *d, *s, *m;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		m = (unsigned char *) mpx;
		for (x = 0; x < w; x++) {
			d[0] = s[0];
			d[1] = s[1];
			d[2] = s[2];
			d[3] = (s[3] * m[0] + 127) / 255;
			d += 4;
			s += 4;
			m += 1;
		}
		px += rs;
		spx += srs;
		mpx += mrs;
	}
}

void
nr_R8G8B8A8_N_EMPTY_R8G8B8A8_P_A8 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs)
{
	int x, y;

	for (y = 0; y < h; y++) {
		unsigned char *d, *s, *m;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		m = (unsigned char *) mpx;
		for (x = 0; x < w; x++) {
			unsigned int a;
			a = NR_PREMUL (s[3], m[0]);
			if (a == 0) {
				d[3] = 0;
			} else {
				d[0] = (s[0] * 255 + (a >> 1)) / a;
				d[1] = (s[1] * 255 + (a >> 1)) / a;
				d[2] = (s[2] * 255 + (a >> 1)) / a;
				d[3] = a;
			}
			d += 4;
			s += 4;
			m += 1;
		}
		px += rs;
		spx += srs;
		mpx += mrs;
	}
}

void
nr_R8G8B8A8_P_EMPTY_R8G8B8A8_N_A8 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs)
{
	int r, c;

	for (r = 0; r < h; r++) {
		unsigned char *d, *s, *m;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		m = (unsigned char *) mpx;
		for (c = 0; c < w; c++) {
			unsigned int a;
			a = NR_PREMUL (s[3], m[0]);
			d[0] = NR_PREMUL (s[0], a);
			d[1] = NR_PREMUL (s[1], a);
			d[2] = NR_PREMUL (s[2], a);
			d[3] = a;
			d += 4;
			s += 4;
			m += 1;
		}
		px += rs;
		spx += srs;
		mpx += mrs;
	}
}

void
nr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_A8 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs)
{
	int r, c;

	for (r = 0; r < h; r++) {
		unsigned char *d, *s, *m;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		m = (unsigned char *) mpx;
		for (c = 0; c < w; c++) {
			if (m[0] == 255) {
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = s[3];
			} else {
				d[0] = NR_PREMUL (s[0], m[0]);
				d[1] = NR_PREMUL (s[1], m[0]);
				d[2] = NR_PREMUL (s[2], m[0]);
				d[3] = NR_PREMUL (s[3], m[0]);
			}
			d += 4;
			s += 4;
			m += 1;
		}
		px += rs;
		spx += srs;
		mpx += mrs;
	}
}

void
nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_A8 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs)
{
	int r, c;

	for (r = 0; r < h; r++) {
		unsigned char *d, *s, *m;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		m = (unsigned char *) mpx;
		for (c = 0; c < w; c++) {
			unsigned int a;
			a = NR_PREMUL (s[3], m[0]);
			if (a == 0) {
				/* Transparent FG, NOP */
			} else if ((a == 255) || (d[3] == 0)) {
				/* Full coverage, COPY */
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = a;
			} else {
				unsigned int ca;
				/* Full composition */
				ca = 65025 - (255 - a) * (255 - d[3]);
				d[0] = NR_COMPOSENNN_A7 (s[0], a, d[0], d[3], ca);
				d[1] = NR_COMPOSENNN_A7 (s[1], a, d[1], d[3], ca);
				d[2] = NR_COMPOSENNN_A7 (s[2], a, d[2], d[3], ca);
				d[3] = (ca + 127) / 255;
			}
			d += 4;
			s += 4;
			m += 1;
		}
		px += rs;
		spx += srs;
		mpx += mrs;
	}
}

void
nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P_A8 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs)
{
	int r, c;

	for (r = 0; r < h; r++) {
		unsigned char *d, *s, *m;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		m = (unsigned char *) mpx;
		for (c = 0; c < w; c++) {
			unsigned int a;
			a = NR_PREMUL (s[3], m[0]);
			if (a == 0) {
				/* Transparent FG, NOP */
			} else if ((a == 255) || (d[3] == 0)) {
				/* Full coverage, demul src */
				d[0] = (s[0] * 255 + (s[3] >> 1)) / s[3];
				d[1] = (s[1] * 255 + (s[3] >> 1)) / s[3];
				d[2] = (s[2] * 255 + (s[3] >> 1)) / s[3];
				d[3] = a;
			} else {
				if (m[0] == 255) {
					unsigned int ca;
					/* Full composition */
					ca = 65025 - (255 - s[3]) * (255 - d[3]);
					d[0] = NR_COMPOSEPNN_A7 (s[0], s[3], d[0], d[3], ca);
					d[1] = NR_COMPOSEPNN_A7 (s[1], s[3], d[1], d[3], ca);
					d[2] = NR_COMPOSEPNN_A7 (s[2], s[3], d[2], d[3], ca);
					d[3] = (65025 - (255 - s[3]) * (255 - d[3]) + 127) / 255;
				} else {
					// calculate premultiplied from two premultiplieds:
					d[0] = NR_COMPOSEPPP(NR_PREMUL (s[0], m[0]), a, NR_PREMUL (d[0], d[3]), 0); // last parameter not used
					d[1] = NR_COMPOSEPPP(NR_PREMUL (s[1], m[0]), a, NR_PREMUL (d[1], d[3]), 0);
					d[2] = NR_COMPOSEPPP(NR_PREMUL (s[2], m[0]), a, NR_PREMUL (d[2], d[3]), 0);
					// total opacity:
					d[3] = (65025 - (255 - a) * (255 - d[3]) + 127) / 255;
					// un-premultiply channels:
					d[0] = d[0]*255/d[3];
					d[1] = d[1]*255/d[3];
					d[2] = d[2]*255/d[3];
				}
			}
			d += 4;
			s += 4;
			m += 1;
		}
		px += rs;
		spx += srs;
		mpx += mrs;
	}
}

void
nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_A8 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs)
{
	int r, c;

	for (r = 0; r < h; r++) {
		unsigned char *d, *s, *m;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		m = (unsigned char *) mpx;
		for (c = 0; c < w; c++) {
			unsigned int a;
			a = NR_PREMUL (s[3], m[0]);
			if (a == 0) {
				/* Transparent FG, NOP */
			} else if ((a == 255) || (d[3] == 0)) {
				/* Transparent BG, premul src */
				d[0] = NR_PREMUL (s[0], a);
				d[1] = NR_PREMUL (s[1], a);
				d[2] = NR_PREMUL (s[2], a);
				d[3] = a;
			} else {
				d[0] = NR_COMPOSENPP (s[0], a, d[0], d[3]);
				d[1] = NR_COMPOSENPP (s[1], a, d[1], d[3]);
				d[2] = NR_COMPOSENPP (s[2], a, d[2], d[3]);
				d[3] = (65025 - (255 - a) * (255 - d[3]) + 127) / 255;
			}
			d += 4;
			s += 4;
			m += 1;
		}
		px += rs;
		spx += srs;
		mpx += mrs;
	}
}

void
nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P_A8 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs)
{
	int r, c;

	for (r = 0; r < h; r++) {
		unsigned char *d, *s, *m;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		m = (unsigned char *) mpx;
		for (c = 0; c < w; c++) {
			unsigned int a;
			a = NR_PREMUL (s[3], m[0]);
			if (a == 0) {
				/* Transparent FG, NOP */
			} else if ((a == 255) || (d[3] == 0)) {
				/* Transparent BG, COPY */
				d[0] = NR_PREMUL (s[0], m[0]);
				d[1] = NR_PREMUL (s[1], m[0]);
				d[2] = NR_PREMUL (s[2], m[0]);
				d[3] = NR_PREMUL (s[3], m[0]);
			} else {
				if (m[0] == 255) {
					/* Simple */
					d[0] = NR_COMPOSEPPP (s[0], s[3], d[0], d[3]);
					d[1] = NR_COMPOSEPPP (s[1], s[3], d[1], d[3]);
					d[2] = NR_COMPOSEPPP (s[2], s[3], d[2], d[3]);
					d[3] = NR_A7_NORMALIZED(s[3], d[3]);
				} else {
					unsigned int c;
					c = NR_PREMUL (s[0], m[0]);
					d[0] = NR_COMPOSEPPP (c, a, d[0], d[3]);
					c = NR_PREMUL (s[1], m[0]);
					d[1] = NR_COMPOSEPPP (c, a, d[1], d[3]);
					c = NR_PREMUL (s[2], m[0]);
					d[2] = NR_COMPOSEPPP (c, a, d[2], d[3]);
					d[3] = NR_A7_NORMALIZED(a, d[3]);
				}
			}
			d += 4;
			s += 4;
			m += 1;
		}
		px += rs;
		spx += srs;
		mpx += mrs;
	}
}

void
nr_R8G8B8A8_N_EMPTY_A8_RGBA32 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned long rgba)
{
	unsigned int r, g, b, a;
	int x, y;

	r = NR_RGBA32_R (rgba);
	g = NR_RGBA32_G (rgba);
	b = NR_RGBA32_B (rgba);
	a = NR_RGBA32_A (rgba);

	if (a == 0) return;

	for (y = 0; y < h; y++) {
		unsigned char *d, *s;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		for (x = 0; x < w; x++) {
			d[0] = r;
			d[1] = g;
			d[2] = b;
			d[3] = NR_PREMUL (s[0], a);
			d += 4;
			s += 1;
		}
		px += rs;
		spx += srs;
	}
}

void
nr_R8G8B8A8_P_EMPTY_A8_RGBA32 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned long rgba)
{
	unsigned int r, g, b, a;
	int x, y;

	r = NR_RGBA32_R (rgba);
	g = NR_RGBA32_G (rgba);
	b = NR_RGBA32_B (rgba);
	a = NR_RGBA32_A (rgba);

	if (a == 0) return;

#ifdef WITH_MMX
	if (NR_PIXOPS_MMX) {
		unsigned char c[4];
		c[0] = NR_PREMUL (r, a);
		c[1] = NR_PREMUL (g, a);
		c[2] = NR_PREMUL (b, a);
		c[3] = a;
		/* WARNING: MMX composer REQUIRES w > 0 and h > 0 */
		nr_mmx_R8G8B8A8_P_EMPTY_A8_RGBAP (px, w, h, rs, spx, srs, c);
		return;
	}
#endif

	for (y = 0; y < h; y++) {
		unsigned char *d, *s;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		for (x = 0; x < w; x++) {
			unsigned int ca;
			ca = s[0] * a;
			d[0] = (r * ca + 32512) / 65025;
			d[1] = (g * ca + 32512) / 65025;
			d[2] = (b * ca + 32512) / 65025;
			d[3] = (ca + 127) / 255;
			d += 4;
			s += 1;
		}
		px += rs;
		spx += srs;
	}
}

void
nr_R8G8B8_R8G8B8_A8_RGBA32 (unsigned char *px, int w, int h, int rs, const unsigned char *mpx, int mrs, unsigned long rgba)
{
	unsigned int r, g, b, a;
	int x, y;

	r = NR_RGBA32_R (rgba);
	g = NR_RGBA32_G (rgba);
	b = NR_RGBA32_B (rgba);
	a = NR_RGBA32_A (rgba);

	if (a == 0) return;

	for (y = 0; y < h; y++) {
		unsigned char *d, *m;
		d = (unsigned char *) px;
		m = (unsigned char *) mpx;
		for (x = 0; x < w; x++) {
			unsigned int alpha;
			alpha = NR_PREMUL (a, m[0]);
			d[0] = NR_COMPOSEN11 (r, alpha, d[0]);
			d[1] = NR_COMPOSEN11 (g, alpha, d[1]);
			d[2] = NR_COMPOSEN11 (b, alpha, d[2]);
			d += 3;
			m += 1;
		}
		px += rs;
		mpx += mrs;
	}
}

void
nr_R8G8B8A8_N_R8G8B8A8_N_A8_RGBA32 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned long rgba)
{
	unsigned int r, g, b, a;
	int x, y;

	r = NR_RGBA32_R (rgba);
	g = NR_RGBA32_G (rgba);
	b = NR_RGBA32_B (rgba);
	a = NR_RGBA32_A (rgba);

	if (a == 0) return;

	for (y = 0; y < h; y++) {
		unsigned char *d, *s;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		for (x = 0; x < w; x++) {
			unsigned int ca;
			ca = NR_PREMUL (s[0], a);
			if (ca == 0) {
				/* Transparent FG, NOP */
			} else if ((ca == 255) || (d[3] == 0)) {
				/* Full coverage, COPY */
				d[0] = r;
				d[1] = g;
				d[2] = b;
				d[3] = ca;
			} else {
				unsigned int da;
				/* Full composition */
				da = 65025 - (255 - ca) * (255 - d[3]);
				d[0] = NR_COMPOSENNN_A7 (r, ca, d[0], d[3], da);
				d[1] = NR_COMPOSENNN_A7 (g, ca, d[1], d[3], da);
				d[2] = NR_COMPOSENNN_A7 (b, ca, d[2], d[3], da);
				d[3] = (da + 127) / 255;
			}
			d += 4;
			s += 1;
		}
		px += rs;
		spx += srs;
	}
}

void
nr_R8G8B8A8_P_R8G8B8A8_P_A8_RGBA32 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned long rgba)
{
	unsigned int r, g, b, a;
	int x, y;

	if (!(rgba & 0xff)) return;

	r = NR_RGBA32_R (rgba);
	g = NR_RGBA32_G (rgba);
	b = NR_RGBA32_B (rgba);
	a = NR_RGBA32_A (rgba);

#ifdef WITH_MMX
	if (NR_PIXOPS_MMX) {
		unsigned char c[4];
		c[0] = NR_PREMUL (r, a);
		c[1] = NR_PREMUL (g, a);
		c[2] = NR_PREMUL (b, a);
		c[3] = a;
		/* WARNING: MMX composer REQUIRES w > 0 and h > 0 */
		nr_mmx_R8G8B8A8_P_R8G8B8A8_P_A8_RGBAP (px, w, h, rs, spx, srs, c);
		return;
	}
#endif

	for (y = 0; y < h; y++) {
		unsigned char *d, *s;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		for (x = 0; x < w; x++) {
			unsigned int ca;
			ca = NR_PREMUL (s[0], a);
			if (ca == 0) {
				/* Transparent FG, NOP */
			} else if ((ca == 255) || (d[3] == 0)) {
				/* Full coverage, COPY */
				d[0] = NR_PREMUL (r, ca);
				d[1] = NR_PREMUL (g, ca);
				d[2] = NR_PREMUL (b, ca);
				d[3] = ca;
			} else {
				/* Full composition */
				d[0] = NR_COMPOSENPP (r, ca, d[0], d[3]);
				d[1] = NR_COMPOSENPP (g, ca, d[1], d[3]);
				d[2] = NR_COMPOSENPP (b, ca, d[2], d[3]);
				d[3] = (65025 - (255 - ca) * (255 - d[3]) + 127) / 255;
			}
			d += 4;
			s += 1;
		}
		px += rs;
		spx += srs;
	}
}

/* RGB */

void
nr_R8G8B8_R8G8B8_R8G8B8A8_P (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	int r, c;

	if (alpha == 0) return;

#ifdef WITH_MMX
	if (NR_PIXOPS_MMX) {
		/* WARNING: MMX composer REQUIRES w > 0 and h > 0 */
		nr_mmx_R8G8B8_R8G8B8_R8G8B8A8_P (px, w, h, rs, spx, srs, alpha);
		return;
	}
#endif

	for (r = 0; r < h; r++) {
		const unsigned char *s;
		unsigned char *d;
		if (alpha == 255) {
			d = px;
			s = spx;
			for (c = 0; c < w; c++) {
				if (s[3] == 0) {
					/* NOP */
				} else if (s[3] == 255) {
					d[0] = s[0];
					d[1] = s[1];
					d[2] = s[2];
				} else {
					d[0] = NR_COMPOSEP11 (s[0], s[3], d[0]);
					d[1] = NR_COMPOSEP11 (s[1], s[3], d[1]);
					d[2] = NR_COMPOSEP11 (s[2], s[3], d[2]);
				}
				d += 3;
				s += 4;
			}
		} else {
			d = px;
			s = spx;
			for (c = 0; c < w; c++) {
				unsigned int a;
				a = NR_PREMUL (s[3], alpha);
				if (a == 0) {
					/* NOP */
				} else {
					d[0] = NR_COMPOSEP11 (s[0], a, d[0]);
					d[1] = NR_COMPOSEP11 (s[1], a, d[1]);
					d[2] = NR_COMPOSEP11 (s[2], a, d[2]);
				}
				/* a == 255 is impossible, because alpha < 255 */
				d += 3;
				s += 4;
			}
		}
		px += rs;
		spx += srs;
	}
}

void
nr_R8G8B8_R8G8B8_R8G8B8A8_N (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	int r, c;

	for (r = 0; r < h; r++) {
		const unsigned char *s;
		unsigned char *d;
		if (alpha == 0) {
			/* NOP */
		} else if (alpha == 255) {
			d = px;
			s = spx;
			for (c = 0; c < w; c++) {
				if (s[3] == 0) {
					/* NOP */
				} else if (s[3] == 255) {
					d[0] = s[0];
					d[1] = s[1];
					d[2] = s[2];
				} else {
					d[0] = NR_COMPOSEN11 (s[0], s[3], d[0]);
					d[1] = NR_COMPOSEN11 (s[1], s[3], d[1]);
					d[2] = NR_COMPOSEN11 (s[2], s[3], d[2]);
				}
				d += 3;
				s += 4;
			}
		} else {
			d = px;
			s = spx;
			for (c = 0; c < w; c++) {
				unsigned int a;
				a = NR_PREMUL (s[3], alpha);
				if (a == 0) {
					/* NOP */
				} else {
					d[0] = NR_COMPOSEN11 (s[0], a, d[0]);
					d[1] = NR_COMPOSEN11 (s[1], a, d[1]);
					d[2] = NR_COMPOSEN11 (s[2], a, d[2]);
				}
				/* a == 255 is impossible, because alpha < 255 */
				d += 3;
				s += 4;
			}
		}
		px += rs;
		spx += srs;
	}
}

void
nr_R8G8B8_R8G8B8_R8G8B8A8_P_A8 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs)
{
	int x, y;

	for (y = 0; y < h; y++) {
		unsigned char *d, *s, *m;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		m = (unsigned char *) mpx;
		for (x = 0; x < w; x++) {
			unsigned int a;
			a = NR_PREMUL (s[3], m[0]);
			if (a != 0) {
				unsigned int r, g, b;
				r = NR_PREMUL (s[0], m[0]);
				d[0] = NR_COMPOSEP11 (r, a, d[0]);
				g = NR_PREMUL (s[1], m[0]);
				d[1] = NR_COMPOSEP11 (g, a, d[1]);
				b = NR_PREMUL (s[2], m[0]);
				d[2] = NR_COMPOSEP11 (b, a, d[2]);
			}
			d += 3;
			s += 4;
			m += 1;
		}
		px += rs;
		spx += srs;
		mpx += mrs;
	}
}

void
nr_R8G8B8_R8G8B8_R8G8B8A8_N_A8 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs)
{
	int x, y;

	for (y = 0; y < h; y++) {
		unsigned char *d, *s, *m;
		d = (unsigned char *) px;
		s = (unsigned char *) spx;
		m = (unsigned char *) mpx;
		for (x = 0; x < w; x++) {
			unsigned int a;
			a = NR_PREMUL (s[3], m[0]);
			if (a != 0) {
				d[0] = NR_COMPOSEP11 (s[0], a, d[0]);
				d[1] = NR_COMPOSEP11 (s[1], a, d[1]);
				d[2] = NR_COMPOSEP11 (s[2], a, d[2]);
			}
			d += 3;
			s += 4;
			m += 1;
		}
		px += rs;
		spx += srs;
		mpx += mrs;
	}
}



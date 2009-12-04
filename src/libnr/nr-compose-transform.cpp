#define __NR_COMPOSE_TRANSFORM_C__

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

#include "nr-pixops.h"
#include "nr-matrix.h"

/*#ifdef WITH_MMX
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* fixme: */
/*int nr_have_mmx (void);
#define NR_PIXOPS_MMX (1 && nr_have_mmx ())
#ifdef __cplusplus
}
#endif //__cplusplus
#endif
*/

/* fixme: Implement missing (Lauris) */
/* fixme: PREMUL colors before calculating average (Lauris) */

/* Fixed point precision */
#define FBITS 12
#define FBITS_HP 18 // In some places we need a higher precision

void nr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_TRANSFORM (unsigned char *px, int w, int h, int rs,
					       const unsigned char *spx, int sw, int sh, int srs,
					       const NR::Matrix &d2s, unsigned int alpha, int xd, int yd);
void nr_R8G8B8A8_N_EMPTY_R8G8B8A8_P_TRANSFORM (unsigned char *px, int w, int h, int rs,
					       const unsigned char *spx, int sw, int sh, int srs,
					       const NR::Matrix &d2s, unsigned int alpha, int xd, int yd);
void nr_R8G8B8A8_P_EMPTY_R8G8B8A8_N_TRANSFORM (unsigned char *px, int w, int h, int rs,
					       const unsigned char *spx, int sw, int sh, int srs,
					       const NR::Matrix &d2s, unsigned int alpha, int xd, int yd);
void nr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_TRANSFORM (unsigned char *px, int w, int h, int rs,
					       const unsigned char *spx, int sw, int sh, int srs,
					       const NR::Matrix &d2s, unsigned int alpha, int xd, int yd);

void
nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_TRANSFORM (unsigned char *px, int w, int h, int rs,
					       const unsigned char *spx, int sw, int sh, int srs,
					       const NR::Matrix &d2s, unsigned int alpha, int xd, int yd)
{
	int xsize, ysize, size, dbits;
	long FFs_x_x, FFs_x_y, FFs_y_x, FFs_y_y, FFs__x, FFs__y;
	long FFs_x_x_S, FFs_x_y_S, FFs_y_x_S, FFs_y_y_S;
	/* Subpixel positions */
	int FF_sx_S[256];
	int FF_sy_S[256];
	unsigned char *d0;
	int FFsx0, FFsy0;
	int x, y;

	if (alpha == 0) return;

	xsize = (1 << xd);
	ysize = (1 << yd);
	size = xsize * ysize;
	dbits = xd + yd;

	/* Set up fixed point matrix */
	FFs_x_x = (long) (d2s[0] * (1 << FBITS) + 0.5);
	FFs_x_y = (long) (d2s[1] * (1 << FBITS) + 0.5);
	FFs_y_x = (long) (d2s[2] * (1 << FBITS) + 0.5);
	FFs_y_y = (long) (d2s[3] * (1 << FBITS) + 0.5);
	FFs__x = (long) (d2s[4] * (1 << FBITS) + 0.5);
	FFs__y = (long) (d2s[5] * (1 << FBITS) + 0.5);

	FFs_x_x_S = FFs_x_x >> xd;
	FFs_x_y_S = FFs_x_y >> xd;
	FFs_y_x_S = FFs_y_x >> yd;
	FFs_y_y_S = FFs_y_y >> yd;

	/* Set up subpixel matrix */
	/* fixme: We can calculate that in floating point (Lauris) */
	for (y = 0; y < ysize; y++) {
		for (x = 0; x < xsize; x++) {
			FF_sx_S[y * xsize + x] = FFs_x_x_S * x + FFs_y_x_S * y;
			FF_sy_S[y * xsize + x] = FFs_x_y_S * x + FFs_y_y_S * y;
		}
	}

	d0 = px;
	FFsx0 = FFs__x;
	FFsy0 = FFs__y;

	for (y = 0; y < h; y++) {
		unsigned char *d;
		long FFsx, FFsy;
		d = d0;
		FFsx = FFsx0;
		FFsy = FFsy0;
		for (x = 0; x < w; x++) {
			unsigned int r, g, b, a;
			long sx, sy;
			int i;
			r = g = b = a = 0;
			for (i = 0; i < size; i++) {
				sx = (FFsx + FF_sx_S[i]) >> FBITS;
				if ((sx >= 0) && (sx < sw)) {
					sy = (FFsy + FF_sy_S[i]) >> FBITS;
					if ((sy >= 0) && (sy < sh)) {
						const unsigned char *s;
						unsigned int ca;
						s = spx + sy * srs + sx * 4;
						ca = NR_PREMUL_112 (s[3], alpha);
						r += NR_PREMUL_121 (s[0], ca);
						g += NR_PREMUL_121 (s[1], ca);
						b += NR_PREMUL_121 (s[2], ca);
						a += NR_NORMALIZE_21(ca);
					}
				}
			}
			a >>= dbits;
			if (a != 0) {
				r = r >> dbits;
				g = g >> dbits;
				b = b >> dbits;
				if (a == 255) {
					/* Transparent BG, premul src */
					d[0] = r;
					d[1] = g;
					d[2] = b;
					d[3] = a;
				} else {
					unsigned int ca;
					/* Full composition */
					ca = NR_COMPOSEA_112(a, d[3]);
					d[0] = NR_COMPOSENNN_111121 (r, a, d[0], d[3], ca);
					d[1] = NR_COMPOSENNN_111121 (g, a, d[1], d[3], ca);
					d[2] = NR_COMPOSENNN_111121 (b, a, d[2], d[3], ca);
					d[3] = NR_NORMALIZE_21(ca);
				}
			}
			/* Advance pointers */
			FFsx += FFs_x_x;
			FFsy += FFs_x_y;
			d += 4;
		}
		FFsx0 += FFs_y_x;
		FFsy0 += FFs_y_y;
		d0 += rs;
	}
}

void nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P_TRANSFORM (unsigned char *px, int w, int h, int rs,
						    const unsigned char *spx, int sw, int sh, int srs,
						    const NR::Matrix &d2s, unsigned int alpha, int xd, int yd);

static void
nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM_0 (unsigned char *px, int w, int h, int rs,
						 const unsigned char *spx, int sw, int sh, int srs,
						 const long long *FFd2s, unsigned int alpha)
{
    unsigned char *d0;
	long long FFsx0, FFsy0;
	int x, y;

	d0 = px;
	FFsx0 = FFd2s[4];
	FFsy0 = FFd2s[5];

	for (y = 0; y < h; y++) {
		unsigned char *d;
		long long FFsx, FFsy;
		d = d0;
		FFsx = FFsx0;
		FFsy = FFsy0;
		for (x = 0; x < w; x++) {
			long sx, sy;
			sx = long(FFsx >> FBITS_HP);
			if ((sx >= 0) && (sx < sw)) {
				sy = long(FFsy >> FBITS_HP);
				if ((sy >= 0) && (sy < sh)) {
					const unsigned char *s;
					unsigned int a;
					s = spx + sy * srs + sx * 4;
					a = NR_PREMUL_112 (s[3], alpha);
					if (a != 0) {
						if ((a == 255*255) || (d[3] == 0)) {
							/* Transparent BG, premul src */
							d[0] = NR_PREMUL_121 (s[0], a);
							d[1] = NR_PREMUL_121 (s[1], a);
							d[2] = NR_PREMUL_121 (s[2], a);
							d[3] = NR_NORMALIZE_21(a);
						} else {
							d[0] = NR_COMPOSENPP_1211 (s[0], a, d[0]);
							d[1] = NR_COMPOSENPP_1211 (s[1], a, d[1]);
							d[2] = NR_COMPOSENPP_1211 (s[2], a, d[2]);
							d[3] = NR_COMPOSEA_211(a, d[3]);
						}
					}
				}
			}
			/* Advance pointers */
			FFsx += FFd2s[0];
			FFsy += FFd2s[1];
			d += 4;
		}
		FFsx0 += FFd2s[2];
		FFsy0 += FFd2s[3];
		d0 += rs;
	}
}

static void
nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM_n (unsigned char *px, int w, int h, int rs,
						 const unsigned char *spx, int sw, int sh, int srs,
						 const long long *FFd2s, const long *FF_S, unsigned int alpha, int dbits)
{
	int size;
	unsigned char *d0;
	long long FFsx0, FFsy0;
	int x, y;

	size = (1 << dbits);
    unsigned alpha_rounding_fix = size * 255;
    unsigned rgb_rounding_fix = size * (255 * 256);
    if (alpha > 127) ++alpha;

	d0 = px;
	FFsx0 = FFd2s[4];
	FFsy0 = FFd2s[5];

	for (y = 0; y < h; y++) {
		unsigned char *d;
		long long FFsx, FFsy;
		d = d0;
		FFsx = FFsx0;
		FFsy = FFsy0;
		for (x = 0; x < w; x++) {
			unsigned int r, g, b, a;
			int i;
			r = g = b = a = 0;
			for (i = 0; i < size; i++) {
				long sx, sy;
				sx = (long (FFsx >> (FBITS_HP - FBITS)) + FF_S[2 * i]) >> FBITS;
				if ((sx >= 0) && (sx < sw)) {
					sy = (long (FFsy >> (FBITS_HP - FBITS)) + FF_S[2 * i + 1]) >> FBITS;
					if ((sy >= 0) && (sy < sh)) {
						const unsigned char *s;
						unsigned int ca;
						s = spx + sy * srs + sx * 4;
						ca = NR_PREMUL_112(s[3], alpha);
						r += NR_PREMUL_123(s[0], ca);
						g += NR_PREMUL_123(s[1], ca);
						b += NR_PREMUL_123(s[2], ca);
						a += ca;
					}
				}
			}
			a = (a + alpha_rounding_fix) >> (8 + dbits);
			if (a != 0) {
				r = (r + rgb_rounding_fix) >> (16 + dbits);
				g = (g + rgb_rounding_fix) >> (16 + dbits);
				b = (b + rgb_rounding_fix) >> (16 + dbits);
				if ((a == 255) || (d[3] == 0)) {
					/* Transparent BG, premul src */
					d[0] = r;
					d[1] = g;
					d[2] = b;
					d[3] = a;
				} else {
					d[0] = NR_COMPOSEPPP_1111 (r, a, d[0]);
					d[1] = NR_COMPOSEPPP_1111 (g, a, d[1]);
					d[2] = NR_COMPOSEPPP_1111 (b, a, d[2]);
					d[3] = NR_COMPOSEA_111(a, d[3]);
				}
			}
			/* Advance pointers */
			FFsx += FFd2s[0];
			FFsy += FFd2s[1];
			d += 4;
		}
		FFsx0 += FFd2s[2];
		FFsy0 += FFd2s[3];
		d0 += rs;
	}
}

void nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM (unsigned char *px, int w, int h, int rs,
						    const unsigned char *spx, int sw, int sh, int srs,
						    const NR::Matrix &d2s, unsigned int alpha, int xd, int yd)
{
	int dbits;
	long FFd2s[6];
	long long FFd2s_HP[6]; // with higher precision
	int i;

	if (alpha == 0) return;

	dbits = xd + yd;

	for (i = 0; i < 6; i++) {
		FFd2s[i] = (long) (d2s[i] * (1 << FBITS) + 0.5);
		FFd2s_HP[i] = (long long) (d2s[i] * (1 << FBITS_HP) + 0.5);;
	}

	if (dbits == 0) {
		nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM_0 (px, w, h, rs, spx, sw, sh, srs, FFd2s_HP, alpha);
	} else {
		int xsize, ysize;
		long FFs_x_x_S, FFs_x_y_S, FFs_y_x_S, FFs_y_y_S;
		long FF_S[2 * 256];
		int x, y;

		xsize = (1 << xd);
		ysize = (1 << yd);

		FFs_x_x_S = FFd2s[0] >> xd;
		FFs_x_y_S = FFd2s[1] >> xd;
		FFs_y_x_S = FFd2s[2] >> yd;
		FFs_y_y_S = FFd2s[3] >> yd;

		/* Set up subpixel matrix */
		/* fixme: We can calculate that in floating point (Lauris) */
		for (y = 0; y < ysize; y++) {
			for (x = 0; x < xsize; x++) {
				FF_S[2 * (y * xsize + x)] = FFs_x_x_S * x + FFs_y_x_S * y;
				FF_S[2 * (y * xsize + x) + 1] = FFs_x_y_S * x + FFs_y_y_S * y;
			}
		}

		nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM_n (px, w, h, rs, spx, sw, sh, srs, FFd2s_HP, FF_S, alpha, dbits);
	}
}

void nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P_TRANSFORM (unsigned char *px, int w, int h, int rs,
						    const unsigned char *spx, int sw, int sh, int srs,
						    const NR::Matrix &d2s, unsigned int alpha, int xd, int yd);

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

// Naming: nr_RESULT_BACKGROUND_FOREGROUND_extra

void
nr_R8G8B8A8_N_EMPTY_R8G8B8A8_N (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	unsigned int r, c;

	for (r = h; r > 0; r--) {
		if (alpha == 0) {
			memset(px, 0x0, 4 * w);
		} else if (alpha == 255) {
			memcpy(px, spx, 4 * w);
		} else {
			unsigned char *d = px;
			const unsigned char *s = spx;
			for (c = w; c > 0; c--) {
				*d++ = *s++;
				*d++ = *s++;
				*d++ = *s++;
				*d++ = NR_PREMUL_111(*s, alpha);
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
	unsigned int r, c;

	for (r = h; r > 0; r--) {
		if (alpha == 0) {
			memset(px, 0x0, 4 * w);
		} else {
			unsigned char *d = px;
			const unsigned char *s = spx;
			for (c = w; c > 0; c--) {
				if (s[3] == 0) {
					d[3] = 0;
				} else if (s[3] == 255) {
					memcpy(d, s, 4);
				} else {
					d[0] = NR_DEMUL_111(s[0], s[3]);
					d[1] = NR_DEMUL_111(s[1], s[3]);
					d[2] = NR_DEMUL_111(s[2], s[3]);
					d[3] = NR_PREMUL_111(s[3], alpha);
				}
				d += 4;
				s += 4;
			}
		}
		px += rs;
		spx += srs;
	}
}

void
nr_R8G8B8A8_P_EMPTY_R8G8B8A8_N (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	unsigned int r, c;

	for (r = h; r > 0; r--) {
		unsigned char *d = px;
		const unsigned char *s = spx;
		if (alpha == 0) {
			memset(px, 0x0, 4 * w);
		} else if (alpha == 255) {
			for (c = w; c > 0; c--) {
				d[0] = NR_PREMUL_111(s[0], s[3]);
				d[1] = NR_PREMUL_111(s[1], s[3]);
				d[2] = NR_PREMUL_111(s[2], s[3]);
				d[3] = s[3];
				d += 4;
				s += 4;
			}
		} else {
			for (c = w; c > 0; c--) {
				if (s[3] == 0) {
					memset(d, 0, 4);
				} else {
					unsigned int a;
					a = NR_PREMUL_112(s[3], alpha);
					d[0] = NR_PREMUL_121(s[0], a);
					d[1] = NR_PREMUL_121(s[1], a);
					d[2] = NR_PREMUL_121(s[2], a);
					d[3] = NR_NORMALIZE_21(a);
				}
				d += 4;
				s += 4;
			}
		}
		px += rs;
		spx += srs;
	}
}

void
nr_R8G8B8A8_P_EMPTY_R8G8B8A8_P (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	unsigned int r, c;

	for (r = h; r > 0; r--) {
		if (alpha == 0) {
			memset(px, 0x0, 4 * w);
		} else if (alpha == 255) {
			memcpy(px, spx, 4 * w);
		} else {
			unsigned char *d = px;
			const unsigned char *s = spx;
			for (c = w; c > 0; c--) {
				d[0] = NR_PREMUL_111(s[0], alpha);
				d[1] = NR_PREMUL_111(s[1], alpha);
				d[2] = NR_PREMUL_111(s[2], alpha);
				d[3] = NR_PREMUL_111(s[3], alpha);
				d += 4;
				s += 4;
			}
		}
		px += rs;
		spx += srs;
	}
}

void
nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	unsigned int r, c;

	if (alpha == 0) {
		/* NOP */
	} else if (alpha == 255) {
		for (r = h; r > 0; r--) {
			unsigned char *d = px;
			const unsigned char *s = spx;
			for (c = w; c > 0; c--) {
				if (s[3] == 0) {
					/* Transparent FG, NOP */
				} else if ((s[3] == 255) || (d[3] == 0)) {
					/* Full coverage, COPY */
					memcpy(d, s, 4);
				} else {
					/* Full composition */
					unsigned int ca;
					ca = NR_COMPOSEA_112(s[3], d[3]);
					d[0] = NR_COMPOSENNN_111121(s[0], s[3], d[0], d[3], ca);
					d[1] = NR_COMPOSENNN_111121(s[1], s[3], d[1], d[3], ca);
					d[2] = NR_COMPOSENNN_111121(s[2], s[3], d[2], d[3], ca);
					d[3] = NR_NORMALIZE_21(ca);
				}
				d += 4;
				s += 4;
			}
			px += rs;
			spx += srs;
		}
	} else {
		for (r = h; r > 0; r--) {
			unsigned char *d = px;
			const unsigned char *s = spx;
			for (c = w; c > 0; c--) {
				unsigned int a;
				a = NR_PREMUL_112(s[3], alpha);
				if (a == 0) {
					/* Transparent FG, NOP */
				} else if ((a == 255*255) || (d[3] == 0)) {
					/* Full coverage, COPY */
					d[0] = s[0];
					d[1] = s[1];
					d[2] = s[2];
					d[3] = NR_NORMALIZE_21(a);
				} else {
					/* Full composition */
					unsigned int ca;
					ca = NR_COMPOSEA_213(a, d[3]);
					d[0] = NR_COMPOSENNN_121131(s[0], a, d[0], d[3], ca);
					d[1] = NR_COMPOSENNN_121131(s[1], a, d[1], d[3], ca);
					d[2] = NR_COMPOSENNN_121131(s[2], a, d[2], d[3], ca);
					d[3] = NR_NORMALIZE_31(ca);
				}
				d += 4;
				s += 4;
			}
			px += rs;
			spx += srs;
		}
	}
}

void
nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	unsigned int r, c;

	if (alpha == 0) {
		/* NOP */
	} else if (alpha == 255) {
		for (r = h; r > 0; r--) {
			unsigned char *d = px;
			const unsigned char *s = spx;
			for (c = w; c > 0; c--) {
				if (s[3] == 0) {
					/* Transparent FG, NOP */
				} else if (s[3] == 255) {
					/* Full coverage, demul src */
					//   dc' = ((1 - sa) * da*dc + sc)/da' = sc/da' = sc
					//   da' = 1 - (1 - sa) * (1 - da) = 1 - 0 * (1 - da) = 1
					memcpy(d, s, 4);
				} else if (d[3] == 0) {
					/* Full coverage, demul src */
					//   dc' = ((1 - sa) * da*dc + sc)/da' = sc/da' = sc/sa = sc/sa
					//   da' = 1 - (1 - sa) * (1 - da) = 1 - (1 - sa) = sa
					d[0] = NR_DEMUL_111(s[0], s[3]);
					d[1] = NR_DEMUL_111(s[1], s[3]);
					d[2] = NR_DEMUL_111(s[2], s[3]);
					d[3] = s[3];
				} else {
					/* Full composition */
					//   dc' = ((1 - sa) * da*dc + sc)/da' = ((1 - sa) * da*dc + sc)/da'
					//   da' = 1 - (1 - sa) * (1 - da) = 1 - (1 - sa) * (1 - da)
					unsigned int da = NR_COMPOSEA_112(s[3], d[3]);
					d[0] = NR_COMPOSEPNN_111121(s[0], s[3], d[0], d[3], da);
					d[1] = NR_COMPOSEPNN_111121(s[1], s[3], d[1], d[3], da);
					d[2] = NR_COMPOSEPNN_111121(s[2], s[3], d[2], d[3], da);
					d[3] = NR_NORMALIZE_21(da);
				}
				d += 4;
				s += 4;
			}
			px += rs;
			spx += srs;
		}
	} else {
		for (r = h; r > 0; r--) {
			unsigned char *d = px;
			const unsigned char *s = spx;
			for (c = w; c > 0; c--) {
				unsigned int a;
				a = NR_PREMUL_112(s[3], alpha);
				if (a == 0) {
					/* Transparent FG, NOP */
				} else if (d[3] == 0) {
					/* Full coverage, demul src */
					//   dc' = ((1 - alpha*sa) * da*dc + alpha*sc)/da' = alpha*sc/da' = alpha*sc/(alpha*sa) = sc/sa
					//   da' = 1 - (1 - alpha*sa) * (1 - da) = 1 - (1 - alpha*sa) = alpha*sa
					d[0] = NR_DEMUL_111(s[0], s[3]);
					d[1] = NR_DEMUL_111(s[1], s[3]);
					d[2] = NR_DEMUL_111(s[2], s[3]);
					d[3] = NR_NORMALIZE_21(a);
				} else {
					//   dc' = ((1 - alpha*sa) * da*dc + alpha*sc)/da'
					//   da' = 1 - (1 - alpha*sa) * (1 - da)
					unsigned int da = NR_COMPOSEA_213(a, d[3]);
					d[0] = NR_COMPOSEPNN_221131(NR_PREMUL_112(s[0], alpha), a, d[0], d[3], da);
					d[1] = NR_COMPOSEPNN_221131(NR_PREMUL_112(s[1], alpha), a, d[1], d[3], da);
					d[2] = NR_COMPOSEPNN_221131(NR_PREMUL_112(s[2], alpha), a, d[2], d[3], da);
					d[3] = NR_NORMALIZE_31(da);
				}
				d += 4;
				s += 4;
			}
			px += rs;
			spx += srs;
		}
	}
}

void
nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	unsigned int r, c;

	if (alpha == 0) {
		/* NOP */
	} else if (alpha == 255) {
		for (r = h; r > 0; r--) {
			unsigned char *d = px;
			const unsigned char *s = spx;
			for (c = w; c > 0; c--) {
				if (s[3] == 0) {
					/* Transparent FG, NOP */
				} else if (s[3] == 255) {
					/* Opaque FG, COPY */
					//   dc' = (1 - sa) * dc + sa*sc = sa*sc = sc
					//   da' = 1 - (1 - sa) * (1 - da) = 1 - 0 * (1 - da) = 1 (= sa)
					memcpy(d, s, 4);
				} else if (d[3] == 0) {
					/* Transparent BG, premul src */
					//   dc' = (1 - sa) * dc + sa*sc = sa*sc
					//   da' = 1 - (1 - sa) * (1 - da) = 1 - (1 - sa) = sa
					d[0] = NR_PREMUL_111(s[0], s[3]);
					d[1] = NR_PREMUL_111(s[1], s[3]);
					d[2] = NR_PREMUL_111(s[2], s[3]);
					d[3] = s[3];
				} else {
					//   dc' = (1 - sa) * dc + sa*sc
					//   da' = 1 - (1 - sa) * (1 - da)
					d[0] = NR_COMPOSENPP_1111(s[0], s[3], d[0]);
					d[1] = NR_COMPOSENPP_1111(s[1], s[3], d[1]);
					d[2] = NR_COMPOSENPP_1111(s[2], s[3], d[2]);
					d[3] = NR_COMPOSEA_111(s[3], d[3]);
				}
				d += 4;
				s += 4;
			}
			px += rs;
			spx += srs;
		}
	} else {
		for (r = h; r > 0; r--) {
			unsigned char *d = px;
			const unsigned char *s = spx;
			for (c = w; c > 0; c--) {
				unsigned int a;
				a = NR_PREMUL_112 (s[3], alpha);
				if (a == 0) {
					/* Transparent FG, NOP */
				} else if (d[3] == 0) {
					/* Transparent BG, premul src */
					//   dc' = (1 - alpha*sa) * dc + alpha*sa*sc = alpha*sa*sc
					//   da' = 1 - (1 - alpha*sa) * (1 - da) = 1 - (1 - alpha*sa) = alpha*sa
					d[0] = NR_PREMUL_121(s[0], a);
					d[1] = NR_PREMUL_121(s[1], a);
					d[2] = NR_PREMUL_121(s[2], a);
					d[3] = NR_NORMALIZE_21(a);
				} else {
					//   dc' = (1 - alpha*sa) * dc + alpha*sa*sc
					//   da' = 1 - (1 - alpha*sa) * (1 - da)
					d[0] = NR_COMPOSENPP_1211(s[0], a, d[0]);
					d[1] = NR_COMPOSENPP_1211(s[1], a, d[1]);
					d[2] = NR_COMPOSENPP_1211(s[2], a, d[2]);
					d[3] = NR_COMPOSEA_211(a, d[3]);
				}
				d += 4;
				s += 4;
			}
			px += rs;
			spx += srs;
		}
	}
}

void
nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	unsigned int r, c;

	if (alpha == 0) {
		/* Transparent FG, NOP */
	} else if (alpha == 255) {
		/* Simple */
		for (r = h; r > 0; r--) {
			unsigned char *d = px;
			const unsigned char *s = spx;
			for (c = w; c > 0; c--) {
				if (s[3] == 0) {
					/* Transparent FG, NOP */
				} else if ((s[3] == 255) || (d[3] == 0)) {
					/* Transparent BG, COPY */
					memcpy(d, s, 4);
				} else {
					d[0] = NR_COMPOSEPPP_1111(s[0], s[3], d[0]);
					d[1] = NR_COMPOSEPPP_1111(s[1], s[3], d[1]);
					d[2] = NR_COMPOSEPPP_1111(s[2], s[3], d[2]);
					d[3] = NR_COMPOSEA_111(s[3], d[3]);
				}
				d += 4;
				s += 4;
			}
			px += rs;
			spx += srs;
		}
	} else {
		for (r = h; r > 0; r--) {
			unsigned char *d = px;
			const unsigned char *s = spx;
			for (c = w; c > 0; c--) {
				if (s[3] == 0) {
					/* Transparent FG, NOP */
				} else if (d[3] == 0) {
					/* Transparent BG, COPY */
					d[0] = NR_PREMUL_111(s[0], alpha);
					d[1] = NR_PREMUL_111(s[1], alpha);
					d[2] = NR_PREMUL_111(s[2], alpha);
					d[3] = NR_PREMUL_111(s[3], alpha);
				} else {
					//   dc' = (1 - alpha*sa) * dc + alpha*sc
					//   da' = 1 - (1 - alpha*sa) * (1 - da)
					unsigned int a;
					a = NR_PREMUL_112(s[3], alpha);
					d[0] = NR_COMPOSEPPP_2211(NR_PREMUL_112(alpha, s[0]), a, d[0]);
					d[1] = NR_COMPOSEPPP_2211(NR_PREMUL_112(alpha, s[1]), a, d[1]);
					d[2] = NR_COMPOSEPPP_2211(NR_PREMUL_112(alpha, s[2]), a, d[2]);
					d[3] = NR_COMPOSEA_211(a, d[3]);
				}
				d += 4;
				s += 4;
			}
			px += rs;
			spx += srs;
		}
	}
}

/* Masked operations */

void
nr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_A8 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs)
{
	unsigned int r, c;

	for (r = h; r > 0; r--) {
		unsigned char *d = px;
		const unsigned char *s = spx;
		const unsigned char *m = mpx;
		for (c = w; c > 0; c--) {
			d[0] = s[0];
			d[1] = s[1];
			d[2] = s[2];
			d[3] = NR_PREMUL_111(s[3], m[0]);
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
	unsigned int r, c;

	for (r = h; r > 0; r--) {
		unsigned char *d = px;
		const unsigned char *s = spx;
		const unsigned char *m = mpx;
		for (c = w; c > 0; c--) {
			unsigned int a;
			a = NR_PREMUL_112 (s[3], m[0]);
			if (a == 0) {
				d[3] = 0;
			} else if (a == 255*255) {
				memcpy(d, s, 4);
			} else {
				//   dc' = ((1 - m*sa) * da*dc + m*sc)/da' = m*sc/da' = m*sc/(m*sa) = sc/sa
				//   da' = 1 - (1 - m*sa) * (1 - da) = 1 - (1 - m*sa) = m*sa
				d[0] = NR_DEMUL_111(s[0], s[3]);
				d[1] = NR_DEMUL_111(s[1], s[3]);
				d[2] = NR_DEMUL_111(s[2], s[3]);
				d[3] = NR_NORMALIZE_21(a);
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
	unsigned int r, c;

	for (r = h; r > 0; r--) {
		unsigned char *d = px;
		const unsigned char *s = spx;
		const unsigned char *m = mpx;
		for (c = w; c > 0; c--) {
			unsigned int a;
			a = NR_PREMUL_112(s[3], m[0]);
			if (a == 0) {
				memset(d, 0, 4);
			} else if (a == 255*255) {
				memcpy(d, s, 4);
			} else {
				d[0] = NR_PREMUL_121(s[0], a);
				d[1] = NR_PREMUL_121(s[1], a);
				d[2] = NR_PREMUL_121(s[2], a);
				d[3] = NR_NORMALIZE_21(a);
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
nr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_A8 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs)
{
	unsigned int r, c;

	for (r = h; r > 0; r--) {
		unsigned char *d = px;
		const unsigned char *s = spx;
		const unsigned char *m = mpx;
		for (c = w; c > 0; c--) {
			d[0] = NR_PREMUL_111(s[0], m[0]);
			d[1] = NR_PREMUL_111(s[1], m[0]);
			d[2] = NR_PREMUL_111(s[2], m[0]);
			d[3] = NR_PREMUL_111(s[3], m[0]);
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
	unsigned int r, c;

	for (r = h; r > 0; r--) {
		unsigned char *d = px;
		const unsigned char *s = spx;
		const unsigned char *m = mpx;
		for (c = w; c > 0; c--) {
			unsigned int a;
			a = NR_PREMUL_112(s[3], m[0]);
			if (a == 0) {
				/* Transparent FG, NOP */
			} else if ((a == 255*255) || (d[3] == 0)) {
				/* Full coverage, COPY */
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = NR_NORMALIZE_21(a);
			} else {
				/* Full composition */
				unsigned int ca;
				ca = NR_COMPOSEA_213(a, d[3]);
				d[0] = NR_COMPOSENNN_121131(s[0], a, d[0], d[3], ca);
				d[1] = NR_COMPOSENNN_121131(s[1], a, d[1], d[3], ca);
				d[2] = NR_COMPOSENNN_121131(s[2], a, d[2], d[3], ca);
				d[3] = NR_NORMALIZE_31(ca);
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
	unsigned int r, c;

	for (r = h; r > 0; r--) {
		unsigned char *d = px;
		const unsigned char *s = spx;
		const unsigned char *m = mpx;
		for (c = w; c > 0; c--) {
			unsigned int a;
			a = NR_PREMUL_112(s[3], m[0]);
			if (a == 0) {
				/* Transparent FG, NOP */
			} else if (a == 255*255) {
				/* Opaque FG, COPY */
				memcpy(d, s, 4);
			} else if (d[3] == 0) {
				/* Full coverage, demul src */
				//   dc' = ((1 - m*sa) * da*dc + m*sc)/da' = m*sc/da' = m*sc/(m*sa) = sc/sa
				//   da' = 1 - (1 - m*sa) * (1 - da) = 1 - (1 - m*sa) = m*sa
				d[0] = NR_DEMUL_111(s[0], s[3]);
				d[1] = NR_DEMUL_111(s[1], s[3]);
				d[2] = NR_DEMUL_111(s[2], s[3]);
				d[3] = NR_NORMALIZE_21(a);
			} else if (m[0] == 255) {
				/* Full composition */
				//   dc' = ((1 - m*sa) * da*dc + m*sc)/da' = ((1 - sa) * da*dc + sc)/da'
				//   da' = 1 - (1 - m*sa) * (1 - da) = 1 - (1 - sa) * (1 - da)
				unsigned int da = NR_COMPOSEA_112(s[3], d[3]);
				d[0] = NR_COMPOSEPNN_111121(s[0], s[3], d[0], d[3], da);
				d[1] = NR_COMPOSEPNN_111121(s[1], s[3], d[1], d[3], da);
				d[2] = NR_COMPOSEPNN_111121(s[2], s[3], d[2], d[3], da);
				d[3] = NR_NORMALIZE_21(da);
			} else {
				//   dc' = ((1 - m*sa) * da*dc + m*sc)/da'
				//   da' = 1 - (1 - m*sa) * (1 - da)
				unsigned int da = NR_COMPOSEA_213(a, d[3]);
				d[0] = NR_COMPOSEPNN_221131(NR_PREMUL_112(s[0], m[0]), a, d[0], d[3], da);
				d[1] = NR_COMPOSEPNN_221131(NR_PREMUL_112(s[1], m[0]), a, d[1], d[3], da);
				d[2] = NR_COMPOSEPNN_221131(NR_PREMUL_112(s[2], m[0]), a, d[2], d[3], da);
				d[3] = NR_NORMALIZE_31(da);
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
	unsigned int r, c;

	for (r = h; r>0; r--) {
		unsigned char *d = px;
		const unsigned char *s = spx;
		const unsigned char *m = mpx;
		for (c = w; c>0; c--) {
			unsigned int a;
			a = NR_PREMUL_112(s[3], m[0]);
			if (a == 0) {
				/* Transparent FG, NOP */
			} else if (a == 255*255) {
				memcpy(d, s, 4);
			} else {
				d[0] = NR_COMPOSENPP_1211(s[0], a, d[0]);
				d[1] = NR_COMPOSENPP_1211(s[1], a, d[1]);
				d[2] = NR_COMPOSENPP_1211(s[2], a, d[2]);
				d[3] = NR_COMPOSEA_211(a, d[3]);
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
	unsigned int r, c;

	for (r = h; r > 0; r--) {
		unsigned char *d = px;
		const unsigned char *s = spx;
		const unsigned char *m = mpx;
		for (c = w; c > 0; c--) {
			unsigned int a;
			a = NR_PREMUL_112 (s[3], m[0]);
			if (a == 0) {
				/* Transparent FG, NOP */
			} else if (a == 255*255) {
				/* Opaque FG, COPY */
				memcpy(d, s, 4);
			} else if (d[3] == 0) {
				/* Transparent BG, COPY */
				//   dc' = (1 - m*sa) * dc + m*sc = m*sc
				//   da' = 1 - (1 - m*sa) * (1 - da) = 1 - (1 - m*sa)  = m*sa
				d[0] = NR_PREMUL_111 (s[0], m[0]);
				d[1] = NR_PREMUL_111 (s[1], m[0]);
				d[2] = NR_PREMUL_111 (s[2], m[0]);
				d[3] = NR_NORMALIZE_21(a);
			} else {
				//   dc' = (1 - m*sa) * dc + m*sc
				//   da' = 1 - (1 - m*sa) * (1 - da)
				d[0] = NR_COMPOSEPPP_2211 (NR_PREMUL_112 (s[0], m[0]), a, d[0]);
				d[1] = NR_COMPOSEPPP_2211 (NR_PREMUL_112 (s[1], m[0]), a, d[1]);
				d[2] = NR_COMPOSEPPP_2211 (NR_PREMUL_112 (s[2], m[0]), a, d[2]);
				d[3] = NR_COMPOSEA_211(a, d[3]);
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

/* FINAL DST MASK COLOR */

void
nr_R8G8B8A8_N_EMPTY_A8_RGBA32 (unsigned char *px, int w, int h, int rs, const unsigned char *mpx, int mrs, unsigned long rgba)
{
	unsigned int r, g, b, a;
	unsigned int x, y;

	r = NR_RGBA32_R (rgba);
	g = NR_RGBA32_G (rgba);
	b = NR_RGBA32_B (rgba);
	a = NR_RGBA32_A (rgba);

	for (y = h; y > 0; y--) {
		if (a == 0) {
			memset(px, 0, w*4);
		} else {
			unsigned char *d = px;
			const unsigned char *m = mpx;
			for (x = w; x > 0; x--) {
				d[0] = r;
				d[1] = g;
				d[2] = b;
				d[3] = NR_PREMUL_111 (m[0], a);
				d += 4;
				m += 1;
			}
		}
		px += rs;
		mpx += mrs;
	}
}

void
nr_R8G8B8A8_P_EMPTY_A8_RGBA32 (unsigned char *px, int w, int h, int rs, const unsigned char *mpx, int mrs, unsigned long rgba)
{
	unsigned int r, g, b, a;
	unsigned int x, y;

	r = NR_RGBA32_R (rgba);
	g = NR_RGBA32_G (rgba);
	b = NR_RGBA32_B (rgba);
	a = NR_RGBA32_A (rgba);

#ifdef WITH_MMX
	if (NR_PIXOPS_MMX) {
		unsigned char c[4];
		c[0] = NR_PREMUL_111 (r, a);
		c[1] = NR_PREMUL_111 (g, a);
		c[2] = NR_PREMUL_111 (b, a);
		c[3] = a;
		/* WARNING: MMX composer REQUIRES w > 0 and h > 0 */
		nr_mmx_R8G8B8A8_P_EMPTY_A8_RGBAP (px, w, h, rs, mpx, mrs, c);
		return;
	}
#endif

	if ( a != 255 ){
		// Pre-premultiply color values
		r *= a;
		g *= a;
		b *= a;
	}

	for (y = h; y > 0; y--) {
		unsigned char *d = px;
		const unsigned char *m = mpx;
		if (a == 0) {
			memset(px, 0, w*4);
		} else if (a == 255) {
			for (x = w; x > 0; x--) {
				d[0] = NR_PREMUL_111(m[0], r);
				d[1] = NR_PREMUL_111(m[0], g);
				d[2] = NR_PREMUL_111(m[0], b);
				d[3] = m[0];
				d += 4;
				m += 1;
			}
		} else {
			for (x = w; x > 0; x--) {
				// Color values are already premultiplied with a
				d[0] = NR_PREMUL_121(m[0], r);
				d[1] = NR_PREMUL_121(m[0], g);
				d[2] = NR_PREMUL_121(m[0], b);
				d[3] = NR_PREMUL_111(m[0], a);
				d += 4;
				m += 1;
			}
		}
		px += rs;
		mpx += mrs;
	}
}

void
nr_R8G8B8_R8G8B8_A8_RGBA32 (unsigned char *px, int w, int h, int rs, const unsigned char *mpx, int mrs, unsigned long rgba)
{
	unsigned int r, g, b, a;
	unsigned int x, y;

	r = NR_RGBA32_R (rgba);
	g = NR_RGBA32_G (rgba);
	b = NR_RGBA32_B (rgba);
	a = NR_RGBA32_A (rgba);

	if (a == 0) {
		/* NOP */
	} else if (a == 255) {
		for (y = h; y > 0; y--) {
			unsigned char *d = px;
			const unsigned char *m = mpx;
			for (x = w; x > 0; x--) {
				d[0] = NR_COMPOSEN11_1111 (r, m[0], d[0]);
				d[1] = NR_COMPOSEN11_1111 (g, m[0], d[1]);
				d[2] = NR_COMPOSEN11_1111 (b, m[0], d[2]);
				d += 3;
				m += 1;
			}
			px += rs;
			mpx += mrs;
		}
	} else {
		for (y = h; y > 0; y--) {
			unsigned char *d = px;
			const unsigned char *m = mpx;
			for (x = w; x > 0; x--) {
				//   dc' = (1 - m*sa) * dc + m*sa*sc
				unsigned int alpha;
				alpha = NR_PREMUL_112 (a, m[0]);
				d[0] = NR_COMPOSEN11_1211 (r, alpha, d[0]);
				d[1] = NR_COMPOSEN11_1211 (g, alpha, d[1]);
				d[2] = NR_COMPOSEN11_1211 (b, alpha, d[2]);
				d += 3;
				m += 1;
			}
			px += rs;
			mpx += mrs;
		}
	}
}

void
nr_R8G8B8A8_N_R8G8B8A8_N_A8_RGBA32 (unsigned char *px, int w, int h, int rs, const unsigned char *mpx, int mrs, unsigned long rgba)
{
	unsigned int r, g, b, a;
	unsigned int x, y;

	r = NR_RGBA32_R (rgba);
	g = NR_RGBA32_G (rgba);
	b = NR_RGBA32_B (rgba);
	a = NR_RGBA32_A (rgba);

	if (a == 0) {
		/* NOP */
	} else if (a == 255) {
		for (y = h; y > 0; y--) {
			unsigned char *d = px;
			const unsigned char *m = mpx;
			for (x = w; x > 0; x--) {
				if (m[0] == 0) {
					/* Transparent FG, NOP */
				} else if (m[0] == 255 || d[3] == 0) {
					/* Full coverage, COPY */
					d[0] = r;
					d[1] = g;
					d[2] = b;
					d[3] = m[0];
				} else {
					/* Full composition */
					unsigned int da = NR_COMPOSEA_112(m[0], d[3]);
					d[0] = NR_COMPOSENNN_111121(r, m[0], d[0], d[3], da);
					d[1] = NR_COMPOSENNN_111121(g, m[0], d[1], d[3], da);
					d[2] = NR_COMPOSENNN_111121(b, m[0], d[2], d[3], da);
					d[3] = NR_NORMALIZE_21(da);
				}
				d += 4;
				m += 1;
			}
			px += rs;
			mpx += mrs;
		}
	} else {
		for (y = h; y > 0; y--) {
			unsigned char *d = px;
			const unsigned char *m = mpx;
			for (x = w; x > 0; x--) {
				unsigned int ca;
				ca = NR_PREMUL_112 (m[0], a);
				if (ca == 0) {
					/* Transparent FG, NOP */
				} else if (d[3] == 0) {
					/* Full coverage, COPY */
					d[0] = r;
					d[1] = g;
					d[2] = b;
					d[3] = NR_NORMALIZE_21(ca);
				} else {
					/* Full composition */
					unsigned int da = NR_COMPOSEA_213(ca, d[3]);
					d[0] = NR_COMPOSENNN_121131(r, ca, d[0], d[3], da);
					d[1] = NR_COMPOSENNN_121131(g, ca, d[1], d[3], da);
					d[2] = NR_COMPOSENNN_121131(b, ca, d[2], d[3], da);
					d[3] = NR_NORMALIZE_31(da);
				}
				d += 4;
				m += 1;
			}
			px += rs;
			mpx += mrs;
		}
	}
}

void
nr_R8G8B8A8_P_R8G8B8A8_P_A8_RGBA32 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned long rgba)
{
	unsigned int r, g, b, a;
	unsigned int x, y;

	r = NR_RGBA32_R (rgba);
	g = NR_RGBA32_G (rgba);
	b = NR_RGBA32_B (rgba);
	a = NR_RGBA32_A (rgba);

#ifdef WITH_MMX
	if (NR_PIXOPS_MMX && a != 0) {
		unsigned char c[4];
		c[0] = NR_PREMUL_111 (r, a);
		c[1] = NR_PREMUL_111 (g, a);
		c[2] = NR_PREMUL_111 (b, a);
		c[3] = a;
		/* WARNING: MMX composer REQUIRES w > 0 and h > 0 */
		nr_mmx_R8G8B8A8_P_R8G8B8A8_P_A8_RGBAP (px, w, h, rs, spx, srs, c);
		return;
	}
#endif

	if (a == 0) {
		/* Transparent FG, NOP */
	} else if (a == 255) {
		/* Simple */
		for (y = h; y > 0; y--) {
			unsigned char *d, *s;
			d = (unsigned char *) px;
			s = (unsigned char *) spx;
			for (x = w; x > 0; x--) {
				if (s[0] == 0) {
					/* Transparent FG, NOP */
				} else {
					/* Full composition */
					unsigned int invca = 255-s[0]; // By swapping the arguments GCC can better optimize these calls
					d[0] = NR_COMPOSENPP_1111(d[0], invca, r);
					d[1] = NR_COMPOSENPP_1111(d[1], invca, g);
					d[2] = NR_COMPOSENPP_1111(d[2], invca, b);
					d[3] = NR_COMPOSEA_111(s[0], d[3]);
				}
				d += 4;
				s += 1;
			}
			px += rs;
			spx += srs;
		}
	} else {
		for (y = h; y > 0; y--) {
			unsigned char *d, *s;
			d = (unsigned char *) px;
			s = (unsigned char *) spx;
			for (x = w; x > 0; x--) {
				unsigned int ca;
				ca = NR_PREMUL_112 (s[0], a);
				if (ca == 0) {
					/* Transparent FG, NOP */
				} else {
					/* Full composition */
					unsigned int invca = 255*255-ca; // By swapping the arguments GCC can better optimize these calls
					d[0] = NR_COMPOSENPP_1211(d[0], invca, r);
					d[1] = NR_COMPOSENPP_1211(d[1], invca, g);
					d[2] = NR_COMPOSENPP_1211(d[2], invca, b);
					d[3] = NR_COMPOSEA_211(ca, d[3]);
				}
				d += 4;
				s += 1;
			}
			px += rs;
			spx += srs;
		}
	}
}

/* RGB */

void
nr_R8G8B8_R8G8B8_R8G8B8A8_P (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	unsigned int r, c;

#ifdef WITH_MMX
	if (NR_PIXOPS_MMX && alpha != 0) {
		/* WARNING: MMX composer REQUIRES w > 0 and h > 0 */
		nr_mmx_R8G8B8_R8G8B8_R8G8B8A8_P (px, w, h, rs, spx, srs, alpha);
		return;
	}
#endif

	if (alpha == 0) {
		/* NOP */
	} else if (alpha == 255) {
		for (r = h; r > 0; r--) {
			unsigned char *d = px;
			const unsigned char *s = spx;
			for (c = w; c > 0; c--) {
				//   dc' = (1 - alpha*sa) * dc + alpha*sc = (1 - sa) * dc + sc
				if (s[3] == 0) {
					/* NOP */
				} else if (s[3] == 255) {
					d[0] = s[0];
					d[1] = s[1];
					d[2] = s[2];
				} else {
					d[0] = NR_COMPOSEP11_1111(s[0], s[3], d[0]);
					d[1] = NR_COMPOSEP11_1111(s[1], s[3], d[1]);
					d[2] = NR_COMPOSEP11_1111(s[2], s[3], d[2]);
				}
				d += 3;
				s += 4;
			}
			px += rs;
			spx += srs;
		}
	} else {
		for (r = h; r > 0; r--) {
			unsigned char *d = px;
			const unsigned char *s = spx;
			for (c = w; c > 0; c--) {
				unsigned int a;
				a = NR_PREMUL_112(s[3], alpha);
				//   dc' = (1 - alpha*sa) * dc + alpha*sc
				if (a == 0) {
					/* NOP */
				} else {
					d[0] = NR_COMPOSEP11_2211(NR_PREMUL_112(s[0], alpha), a, d[0]);
					d[1] = NR_COMPOSEP11_2211(NR_PREMUL_112(s[1], alpha), a, d[1]);
					d[2] = NR_COMPOSEP11_2211(NR_PREMUL_112(s[2], alpha), a, d[2]);
				}
				/* a == 255 is impossible, because alpha < 255 */
				d += 3;
				s += 4;
			}
			px += rs;
			spx += srs;
		}
	}
}

void
nr_R8G8B8_R8G8B8_R8G8B8A8_N (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, unsigned int alpha)
{
	unsigned int r, c;

	if (alpha == 0) {
		/* NOP */
	} else if (alpha == 255) {
		for (r = h; r > 0; r--) {
			unsigned char *d = px;
			const unsigned char *s = spx;
			for (c = w; c > 0; c--) {
				//   dc' = (1 - alpha*sa) * dc + alpha*sa*sc = (1 - sa) * dc + sa*sc
				if (s[3] == 0) {
					/* NOP */
				} else if (s[3] == 255) {
					d[0] = s[0];
					d[1] = s[1];
					d[2] = s[2];
				} else {
					d[0] = NR_COMPOSEN11_1111(s[0], s[3], d[0]);
					d[1] = NR_COMPOSEN11_1111(s[1], s[3], d[1]);
					d[2] = NR_COMPOSEN11_1111(s[2], s[3], d[2]);
				}
				d += 3;
				s += 4;
			}
			px += rs;
			spx += srs;
		}
	} else {
		for (r = h; r > 0; r--) {
			unsigned char *d = px;
			const unsigned char *s = spx;
			for (c = w; c > 0; c--) {
				unsigned int a;
				a = NR_PREMUL_112(s[3], alpha);
				//   dc' = (1 - alpha*sa) * dc + alpha*sa*sc
				if (a == 0) {
					/* NOP */
				} else {
					d[0] = NR_COMPOSEN11_1211(s[0], a, d[0]);
					d[1] = NR_COMPOSEN11_1211(s[1], a, d[1]);
					d[2] = NR_COMPOSEN11_1211(s[2], a, d[2]);
				}
				/* a == 255 is impossible, because alpha < 255 */
				d += 3;
				s += 4;
			}
			px += rs;
			spx += srs;
		}
	}
}

void
nr_R8G8B8_R8G8B8_R8G8B8A8_P_A8 (unsigned char *px, int w, int h, int rs, const unsigned char *spx, int srs, const unsigned char *mpx, int mrs)
{
	unsigned int x, y;

	for (y = h; y > 0; y--) {
		unsigned char* d = px;
		const unsigned char* s = spx;
		const unsigned char* m = mpx;
		for (x = w; x > 0; x--) {
			unsigned int a;
			a = NR_PREMUL_112(s[3], m[0]);
			if (a == 0) {
				/* NOP */
			} else if (a == 255*255) {
				memcpy(d, s, 3);
			} else {
				//   dc' = (1 - m*sa) * dc + m*sc
				d[0] = NR_COMPOSEP11_2211(NR_PREMUL_112(s[0], m[0]), a, d[0]);
				d[1] = NR_COMPOSEP11_2211(NR_PREMUL_112(s[1], m[0]), a, d[1]);
				d[2] = NR_COMPOSEP11_2211(NR_PREMUL_112(s[2], m[0]), a, d[2]);
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
	unsigned int x, y;

	for (y = h; y > 0; y--) {
		unsigned char* d = px;
		const unsigned char* s = spx;
		const unsigned char* m = mpx;
		for (x = w; x > 0; x--) {
			unsigned int a;
			a = NR_PREMUL_112(s[3], m[0]);
			if (a == 0) {
				/* NOP */
			} else if (a == 255*255) {
				memcpy(d, s, 3);
			} else {
				//   dc' = (1 - m*sa) * dc + m*sa*sc
				d[0] = NR_COMPOSEN11_1211(s[0], a, d[0]);
				d[1] = NR_COMPOSEN11_1211(s[1], a, d[1]);
				d[2] = NR_COMPOSEN11_1211(s[2], a, d[2]);
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



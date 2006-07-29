#define __NR_PLAIN_STUFF_C__

/*
 * Miscellaneous simple rendering utilities
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Lauris Kaplinski and Ximian, Inc.
 *
 * Released under GNU GPL
 */

#include <glib/gmessages.h>
#include <libnr/nr-pixops.h>
#include "nr-plain-stuff.h"

#define NR_DEFAULT_CHECKERSIZEP2 2
#define NR_DEFAULT_CHECKERCOLOR0 0xbfbfbfff
#define NR_DEFAULT_CHECKERCOLOR1 0x808080ff

void
nr_render_checkerboard_rgb (guchar *px, gint w, gint h, gint rs, gint xoff, gint yoff)
{
	g_return_if_fail (px != NULL);

	nr_render_checkerboard_rgb_custom (px, w, h, rs, xoff, yoff, NR_DEFAULT_CHECKERCOLOR0, NR_DEFAULT_CHECKERCOLOR1, NR_DEFAULT_CHECKERSIZEP2);
}

void
nr_render_checkerboard_rgb_custom (guchar *px, gint w, gint h, gint rs, gint xoff, gint yoff, guint32 c0, guint32 c1, gint sizep2)
{
	gint x, y, m;
	guint r0, g0, b0;
	guint r1, g1, b1;

	g_return_if_fail (px != NULL);
	g_return_if_fail (sizep2 >= 0);
	g_return_if_fail (sizep2 <= 8);

	xoff &= 0x1ff;
	yoff &= 0x1ff;
	m = 0x1 << sizep2;
	r0 = NR_RGBA32_R (c0);
	g0 = NR_RGBA32_G (c0);
	b0 = NR_RGBA32_B (c0);
	r1 = NR_RGBA32_R (c1);
	g1 = NR_RGBA32_G (c1);
	b1 = NR_RGBA32_B (c1);

	for (y = 0; y < h; y++) {
		guchar *p;
		p = px;
		for (x = 0; x < w; x++) {
			if (((x + xoff) ^ (y + yoff)) & m) {
				*p++ = r0;
				*p++ = g0;
				*p++ = b0;
			} else {
				*p++ = r1;
				*p++ = g1;
				*p++ = b1;
			}
		}
		px += rs;
	}
}

void
nr_render_rgba32_rgb (guchar *px, gint w, gint h, gint rs, gint xoff, gint yoff, guint32 c)
{
	guint32 c0, c1;
	gint a, r, g, b, cr, cg, cb;

	g_return_if_fail (px != NULL);

	r = NR_RGBA32_R (c);
	g = NR_RGBA32_G (c);
	b = NR_RGBA32_B (c);
	a = NR_RGBA32_A (c);

	cr = NR_COMPOSEN11_1111 (r, a, NR_RGBA32_R (NR_DEFAULT_CHECKERCOLOR0));
	cg = NR_COMPOSEN11_1111 (g, a, NR_RGBA32_G (NR_DEFAULT_CHECKERCOLOR0));
	cb = NR_COMPOSEN11_1111 (b, a, NR_RGBA32_B (NR_DEFAULT_CHECKERCOLOR0));
	c0 = (cr << 24) | (cg << 16) | (cb << 8) | 0xff;

	cr = NR_COMPOSEN11_1111 (r, a, NR_RGBA32_R (NR_DEFAULT_CHECKERCOLOR1));
	cg = NR_COMPOSEN11_1111 (g, a, NR_RGBA32_G (NR_DEFAULT_CHECKERCOLOR1));
	cb = NR_COMPOSEN11_1111 (b, a, NR_RGBA32_B (NR_DEFAULT_CHECKERCOLOR1));
	c1 = (cr << 24) | (cg << 16) | (cb << 8) | 0xff;

	nr_render_checkerboard_rgb_custom (px, w, h, rs, xoff, yoff, c0, c1, NR_DEFAULT_CHECKERSIZEP2);
}


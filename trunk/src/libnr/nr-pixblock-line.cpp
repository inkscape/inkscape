#define __NR_PIXBLOCK_LINE_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-pixops.h>
#include <libnr/nr-pixblock-pixel.h>

void
nr_pixblock_draw_line_rgba32 (NRPixBlock *d, long x0, long y0, long x1, long y1, short /*first*/, unsigned long rgba)
{
	long deltax, deltay, xinc1, xinc2, yinc1, yinc2;
	long den, num, numadd, numpixels;
	long x, y, curpixel;
	/* Pixblock */
	int dbpp;
	NRPixBlock spb;
	unsigned char *spx;

	if (x1 >= x0) {
		deltax = x1 - x0;
		xinc1 = 1;
		xinc2 = 1;
	} else {
		deltax = x0 - x1;
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y1 >= y0) {
		deltay = y1 - y0;
		yinc1 = 1;
		yinc2 = 1;
	} else {
		deltay = y0 - y1;
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay) {
		xinc1 = 0;
		yinc2 = 0;
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;
	} else {
		xinc2 = 0;
		yinc1 = 0;
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;
	}

	/* We can be quite sure 1x1 pixblock is TINY */
	nr_pixblock_setup_fast (&spb, NR_PIXBLOCK_MODE_R8G8B8A8N, 0, 0, 1, 1, 0);
	spb.empty = 0;
	spx = NR_PIXBLOCK_PX (&spb);
	spx[0] = NR_RGBA32_R (rgba);
	spx[1] = NR_RGBA32_G (rgba);
	spx[2] = NR_RGBA32_B (rgba);
	spx[3] = NR_RGBA32_A (rgba);

	dbpp = NR_PIXBLOCK_BPP (d);

	x = x0;
	y = y0;

	for (curpixel = 0; curpixel <= numpixels; curpixel++) {
		if ((x >= d->area.x0) && (y >= d->area.y0) && (x < d->area.x1) && (y < d->area.y1)) {
			nr_compose_pixblock_pixblock_pixel (d, NR_PIXBLOCK_PX (d) + (y - d->area.y0) * d->rs + (x - d->area.x0) * dbpp, &spb, spx);
		}
		num += numadd;
		if (num >= den) {
			num -= den;
			x += xinc1;
			y += yinc1;
		}
		x += xinc2;
		y += yinc2;
	}

	nr_pixblock_release (&spb);
}


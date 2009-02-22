#define __TESTNR_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#if defined (_WIN32) || defined (__WIN32__)
# include <windows.h> 
#include <glib.h>
#endif


#include "nr-blit.h"

static double
get_time (void)
{
    GTimeVal tv;
    g_get_current_time (&tv);
    return tv.tv_sec + 1e-6 * tv.tv_usec;
}

static unsigned int
rand_byte (void)
{
	return (int) (256.0 * rand () / (RAND_MAX + 1.0));
}

int
main (int argc, const char **argv)
{
	double start, end;
	NRPixBlock d, m[16];
	int count, i;

	srand (time (NULL));

	printf ("Initializing buffers\n");

	/* Destination */
	nr_pixblock_setup_fast (&d, NR_PIXBLOCK_MODE_R8G8B8A8P, 0, 0, 64, 64, 1);
	d.empty = 0;

	/* Masks */
	for (i = 0; i < 16; i++) {
		int r, b, c;
		nr_pixblock_setup_fast (&m[i], NR_PIXBLOCK_MODE_A8, 0, 0, 64, 64, 0);
		for (r = 0; r < 64; r++) {
			unsigned int q;
			unsigned char *p;
			p = NR_PIXBLOCK_PX (&m[i]) + r * m[i].rs;
			for (b = 0; b < 8; b++) {
				q = rand_byte ();
				if (q < 120) {
					for (c = 0; c < 8; c++) *p++ = 0;
				} else if (q < 240) {
					for (c = 0; c < 8; c++) *p++ = 255;
				} else {
					for (c = 0; c < 8; c++) *p++ = rand_byte ();
				}
			}
		}
		m[i].empty = 0;
	}

	printf ("Random transparency\n");
	count = 0;
	start = end = get_time ();
	while ((end - start) < 5.0) {
		unsigned char r, g, b, a;
		r = rand_byte ();
		g = rand_byte ();
		b = rand_byte ();
		a = rand_byte ();

		for (i = 0; i < 16; i++) {
			nr_blit_pixblock_mask_rgba32 (&d, &m[i], (a << 24) | (g << 16) | (b << 8) | a);
			count += 1;
		}
		end = get_time ();
	}
	printf ("Did %d [64x64] random buffers in %f sec\n", count, end - start); // localizing ok
	printf ("%f buffers per second\n", count / (end - start)); // localizing ok
	printf ("%f pixels per second\n", count * (64 * 64) / (end - start)); // localizing ok

	return 0;
}

#define __NR_PIXBLOCK_PATTERN_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */


#include <glib/gmem.h>
#include "nr-pixops.h"
#include "nr-pixblock-pattern.h"

#define NR_NOISE_SIZE 1024

void
nr_pixblock_render_gray_noise (NRPixBlock *pb, NRPixBlock *mask)
{
	static unsigned char *noise = NULL;
	static unsigned int seed = 0;
	unsigned int v;
	NRRectL clip;
	int x, y, bpp;

	if (mask) {
		if (mask->empty) return;
		nr_rect_l_intersect (&clip, &pb->area, &mask->area);
		if (nr_rect_l_test_empty(clip)) return;
	} else {
		clip = pb->area;
	}

	if (!noise) {
		int i;
		noise = g_new (unsigned char, NR_NOISE_SIZE);
		for (i = 0; i < NR_NOISE_SIZE; i++) noise[i] = (rand () / (RAND_MAX >> 8)) & 0xff;
	}

	bpp = NR_PIXBLOCK_BPP (pb);

	v = (rand () / (RAND_MAX >> 8)) & 0xff;

	if (mask) {
		for (y = clip.y0; y < clip.y1; y++) {
			unsigned char *d, *m;
			d = NR_PIXBLOCK_PX (pb) + (y - pb->area.y0) * pb->rs + (clip.x0 - pb->area.x0) * bpp;
			m = NR_PIXBLOCK_PX (mask) + (y - mask->area.y0) * pb->rs + (clip.x0 - mask->area.x0);
			for (x = clip.x0; x < clip.x1; x++) {
				v = v ^ noise[seed];
				switch (pb->mode) {
				case NR_PIXBLOCK_MODE_A8:
					d[0] = NR_COMPOSEA_111(m[0], d[0]);
					break;
				case NR_PIXBLOCK_MODE_R8G8B8:
					d[0] = NR_COMPOSEN11_1111 (v, m[0], d[0]);
					d[1] = NR_COMPOSEN11_1111 (v, m[0], d[1]);
					d[2] = NR_COMPOSEN11_1111 (v, m[0], d[2]);
					break;
				case NR_PIXBLOCK_MODE_R8G8B8A8N:
					if (m[0] != 0) {
						unsigned int ca;
						ca = NR_COMPOSEA_112(m[0], d[3]);
						d[0] = NR_COMPOSENNN_111121 (v, m[0], d[0], d[3], ca);
						d[1] = NR_COMPOSENNN_111121 (v, m[0], d[1], d[3], ca);
						d[2] = NR_COMPOSENNN_111121 (v, m[0], d[2], d[3], ca);
						d[3] = NR_NORMALIZE_21(ca);
					}
					break;
				case NR_PIXBLOCK_MODE_R8G8B8A8P:
					d[0] = NR_COMPOSENPP_1111 (v, m[0], d[0]);
					d[1] = NR_COMPOSENPP_1111 (v, m[0], d[1]);
					d[2] = NR_COMPOSENPP_1111 (v, m[0], d[2]);
					d[3] = NR_COMPOSEA_111(d[3], m[0]);
					break;
				default:
					break;
				}
				d += bpp;
				m += 1;
				if (++seed >= NR_NOISE_SIZE) {
					int i;
					i = (rand () / (RAND_MAX / NR_NOISE_SIZE)) % NR_NOISE_SIZE;
					noise[i] ^= v;
					seed = i % (NR_NOISE_SIZE >> 2);
				}
			}
		}
	} else {
		for (y = clip.y0; y < clip.y1; y++) {
			unsigned char *d;
			d = NR_PIXBLOCK_PX (pb) + (y - pb->area.y0) * pb->rs + (clip.x0 - pb->area.x0) * bpp;
			for (x = clip.x0; x < clip.x1; x++) {
				v = v ^ noise[seed];
				switch (pb->mode) {
				case NR_PIXBLOCK_MODE_A8:
					d[0] = 255;
					break;
				case NR_PIXBLOCK_MODE_R8G8B8:
					d[0] = v;
					d[1] = v;
					d[2] = v;
					break;
				case NR_PIXBLOCK_MODE_R8G8B8A8N:
				case NR_PIXBLOCK_MODE_R8G8B8A8P:
					d[0] = v;
					d[1] = v;
					d[2] = v;
					d[3] = 255;
				default:
					break;
				}
				d += bpp;
				if (++seed >= NR_NOISE_SIZE) {
					int i;
					i = (rand () / (RAND_MAX / NR_NOISE_SIZE)) % NR_NOISE_SIZE;
					noise[i] ^= v;
					seed = i % (NR_NOISE_SIZE >> 2);
				}
			}
		}
	}

	pb->empty = 0;
}

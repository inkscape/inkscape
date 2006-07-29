#define __NR_PIXBLOCK_PIXEL_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include "nr-pixops.h"
#include "nr-pixblock-pixel.h"

void
nr_compose_pixblock_pixblock_pixel (NRPixBlock *dpb, unsigned char *d, const NRPixBlock *spb, const unsigned char *s)
{
	if (spb->empty) return;

	if (dpb->empty) {
		/* Empty destination */
		switch (dpb->mode) {
		case NR_PIXBLOCK_MODE_A8:
			switch (spb->mode) {
			case NR_PIXBLOCK_MODE_A8:
				break;
			case NR_PIXBLOCK_MODE_R8G8B8:
				d[0] = 255;
				break;
			case NR_PIXBLOCK_MODE_R8G8B8A8N:
				d[0] = s[3];
				break;
			case NR_PIXBLOCK_MODE_R8G8B8A8P:
				d[0] = s[3];
				break;
			default:
				break;
			}
			break;
		case NR_PIXBLOCK_MODE_R8G8B8:
			switch (spb->mode) {
			case NR_PIXBLOCK_MODE_A8:
				break;
			case NR_PIXBLOCK_MODE_R8G8B8:
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				break;
			case NR_PIXBLOCK_MODE_R8G8B8A8N:
				d[0] = NR_COMPOSEN11_1111 (s[0], s[3], 255);
				d[1] = NR_COMPOSEN11_1111 (s[1], s[3], 255);
				d[2] = NR_COMPOSEN11_1111 (s[2], s[3], 255);
				break;
			case NR_PIXBLOCK_MODE_R8G8B8A8P:
				d[0] = NR_COMPOSEP11_1111 (s[0], s[3], 255);
				d[1] = NR_COMPOSEP11_1111 (s[1], s[3], 255);
				d[2] = NR_COMPOSEP11_1111 (s[2], s[3], 255);
				break;
			default:
				break;
			}
			break;
		case NR_PIXBLOCK_MODE_R8G8B8A8N:
			switch (spb->mode) {
			case NR_PIXBLOCK_MODE_A8:
				break;
			case NR_PIXBLOCK_MODE_R8G8B8:
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = 255;
				break;
			case NR_PIXBLOCK_MODE_R8G8B8A8N:
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = s[3];
				break;
			case NR_PIXBLOCK_MODE_R8G8B8A8P:
				if (s[3] == 0) {
					d[0] = 255;
					d[1] = 255;
					d[2] = 255;
				} else {
					d[0] = NR_DEMUL_111(s[0], s[3]);
					d[1] = NR_DEMUL_111(s[0], s[3]);
					d[2] = NR_DEMUL_111(s[0], s[3]);
				}
				d[3] = s[3];
				break;
			default:
				break;
			}
			break;
		case NR_PIXBLOCK_MODE_R8G8B8A8P:
			switch (spb->mode) {
			case NR_PIXBLOCK_MODE_A8:
				break;
			case NR_PIXBLOCK_MODE_R8G8B8:
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = 255;
				break;
			case NR_PIXBLOCK_MODE_R8G8B8A8N:
				d[0] = NR_PREMUL_111 (s[0], s[3]);
				d[1] = NR_PREMUL_111 (s[1], s[3]);
				d[2] = NR_PREMUL_111 (s[2], s[3]);
				d[3] = s[3];
				break;
			case NR_PIXBLOCK_MODE_R8G8B8A8P:
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = s[3];
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	} else {
		/* Image destination */
		switch (dpb->mode) {
		case NR_PIXBLOCK_MODE_A8:
			switch (spb->mode) {
			case NR_PIXBLOCK_MODE_A8:
				break;
			case NR_PIXBLOCK_MODE_R8G8B8:
				d[0] = 255;
				break;
			case NR_PIXBLOCK_MODE_R8G8B8A8N:
				d[0] = NR_COMPOSEA_111(s[3], d[0]);
				break;
			case NR_PIXBLOCK_MODE_R8G8B8A8P:
				d[0] = NR_COMPOSEA_111(s[3], d[0]);
				break;
			default:
				break;
			}
			break;
		case NR_PIXBLOCK_MODE_R8G8B8:
			switch (spb->mode) {
			case NR_PIXBLOCK_MODE_A8:
				break;
			case NR_PIXBLOCK_MODE_R8G8B8:
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				break;
			case NR_PIXBLOCK_MODE_R8G8B8A8N:
				d[0] = NR_COMPOSEN11_1111 (s[0], s[3], d[0]);
				d[1] = NR_COMPOSEN11_1111 (s[1], s[3], d[1]);
				d[2] = NR_COMPOSEN11_1111 (s[2], s[3], d[2]);
				break;
			case NR_PIXBLOCK_MODE_R8G8B8A8P:
				d[0] = NR_COMPOSEP11_1111 (s[0], s[3], d[0]);
				d[1] = NR_COMPOSEP11_1111 (s[1], s[3], d[1]);
				d[2] = NR_COMPOSEP11_1111 (s[2], s[3], d[2]);
				break;
			default:
				break;
			}
			break;
		case NR_PIXBLOCK_MODE_R8G8B8A8N:
			switch (spb->mode) {
			case NR_PIXBLOCK_MODE_A8:
				break;
			case NR_PIXBLOCK_MODE_R8G8B8:
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				break;
			case NR_PIXBLOCK_MODE_R8G8B8A8N:
				if (s[3] != 0) {
					unsigned int ca;
					ca = NR_COMPOSEA_112(s[3], d[3]);
					d[0] = NR_COMPOSENNN_111121 (s[0], s[3], d[0], d[3], ca);
					d[1] = NR_COMPOSENNN_111121 (s[1], s[3], d[1], d[3], ca);
					d[2] = NR_COMPOSENNN_111121 (s[2], s[3], d[2], d[3], ca);
					d[3] = NR_NORMALIZE_21(ca);
				}
				break;
			case NR_PIXBLOCK_MODE_R8G8B8A8P:
				if (s[3] != 0) {
					unsigned int ca;
					ca = NR_COMPOSEA_112(s[3], d[3]);
					d[0] = NR_COMPOSEPNN_111121 (s[0], s[3], d[0], d[3], ca);
					d[1] = NR_COMPOSEPNN_111121 (s[1], s[3], d[0], d[3], ca);
					d[2] = NR_COMPOSEPNN_111121 (s[2], s[3], d[0], d[3], ca);
					d[3] = NR_NORMALIZE_21(ca);
				}
				break;
			default:
				break;
			}
			break;
		case NR_PIXBLOCK_MODE_R8G8B8A8P:
			switch (spb->mode) {
			case NR_PIXBLOCK_MODE_A8:
				break;
			case NR_PIXBLOCK_MODE_R8G8B8:
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				break;
			case NR_PIXBLOCK_MODE_R8G8B8A8N:
				d[0] = NR_COMPOSENPP_1111 (s[0], s[3], d[0]);
				d[1] = NR_COMPOSENPP_1111 (s[1], s[3], d[1]);
				d[2] = NR_COMPOSENPP_1111 (s[2], s[3], d[2]);
				d[3] = NR_COMPOSEA_111(s[3], d[3]);
				break;
			case NR_PIXBLOCK_MODE_R8G8B8A8P:
				d[0] = NR_COMPOSEPPP_1111 (s[0], s[3], d[0]);
				d[1] = NR_COMPOSEPPP_1111 (s[1], s[3], d[1]);
				d[2] = NR_COMPOSEPPP_1111 (s[2], s[3], d[2]);
				d[3] = NR_COMPOSEA_111(s[3], d[3]);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}


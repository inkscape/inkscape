#define __NR_PIXBLOCK_C__

/** \file
 * \brief Allocation/Setup of NRPixBlock objects. Pixel store functions.
 *
 * Authors:
 *   (C) 1999-2002 Lauris Kaplinski <lauris@kaplinski.com>
 *   2008, Jasper van de Gronde <th.v.d.gonde@hccnet.nl>
 *
 * This code is in the Public Domain
 */

#include <cstring>
#include <string>
#include <string.h>
#include <glib/gmem.h>
#include "nr-pixblock.h"

/// Size of buffer that needs no allocation (default 4).
#define NR_TINY_MAX sizeof (unsigned char *)

/**
 * Pixbuf initialisation using homegrown memory handling ("pixelstore").
 *
 * Pixbuf sizes are differentiated into tiny, <4K, <16K, <64K, and more,
 * with each type having its own method of memory handling. After allocating
 * memory, the buffer is cleared if the clear flag is set. Intended to
 * reduce memory fragmentation.
 * \param pb Pointer to the pixbuf struct.
 * \param mode Indicates grayscale/RGB/RGBA.
 * \param clear True if buffer should be cleared.
 * \pre x1>=x0 && y1>=y0 && pb!=NULL
 */
void
nr_pixblock_setup_fast (NRPixBlock *pb, NR_PIXBLOCK_MODE mode, int x0, int y0, int x1, int y1, bool clear)
{
	int w, h, bpp;
	size_t size;

	w = x1 - x0;
	h = y1 - y0;
	bpp = (mode == NR_PIXBLOCK_MODE_A8) ? 1 : (mode == NR_PIXBLOCK_MODE_R8G8B8) ? 3 : 4;

	size = bpp * w * h;

	if (size <= NR_TINY_MAX) {
		pb->size = NR_PIXBLOCK_SIZE_TINY;
		if (clear) memset (pb->data.p, 0x0, size);
	} else if (size <= 4096) {
		pb->size = NR_PIXBLOCK_SIZE_4K;
		pb->data.px = nr_pixelstore_4K_new (clear, 0x0);
	} else if (size <= 16384) {
		pb->size = NR_PIXBLOCK_SIZE_16K;
		pb->data.px = nr_pixelstore_16K_new (clear, 0x0);
	} else if (size <= 65536) {
		pb->size = NR_PIXBLOCK_SIZE_64K;
		pb->data.px = nr_pixelstore_64K_new (clear, 0x0);
	} else if (size <= 262144) {
		pb->size = NR_PIXBLOCK_SIZE_256K;
		pb->data.px = nr_pixelstore_256K_new (clear, 0x0);
	} else if (size <= 1048576) {
		pb->size = NR_PIXBLOCK_SIZE_1M;
		pb->data.px = nr_pixelstore_1M_new (clear, 0x0);
	} else {
		pb->size = NR_PIXBLOCK_SIZE_BIG;
             pb->data.px = NULL;
		if (size > 100000000) { // Don't even try to allocate more than 100Mb (5000x5000 RGBA
                            // pixels). It'll just bog the system down even if successful. FIXME:
                            // Can anyone suggest something better than the magic number?
                g_warning ("%lu bytes requested for pixel buffer, I won't try to allocate that.", (long unsigned) size);
                return;
             }
		pb->data.px = g_try_new (unsigned char, size);
		if (pb->data.px == NULL) { // memory allocation failed
                g_warning ("Could not allocate %lu bytes for pixel buffer!", (long unsigned) size);
                return;
             }
		if (clear) memset (pb->data.px, 0x0, size);
	}

	pb->mode = mode;
	pb->empty = 1;
    pb->visible_area.x0 = pb->area.x0 = x0;
    pb->visible_area.y0 = pb->area.y0 = y0;
    pb->visible_area.x1 = pb->area.x1 = x1;
    pb->visible_area.y1 = pb->area.y1 = y1;
	pb->rs = bpp * w;
}

/**
 * Pixbuf initialisation using g_new.
 *
 * After allocating memory, the buffer is cleared if the clear flag is set.
 * \param pb Pointer to the pixbuf struct.
 * \param mode Indicates grayscale/RGB/RGBA.
 * \param clear True if buffer should be cleared.
 * \pre x1>=x0 && y1>=y0 && pb!=NULL
 FIXME: currently unused except for nr_pixblock_new and pattern tiles, replace with _fast and delete?
 */
void
nr_pixblock_setup (NRPixBlock *pb, NR_PIXBLOCK_MODE mode, int x0, int y0, int x1, int y1, bool clear)
{
	int w, h, bpp;
	size_t size;

	w = x1 - x0;
	h = y1 - y0;
	bpp = (mode == NR_PIXBLOCK_MODE_A8) ? 1 : (mode == NR_PIXBLOCK_MODE_R8G8B8) ? 3 : 4;

	size = bpp * w * h;

	if (size <= NR_TINY_MAX) {
		pb->size = NR_PIXBLOCK_SIZE_TINY;
		if (clear) memset (pb->data.p, 0x0, size);
	} else {
		pb->size = NR_PIXBLOCK_SIZE_BIG;
		pb->data.px = g_new (unsigned char, size);
		if (clear) memset (pb->data.px, 0x0, size);
	}

	pb->mode = mode;
	pb->empty = 1;
    pb->visible_area.x0 = pb->area.x0 = x0;
    pb->visible_area.y0 = pb->area.y0 = y0;
    pb->visible_area.x1 = pb->area.x1 = x1;
    pb->visible_area.y1 = pb->area.y1 = y1;
	pb->rs = bpp * w;
}

/**
 * Pixbuf initialisation with preset values.
 *
 * After copying all parameters into the NRPixBlock struct, the pixel buffer is cleared if the clear flag is set.
 * \param pb Pointer to the pixbuf struct.
 * \param mode Indicates grayscale/RGB/RGBA.
 * \param clear True if buffer should be cleared.
 * \pre x1>=x0 && y1>=y0 && pb!=NULL
 */
void
nr_pixblock_setup_extern (NRPixBlock *pb, NR_PIXBLOCK_MODE mode, int x0, int y0, int x1, int y1, unsigned char *px, int rs, bool empty, bool clear)
{
	int w, bpp;

	w = x1 - x0;
	bpp = (mode == NR_PIXBLOCK_MODE_A8) ? 1 : (mode == NR_PIXBLOCK_MODE_R8G8B8) ? 3 : 4;

	pb->size = NR_PIXBLOCK_SIZE_STATIC; 
	pb->mode = mode;
	pb->empty = empty;
	pb->visible_area.x0 = pb->area.x0 = x0;
	pb->visible_area.y0 = pb->area.y0 = y0;
	pb->visible_area.x1 = pb->area.x1 = x1;
	pb->visible_area.y1 = pb->area.y1 = y1;
	pb->data.px = px;
	pb->rs = rs;

        g_assert (pb->data.px != NULL);
	if (clear) {
		if (rs == bpp * w) {
			/// \todo How do you recognise if 
                        /// px was an uncleared tiny buffer? 
			if (pb->data.px) 
				memset (pb->data.px, 0x0, bpp * (y1 - y0) * w);
		} else {
			int y;
			for (y = y0; y < y1; y++) {
				memset (pb->data.px + (y - y0) * rs, 0x0, bpp * w);
			}
		}
	}
}

/**
 * Frees memory taken by pixel data in NRPixBlock.
 * \param pb Pointer to pixblock.
 * \pre pb and pb->data.px point to valid addresses.
 *
 * According to pb->size, one of the functions for freeing the pixelstore
 * is called. May be called regardless of how pixbuf was set up.
 */
void
nr_pixblock_release (NRPixBlock *pb)
{
	switch (pb->size) {
	case NR_PIXBLOCK_SIZE_TINY:
		break;
	case NR_PIXBLOCK_SIZE_4K:
		nr_pixelstore_4K_free (pb->data.px);
		break;
	case NR_PIXBLOCK_SIZE_16K:
		nr_pixelstore_16K_free (pb->data.px);
		break;
	case NR_PIXBLOCK_SIZE_64K:
		nr_pixelstore_64K_free (pb->data.px);
		break;
	case NR_PIXBLOCK_SIZE_256K:
		nr_pixelstore_256K_free (pb->data.px);
		break;
	case NR_PIXBLOCK_SIZE_1M:
		nr_pixelstore_1M_free (pb->data.px);
		break;
	case NR_PIXBLOCK_SIZE_BIG:
		g_free (pb->data.px);
		break;
	case NR_PIXBLOCK_SIZE_STATIC:
		break;
	default:
		break;
	}
}

/**
 * Allocates NRPixBlock and sets it up.
 *
 * \return Pointer to fresh pixblock.
 * Calls g_new() and nr_pixblock_setup().
FIXME: currently unused, delete? JG: Should be used more often! (To simplify memory management.)
 */
NRPixBlock *
nr_pixblock_new (NR_PIXBLOCK_MODE mode, int x0, int y0, int x1, int y1, bool clear)
{
	NRPixBlock *pb;

	pb = g_new (NRPixBlock, 1);
    if (!pb) return 0;

	nr_pixblock_setup (pb, mode, x0, y0, x1, y1, clear);
    if (pb->size!=NR_PIXBLOCK_SIZE_TINY && !pb->data.px) {
        g_free(pb);
        return 0;
    }

	return pb;
}

/**
 * Allocates NRPixBlock and sets it up.
 *
 * \return Pointer to fresh pixblock.
 * Calls g_new() and nr_pixblock_setup().
 */
NRPixBlock *
nr_pixblock_new_fast (NR_PIXBLOCK_MODE mode, int x0, int y0, int x1, int y1, bool clear)
{
    NRPixBlock *pb;

    pb = g_new (NRPixBlock, 1);
    if (!pb) return 0;

    nr_pixblock_setup_fast (pb, mode, x0, y0, x1, y1, clear);
    if (pb->size!=NR_PIXBLOCK_SIZE_TINY && !pb->data.px) {
        g_free(pb);
        return 0;
    }

    return pb;
}

/**
 * Frees all memory taken by pixblock.
 *
 * \return NULL
 */
NRPixBlock *
nr_pixblock_free (NRPixBlock *pb)
{
	nr_pixblock_release (pb);

	g_free (pb);

	return NULL;
}

/* PixelStore operations */

#define NR_4K_BLOCK 32
static unsigned char **nr_4K_px = NULL;
static unsigned int nr_4K_len = 0;
static unsigned int nr_4K_size = 0;

unsigned char *
nr_pixelstore_4K_new (bool clear, unsigned char val)
{
	unsigned char *px;

	if (nr_4K_len != 0) {
		nr_4K_len -= 1;
		px = nr_4K_px[nr_4K_len];
	} else {
		px = g_new (unsigned char, 4096);
	}
	
	if (clear) memset (px, val, 4096);

	return px;
}

void
nr_pixelstore_4K_free (unsigned char *px)
{
	if (nr_4K_len == nr_4K_size) {
		nr_4K_size += NR_4K_BLOCK;
		nr_4K_px = g_renew (unsigned char *, nr_4K_px, nr_4K_size);
	}

	nr_4K_px[nr_4K_len] = px;
	nr_4K_len += 1;
}

#define NR_16K_BLOCK 32
static unsigned char **nr_16K_px = NULL;
static unsigned int nr_16K_len = 0;
static unsigned int nr_16K_size = 0;

unsigned char *
nr_pixelstore_16K_new (bool clear, unsigned char val)
{
	unsigned char *px;

	if (nr_16K_len != 0) {
		nr_16K_len -= 1;
		px = nr_16K_px[nr_16K_len];
	} else {
		px = g_new (unsigned char, 16384);
	}
	
	if (clear) memset (px, val, 16384);

	return px;
}

void
nr_pixelstore_16K_free (unsigned char *px)
{
	if (nr_16K_len == nr_16K_size) {
		nr_16K_size += NR_16K_BLOCK;
		nr_16K_px = g_renew (unsigned char *, nr_16K_px, nr_16K_size);
	}

	nr_16K_px[nr_16K_len] = px;
	nr_16K_len += 1;
}

#define NR_64K_BLOCK 32
static unsigned char **nr_64K_px = NULL;
static unsigned int nr_64K_len = 0;
static unsigned int nr_64K_size = 0;

unsigned char *
nr_pixelstore_64K_new (bool clear, unsigned char val)
{
	unsigned char *px;

	if (nr_64K_len != 0) {
		nr_64K_len -= 1;
		px = nr_64K_px[nr_64K_len];
	} else {
		px = g_new (unsigned char, 65536);
	}

	if (clear) memset (px, val, 65536);

	return px;
}

void
nr_pixelstore_64K_free (unsigned char *px)
{
	if (nr_64K_len == nr_64K_size) {
		nr_64K_size += NR_64K_BLOCK;
		nr_64K_px = g_renew (unsigned char *, nr_64K_px, nr_64K_size);
	}

	nr_64K_px[nr_64K_len] = px;
	nr_64K_len += 1;
}

#define NR_256K_BLOCK 32
#define NR_256K 262144
static unsigned char **nr_256K_px = NULL;
static unsigned int nr_256K_len = 0;
static unsigned int nr_256K_size = 0;

unsigned char *
nr_pixelstore_256K_new (bool clear, unsigned char val)
{
	unsigned char *px;

	if (nr_256K_len != 0) {
		nr_256K_len -= 1;
		px = nr_256K_px[nr_256K_len];
	} else {
           px = g_new (unsigned char, NR_256K);
	}

	if (clear) memset (px, val, NR_256K);

	return px;
}

void
nr_pixelstore_256K_free (unsigned char *px)
{
	if (nr_256K_len == nr_256K_size) {
		nr_256K_size += NR_256K_BLOCK;
		nr_256K_px = g_renew (unsigned char *, nr_256K_px, nr_256K_size);
	}

	nr_256K_px[nr_256K_len] = px;
	nr_256K_len += 1;
}

#define NR_1M_BLOCK 32
#define NR_1M 1048576
static unsigned char **nr_1M_px = NULL;
static unsigned int nr_1M_len = 0;
static unsigned int nr_1M_size = 0;

unsigned char *
nr_pixelstore_1M_new (bool clear, unsigned char val)
{
	unsigned char *px;

	if (nr_1M_len != 0) {
		nr_1M_len -= 1;
		px = nr_1M_px[nr_1M_len];
	} else {
           px = g_new (unsigned char, NR_1M);
	}

	if (clear) memset (px, val, NR_1M);

	return px;
}

void
nr_pixelstore_1M_free (unsigned char *px)
{
	if (nr_1M_len == nr_1M_size) {
		nr_1M_size += NR_1M_BLOCK;
		nr_1M_px = g_renew (unsigned char *, nr_1M_px, nr_1M_size);
	}

	nr_1M_px[nr_1M_len] = px;
	nr_1M_len += 1;
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

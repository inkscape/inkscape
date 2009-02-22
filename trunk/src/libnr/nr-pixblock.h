#ifndef __NR_PIXBLOCK_H__
#define __NR_PIXBLOCK_H__

/** \file
 * \brief Pixel block structure. Used for low-level rendering.
 *
 * Authors:
 *   (C) 1999-2002 Lauris Kaplinski <lauris@kaplinski.com>
 *   (C) 2005 Ralf Stephan <ralf@ark.in-berlin.de> (some cleanup)
 *
 * This code is in the Public Domain.
 */

#include <libnr/nr-rect-l.h>
#include <libnr/nr-forward.h>

/// Size indicator. Hardcoded to max. 3 bits.
typedef enum {
	NR_PIXBLOCK_SIZE_TINY, ///< Fits in (unsigned char *)
	NR_PIXBLOCK_SIZE_4K,   ///< Pixelstore 
	NR_PIXBLOCK_SIZE_16K,  ///< Pixelstore 
	NR_PIXBLOCK_SIZE_64K,  ///< Pixelstore 
	NR_PIXBLOCK_SIZE_256K,  ///< Pixelstore 
	NR_PIXBLOCK_SIZE_1M,  ///< Pixelstore 
	NR_PIXBLOCK_SIZE_BIG,  ///< Normally allocated 
	NR_PIXBLOCK_SIZE_STATIC ///< Externally managed 
} NR_PIXBLOCK_SIZE;

/// Mode indicator. Hardcoded to max. 2 bits.
typedef enum {
	NR_PIXBLOCK_MODE_A8,        ///< Grayscale
	NR_PIXBLOCK_MODE_R8G8B8,    ///< 8 bit RGB
	NR_PIXBLOCK_MODE_R8G8B8A8N, ///< Normal 8 bit RGBA
	NR_PIXBLOCK_MODE_R8G8B8A8P  ///< Premultiplied 8 bit RGBA
} NR_PIXBLOCK_MODE;

/// The pixel block struct.
struct NRPixBlock {
	NR_PIXBLOCK_SIZE size : 3; ///< Size indicator
	NR_PIXBLOCK_MODE mode : 2; ///< Mode indicator
	bool empty : 1;            ///< Empty flag
	unsigned int rs;           ///< Size of line in bytes
	NRRectL area;
    NRRectL visible_area;
	union {
		unsigned char *px; ///< Pointer to buffer
		unsigned char p[sizeof (unsigned char *)]; ///< Tiny buffer
	} data;
};

/// Returns number of bytes per pixel (1, 3, or 4).
inline int 
NR_PIXBLOCK_BPP (NRPixBlock *pb)
{ 
    return ((pb->mode == NR_PIXBLOCK_MODE_A8) ? 1 : 
            (pb->mode == NR_PIXBLOCK_MODE_R8G8B8) ? 3 : 4); 
}
    
/// Returns pointer to pixel data.
inline unsigned char*
NR_PIXBLOCK_PX (NRPixBlock *pb) 
{ 
    return ((pb->size == NR_PIXBLOCK_SIZE_TINY) ? 
            pb->data.p : pb->data.px);
}

void nr_pixblock_setup (NRPixBlock *pb, NR_PIXBLOCK_MODE mode, int x0, int y0, int x1, int y1, bool clear);
void nr_pixblock_setup_fast (NRPixBlock *pb, NR_PIXBLOCK_MODE mode, int x0, int y0, int x1, int y1, bool clear);
void nr_pixblock_setup_extern (NRPixBlock *pb, NR_PIXBLOCK_MODE mode, int x0, int y0, int x1, int y1, unsigned char *px, int rs, bool empty, bool clear);
void nr_pixblock_release (NRPixBlock *pb);

NRPixBlock *nr_pixblock_new (NR_PIXBLOCK_MODE mode, int x0, int y0, int x1, int y1, bool clear);
NRPixBlock *nr_pixblock_new_fast (NR_PIXBLOCK_MODE mode, int x0, int y0, int x1, int y1, bool clear);
NRPixBlock *nr_pixblock_free (NRPixBlock *pb);

unsigned char *nr_pixelstore_4K_new (bool clear, unsigned char val);
void nr_pixelstore_4K_free (unsigned char *px);
unsigned char *nr_pixelstore_16K_new (bool clear, unsigned char val);
void nr_pixelstore_16K_free (unsigned char *px);
unsigned char *nr_pixelstore_64K_new (bool clear, unsigned char val);
void nr_pixelstore_64K_free (unsigned char *px);
unsigned char *nr_pixelstore_256K_new (bool clear, unsigned char val);
void nr_pixelstore_256K_free (unsigned char *px);
unsigned char *nr_pixelstore_1M_new (bool clear, unsigned char val);
void nr_pixelstore_1M_free (unsigned char *px);

#endif
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

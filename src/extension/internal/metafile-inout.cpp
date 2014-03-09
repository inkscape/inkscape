/** @file
 * @brief Metafile input - common routines
 *//*
 * Authors:
 *   David Mathog
 *
 * Copyright (C) 2013 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <fstream>
#include <glib.h>
#include <glibmm/miscutils.h>

#include "display/curve.h"
#include "extension/internal/metafile-inout.h" // picks up PNG
#include "extension/print.h"
#include "path-prefix.h"
#include "sp-gradient.h"
#include "sp-image.h"
#include "sp-linear-gradient.h"
#include "sp-pattern.h"
#include "sp-radial-gradient.h"
#include "style.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

Metafile::~Metafile()
{
    return;
}

/** Construct a PNG in memory from an RGB from the EMF file

from:
http://www.lemoda.net/c/write-png/

which was based on:
http://stackoverflow.com/questions/1821806/how-to-encode-png-to-buffer-using-libpng

gcc -Wall -o testpng testpng.c -lpng

Originally here, but moved up

#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
*/


/*  Given "bitmap", this returns the pixel of bitmap at the point
    ("x", "y"). */

pixel_t * Metafile::pixel_at (bitmap_t * bitmap, int x, int y)
{
    return bitmap->pixels + bitmap->width * y + x;
}


/*  Write "bitmap" to a PNG file specified by "path"; returns 0 on
    success, non-zero on error. */

void
Metafile::my_png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    PMEMPNG p=(PMEMPNG)png_get_io_ptr(png_ptr);

    size_t nsize = p->size + length;

    /* allocate or grow buffer */
    if(p->buffer){ p->buffer = (char *) realloc(p->buffer, nsize); }
    else{          p->buffer = (char *) malloc(nsize);             }

    if(!p->buffer){ png_error(png_ptr, "Write Error"); }

    /* copy new bytes to end of buffer */
    memcpy(p->buffer + p->size, data, length);
    p->size += length;
}

void Metafile::toPNG(PMEMPNG accum, int width, int height, const char *px){
    bitmap_t bmStore;
    bitmap_t *bitmap = &bmStore;
    accum->buffer=NULL;  // PNG constructed in memory will end up here, caller must free().
    accum->size=0;
    bitmap->pixels=(pixel_t *)px;
    bitmap->width  = width;
    bitmap->height = height;

    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    size_t x, y;
    png_byte ** row_pointers = NULL;
    /*  The following number is set by trial and error only. I cannot
        see where it it is documented in the libpng manual.
    */
    int pixel_size = 3;
    int depth = 8;

    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL){
        accum->buffer=NULL;
        return;
    }

    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL){
        png_destroy_write_struct (&png_ptr, &info_ptr);
        accum->buffer=NULL;
        return;
    }

    /* Set up error handling. */

    if (setjmp (png_jmpbuf (png_ptr))) {
        png_destroy_write_struct (&png_ptr, &info_ptr);
        accum->buffer=NULL;
        return;
    }

    /* Set image attributes. */

    png_set_IHDR (
        png_ptr,
        info_ptr,
        bitmap->width,
        bitmap->height,
        depth,
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );

    /* Initialize rows of PNG. */

    row_pointers = (png_byte **) png_malloc (png_ptr, bitmap->height * sizeof (png_byte *));
    for (y = 0; y < bitmap->height; ++y) {
        png_byte *row =
            (png_byte *) png_malloc (png_ptr, sizeof (uint8_t) * bitmap->width * pixel_size);
        row_pointers[bitmap->height - y - 1] = row;  // Row order in EMF is reversed.
        for (x = 0; x < bitmap->width; ++x) {
            pixel_t * pixel = pixel_at (bitmap, x, y);
            *row++ = pixel->red;   // R & B channels were set correctly by DIB_to_RGB
            *row++ = pixel->green;
            *row++ = pixel->blue;
        }
    }

    /* Write the image data to memory */

    png_set_rows (png_ptr, info_ptr, row_pointers);

    png_set_write_fn(png_ptr, accum, my_png_write_data, NULL);

    png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    for (y = 0; y < bitmap->height; y++) {
        png_free (png_ptr, row_pointers[y]);
    }
    png_free (png_ptr, row_pointers);
    png_destroy_write_struct(&png_ptr, &info_ptr);

}


/* convert an EMF RGB(A) color to 0RGB
inverse of gethexcolor() in emf-print.cpp
*/
uint32_t Metafile::sethexcolor(U_COLORREF color){

    uint32_t out;
    out = (U_RGBAGetR(color) << 16) +
          (U_RGBAGetG(color) << 8 ) +
          (U_RGBAGetB(color)      );
    return(out);
}

/* Return the base64 encoded png which is shown for all bad images.
Currently a random 3x4 blotch.
Caller must free.
*/
gchar *Metafile::bad_image_png(void){
    gchar *gstring = g_strdup("iVBORw0KGgoAAAANSUhEUgAAAAQAAAADCAIAAAA7ljmRAAAAA3NCSVQICAjb4U/gAAAALElEQVQImQXBQQ2AMAAAsUJQMSWI2H8qME1yMshojwrvGB8XcHKvR1XtOTc/8HENumHCsOMAAAAASUVORK5CYII=");
    return(gstring);
}



} // namespace Internal
} // namespace Extension
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

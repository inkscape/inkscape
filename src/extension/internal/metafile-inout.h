/** @file
 * @brief Metafile input - common functions
 *//*
 * Authors:
 *   David Mathog
 *
 * Copyright (C) 2013 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_EXTENSION_INTERNAL_METAFILE_INOUT_H
#define SEEN_INKSCAPE_EXTENSION_INTERNAL_METAFILE_INOUT_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define PNG_SKIP_SETJMP_CHECK // else any further png.h include blows up in the compiler
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <map>
#include <stack>
#include <glibmm/ustring.h>
#include <libuemf/uemf.h>
#include <2geom/affine.h>
#include <2geom/pathvector.h>

#include "extension/implementation/implementation.h"

class SPObject;

namespace Inkscape {
class Pixbuf;

namespace Extension {
namespace Internal {

/* A coloured pixel. */
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t opacity;
} pixel_t;

/* A picture. */
    typedef struct  {
    pixel_t *pixels;
    size_t width;
    size_t height;
} bitmap_t;
    
/* structure to store PNG image bytes */
typedef struct {
      char *buffer;
      size_t size;
} MEMPNG, *PMEMPNG;

class Metafile
    : public Inkscape::Extension::Implementation::Implementation
{
public:
    Metafile() {}
    ~Metafile();

protected:
    static uint32_t    sethexcolor(U_COLORREF color);
    static pixel_t    *pixel_at (bitmap_t * bitmap, int x, int y);
    static void        my_png_write_data(png_structp png_ptr, png_bytep data, png_size_t length);
    static void        toPNG(PMEMPNG accum, int width, int height, const char *px);
    static gchar      *bad_image_png(void);
    static void        setViewBoxIfMissing(SPDocument *doc);
    static int         combine_ops_to_livarot(const int op);


private:
};

} // namespace Internal
} // namespace Extension
} // namespace Inkscape

#endif // SEEN_INKSCAPE_EXTENSION_INTERNAL_METAFILE_INOUT_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/

#define __SP_CURSOR_C__

/*
 * Some convenience stuff
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstdio>
#include <cstring>
#include <string>
#include <ctype.h>
#include "sp-cursor.h"

void
sp_cursor_bitmap_and_mask_from_xpm(GdkBitmap **bitmap, GdkBitmap **mask, gchar const *const *xpm)
{
    int height;
    int width;
    int colors;
    int pix;
    sscanf(xpm[0], "%d %d %d %d", &height, &width, &colors, &pix);

    g_return_if_fail (height == 32);
    g_return_if_fail (width == 32);
    g_return_if_fail (colors >= 3);

    int transparent_color = ' ';
    int black_color = '.';

    char pixmap_buffer[(32 * 32)/8];
    char mask_buffer[(32 * 32)/8];

    for (int i = 0; i < colors; i++) {

        char const *p = xpm[1 + i];
        char const ccode = *p;

        p++;
        while (isspace(*p)) {
            p++;
        }
        p++;
        while (isspace(*p)) {
            p++;
        }

	if (strcmp(p, "None") == 0) {
            transparent_color = ccode;
        }

        if (strcmp(p, "#000000") == 0) {
            black_color = ccode;
        }
    }

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; ) {

            char value = 0;
            char maskv = 0;
			
            for (int pix = 0; pix < 8; pix++, x++){
                if (xpm[4+y][x] != transparent_color) {
                    maskv |= 1 << pix;

                    if (xpm[4+y][x] == black_color) {
                        value |= 1 << pix;
                    }
                }
            }

            pixmap_buffer[(y * 4 + x/8)-1] = value;
            mask_buffer[(y * 4 + x/8)-1] = maskv;
        }
    }

    *bitmap = gdk_bitmap_create_from_data(NULL, pixmap_buffer, 32, 32);
    *mask   = gdk_bitmap_create_from_data(NULL, mask_buffer, 32, 32);
}

GdkCursor *
sp_cursor_new_from_xpm(gchar const *const *xpm, gint hot_x, gint hot_y)
{
    GdkColor const fg = { 0, 0, 0, 0 };
    GdkColor const bg = { 0, 65535, 65535, 65535 };

    GdkBitmap *bitmap = NULL;
    GdkBitmap *mask = NULL;

    sp_cursor_bitmap_and_mask_from_xpm (&bitmap, &mask, xpm);
    if ( bitmap != NULL && mask != NULL ) {
        GdkCursor *new_cursor = gdk_cursor_new_from_pixmap (bitmap, mask,
                                           &fg, &bg,
                                           hot_x, hot_y);
        g_object_unref (bitmap);
        g_object_unref (mask);
        return new_cursor;
    }

    return NULL;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

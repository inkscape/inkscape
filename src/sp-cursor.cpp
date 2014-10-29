/*
 * Some convenience stuff
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 2010 Jasper van de Gronde
 * Copyright (C) 2010 Jon A. Cruz
 * Copyright (C) 2012 Kris De Gussem
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstring>
#include <gdk/gdk.h>
#include <map>
#include <sstream>

#include "color.h"
#include "sp-cursor.h"

static void free_cursor_data(unsigned char *pixels, void* /*data*/) {
    delete [] reinterpret_cast<guint32*>(pixels);
}

struct RGBA {
    guchar v[4];

    RGBA() { 
        v[0] = 0;
        v[1] = 0;
        v[2] = 0;
        v[3] = 0;
    }

    RGBA(guchar r, guchar g, guchar b, guchar a) {
        v[0] = r;
        v[1] = g;
        v[2] = b;
        v[3] = a;
    }

    operator guint32() const {
        guint32 result = (static_cast<guint32>(v[0]) << 0)
            | (static_cast<guint32>(v[1]) << 8)
            | (static_cast<guint32>(v[2]) << 16)
            | (static_cast<guint32>(v[3]) << 24);
        return result;
    }
};

GdkPixbuf *sp_cursor_pixbuf_from_xpm(char const *const *xpm, GdkColor const& black, GdkColor const& white, guint32 fill, guint32 stroke)
{
    int height = 0;
    int width = 0;
    int colors = 0;
    int pix = 0;
    std::stringstream ss (std::stringstream::in | std::stringstream::out);
    ss << xpm[0];
    ss >> height;
    ss >> width;
    ss >> colors;
    ss >> pix;
    
    std::map<char, RGBA> colorMap;

    for (int i = 0; i < colors; i++) {

        char const *p = xpm[1 + i];
        g_assert(*p >=0);
        unsigned char const ccode = (guchar) *p;

        p++;
        while (isspace(*p)) {
            p++;
        }
        p++;
        while (isspace(*p)) {
            p++;
        }

        if (strcmp(p, "None") == 0) {
            colorMap[ccode] = RGBA();
        } else if (strcmp(p, "Fill") == 0) {
            colorMap[ccode] = RGBA(SP_RGBA32_R_U(fill), SP_RGBA32_G_U(fill), SP_RGBA32_B_U(fill), SP_RGBA32_A_U(fill));
        } else if (strcmp(p, "Stroke") == 0) {
            colorMap[ccode] = RGBA(SP_RGBA32_R_U(stroke), SP_RGBA32_G_U(stroke), SP_RGBA32_B_U(stroke), SP_RGBA32_A_U(stroke));
        } else if (strcmp(p, "#000000") == 0) {
            colorMap[ccode] = RGBA(black.red, black.green, black.blue, 255);
        } else if (strcmp(p, "#FFFFFF") == 0) {
            colorMap[ccode] = RGBA(white.red, white.green, white.blue, 255);
        } else {
            colorMap[ccode] = RGBA();
        }
    }

    guint32 *pixmap_buffer = new guint32[width * height];

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            std::map<char, RGBA>::const_iterator it = colorMap.find(xpm[1 + colors + y][x]);
            pixmap_buffer[y * width + x] = (it == colorMap.end()) ? 0u : it->second;
        }
    }

    return gdk_pixbuf_new_from_data(reinterpret_cast<guchar*>(pixmap_buffer), GDK_COLORSPACE_RGB, TRUE, 8, width, height, width * sizeof(guint32), free_cursor_data, NULL);
}

GdkCursor *sp_cursor_new_from_xpm(char const *const *xpm, int hot_x, int hot_y)
{
    GdkCursor *cursor = 0;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **)xpm);
    
    if (pixbuf) {
        cursor = gdk_cursor_new_from_pixbuf(gdk_display_get_default(),
			pixbuf, hot_x, hot_y);

        g_object_unref(pixbuf);
    }

    return cursor;
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

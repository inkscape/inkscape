#define __SP_CURSOR_C__

/*
 * Use a pixmap to make a cursor.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   James ----
 *
 * Copyright (C) 1999-2006 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstdio>
#include <string.h>
#include <ctype.h>
#include "sp-cursor.h"

GdkCursor *sp_cursor_new (GdkDisplay *display, GdkPixbuf *pixbuf, gchar **xpm, gint hot_x, gint hot_y)
{
    GdkCursor *new_cursor=NULL;

    if ((!pixbuf && xpm) || !gdk_display_supports_cursor_alpha(display)) 
        //if there is no pixbuf, but there is xpm data, or the display 
        //doesn't support alpha, use bitmap cursor.
    {
        pixbuf=gdk_pixbuf_new_from_xpm_data((const char**)xpm);
    }
    if(pixbuf) {
        new_cursor = gdk_cursor_new_from_pixbuf(display,pixbuf,hot_x,hot_y);
    }
    return new_cursor;
}



GdkCursor *sp_cursor_new_from_xpm (gchar **xpm, gint hot_x, gint hot_y)
{
    GdkDisplay *display=gdk_display_get_default();
    GdkPixbuf *pixbuf=NULL;
    GdkCursor *new_cursor=NULL;
    pixbuf=gdk_pixbuf_new_from_xpm_data((const char**)xpm);
    if (pixbuf != NULL){
        new_cursor = gdk_cursor_new_from_pixbuf(display,pixbuf,hot_x,hot_y);
        g_message("xpm cursor defined\n");
        return new_cursor;
    }
    g_warning("xpm cursor not defined\n");

    return NULL;
}



GdkCursor *sp_cursor_new_from_pixbuf (GdkPixbuf *pixbuf, gint hot_x, gint hot_y)
{
    GdkDisplay *display=gdk_display_get_default();
    GdkCursor *new_cursor=NULL;
    if (pixbuf) {
        new_cursor = gdk_cursor_new_from_pixbuf(display,pixbuf,hot_x,hot_y);
        g_message("pixbuf cursor defined\n");
        return new_cursor;
     }
    g_warning("pixbuf cursor not defined\n");
        return new_cursor;
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

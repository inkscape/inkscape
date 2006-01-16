#ifndef __NR_PLAIN_STUFF_GDK_H__
#define __NR_PLAIN_STUFF_GDK_H__

/*
 * Miscellaneous simple rendering utilities
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Lauris Kaplinski and Ximian, Inc.
 *
 * Released under GNU GPL
 */

#include <gdk/gdk.h>

void nr_gdk_draw_rgba32_solid (GdkDrawable *drawable, GdkGC *gc, gint x, gint y, gint w, gint h, guint32 rgba);

void nr_gdk_draw_gray_garbage (GdkDrawable *drawable, GdkGC *gc, gint x, gint y, gint w, gint h);

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

#define __NR_PLAIN_STUFF_GDK_C__

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

#include <libnr/nr-pixblock-pattern.h>
#include "nr-plain-stuff.h"
#include "nr-plain-stuff-gdk.h"

void
nr_gdk_draw_rgba32_solid (GdkDrawable *drawable, GdkGC *gc, gint x, gint y, gint w, gint h, guint32 rgba)
{
	NRPixBlock pb;

	nr_pixblock_setup_fast (&pb, NR_PIXBLOCK_MODE_R8G8B8A8N, 0, 0, w, h, FALSE);

	nr_render_rgba32_rgb (NR_PIXBLOCK_PX (&pb), w, h, pb.rs, x, y, rgba);
	gdk_draw_rgb_image (drawable, gc, x, y, w, h, GDK_RGB_DITHER_MAX, NR_PIXBLOCK_PX (&pb), pb.rs);

	nr_pixblock_release (&pb);
}

void
nr_gdk_draw_gray_garbage (GdkDrawable *drawable, GdkGC *gc, gint x, gint y, gint w, gint h)
{
	for (gint yy = y; yy < y + h; yy += 64) {
		for (gint xx = x; xx < x + w; xx += 64) {
			NRPixBlock pb;
			gint ex = MIN (xx + 64, x + w);
			gint ey = MIN (yy + 64, y + h);
			nr_pixblock_setup_fast (&pb, NR_PIXBLOCK_MODE_R8G8B8, xx, yy, ex, ey, FALSE);
			nr_pixblock_render_gray_noise (&pb, NULL);
			gdk_draw_rgb_image (drawable, gc, xx, yy, ex - xx, ey - yy, GDK_RGB_DITHER_NONE, NR_PIXBLOCK_PX (&pb), pb.rs);
			nr_pixblock_release (&pb);
		}
	}
}


#ifndef __SP_PIXBUF_OPS_H__
#define __SP_PIXBUF_OPS_H__

/*
 * Helpers for SPItem -> gdk_pixbuf related stuff
 *
 * Authors:
 *   John Cliff <simarilius@yahoo.com>
 *
 * Copyright (C) 2008 John Cliff
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gtypes.h>

struct Document;

bool sp_export_jpg_file (Document *doc, gchar const *filename, double x0, double y0, double x1, double y1,
             unsigned int width, unsigned int height, double xdpi, double ydpi, unsigned long bgcolor, double quality, GSList *items_only = NULL);

GdkPixbuf* sp_generate_internal_bitmap(Document *doc, gchar const *filename,
                   double x0, double y0, double x1, double y1,
                   unsigned width, unsigned height, double xdpi, double ydpi,
                   unsigned long bgcolor, GSList *items_only = NULL);

#endif

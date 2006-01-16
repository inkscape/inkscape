#ifndef __SP_PNG_WRITE_H__
#define __SP_PNG_WRITE_H__

/*
 * PNG file format utilities
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gtypes.h>

int sp_png_write_rgba(gchar const *filename, guchar const *px, int width, int height, int rowstride);

int sp_png_write_rgba_striped(gchar const *filename, int width, int height,
			      int (* get_rows) (guchar const **rows, int row, int num_rows, void *data),
			      void *data);

#endif

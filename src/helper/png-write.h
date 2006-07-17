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

int sp_png_write_rgba(gchar const *filename, guchar const *px, int width, int height, double xdpi, double ydpi, int rowstride);

int sp_png_write_rgba_striped(gchar const *filename, int width, int height, double xdpi, double ydpi,
			      int (* get_rows) (guchar const **rows, int row, int num_rows, void *data),
			      void *data);

/**
 * Export the given document as a Portable Network Graphics (PNG)
 * file.  Returns FALSE if an error was encountered while writing
 * the file, TRUE otherwise.
 */
int sp_export_png_file (SPDocument *doc, const gchar *filename,
			 double x0, double y0, double x1, double y1,
			 unsigned int width, unsigned int height, double xdpi, double ydpi,
			 unsigned long bgcolor,
			 unsigned int (*status) (float, void *), void *data, bool force_overwrite = false, GSList *items_only = NULL);

#endif

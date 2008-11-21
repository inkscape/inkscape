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
#include <2geom/forward.h>
struct SPDocument;

bool sp_export_png_file (SPDocument *doc, gchar const *filename,
            double x0, double y0, double x1, double y1,
            unsigned long int width, unsigned long int height, double xdpi, double ydpi,
            unsigned long bgcolor,
            unsigned int (*status) (float, void *), void *data, bool force_overwrite = false, GSList *items_only = NULL);
bool sp_export_png_file (SPDocument *doc, gchar const *filename,
            Geom::Rect const &area,
            unsigned long int width, unsigned long int height, double xdpi, double ydpi,
            unsigned long bgcolor,
            unsigned int (*status) (float, void *), void *data, bool force_overwrite = false, GSList *items_only = NULL);

#endif

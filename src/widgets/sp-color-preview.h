#ifndef SEEN_COLOR_PREVIEW_H
#define SEEN_COLOR_PREVIEW_H

/*
 * A simple color preview widget
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtkwidget.h>

#include <glib.h>



#define SP_TYPE_COLOR_PREVIEW (sp_color_preview_get_type ())
#define SP_COLOR_PREVIEW(o) (GTK_CHECK_CAST ((o), SP_TYPE_COLOR_PREVIEW, SPColorPreview))
#define SP_COLOR_PREVIEW_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_COLOR_PREVIEW, SPColorPreviewClass))
#define SP_IS_COLOR_PREVIEW(o) (GTK_CHECK_TYPE ((o), SP_TYPE_COLOR_PREVIEW))
#define SP_IS_COLOR_PREVIEW_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_COLOR_PREVIEW))

struct SPColorPreview {
	GtkWidget widget;

	guint32 rgba;
};

struct SPColorPreviewClass {
	GtkWidgetClass parent_class;
};

GtkType sp_color_preview_get_type (void);

GtkWidget *sp_color_preview_new (guint32 rgba);

void sp_color_preview_set_rgba32 (SPColorPreview *cp, guint32 color);


#endif // SEEN_COLOR_PREVIEW_H

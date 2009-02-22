#ifndef __SP_WIDGET_H__
#define __SP_WIDGET_H__

/*
 * Abstract base class for dynamic control widgets
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

#define SP_TYPE_WIDGET (sp_widget_get_type ())
#define SP_WIDGET(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_WIDGET, SPWidget))
#define SP_WIDGET_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_WIDGET, SPWidgetClass))
#define SP_IS_WIDGET(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_WIDGET))
#define SP_IS_WIDGET_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_WIDGET))

#include <gtk/gtkbin.h>

namespace Inkscape {
	class Application;
	class Selection;
}

struct SPWidget {
	GtkBin bin;
	Inkscape::Application *inkscape;
};

struct SPWidgetClass {
	GtkBinClass bin_class;
	void (* construct) (SPWidget *spw);
	/* Selection change handlers */
	void (* modify_selection) (SPWidget *spw, Inkscape::Selection *selection, guint flags);
	void (* change_selection) (SPWidget *spw, Inkscape::Selection *selection);
	void (* set_selection) (SPWidget *spw, Inkscape::Selection *selection);
};

GtkType sp_widget_get_type (void);

/* fixme: Think (Lauris) */
/* Generic constructor for global widget */
GtkWidget *sp_widget_new_global (Inkscape::Application *inkscape);
GtkWidget *sp_widget_construct_global (SPWidget *spw, Inkscape::Application *inkscape);

#endif

#define __SLIDESHOW_C__

/*
 * Help/About window
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 authors
 * Copyright (C) 2000-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gdk/gdkkeysyms.h>
#include <gtk/gtkwindow.h>

#include <glibmm/i18n.h>
#include "document.h"
#include "svg-view-widget.h"
#include "svg-view.h"

static gint
sp_slideshow_event (SPViewWidget *vw, GdkEvent *event, GtkWidget *window)
{
	GSList *slides;
	const gchar *fname, *nname;
	int idx;

	slides = (GSList*)g_object_get_data (G_OBJECT (window), "slides");
	fname = (const gchar*)g_object_get_data (G_OBJECT (window), "current");
	idx = g_slist_index (slides, fname);

	switch (event->type) {
	case GDK_KEY_PRESS:
		switch (event->key.keyval) {
		case GDK_BackSpace:
		case GDK_Delete:
		case GDK_Left:
			idx -= 1;
			break;
		case GDK_Escape:
			gtk_widget_destroy (window);
			return TRUE;
			break;
		default:
			idx += 1;
			break;
		}
		break;
	case GDK_BUTTON_PRESS:
		idx += 1;
		break;
	default:
		break;
	}

	nname = (const gchar*)g_slist_nth_data (slides, idx);
//	g_print ("Old %s new %s\n", fname, nname);

	if (nname && (nname != fname)) {
		SPDocument *doc;
		g_print ("Trying to load %s\n", nname);
		doc = sp_document_new (nname, TRUE);
		if (doc) {
			reinterpret_cast<SPSVGView*>(SP_VIEW_WIDGET_VIEW (vw))->setDocument (doc);
			sp_document_unref (doc);
		}
		g_object_set_data (G_OBJECT (window), "current", (gpointer) nname);
	}

	return TRUE;
}

GtkWidget *
sp_slideshow_new (const GSList *files)
{
	SPDocument *doc;
	GtkWidget *w, *v;

	doc = sp_document_new ((const gchar*)files->data, TRUE);
	g_return_val_if_fail (doc != NULL, NULL);

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (w), _("Inkscape slideshow"));
	gtk_window_set_default_size (GTK_WINDOW (w), 480, 360);
	gtk_window_set_policy (GTK_WINDOW (w), TRUE, TRUE, FALSE);

	v = sp_svg_view_widget_new (doc);
	sp_svg_view_widget_set_resize (SP_SVG_VIEW_WIDGET (v), FALSE, sp_document_width (doc), sp_document_height (doc));
	sp_document_unref (doc);
	gtk_widget_show (v);
	gtk_container_add (GTK_CONTAINER (w), v);

	g_object_set_data (G_OBJECT (w), "slides", (gpointer) files);
	g_object_set_data (G_OBJECT (w), "current", files->data);

	g_signal_connect (G_OBJECT (v), "event", G_CALLBACK (sp_slideshow_event), w);

	return w;
}

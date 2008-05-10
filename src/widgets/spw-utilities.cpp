#define __SPW_UTILITIES_C__

/*
 * Inkscape Widget Utilities
 *
 * Authors:
 *   Bryce W. Harrington <brycehar@bryceharrington.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2003 Bryce W. Harrington
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>
#include <gtk/gtk.h>

#include "selection.h"

#include "helper/unit-menu.h"

/**
 * Creates a label widget with the given text, at the given col, row
 * position in the table.
 */
GtkWidget *
spw_label(GtkWidget * table, const gchar *label_text, int col, int row)
{
  GtkWidget *label_widget;

  label_widget = gtk_label_new (label_text);
  g_assert(label_widget != NULL);
  gtk_misc_set_alignment (GTK_MISC (label_widget), 1.0, 0.5);
  gtk_widget_show (label_widget);
  gtk_table_attach (GTK_TABLE (table), label_widget, col, col+1, row, row+1,
		    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 4, 0);
  return label_widget;
}

/**
 * Creates a horizontal layout manager with 4-pixel spacing between children
 * and space for 'width' columns.
 */
GtkWidget *
spw_hbox(GtkWidget * table, int width, int col, int row)
{
  GtkWidget *hb;
  /* Create a new hbox with a 4-pixel spacing between children */
  hb = gtk_hbox_new (FALSE, 4);
  g_assert(hb != NULL);
  gtk_widget_show (hb);
  gtk_table_attach (GTK_TABLE (table), hb, col, col+width, row, row+1,
		    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 0, 0);
  return hb;
}

/**
 * Creates a checkbutton widget and adds it to a vbox.
 * This is a compound widget that includes a label.
 */
GtkWidget *spw_vbox_checkbutton(GtkWidget *dialog, GtkWidget *vbox,
					const gchar *label, const gchar *tip, gchar *key, GCallback cb)
{
  g_assert (dialog != NULL);
  g_assert (vbox != NULL);

  GtkTooltips *tt = gtk_tooltips_new ();

  GtkWidget *b = gtk_check_button_new_with_label (label);
  gtk_tooltips_set_tip(tt, b, tip, NULL);
  g_assert (b != NULL);
  gtk_widget_show (b);
  gtk_box_pack_start (GTK_BOX (vbox), b, FALSE, FALSE, 0);
  gtk_object_set_data (GTK_OBJECT (b), "key", key);
  gtk_object_set_data (GTK_OBJECT (dialog), key, b);
  g_signal_connect (G_OBJECT (b), "toggled", cb, dialog);
  return b;
}


/**
 * Creates a checkbutton widget and adds it to a table.
 * This is a compound widget that includes a label.
 */
GtkWidget *
spw_checkbutton(GtkWidget * dialog, GtkWidget * table,
                const gchar * label, gchar * key, int /*col*/, int row,
                int insensitive, GCallback cb)
{
  GtkWidget *b;

  g_assert(dialog != NULL);
  g_assert(table  != NULL);

	GtkWidget *l = gtk_label_new (label);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_widget_show (l);
  gtk_table_attach (GTK_TABLE (table), l, 0, 1, row, row+1,
		    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 0, 0);

  b = gtk_check_button_new ();
  gtk_widget_show (b);
  gtk_table_attach (GTK_TABLE (table), b, 1, 2, row, row+1,
		    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 0, 0);
  gtk_object_set_data (GTK_OBJECT (b), "key", key);
  gtk_object_set_data (GTK_OBJECT (dialog), key, b);
  g_signal_connect (G_OBJECT (b), "toggled", cb, dialog);
  if (insensitive == 1) {
    gtk_widget_set_sensitive (b, FALSE);
  }
  return b;
}

/**
 * Creates a dropdown widget.  This is a compound widget that includes
 * a label as well as the dropdown.
 */
GtkWidget *
spw_dropdown(GtkWidget * dialog, GtkWidget * table,
	     const gchar * label_text, gchar * key, int row,
	     GtkWidget * selector
	     )
{
  g_assert(dialog   != NULL);
  g_assert(table    != NULL);
  g_assert(selector != NULL);

  spw_label(table, label_text, 0, row);

  gtk_widget_show (selector);
  gtk_table_attach (GTK_TABLE (table), selector, 1, 2, row, row+1,
		    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 0, 0);
  gtk_object_set_data (GTK_OBJECT (dialog), key, selector);
  return selector;
}

/**
 * Creates a unit selector widget, used for selecting whether one wishes
 * to measure screen elements in millimeters, points, etc.  This is a
 * compound unit that includes a label as well as the dropdown selector.
 */
GtkWidget *
spw_unit_selector(GtkWidget * dialog, GtkWidget * table,
		  const gchar * label_text, gchar * key, int row,
		GtkWidget * us, GCallback cb, bool can_be_negative)
{
  GtkWidget * sb;
  GtkObject * a;

  g_assert(dialog != NULL);
  g_assert(table  != NULL);
  g_assert(us     != NULL);

  spw_label(table, label_text, 0, row);

  a = gtk_adjustment_new (0.0, can_be_negative?-1e6:0, 1e6, 1.0, 10.0, 10.0);
  g_assert(a != NULL);
  gtk_object_set_data (GTK_OBJECT (a), "key", key);
  gtk_object_set_data (GTK_OBJECT (a), "unit_selector", us);
  gtk_object_set_data (GTK_OBJECT (dialog), key, a);
  sp_unit_selector_add_adjustment (SP_UNIT_SELECTOR (us), GTK_ADJUSTMENT (a));
  sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1.0, 4);
  g_assert(sb != NULL);
  gtk_widget_show (sb);
  gtk_table_attach (GTK_TABLE (table), sb, 1, 2, row, row+1,
		    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 0, 0);
  g_signal_connect (G_OBJECT (a), "value_changed", cb, dialog);
  return sb;
}

void
sp_set_font_size_recursive (GtkWidget *w, gpointer font)
{
	guint size = GPOINTER_TO_UINT (font);

	PangoFontDescription* pan = pango_font_description_new ();
	pango_font_description_set_size (pan, size);

	gtk_widget_modify_font (w, pan);

	if (GTK_IS_CONTAINER(w)) {
		gtk_container_foreach (GTK_CONTAINER(w), (GtkCallback) sp_set_font_size_recursive, font);
	}

	pango_font_description_free (pan);
}

void
sp_set_font_size (GtkWidget *w, guint font)
{
	sp_set_font_size_recursive (w, GUINT_TO_POINTER(font));
}

void
sp_set_font_size_smaller (GtkWidget *w)
{
	PangoContext *pc = gtk_widget_get_pango_context (w);
	PangoFontDescription* pfd = pango_context_get_font_description (pc);
	guint size = pango_font_description_get_size (pfd);
	sp_set_font_size_recursive (w, GUINT_TO_POINTER((int) (0.8*size)));
}

/**
\brief  Finds the descendant of w which has the data with the given key and returns the data, or NULL if there's none
*/
gpointer
sp_search_by_data_recursive (GtkWidget *w, gpointer key)
{
	gpointer r = NULL;

	if (w && GTK_IS_OBJECT(w)) {
		r = gtk_object_get_data (GTK_OBJECT(w), (gchar *) key);
	}
	if (r) return r;

	if (GTK_IS_CONTAINER(w)) {
		GList *ch = gtk_container_get_children (GTK_CONTAINER(w));
		for (GList *i = ch; i != NULL; i = i->next) {
			r = sp_search_by_data_recursive(GTK_WIDGET(i->data), key);
			if (r) return r;
		}
	}

	return NULL;
}

/**
\brief  Returns the descendant of w which has the given key and value pair, or NULL if there's none
*/
GtkWidget *
sp_search_by_value_recursive (GtkWidget *w, gchar *key, gchar *value)
{
	gchar *r = NULL;
	GtkWidget *child;

	if (w && GTK_IS_OBJECT(w)) {
		r = (gchar *) gtk_object_get_data (GTK_OBJECT(w), key);
	}
	if (r && !strcmp (r, value)) return w;

	if (GTK_IS_CONTAINER(w)) {
		GList *ch = gtk_container_get_children (GTK_CONTAINER(w));
		for (GList *i = ch; i != NULL; i = i->next) {
			child = sp_search_by_value_recursive(GTK_WIDGET(i->data), key, value);
			if (child) return child;
		}
	}

	return NULL;
}


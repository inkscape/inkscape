/*
 * Inkscape Widget Utilities
 *
 * Authors:
 *   Bryce W. Harrington <brycehar@bryceharrington.org>
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

#include <gtkmm/box.h>
#include <gtkmm/label.h>

#if GTK_CHECK_VERSION(3,0,0)
#include <gtkmm/grid.h>
#else
#include <gtkmm/table.h>
#endif

#include "selection.h"

#include "spw-utilities.h"

#include <gtk/gtk.h>

/**
 * Creates a label widget with the given text, at the given col, row
 * position in the table.
 */
#if GTK_CHECK_VERSION(3,0,0)
Gtk::Label * spw_label(Gtk::Grid *table, const gchar *label_text, int col, int row, Gtk::Widget* target)
#else
Gtk::Label * spw_label(Gtk::Table *table, const gchar *label_text, int col, int row, Gtk::Widget* target)
#endif
{
  Gtk::Label *label_widget = new Gtk::Label();
  g_assert(label_widget != NULL);
  if (target != NULL)
  {
    label_widget->set_text_with_mnemonic(label_text);
	label_widget->set_mnemonic_widget(*target);
  }
  else
  {
    label_widget->set_text(label_text);
  }
  label_widget->set_alignment(1.0, 0.5);
  label_widget->show();

#if GTK_CHECK_VERSION(3,0,0)
  label_widget->set_hexpand();
  label_widget->set_halign(Gtk::ALIGN_FILL);
  label_widget->set_valign(Gtk::ALIGN_CENTER);

  #if GTK_CHECK_VERSION(3,12,0)
  label_widget->set_margin_start(4);
  label_widget->set_margin_end(4);
  #else
  label_widget->set_margin_left(4);
  label_widget->set_margin_right(4);
  #endif

  table->attach(*label_widget, col, row, 1, 1);
#else
  table->attach(*label_widget, col, col+1, row, row+1, (Gtk::EXPAND | Gtk::FILL), static_cast<Gtk::AttachOptions>(0), 4, 0);
#endif

  return label_widget;
}

GtkWidget *
spw_label_old(GtkWidget *table, const gchar *label_text, int col, int row)
{
  GtkWidget *label_widget;

  label_widget = gtk_label_new (label_text);
  g_assert(label_widget != NULL);

#if GTK_CHECK_VERSION(3,0,0)
  gtk_widget_set_halign(label_widget, GTK_ALIGN_END);
#else
  gtk_misc_set_alignment (GTK_MISC (label_widget), 1.0, 0.5);
#endif

  gtk_widget_show (label_widget);

#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION(3,12,0)
  gtk_widget_set_margin_start(label_widget, 4);
  gtk_widget_set_margin_end(label_widget, 4);
#else
  gtk_widget_set_margin_left(label_widget, 4);
  gtk_widget_set_margin_right(label_widget, 4);
#endif
  gtk_widget_set_hexpand(label_widget, TRUE);
  gtk_widget_set_halign(label_widget, GTK_ALIGN_FILL);
  gtk_widget_set_valign(label_widget, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label_widget, col, row, 1, 1);
#else
  gtk_table_attach(GTK_TABLE (table), label_widget, col, col+1, row, row+1,
		    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 4, 0);
#endif

  return label_widget;
}

/**
 * Creates a horizontal layout manager with 4-pixel spacing between children
 * and space for 'width' columns.
 */
#if GTK_CHECK_VERSION(3,0,0)
Gtk::HBox * spw_hbox(Gtk::Grid * table, int width, int col, int row)
#else
Gtk::HBox * spw_hbox(Gtk::Table * table, int width, int col, int row)
#endif
{
  /* Create a new hbox with a 4-pixel spacing between children */
  Gtk::HBox *hb = new Gtk::HBox(false, 4);
  g_assert(hb != NULL);
  hb->show();

#if GTK_CHECK_VERSION(3,0,0)
  hb->set_hexpand();
  hb->set_halign(Gtk::ALIGN_FILL);
  hb->set_valign(Gtk::ALIGN_CENTER);
  table->attach(*hb, col, row, width, 1);
#else
  table->attach(*hb, col, col+width, row, row+1, (Gtk::EXPAND | Gtk::FILL), static_cast<Gtk::AttachOptions>(0), 0, 0);
#endif

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

  GtkWidget *b = gtk_check_button_new_with_label (label);
  gtk_widget_set_tooltip_text(b, tip);
  g_assert (b != NULL);
  gtk_widget_show (b);
  gtk_box_pack_start (GTK_BOX (vbox), b, FALSE, FALSE, 0);
  g_object_set_data (G_OBJECT (b), "key", key);
  g_object_set_data (G_OBJECT (dialog), key, b);
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

#if GTK_CHECK_VERSION(3,0,0)
  gtk_widget_set_halign(l, GTK_ALIGN_END);
#else
  gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
#endif

  gtk_widget_show (l);

#if GTK_CHECK_VERSION(3,0,0)
  gtk_widget_set_halign(l, GTK_ALIGN_FILL);
  gtk_widget_set_hexpand(l, TRUE);
  gtk_widget_set_valign(l, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), l, 0, row, 1, 1);
#else
  gtk_table_attach (GTK_TABLE (table), l, 0, 1, row, row+1,
		    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 0, 0);
#endif

  b = gtk_check_button_new ();
  gtk_widget_show (b);

#if GTK_CHECK_VERSION(3,0,0)
  gtk_widget_set_halign(b, GTK_ALIGN_FILL);
  gtk_widget_set_hexpand(b, TRUE);
  gtk_widget_set_valign(b, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), b, 1, row, 1, 1);
#else
  gtk_table_attach (GTK_TABLE (table), b, 1, 2, row, row+1,
		    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 0, 0);
#endif

  g_object_set_data (G_OBJECT (b), "key", key);
  g_object_set_data (G_OBJECT (dialog), key, b);
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

  spw_label_old(table, label_text, 0, row);

  gtk_widget_show (selector);

#if GTK_CHECK_VERSION(3,0,0)
  gtk_widget_set_halign(selector, GTK_ALIGN_FILL);
  gtk_widget_set_hexpand(selector, TRUE);
  gtk_widget_set_valign(selector, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), selector, 1, row, 1, 1);
#else
  gtk_table_attach (GTK_TABLE (table), selector, 1, 2, row, row+1,
		    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 0, 0);
#endif

  g_object_set_data (G_OBJECT (dialog), key, selector);
  return selector;
}

static void
sp_set_font_size_recursive (GtkWidget *w, gpointer font)
{
	guint size = GPOINTER_TO_UINT (font);

#if GTK_CHECK_VERSION(3,0,0)
        GtkCssProvider *css_provider = gtk_css_provider_new();

        const double pt_size = size / static_cast<double>(PANGO_SCALE);
        std::ostringstream css_data;
        css_data << "GtkWidget {\n"
                 << "  font-size: " << pt_size << "pt;\n"
                 << "}\n";

        gtk_css_provider_load_from_data(css_provider,
                                        css_data.str().c_str(),
                                        -1, NULL);

        GtkStyleContext *style_context = gtk_widget_get_style_context(w);
        gtk_style_context_add_provider(style_context,
                                       GTK_STYLE_PROVIDER(css_provider),
                                       GTK_STYLE_PROVIDER_PRIORITY_USER);
#else
	PangoFontDescription* pan = pango_font_description_new ();
	pango_font_description_set_size (pan, size);
	gtk_widget_modify_font (w, pan);
#endif

	if (GTK_IS_CONTAINER(w)) {
		gtk_container_foreach (GTK_CONTAINER(w), (GtkCallback) sp_set_font_size_recursive, font);
	}

#if GTK_CHECK_VERSION(3,0,0)
        g_object_unref(css_provider);
#else
	pango_font_description_free (pan);
#endif
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
 * Finds the descendant of w which has the data with the given key and returns the data, or NULL if there's none.
 */
gpointer sp_search_by_data_recursive(GtkWidget *w, gpointer key)
{
	gpointer r = NULL;

	if (w && G_IS_OBJECT(w)) {
		r = g_object_get_data(G_OBJECT(w), (gchar *) key);
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
 * Returns the descendant of w which has the given key and value pair, or NULL if there's none.
 */
GtkWidget *sp_search_by_value_recursive(GtkWidget *w, gchar *key, gchar *value)
{
	gchar *r = NULL;

	if (w && G_IS_OBJECT(w)) {
		r = (gchar *) g_object_get_data(G_OBJECT(w), key);
	}
	if (r && !strcmp (r, value)) return w;

	if (GTK_IS_CONTAINER(w)) {
		GList *ch = gtk_container_get_children (GTK_CONTAINER(w));
		for (GList *i = ch; i != NULL; i = i->next) {
			GtkWidget *child = sp_search_by_value_recursive(GTK_WIDGET(i->data), key, value);
			if (child) return child;
		}
	}

	return NULL;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

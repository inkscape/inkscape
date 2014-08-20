#ifndef __SPW_UTILITIES_H__
#define __SPW_UTILITIES_H__

/*
 * Inkscape Widget Utilities
 *
 * Author:
 *   Bryce W. Harrington <brycehar@bryceharrington.org>
 * 
 * Copyright (C) 2003 Bryce Harrington
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/* The following are helper routines for making Inkscape dialog widgets.
   All are prefixed with spw_, short for inkscape_widget.  This is not to
   be confused with SPWidget, an existing datatype associated with Inkscape::XML::Node/
   SPObject, that reacts to modification.
*/

namespace Gtk {
  class Label;

#if GTK_CHECK_VERSION(3,0,0)
  class Grid;
#else
  class Table;
#endif

  class HBox;
  class Widget;
}

#if GTK_CHECK_VERSION(3,0,0)
Gtk::Label * spw_label(Gtk::Grid *table, gchar const *label_text, int col, int row, Gtk::Widget *target);
Gtk::HBox * spw_hbox(Gtk::Grid *table, int width, int col, int row);
#else
Gtk::Label * spw_label(Gtk::Table *table, gchar const *label_text, int col, int row, Gtk::Widget *target);
Gtk::HBox * spw_hbox(Gtk::Table *table, int width, int col, int row);
#endif

GtkWidget * spw_label_old(GtkWidget *table, gchar const *label_text, int col, int row);

GtkWidget *
spw_vbox_checkbutton(GtkWidget *dialog, GtkWidget *table,
		     const gchar *label, const gchar *tip, gchar *key, GCallback cb);

GtkWidget *
spw_checkbutton(GtkWidget *dialog, GtkWidget *table,
		gchar const *label, gchar *key, int col, int row,
		int sensitive, GCallback cb);

GtkWidget *
spw_dropdown(GtkWidget *dialog, GtkWidget *table,
	     gchar const *label, gchar *key, int row,
	     GtkWidget *selector
	     );

void sp_set_font_size (GtkWidget *w, guint font);
void sp_set_font_size_smaller (GtkWidget *w);

gpointer sp_search_by_data_recursive(GtkWidget *w, gpointer data);
GtkWidget *sp_search_by_value_recursive(GtkWidget *w, gchar *key, gchar *value);

#endif

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

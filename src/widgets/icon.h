#ifndef SEEN_SP_ICON_H
#define SEEN_SP_ICON_H

/*
 * Generic icon widget
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

#include "icon-size.h"

#define SP_TYPE_ICON (sp_icon_get_type ())
#define SP_ICON(o) (GTK_CHECK_CAST ((o), SP_TYPE_ICON, SPIcon))
#define SP_IS_ICON(o) (GTK_CHECK_TYPE ((o), SP_TYPE_ICON))

#include <gtk/gtkwidget.h>

struct SPIcon {
    GtkWidget widget;

    Inkscape::IconSize lsize;
    int psize;
    gchar *name;

    GdkPixbuf *pb;
};

struct SPIconClass {
    GtkWidgetClass parent_class;
};

GType sp_icon_get_type (void);

GtkWidget *sp_icon_new( Inkscape::IconSize size, const gchar *name );

#include <glibmm/ustring.h>
#include <gtkmm/widget.h>

// Might return a wrapped SPIcon, or Gtk::Image
Gtk::Widget *sp_icon_get_icon( const Glib::ustring &oid, Inkscape::IconSize size = Inkscape::ICON_SIZE_BUTTON );

void sp_icon_fetch_pixbuf( SPIcon *icon );
int sp_icon_get_phys_size(int size);


#endif // SEEN_SP_ICON_H

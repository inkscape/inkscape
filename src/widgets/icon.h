#ifndef SEEN_SP_ICON_H
#define SEEN_SP_ICON_H

/*
 * Generic icon widget
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 * Copyright (C) 2010 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

#include "icon-size.h"

#define SP_TYPE_ICON SPIcon::getType()
#define SP_ICON(o) (GTK_CHECK_CAST ((o), SP_TYPE_ICON, SPIcon))
#define SP_IS_ICON(o) (GTK_CHECK_TYPE ((o), SP_TYPE_ICON))

#include <gtk/gtkwidget.h>

struct SPIconClass {
    GtkWidgetClass parent_class;
};

struct SPIcon {
    GtkWidget widget;

    Inkscape::IconSize lsize;
    int psize;
    gchar *name;

    GdkPixbuf *pb;

    static GType getType(void);

    friend class SPIconImpl;
};


GtkWidget *sp_icon_new( Inkscape::IconSize size, const gchar *name );

#include <glibmm/ustring.h>
#include <gtkmm/widget.h>

// Might return a wrapped SPIcon, or Gtk::Image
Gtk::Widget *sp_icon_get_icon( const Glib::ustring &oid, Inkscape::IconSize size = Inkscape::ICON_SIZE_BUTTON );

void sp_icon_fetch_pixbuf( SPIcon *icon );
int sp_icon_get_phys_size(int size);

namespace Inkscape {
    void queueIconPrerender( Glib::ustring const &oid, Inkscape::IconSize size = Inkscape::ICON_SIZE_BUTTON );
}

#endif // SEEN_SP_ICON_H

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

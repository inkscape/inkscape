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

#include <gtkmm/widget.h>
#include "icon-size.h"

#define SP_TYPE_ICON sp_icon_get_type()
#define SP_ICON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_ICON, SPIcon))
#define SP_IS_ICON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_ICON))

namespace Glib {
class ustring;
}

struct SPIconClass {
    GtkWidgetClass parent_class;
};

GType sp_icon_get_type() G_GNUC_CONST;

struct SPIcon {
    GtkWidget widget;

    Inkscape::IconSize lsize;
    int psize;
    gchar *name;

    GdkPixbuf *pb;

    friend class SPIconImpl;
};


GtkWidget *sp_icon_new( Inkscape::IconSize size, const gchar *name );
GdkPixbuf *sp_pixbuf_new( Inkscape::IconSize size, const gchar *name );

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

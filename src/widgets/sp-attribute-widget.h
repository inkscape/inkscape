/**
 * @file
 * Widget that listens and modifies repr attributes.
 */
/* Authors:
 *  Lauris Kaplinski <lauris@kaplinski.com>
 *  Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2002,2011 authors
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Licensed under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_DIALOGS_SP_ATTRIBUTE_WIDGET_H
#define SEEN_DIALOGS_SP_ATTRIBUTE_WIDGET_H

#include <gtk/gtk.h>
#include <gtkmm.h>
#include <glib.h>
#include <stddef.h>
#include <sigc++/connection.h>

namespace Inkscape {
namespace XML {
class Node;
}
}

struct SPAttributeTable;
struct SPAttributeTableClass;
class  SPObject;

class SPAttributeTable : public Gtk::Widget {
public:
    SPAttributeTable ();
    SPAttributeTable (SPObject *object, std::vector<Glib::ustring> &labels, std::vector<Glib::ustring> &attributes, GtkWidget* parent);
    ~SPAttributeTable ();
    
    void set_object(SPObject *object, std::vector<Glib::ustring> &labels, std::vector<Glib::ustring> &attributes, GtkWidget* parent);
    void change_object(SPObject *object);
    // void set_repr(Inkscape::XML::Node *repr, std::vector<Glib::ustring> &labels, std::vector<Glib::ustring> &attributes, GtkWidget* parent);
    void clear(void);
    
    std::vector<Glib::ustring> get_attributes(void) {return _attributes;};
    std::vector<Gtk::Entry *> get_entries(void) {return _entries;};
    union {
        SPObject *object;
        Inkscape::XML::Node *repr;
    } src;
    guint blocked;
    guint hasobj;

private:
    Gtk::Table *table;
    std::vector<Glib::ustring> _attributes;
    std::vector<Gtk::Entry *> _entries;
    sigc::connection modified_connection;
    sigc::connection release_connection;
};

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

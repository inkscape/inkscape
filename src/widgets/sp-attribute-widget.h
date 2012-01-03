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

class  SPObject;

class SPAttributeTable : public Gtk::Widget {
/**
 * \class SPAttributeTable
 * \brief A base class for dialogs to enter the value of several properties.
 *
 * SPAttributeTable is used if you want to alter several properties of
 * an object. For each property, it creates an entry next to a label and
 * positiones these labels and entries one by one below each other.
 */
public:
    SPAttributeTable ();
    SPAttributeTable (SPObject *object, std::vector<Glib::ustring> &labels, std::vector<Glib::ustring> &attributes, GtkWidget* parent);
    ~SPAttributeTable ();
    
    void set_object(SPObject *object, std::vector<Glib::ustring> &labels, std::vector<Glib::ustring> &attributes, GtkWidget* parent);
    void change_object(SPObject *object);
    // void set_repr(Inkscape::XML::Node *repr, std::vector<Glib::ustring> &labels, std::vector<Glib::ustring> &attributes, GtkWidget* parent);
    void clear(void);
    
	/**
     * \brief Gives access to the attributes list.
     */
    std::vector<Glib::ustring> get_attributes(void) {return _attributes;};
    
	/**
     * \brief Gives access to the Gtk::Entry list.
     */
    std::vector<Gtk::Entry *> get_entries(void) {return _entries;};
    
	/**
     * \brief Stores pointer to the selected object.
     */
    union {
        SPObject *object;
        Inkscape::XML::Node *repr;
    } src;
    
	/**
     * \brief Indicates whether SPAttributeTable is processing callbacks and whether it should accept any updating.
     */
    guint blocked;
    guint hasobj; //currently unused: set_repr is not used to data: decide later on whether to keep it

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

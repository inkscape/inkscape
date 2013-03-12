/**
 * @file
 * Widget that listens and modifies repr attributes.
 */
/* Authors:
 *  Lauris Kaplinski <lauris@kaplinski.com>
 *  Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2002,2011-2012 authors
 *
 * Licensed under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_DIALOGS_SP_ATTRIBUTE_WIDGET_H
#define SEEN_DIALOGS_SP_ATTRIBUTE_WIDGET_H

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtkmm/widget.h>
#include <stddef.h>
#include <sigc++/connection.h>

namespace Gtk {
class Entry;

#if WITH_GTKMM_3_0
class Grid;
#else
class Table;
#endif
}

namespace Inkscape {
namespace XML {
class Node;
}
}

class  SPObject;

/**
 * A base class for dialogs to enter the value of several properties.
 *
 * SPAttributeTable is used if you want to alter several properties of
 * an object. For each property, it creates an entry next to a label and
 * positiones these labels and entries one by one below each other.
 */
class SPAttributeTable : public Gtk::Widget {
public:
    /**
     * Constructor defaulting to no content.
     */
    SPAttributeTable ();
    
    /**
     * Constructor referring to a specific object.
     *
     * This constructor initializes all data fields and creates the necessary widgets.
     * set_object is called for this purpose.
     * 
     * @param object the SPObject to which this instance is referring to. It should be the object that is currently selected and whose properties are being shown by this SPAttributeTable instance.
     * @param labels list of labels to be shown for the different attributes.
     * @param attributes list of attributes whose value can be edited.
     * @param parent the parent object owning the SPAttributeTable instance.
     * 
     * @see set_object
     */
    SPAttributeTable (SPObject *object, std::vector<Glib::ustring> &labels, std::vector<Glib::ustring> &attributes, GtkWidget* parent);
    
    ~SPAttributeTable ();
    
    /**
     * Sets class properties and creates child widgets
     *
     * set_object initializes all data fields, creates links to the
     * SPOject item and creates the necessary widgets. For n properties
     * n labels and n entries are created and shown in tabular format.
     * 
     * @param object the SPObject to which this instance is referring to. It should be the object that is currently selected and whose properties are being shown by this SPAttribuTable instance.
     * @param labels list of labels to be shown for the different attributes.
     * @param attributes list of attributes whose value can be edited.
     * @param parent the parent object owning the SPAttributeTable instance.
     */
    void set_object(SPObject *object, std::vector<Glib::ustring> &labels, std::vector<Glib::ustring> &attributes, GtkWidget* parent);
    
    /**
     * Update values in entry boxes on change of object.
     *
     * change_object updates the values of the entry boxes in case the user
     * of Inkscape selects an other object.
     * change_object is a subset of set_object and should only be called by
     * the parent class (holding the SPAttributeTable instance). This function
     * should only be called when the number of properties/entries nor
     * the labels do not change.
     * 
     * @param object the SPObject to which this instance is referring to. It should be the object that is currently selected and whose properties are being shown by this SPAttribuTable instance.
     */
    void change_object(SPObject *object);
    
    /**
     * Clears data of SPAttributeTable instance, destroys all child widgets and closes connections.
     */
    void clear(void);
    
    /**
     * Reads the object attributes.
     * 
     * Reads the object attributes and shows the new object attributes in the
     * entry boxes. Caution: function should only be used when which there is
     * no change in which objects are selected.
     */
    void reread_properties(void);
    
	/**
     * Gives access to the attributes list.
     */
    std::vector<Glib::ustring> get_attributes(void) {return _attributes;};
    
	/**
     * Gives access to the Gtk::Entry list.
     */
    std::vector<Gtk::Entry *> get_entries(void) {return _entries;};
    
	/**
     * Stores pointer to the selected object.
     */
    SPObject *_object;
    
	/**
     * Indicates whether SPAttributeTable is processing callbacks and whether it should accept any updating.
     */
    bool blocked;

private:
    /**
     * Container widget for the dynamically created child widgets (labels and entry boxes).
     */
#if WITH_GTKMM_3_0
    Gtk::Grid  *table;
#else
    Gtk::Table *table;
#endif
    
    /**
     * List of attributes.
     * 
     * _attributes stores the attribute names of the selected object that
	 * are valid and can be modified through this widget.
     */
    std::vector<Glib::ustring> _attributes;
	/**
     * List of pointers to the respective entry boxes.
     * 
     * _entries stores pointers to the dynamically created entry boxes in which
	 * the user can midify the attributes of the selected object.
     */
    std::vector<Gtk::Entry *> _entries;
    
	/**
     * Sets the callback for a modification of the selection.
     */
    sigc::connection modified_connection;
    
	/**
     * Sets the callback for the deletion of the selected object.
     */
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

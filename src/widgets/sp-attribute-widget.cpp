/**
 * @file
 * Base widget for user input of object properties.
 */
/* Authors:
 *  Lauris Kaplinski <lauris@ximian.com>
 *  Abhishek Sharma
 *  Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2011, authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>
#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

#include "sp-object.h"
#include "xml/repr.h"
#include "macros.h"
#include "document.h"
#include "sp-attribute-widget.h"

using Inkscape::DocumentUndo;

static void sp_attribute_table_entry_changed (Gtk::Editable *editable, SPAttributeTable *spat);
static void sp_attribute_table_object_modified (SPObject *object, guint flags, SPAttributeTable *spaw);
static void sp_attribute_table_object_release (SPObject */*object*/, SPAttributeTable *spat);

#define XPAD 4
#define YPAD 0


/**
 * \brief Constructor defaulting to no content.
 */
SPAttributeTable::SPAttributeTable () : 
    blocked(0),
    hasobj(0),
    table(0),
    _attributes(),
    _entries(),
    modified_connection(),
    release_connection()
{
    src.object = NULL;
}

 /**
 * \brief Constructor referring to a specific object.
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
SPAttributeTable::SPAttributeTable (SPObject *object, std::vector<Glib::ustring> &labels, std::vector<Glib::ustring> &attributes, GtkWidget* parent) : 
   blocked(0),
    hasobj(0),
    table(0),
    _attributes(),
    _entries(),
    modified_connection(),
    release_connection()
{
    src.object = NULL;
    src.repr   = NULL;
    set_object(object, labels, attributes, parent);
}

/**
 * \brief Destructor.
 */
SPAttributeTable::~SPAttributeTable ()
{
    clear();
}

/**
 * \brief Clears data of SPAttributeTable instance, destroys all child widgets and closes connections.
 */
void SPAttributeTable::clear(void)
{
    Gtk::Widget *w;
    
    if (table)
    {
        std::vector<Gtk::Widget*> ch = table->get_children();
        for (int i = (ch.size())-1; i >=0 ; i--)
        {
            w = ch[i];
            sp_signal_disconnect_by_data (w->gobj(), this);
            ch.pop_back();
            if (w != NULL)
            {
                try
                {
                    delete w;
                }
                catch(...)
                {
                }
            }
        }
        ch.clear();
        _attributes.clear();
        _entries.clear();
        
        delete table;
        table = NULL;
    }

    if (hasobj) {
        if (src.object) {
            modified_connection.disconnect();
            release_connection.disconnect();
            src.object = NULL;
        }
    } else {
        if (src.repr) {
            src.repr = Inkscape::GC::release(src.repr);
        }
    }
}

/**
 * \brief Sets class properties and creates child widgets
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
void SPAttributeTable::set_object(SPObject *object,
                            std::vector<Glib::ustring> &labels,
                            std::vector<Glib::ustring> &attributes,
                            GtkWidget* parent)
{
    g_return_if_fail (!object || SP_IS_OBJECT (object));
    g_return_if_fail (!object || !labels.empty() || !attributes.empty());
    g_return_if_fail (labels.size() == attributes.size());

    clear();
    hasobj = true;
    src.object = object;

    if (object) {
        blocked = true;

        // Set up object
        modified_connection = object->connectModified(sigc::bind<2>(sigc::ptr_fun(&sp_attribute_table_object_modified), this));
        release_connection  = object->connectRelease (sigc::bind<1>(sigc::ptr_fun(&sp_attribute_table_object_release), this));

        // Create table
        table = new Gtk::Table (attributes.size(), 2, false);
        if (!(parent == NULL))
        {
            gtk_container_add (GTK_CONTAINER (parent),(GtkWidget*)table->gobj());
        }
        
        // Fill rows
        _attributes = attributes;
        Gtk::Label *ll;
        Gtk::Entry *ee;
        Gtk::Widget *w;
        const gchar *val;
        for (guint i = 0; i < (attributes.size()); i++) {
            ll = new Gtk::Label (_(labels[i].c_str()));
            w = (Gtk::Widget *) ll;
            ll->show();
            ll->set_alignment (1.0, 0.5);
            table->attach (*w, 0, 1, i, i + 1,
                               Gtk::FILL,
                               (Gtk::EXPAND | Gtk::FILL),
                               XPAD, YPAD );
            ee = new Gtk::Entry();
            w = (Gtk::Widget *) ee;
            ee->show();
            val = object->getRepr()->attribute(attributes[i].c_str());
            ee->set_text (val ? val : (const gchar *) "");
            table->attach (*w, 1, 2, i, i + 1,
                               (Gtk::EXPAND | Gtk::FILL),
                               (Gtk::EXPAND | Gtk::FILL),
                               XPAD, YPAD );
            _entries.push_back(ee);
            g_signal_connect ( w->gobj(), "changed",
                               G_CALLBACK (sp_attribute_table_entry_changed),
                               this );
        }
        /* Show table */
        table->show ();
        blocked = false;
    }
}

/**
 * \brief Update values in entry boxes on change of object.
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
void SPAttributeTable::change_object(SPObject *object)
{
    g_return_if_fail (!object || SP_IS_OBJECT (object));
    if (hasobj) {
        if (src.object) {
            modified_connection.disconnect();
            release_connection.disconnect();
            src.object = NULL;
        }
    } else {
        if (src.repr) {
            src.repr = Inkscape::GC::release(src.repr);
        }
    }

    hasobj = true;
    src.object = object;

    if (object) {
        blocked = true;

        // Set up object
        modified_connection = object->connectModified(sigc::bind<2>(sigc::ptr_fun(&sp_attribute_table_object_modified), this));
        release_connection  = object->connectRelease (sigc::bind<1>(sigc::ptr_fun(&sp_attribute_table_object_release), this));
        Gtk::Entry *ee;
        Gtk::Widget *w;
        const gchar *val;
        for (guint i = 0; i < (_attributes.size()); i++) {
            ee = (Gtk::Entry *)_entries[i];
            w = (Gtk::Widget *) ee;
            val = object->getRepr()->attribute(_attributes[i].c_str());
            ee->set_text (val ? val : (const gchar *) "");
        }
    }

}

/*void SPAttributeTable::set_repr (Inkscape::XML::Node *repr,
                            std::vector<Glib::ustring> &labels,
                            std::vector<Glib::ustring> &attributes,
                            GtkWidget* parent)
{
    g_return_if_fail (!labels.empty() || !attributes.empty());
    g_return_if_fail (labels.size() == attributes.size());

    clear();

    hasobj = false;

    if (repr) {
        blocked = true;

        // Set up repr
        src.repr = Inkscape::GC::anchor(repr);
        
        // Create table
        table = new Gtk::Table (attributes.size(), 2, false);
        if (!(parent == NULL))
        {
            gtk_container_add (GTK_CONTAINER (parent),(GtkWidget*)table->gobj());
        }
        
        // Fill rows
        _attributes = attributes;
        Gtk::Label *ll;
        Gtk::Entry *ee;
        Gtk::Widget *w;
        const gchar *val;
        for (guint i = 0; i < (attributes.size()); i++) {
            ll = new Gtk::Label (_(labels[i].c_str()));
            w = (Gtk::Widget *) ll;
            ll->show ();
            ll->set_alignment (1.0, 0.5);
            table->attach (*w, 0, 1, i, i + 1,
                               Gtk::FILL,
                               (Gtk::EXPAND | Gtk::FILL),
                               XPAD, YPAD );
            ee = new Gtk::Entry();
            w = (Gtk::Widget *) ee;
            ee->show();
            val = repr->attribute(attributes[i].c_str());
            ee->set_text (val ? val : (const gchar *) "");
            table->attach (*w, 1, 2, i, i + 1,
                               (Gtk::EXPAND | Gtk::FILL),
                               (Gtk::EXPAND | Gtk::FILL),
                               XPAD, YPAD );
            _entries.push_back(w);
            g_signal_connect ( w->gobj(), "changed",
                               G_CALLBACK (sp_attribute_table_entry_changed),
                               this );
        }
        // Show table
        table->show ();
        blocked = false;
    }
}
*/

/**
 * \brief Callback for a modification of the selected object (size, color, properties, etc.).
 *
 * sp_attribute_table_object_modified rereads the object properties
 * and shows the values in the entry boxes. It is a callback from a
 * connection of the SPObject.
 * 
 * @param object the SPObject to which this instance is referring to.
 * @param flags gives the applied modifications
 * @param spat pointer to the SPAttributeTable instance.
 */
static void sp_attribute_table_object_modified ( SPObject */*object*/,
                                     guint flags,
                                     SPAttributeTable *spat )
{
    if (flags && SP_OBJECT_MODIFIED_FLAG)
    {
        guint i;
        std::vector<Glib::ustring> attributes = spat->get_attributes();
        std::vector<Gtk::Entry *> entries = spat->get_entries();
        Gtk::Entry* e;
        Glib::ustring text;
        for (i = 0; i < (attributes.size()); i++) {
            const gchar *val;
            e = entries[i];
            val = spat->src.object->getRepr()->attribute(attributes[i].c_str());
            text = e->get_text ();
            if (val || !text.empty()) {
                if (text != val) {
                    /* We are different */
                    spat->blocked = true;
                    e->set_text (val ? val : (const gchar *) "");
                    spat->blocked = false;
                }
            }
        }
    }

} // end of sp_attribute_table_object_modified()

/**
 * \brief Callback for user input in one of the entries.
 *
 * sp_attribute_table_entry_changed set the object property
 * to the new value and updates history. It is a callback from
 * the entries created by SPAttributeTable.
 * 
 * @param editable pointer to the entry box.
 * @param spat pointer to the SPAttributeTable instance.
 */
static void sp_attribute_table_entry_changed ( Gtk::Editable *editable,
                                   SPAttributeTable *spat )
{
    if (!spat->blocked)
    {
        guint i;
        std::vector<Glib::ustring> attributes = spat->get_attributes();
        std::vector<Gtk::Entry *> entries = spat->get_entries();
        Gtk::Entry *e;
        for (i = 0; i < (attributes.size()); i++) {
            e = entries[i];
            if ((GtkWidget*) (editable) == (GtkWidget*) e->gobj()) {
                spat->blocked = true;
                Glib::ustring text = e->get_text ();

                if (spat->hasobj && spat->src.object) {
                    spat->src.object->getRepr()->setAttribute(attributes[i].c_str(), text.c_str(), false);
                    DocumentUndo::done(spat->src.object->document, SP_VERB_NONE,
                                       _("Set attribute"));

                } else if (spat->src.repr) {

                    spat->src.repr->setAttribute(attributes[i].c_str(), text.c_str(), false);
                    /* TODO: Warning! Undo will not be flushed in given case */
                }
                spat->blocked = false;
                return;
            }
        }
        g_warning ("file %s: line %d: Entry signalled change, but there is no such entry", __FILE__, __LINE__);
    }

} // end of sp_attribute_table_entry_changed()

/**
 * \brief Callback for the delection of the selected object.
 *
 * sp_attribute_table_object_release invalidates all data of 
 * SPAttributeTable and disables the widget.
 */
static void sp_attribute_table_object_release (SPObject */*object*/, SPAttributeTable *spat)
{
    std::vector<Glib::ustring> labels;
    std::vector<Glib::ustring> attributes;
    spat->set_object (NULL, labels, attributes, NULL);
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

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
 * Copyright (C) 2012, authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-attribute-widget.h"

#include <glibmm/i18n.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>

#if WITH_GTKMM_3_0
# include <gtkmm/grid.h>
#else
# include <gtkmm/table.h>
#endif

#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

#include "sp-object.h"
#include "xml/repr.h"
#include "macros.h"
#include "document.h"
#include "document-undo.h"
#include "verbs.h"

using Inkscape::DocumentUndo;

/**
 * Callback for user input in one of the entries.
 *
 * sp_attribute_table_entry_changed set the object property
 * to the new value and updates history. It is a callback from
 * the entries created by SPAttributeTable.
 * 
 * @param editable pointer to the entry box.
 * @param spat pointer to the SPAttributeTable instance.
 */
static void sp_attribute_table_entry_changed (Gtk::Entry *editable, SPAttributeTable *spat);
/**
 * Callback for a modification of the selected object (size, color, properties, etc.).
 *
 * sp_attribute_table_object_modified rereads the object properties
 * and shows the values in the entry boxes. It is a callback from a
 * connection of the SPObject.
 * 
 * @param object the SPObject to which this instance is referring to.
 * @param flags gives the applied modifications
 * @param spat pointer to the SPAttributeTable instance.
 */
static void sp_attribute_table_object_modified (SPObject *object, guint flags, SPAttributeTable *spaw);
/**
 * Callback for the delection of the selected object.
 *
 * sp_attribute_table_object_release invalidates all data of 
 * SPAttributeTable and disables the widget.
 */
static void sp_attribute_table_object_release (SPObject */*object*/, SPAttributeTable *spat);

#define XPAD 4
#define YPAD 0


SPAttributeTable::SPAttributeTable () : 
    _object(NULL),
    blocked(false),
    table(NULL),
    _attributes(),
    _entries(),
    modified_connection(),
    release_connection()
{
}

SPAttributeTable::SPAttributeTable (SPObject *object, std::vector<Glib::ustring> &labels, std::vector<Glib::ustring> &attributes, GtkWidget* parent) : 
    _object(NULL),
    blocked(false),
    table(NULL),
    _attributes(),
    _entries(),
    modified_connection(),
    release_connection()
{
    set_object(object, labels, attributes, parent);
}

SPAttributeTable::~SPAttributeTable ()
{
    clear();
}

void SPAttributeTable::clear(void)
{
    if (table)
    {
        std::vector<Gtk::Widget*> ch = table->get_children();
        for (int i = (ch.size())-1; i >=0 ; i--)
        {
            Gtk::Widget *w = ch[i];
            ch.pop_back();
            if (w != NULL)
            {
                try
                {
                    sp_signal_disconnect_by_data (w->gobj(), this);
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

    if (_object)
    {
        modified_connection.disconnect();
        release_connection.disconnect();
        _object = NULL;
    }
}

void SPAttributeTable::set_object(SPObject *object,
                            std::vector<Glib::ustring> &labels,
                            std::vector<Glib::ustring> &attributes,
                            GtkWidget* parent)
{
    g_return_if_fail (!object || SP_IS_OBJECT (object));
    g_return_if_fail (!object || !labels.empty() || !attributes.empty());
    g_return_if_fail (labels.size() == attributes.size());

    clear();
    _object = object;

    if (object) {
        blocked = true;

        // Set up object
        modified_connection = object->connectModified(sigc::bind<2>(sigc::ptr_fun(&sp_attribute_table_object_modified), this));
        release_connection  = object->connectRelease (sigc::bind<1>(sigc::ptr_fun(&sp_attribute_table_object_release), this));

        // Create table
#if WITH_GTKMM_3_0
        table = new Gtk::Grid();
#else
        table = new Gtk::Table(attributes.size(), 2, false);
#endif

        if (!(parent == NULL))
            gtk_container_add(GTK_CONTAINER(parent), (GtkWidget*)table->gobj());
        
        // Fill rows
        _attributes = attributes;
        for (guint i = 0; i < (attributes.size()); i++) {
            Gtk::Label *ll = new Gtk::Label (_(labels[i].c_str()));
            ll->show();
            ll->set_alignment (1.0, 0.5);

#if WITH_GTKMM_3_0
            ll->set_vexpand();
            ll->set_margin_left(XPAD);
            ll->set_margin_right(XPAD);
            ll->set_margin_top(XPAD);
            ll->set_margin_bottom(XPAD);
            table->attach(*ll, 0, i, 1, 1);
#else
            table->attach (*ll, 0, 1, i, i + 1,
                               Gtk::FILL,
                               (Gtk::EXPAND | Gtk::FILL),
                               XPAD, YPAD );
#endif

            Gtk::Entry *ee = new Gtk::Entry();
            ee->show();
            const gchar *val = object->getRepr()->attribute(attributes[i].c_str());
            ee->set_text (val ? val : (const gchar *) "");

#if WITH_GTKMM_3_0
            ee->set_hexpand();
            ee->set_vexpand();
            ee->set_margin_left(XPAD);
            ee->set_margin_right(XPAD);
            ee->set_margin_top(XPAD);
            ee->set_margin_bottom(XPAD);
            table->attach(*ee, 1, i, 1, 1);
#else
            table->attach (*ee, 1, 2, i, i + 1,
                               (Gtk::EXPAND | Gtk::FILL),
                               (Gtk::EXPAND | Gtk::FILL),
                               XPAD, YPAD );
#endif

            _entries.push_back(ee);
            g_signal_connect ( ee->gobj(), "changed",
                               G_CALLBACK (sp_attribute_table_entry_changed),
                               this );
        }
        /* Show table */
        table->show ();
        blocked = false;
    }
}

void SPAttributeTable::change_object(SPObject *object)
{
    g_return_if_fail (!object || SP_IS_OBJECT (object));
    if (_object)
    {
        modified_connection.disconnect();
        release_connection.disconnect();
        _object = NULL;
    }

    _object = object;
    if (_object) {
        blocked = true;

        // Set up object
        modified_connection = _object->connectModified(sigc::bind<2>(sigc::ptr_fun(&sp_attribute_table_object_modified), this));
        release_connection  = _object->connectRelease (sigc::bind<1>(sigc::ptr_fun(&sp_attribute_table_object_release), this));
        for (guint i = 0; i < (_attributes.size()); i++) {
            const gchar *val = _object->getRepr()->attribute(_attributes[i].c_str());
            _entries[i]->set_text(val ? val : "");
        }
        
        blocked = false;
    }

}

void SPAttributeTable::reread_properties(void)
{
    blocked = true;
	for (guint i = 0; i < (_attributes.size()); i++)
    {
        const gchar *val = _object->getRepr()->attribute(_attributes[i].c_str());
        _entries[i]->set_text(val ? val : "");
    }
	blocked = false;
}

static void sp_attribute_table_object_modified ( SPObject */*object*/,
                                     guint flags,
                                     SPAttributeTable *spat )
{
    if (flags & SP_OBJECT_MODIFIED_FLAG)
    {
        std::vector<Glib::ustring> attributes = spat->get_attributes();
        std::vector<Gtk::Entry *> entries = spat->get_entries();
        Glib::ustring text="";
        for (guint i = 0; i < (attributes.size()); i++) {
            Gtk::Entry* e = entries[i];
            const gchar *val = spat->_object->getRepr()->attribute(attributes[i].c_str());
            text = e->get_text ();
            if (val || !text.empty()) {
                if (text != val) {
                    // We are different
                    spat->blocked = true;
                    e->set_text (val ? val : (const gchar *) "");
                    spat->blocked = false;
                }
            }
        }
    }

} // end of sp_attribute_table_object_modified()

static void sp_attribute_table_entry_changed ( Gtk::Entry *editable,
                                   SPAttributeTable *spat )
{
    if (!spat->blocked)
    {
        std::vector<Glib::ustring> attributes = spat->get_attributes();
        std::vector<Gtk::Entry *> entries = spat->get_entries();
        for (guint i = 0; i < (attributes.size()); i++) {
            Gtk::Entry *e = entries[i];
            if ((GtkWidget*)editable == (GtkWidget*)e->gobj()) {
                spat->blocked = true;
                Glib::ustring text = e->get_text ();
                if (spat->_object) {
                    spat->_object->getRepr()->setAttribute(attributes[i].c_str(), text.c_str(), false);
                    DocumentUndo::done(spat->_object->document, SP_VERB_NONE,
                                       _("Set attribute"));
                }
                spat->blocked = false;
                return;
            }
        }
        g_warning ("file %s: line %d: Entry signalled change, but there is no such entry", __FILE__, __LINE__);
    }

} // end of sp_attribute_table_entry_changed()

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

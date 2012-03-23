/*
 * Authors:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_ENTRY__H
#define INKSCAPE_UI_WIDGET_ENTRY__H

#include "labelled.h"
#include <gtkmm.h>

#include <gtkmm/entry.h>
#include <gtkmm/comboboxtext.h>

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Helperclass for Gtk::Entry widgets.
 */
class Entry : public Labelled
{
public:
    Entry( Glib::ustring const &label,
           Glib::ustring const &tooltip,
           Glib::ustring const &suffix = "",
           Glib::ustring const &icon = "",
           bool mnemonic = true);

    // TO DO: add methods to access Gtk::Entry widget
    
    Gtk::Entry*  getEntry() {return (Gtk::Entry*)(_widget);};    
};

class ComboBoxText : public Labelled
{
public:
    ComboBoxText( Glib::ustring const &label,
           Glib::ustring const &tooltip,
           Glib::ustring const &suffix = "",
           Glib::ustring const &icon = "",
           bool mnemonic = true);

    Gtk::ComboBoxText*  getWidget() { return (Gtk::ComboBoxText *)_widget; };
    Gtk::Entry*  getEntry() { return (Gtk::Entry *)getWidget()->get_child(); };
    Glib::ustring get_text() { return getEntry()->get_text(); };
    void prepend_text(const Glib::ustring& text) {
#if WITH_GTKMM_2_24
        getWidget()->prepend(text);
#else
        getWidget()->prepend_text(text);
#endif
    };
    void append_text(const Glib::ustring& text) {
#if WITH_GTKMM_2_24
        getWidget()->append(text);
#else
        getWidget()->append_text(text);
#endif
    };
    void insert_text(gint position, const Glib::ustring& text) {
#if WITH_GTKMM_2_24
        getWidget()->insert(position, text);
#else
        getWidget()->insert_text(position, text);
#endif
    };
    void remove_text(const Glib::ustring& text) { getWidget()->remove_text(text); };
    void remove_all() {
#if WITH_GTKMM_2_24
        getWidget()->remove_all();
#else
        getWidget()->clear_items();
#endif
    };

};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_ENTRY__H

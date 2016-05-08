/** 
 * @file Object properties dialog.
 */
/* 
 * Inkscape, an Open Source vector graphics editor
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Copyright (C) 2012 Kris De Gussem <Kris.DeGussem@gmail.com>
 * c++version based on former C-version (GPL v2+) with authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *   Abhishek Sharma
 */

#ifndef SEEN_DIALOGS_ITEM_PROPERTIES_H
#define SEEN_DIALOGS_ITEM_PROPERTIES_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ui/widget/panel.h"
#include "ui/widget/frame.h"

#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/expander.h>
#include <gtkmm/frame.h>
#include <gtkmm/textview.h>
#include <gtkmm/comboboxtext.h>

#include "ui/dialog/desktop-tracker.h"

class SPAttributeTable;
class SPDesktop;
class SPItem;

namespace Gtk {
#if WITH_GTKMM_3_0
class Grid;
#else
class Table;
#endif
}

namespace Inkscape {
namespace UI {
namespace Dialog {

/**
 * A dialog widget to show object properties.
 *
 * A widget to enter an ID, label, title and description for an object.
 * In addition it allows to edit the properties of an object.
 */
class ObjectProperties : public Widget::Panel {
public:
    ObjectProperties();
    ~ObjectProperties();
    
    static ObjectProperties &getInstance() { return *new ObjectProperties(); }
    
    /// Updates entries and other child widgets on selection change, object modification, etc.
    void update();

private:
    bool _blocked;
    SPItem *_current_item; //to store the current item, for not wasting resources
    std::vector<Glib::ustring> _int_attrs;
    std::vector<Glib::ustring> _int_labels;

    Gtk::Label _label_id; //the label for the object ID
    Gtk::Entry _entry_id; //the entry for the object ID
    Gtk::Label _label_label; //the label for the object label
    Gtk::Entry _entry_label; //the entry for the object label
    Gtk::Label _label_title; //the label for the object title
    Gtk::Entry _entry_title; //the entry for the object title
    Gtk::Label _label_image_rendering; // the label for 'image-rendering'
    Gtk::ComboBoxText _combo_image_rendering; // the combo box text for 'image-rendering'
    
    Gtk::Frame  _ft_description; //the frame for the text of the object description
    Gtk::TextView _tv_description; //the text view object showing the object description

    Gtk::CheckButton _cb_hide; //the check button hide
    Gtk::CheckButton _cb_lock; //the check button lock

    Gtk::Expander _exp_interactivity; //the expander for interactivity
    SPAttributeTable *_attr_table; //the widget for showing the on... names at the bottom
    
    SPDesktop *_desktop;
    DesktopTracker _desktop_tracker;
    sigc::connection _desktop_changed_connection;
    sigc::connection _selection_changed_connection;
    sigc::connection _subselection_changed_connection;
    
    /// Constructor auxiliary function creating the child widgets.
    void _init();

    /// Sets object properties (ID, label, title, description) on user input.
    void _labelChanged();

    /// Callback for 'image-rendering'.
    void _imageRenderingChanged();

    /// Callback for checkbox Lock.
    void _sensitivityToggled();

    /// Callback for checkbox Hide.
    void _hiddenToggled();

    /// Can be invoked for setting the desktop. Currently not used.
    void _setDesktop(SPDesktop *desktop);
    
    /// Is invoked by the desktop tracker when the desktop changes.
    void _setTargetDesktop(SPDesktop *desktop);
};

}
}
}

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

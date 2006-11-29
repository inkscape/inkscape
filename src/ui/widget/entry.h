/** \file 
 *
 * \brief Helperclass for Gtk::Entry widgets
 *
 * Authors:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_ENTRY__H
#define INKSCAPE_UI_WIDGET_ENTRY__H

#include <gtkmm/entry.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>

namespace Inkscape {
namespace UI {
namespace Widget {

class Entry : public Gtk::HBox
{
public:
    Entry(Glib::ustring const &label, Glib::ustring const &tooltip);
        
    // TO DO: add methods to access _entry    
        
protected:
    Gtk::Tooltips _tooltips;
    Gtk::Label    _label;
    Gtk::Entry    _entry;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_ENTRY__H

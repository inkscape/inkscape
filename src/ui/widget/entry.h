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
#include "labelled.h"

namespace Inkscape {
namespace UI {
namespace Widget {

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

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_ENTRY__H

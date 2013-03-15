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

namespace Gtk {
class Entry;
}

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

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_ENTRY__H

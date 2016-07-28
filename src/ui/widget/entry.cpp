/*
 * Authors:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "entry.h"

#include <gtkmm/entry.h>

namespace Inkscape {
namespace UI {
namespace Widget {

Entry::Entry(  Glib::ustring const &label, Glib::ustring const &tooltip,
               Glib::ustring const &suffix,
               Glib::ustring const &icon,
               bool mnemonic)
    : Labelled(label, tooltip, new Gtk::Entry(), suffix, icon, mnemonic)
{    
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape


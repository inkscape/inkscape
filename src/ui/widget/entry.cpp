/** \file
 *
 * \brief Helperclass for Gtk::Entry widgets
 *
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

namespace Inkscape {
namespace UI {
namespace Widget {

Entry::Entry(Glib::ustring const &label, Glib::ustring const &tooltip)
    : _label(label, true), _entry(), _tooltips()
{
    pack_start(_label);
    pack_start(_entry);
    
    _tooltips.set_tip(*this, tooltip);
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape


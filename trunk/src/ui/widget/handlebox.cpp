/**
 * \brief HandleBox Widget - Adds a detachment handle to another widget.
 *
 * This work really doesn't amount to much more than a convenience constructor
 * for Gtk::HandleBox.  Maybe this could be contributed back to Gtkmm, as
 * Gtkmm provides several convenience constructors for other widgets as well.
 *
 * Author:
 *   Derek P. Moore <derekm@hackunix.org>
 *
 * Copyright (C) 2004 Derek P. Moore
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "handlebox.h"

namespace Inkscape {
namespace UI {
namespace Widget {

HandleBox::HandleBox(Gtk::Widget *widget,
                     Gtk::PositionType position)
    : _widget(widget)
{
    g_assert(_widget != NULL);
    add(*Gtk::manage(_widget));
    set_handle_position(position);
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape

/* 
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

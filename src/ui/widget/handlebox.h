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

#ifndef INKSCAPE_UI_WIDGET_HANDLEBOX_H
#define INKSCAPE_UI_WIDGET_HANDLEBOX_H

#include <gtkmm/handlebox.h>

namespace Inkscape {
namespace UI {
namespace Widget {

class HandleBox : public Gtk::HandleBox
{
public:
    HandleBox(Gtk::Widget *widget,
              Gtk::PositionType position = Gtk::POS_LEFT);

protected:
    Gtk::Widget *_widget;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_HANDLEBOX_H

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

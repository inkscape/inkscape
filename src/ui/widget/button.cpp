/**
 * \brief Button and CheckButton widgets
 *
 * Author:
 *   buliabyak@gmail.com
 *
 * Copyright (C) 2005 author
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "button.h"

namespace Inkscape {
namespace UI {
namespace Widget {

Button::Button(Glib::ustring const &label, Glib::ustring const &tooltip)
    : _tooltips()
{
    set_use_underline (true);
    set_label (label);
    _tooltips.set_tip(*this, tooltip);
}

CheckButton::CheckButton(Glib::ustring const &label, Glib::ustring const &tooltip)
    : _tooltips()
{
    set_use_underline (true);
    set_label (label);
    _tooltips.set_tip(*this, tooltip);
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

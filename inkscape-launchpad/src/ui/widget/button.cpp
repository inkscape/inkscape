/*
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
{
    set_use_underline (true);
    set_label (label);
    set_tooltip_text(tooltip);
}

CheckButton::CheckButton(Glib::ustring const &label, Glib::ustring const &tooltip)
{
    set_use_underline (true);
    set_label (label);
    set_tooltip_text(tooltip);
}

CheckButton::CheckButton(Glib::ustring const &label, Glib::ustring const &tooltip, bool active)
{
    set_use_underline (true);
    set_label (label);
    set_tooltip_text(tooltip);
    set_active(active);
}

RadioButton::RadioButton(Glib::ustring const &label, Glib::ustring const &tooltip)
{
    set_use_underline (true);
    set_label (label);
    set_tooltip_text(tooltip);
}


} // namespace Widget
} // namespace UI
} // namespace Inkscape

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

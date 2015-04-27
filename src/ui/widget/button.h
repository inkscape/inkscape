/*
 * Author:
 *   buliabyak@gmail.com
 *
 * Copyright (C) 2005 author
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_BUTTON_H
#define INKSCAPE_UI_WIDGET_BUTTON_H

#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Button widget.
 * @deprecated - no need for an explicit subclass... just perhaps a helper function.
 */
class Button : public Gtk::Button
{
public:
    Button();
    Button(Glib::ustring const &label, Glib::ustring const &tooltip);
};

/**
 * CheckButton widget.
 * @deprecated - no need for an explicit subclass... just perhaps a helper function.
 */
class CheckButton : public Gtk::CheckButton
{
public:
    CheckButton();
    CheckButton(Glib::ustring const &label, Glib::ustring const &tooltip);
    CheckButton(Glib::ustring const &label, Glib::ustring const &tooltip, bool active);

};

/**
 * RadioButton widget.
 * @deprecated - no need for an explicit subclass... just perhaps a helper function.
 */
class RadioButton : public Gtk::RadioButton
{
public:
    RadioButton();
    RadioButton(Glib::ustring const &label, Glib::ustring const &tooltip);

};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_BUTTON_H

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

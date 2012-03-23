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
 */
class Button : public Gtk::Button
{
public:
    Button();
    Button(Glib::ustring const &label, Glib::ustring const &tooltip);
};

/**
 * CheckButton widget.
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
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

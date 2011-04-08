/**
 * \brief SpinButton widget, that allows entry of both '.' and ',' for the decimal, even when in numeric mode.
 */
/*
 * Author:
 *   Johan B. C. Engelen
 *
 * Copyright (C) 2011 Author
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_SPINBUTTON_H
#define INKSCAPE_UI_WIDGET_SPINBUTTON_H

#include <gtkmm/spinbutton.h>

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * SpinButton widget, that allows entry of both '.' and ',' for the decimal, even when in numeric mode.
 */
class SpinButton : public Gtk::SpinButton
{
public:
  SpinButton() : Gtk::SpinButton() {};
  /// @todo perhaps more constructors should be added here

  virtual ~SpinButton() {};

protected:
  virtual void on_insert_text(const Glib::ustring& text, int* position);

private:
  // noncopyable
  SpinButton(const SpinButton&);
  SpinButton& operator=(const SpinButton&);
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_SPINBUTTON_H

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

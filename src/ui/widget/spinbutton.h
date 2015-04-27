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

class UnitMenu;
class UnitTracker;

/**
 * SpinButton widget, that allows entry of simple math expressions (also units, when linked with UnitMenu),
 * and allows entry of both '.' and ',' for the decimal, even when in numeric mode.
 *
 * Calling "set_numeric()" effectively disables the expression parsing. If no unit menu is linked, all unitlike characters are ignored.
 */
class SpinButton : public Gtk::SpinButton
{
public:
  SpinButton(double climb_rate = 0.0, guint digits = 0)
    : Gtk::SpinButton(climb_rate, digits),
      _unit_menu(NULL),
      _unit_tracker(NULL),
      _on_focus_in_value(0.)
  {
      connect_signals();
  };
#if GTK_CHECK_VERSION(3,0,0)
  explicit SpinButton(Glib::RefPtr<Gtk::Adjustment>& adjustment, double climb_rate = 0.0, guint digits = 0)
#else
  explicit SpinButton(Gtk::Adjustment& adjustment, double climb_rate = 0.0, guint digits = 0)
#endif
    : Gtk::SpinButton(adjustment, climb_rate, digits),
      _unit_menu(NULL),
      _unit_tracker(NULL),
      _on_focus_in_value(0.)
  {
      connect_signals();
  };

  virtual ~SpinButton() {};

  void setUnitMenu(UnitMenu* unit_menu) { _unit_menu = unit_menu; };
  
  void addUnitTracker(UnitTracker* ut) { _unit_tracker = ut; };

protected:
  UnitMenu *_unit_menu; /// Linked unit menu for unit conversion in entered expressions.
  UnitTracker *_unit_tracker; // Linked unit tracker for unit conversion in entered expressions.
  double _on_focus_in_value;

  void connect_signals();

    /**
     * This callback function should try to convert the entered text to a number and write it to newvalue.
     * It calls a method to evaluate the (potential) mathematical expression.
     *
     * @retval false No conversion done, continue with default handler.
     * @retval true  Conversion successful, don't call default handler. 
     */
    int on_input(double* newvalue);

    /**
     * When focus is obtained, save the value to enable undo later.
     * @retval false continue with default handler.
     * @retval true  don't call default handler. 
     */
    bool on_my_focus_in_event(GdkEventFocus* event);

    /**
     * Handle specific keypress events, like Ctrl+Z.
     *
     * @retval false continue with default handler.
     * @retval true  don't call default handler. 
     */
    bool on_my_key_press_event(GdkEventKey* event);

    /**
     * Undo the editing, by resetting the value upon when the spinbutton got focus.
     */
    void undo();

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
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

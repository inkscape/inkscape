/**
 * \brief Groups an HScale and a SpinButton together using the same Adjustment
 *
 * Author:
 *   Nicholas Bishop <nicholasbishop@gmail.com>
 *
 * Copyright (C) 2007 Author
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_SPIN_SLIDER_H
#define INKSCAPE_UI_WIDGET_SPIN_SLIDER_H

#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/scale.h>
#include <gtkmm/spinbutton.h>

namespace Inkscape {
namespace UI {
namespace Widget {

class SpinSlider : public Gtk::HBox
{
public:
    SpinSlider(double value, double lower, double upper, double step_inc,
	       double climb_rate, int digits);

    // Shortcuts to _adjustment
    Glib::SignalProxy0<void> signal_value_changed();
    double get_value() const;
    void set_value(const double);

    const Gtk::Adjustment& get_adjustment() const;
    Gtk::Adjustment& get_adjustment();
    
    const Gtk::HScale& get_scale() const;
    Gtk::HScale& get_scale();

    const Gtk::SpinButton& get_spin_button() const;
    Gtk::SpinButton& get_spin_button();
private:
    Gtk::Adjustment _adjustment;
    Gtk::HScale _scale;
    Gtk::SpinButton _spin;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_SPIN_SLIDER_H

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

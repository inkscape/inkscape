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

#include "spin-slider.h"

namespace Inkscape {
namespace UI {
namespace Widget {


SpinSlider::SpinSlider(double value, double lower, double upper, double step_inc,
                       double climb_rate, int digits)
    : _adjustment(value, lower, upper, step_inc),
      _scale(_adjustment), _spin(_adjustment, climb_rate, digits)
{
    pack_start(_scale);
    pack_start(_spin, false, false);

    _scale.set_draw_value(false);

    show_all_children();
}

Glib::SignalProxy0<void> SpinSlider::signal_value_changed()
{
    return _adjustment.signal_value_changed();
}

double SpinSlider::get_value() const
{
    return _adjustment.get_value();
}

void SpinSlider::set_value(const double val)
{
    _adjustment.set_value(val);
}

const Gtk::Adjustment& SpinSlider::get_adjustment() const
{
    return _adjustment;
}
Gtk::Adjustment& SpinSlider::get_adjustment()
{
    return _adjustment;
}

const Gtk::HScale& SpinSlider::get_scale() const
{
    return _scale;
}
Gtk::HScale& SpinSlider::get_scale()
{
    return _scale;
}

const Gtk::SpinButton& SpinSlider::get_spin_button() const
{
    return _spin;
}
Gtk::SpinButton& SpinSlider::get_spin_button()
{
    return _spin;
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

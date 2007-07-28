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

#include "glib/gstrfuncs.h"

#include "spin-slider.h"

namespace Inkscape {
namespace UI {
namespace Widget {

SpinSlider::SpinSlider(double value, double lower, double upper, double step_inc,
                       double climb_rate, int digits, const SPAttributeEnum a)
    : AttrWidget(a), _adjustment(value, lower, upper, step_inc),
      _scale(_adjustment), _spin(_adjustment, climb_rate, digits)
{
    pack_start(_scale);
    pack_start(_spin, false, false);

    _scale.set_draw_value(false);

    show_all_children();
}

Glib::ustring SpinSlider::get_as_attribute() const
{
    const double val = _adjustment.get_value();

    if(_spin.get_digits() == 0)
        return Glib::Ascii::dtostr((int)val);
    else
        return Glib::Ascii::dtostr(val);
}

void SpinSlider::set_from_attribute(SPObject* o)
{
    const gchar* val = attribute_value(o);
    if(val)
        _adjustment.set_value(Glib::Ascii::strtod(val));
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

void SpinSlider::set_update_policy(const Gtk::UpdateType u)
{
    _scale.set_update_policy(u);
}

void SpinSlider::remove_scale()
{
    remove(_scale);
}

DualSpinSlider::DualSpinSlider(double value, double lower, double upper, double step_inc,
                               double climb_rate, int digits, const SPAttributeEnum a)
    : AttrWidget(a), _s1(value, lower, upper, step_inc, climb_rate, digits),
      _s2(value, lower, upper, step_inc, climb_rate, digits)
{
    _s1.get_adjustment().signal_value_changed().connect(_signal_value_changed.make_slot());
    _s2.get_adjustment().signal_value_changed().connect(_signal_value_changed.make_slot());
}

Glib::ustring DualSpinSlider::get_as_attribute() const
{
    return _s1.get_as_attribute() + " " + _s2.get_as_attribute();
}

void DualSpinSlider::set_from_attribute(SPObject* o)
{
    const gchar* val = attribute_value(o);
    if(val) {
        // Split val into parts
        gchar** toks = g_strsplit(val, " ", 2);

        if(toks) {
            double v1, v2;
            if(toks[0])
                v1 = v2 = Glib::Ascii::strtod(toks[0]);
            if(toks[1])
                v2 = Glib::Ascii::strtod(toks[1]);

            _s1.get_adjustment().set_value(v1);
            _s2.get_adjustment().set_value(v2);

            g_strfreev(toks);
        }
    }
}

sigc::signal<void>& DualSpinSlider::signal_value_changed()
{
    return _signal_value_changed;
}

const SpinSlider& DualSpinSlider::get_spinslider1() const
{
    return _s1;
}

SpinSlider& DualSpinSlider::get_spinslider1()
{
    return _s1;
}

const SpinSlider& DualSpinSlider::get_spinslider2() const
{
    return _s2;
}

SpinSlider& DualSpinSlider::get_spinslider2()
{
    return _s2;
}

void DualSpinSlider::set_update_policy(const Gtk::UpdateType u)
{
    _s1.set_update_policy(u);
    _s2.set_update_policy(u);
}

void DualSpinSlider::remove_scale()
{
    _s1.remove_scale();
    _s2.remove_scale();
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

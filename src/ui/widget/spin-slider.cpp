/**
 * \brief Groups an HScale and a SpinButton together using the same Adjustment
 *
 * Author:
 *   Nicholas Bishop <nicholasbishop@gmail.com>
 *   Felipe C. da S. Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2007 Author
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include "glib/gstrfuncs.h"
#include "glibmm/i18n.h"

#include "spin-slider.h"

namespace Inkscape {
namespace UI {
namespace Widget {

SpinSlider::SpinSlider(double value, double lower, double upper, double step_inc,
                       double climb_rate, int digits, const SPAttributeEnum a, char* tip_text)
    : AttrWidget(a, value), _adjustment(value, lower, upper, step_inc),
      _scale(_adjustment), _spin(_adjustment, climb_rate, digits)
{
    signal_value_changed().connect(signal_attr_changed().make_slot());

    pack_start(_scale);
    pack_start(_spin, false, false);
    if (tip_text){
        _tt.set_tip(_scale, tip_text);
        _tt.set_tip(_spin, tip_text);
    }

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
    else
        _adjustment.set_value(get_default()->as_double());
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
                               double climb_rate, int digits, const SPAttributeEnum a, char* tip_text1, char* tip_text2)
    : AttrWidget(a),
      _s1(value, lower, upper, step_inc, climb_rate, digits, SP_ATTR_INVALID, tip_text1),
      _s2(value, lower, upper, step_inc, climb_rate, digits, SP_ATTR_INVALID, tip_text2),
      //TRANSLATORS: only translate "string" in "context|string".
      // For more details, see http://developer.gnome.org/doc/API/2.0/glib/glib-I18N.html#Q-:CAPS
      // "Link" means to _link_ two sliders together
      _link(Q_("sliders|Link"))
{
    signal_value_changed().connect(signal_attr_changed().make_slot());

    _s1.get_adjustment().signal_value_changed().connect(_signal_value_changed.make_slot());
    _s2.get_adjustment().signal_value_changed().connect(_signal_value_changed.make_slot());
    _s1.get_adjustment().signal_value_changed().connect(sigc::mem_fun(*this, &DualSpinSlider::update_linked));
    _link.signal_toggled().connect(sigc::mem_fun(*this, &DualSpinSlider::link_toggled));

    Gtk::VBox* vb = Gtk::manage(new Gtk::VBox);
    vb->add(_s1);
    vb->add(_s2);
    pack_start(*vb);
    pack_start(_link, false, false);
    _link.set_active(true);

    show_all();
}

Glib::ustring DualSpinSlider::get_as_attribute() const
{
    if(_link.get_active())
        return _s1.get_as_attribute();
    else
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

            _link.set_active(toks[1] == 0);

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

void DualSpinSlider::link_toggled()
{
    _s2.set_sensitive(!_link.get_active());
    update_linked();
}

void DualSpinSlider::update_linked()
{
    if(_link.get_active())
        _s2.set_value(_s1.get_value());
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

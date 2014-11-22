/*
 * Author:
 *
 * Copyright (C) 2012 Author
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include "spin-scale.h"

#include <gtkmm/adjustment.h>
#include <glibmm/i18n.h>
#include <glibmm/stringutils.h>

#include "ui/widget/gimpspinscale.h"

namespace Inkscape {
namespace UI {
namespace Widget {

SpinScale::SpinScale(const char* label, double value, double lower, double upper, double step_inc,
                     double /*climb_rate*/, int digits, const SPAttributeEnum a, const char* tip_text)
    : AttrWidget(a, value)
{
#if WITH_GTKMM_3_0
    _adjustment = Gtk::Adjustment::create(value, lower, upper, step_inc);
    _spinscale = gimp_spin_scale_new (_adjustment->gobj(), label, digits);
#else
    _adjustment = new Gtk::Adjustment(value, lower, upper, step_inc);
    _spinscale = gimp_spin_scale_new (_adjustment->gobj(), label, digits);
#endif

    signal_value_changed().connect(signal_attr_changed().make_slot());

    pack_start(*Gtk::manage(Glib::wrap(_spinscale)));

    if (tip_text){
        gtk_widget_set_tooltip_text( _spinscale, tip_text );
    }

    show_all_children();
}

SpinScale::SpinScale(const char* label,
#if WITH_GTKMM_3_0
       Glib::RefPtr<Gtk::Adjustment> adj,
#else
       Gtk::Adjustment *adj,
#endif
        int digits, const SPAttributeEnum a, const char* tip_text)
    : AttrWidget(a, 0.0),
      _adjustment(adj)

{

    _spinscale = gimp_spin_scale_new (_adjustment->gobj(), label, digits);

    signal_value_changed().connect(signal_attr_changed().make_slot());

    pack_start(*Gtk::manage(Glib::wrap(_spinscale)));

    if (tip_text){
        gtk_widget_set_tooltip_text( _spinscale, tip_text );
    }

    show_all_children();
}

Glib::ustring SpinScale::get_as_attribute() const
{
    const double val = _adjustment->get_value();

    //if(_spin.get_digits() == 0)
    //    return Glib::Ascii::dtostr((int)val);
    //else
        return Glib::Ascii::dtostr(val);
}

void SpinScale::set_from_attribute(SPObject* o)
{
    const gchar* val = attribute_value(o);
    if(val)
        _adjustment->set_value(Glib::Ascii::strtod(val));
    else
        _adjustment->set_value(get_default()->as_double());
}

Glib::SignalProxy0<void> SpinScale::signal_value_changed()
{
    return _adjustment->signal_value_changed();
}

double SpinScale::get_value() const
{
    return _adjustment->get_value();
}

void SpinScale::set_value(const double val)
{
    _adjustment->set_value(val);
}

void SpinScale::set_focuswidget(GtkWidget *widget)
{
    gimp_spin_scale_set_focuswidget(_spinscale, widget);
}


void SpinScale::set_appearance(const gchar* appearance)
{
    gimp_spin_scale_set_appearance(_spinscale, appearance);
}

#if WITH_GTKMM_3_0
const Glib::RefPtr<Gtk::Adjustment> SpinScale::get_adjustment() const
#else
const Gtk::Adjustment *SpinScale::get_adjustment() const
#endif
{
    return _adjustment;
}
#if WITH_GTKMM_3_0
Glib::RefPtr<Gtk::Adjustment> SpinScale::get_adjustment()
#else
Gtk::Adjustment *SpinScale::get_adjustment()
#endif
{
    return _adjustment;
}


DualSpinScale::DualSpinScale(const char* label1, const char* label2, double value, double lower, double upper, double step_inc,
                               double climb_rate, int digits, const SPAttributeEnum a, char* tip_text1, char* tip_text2)
    : AttrWidget(a),
      _s1(label1, value, lower, upper, step_inc, climb_rate, digits, SP_ATTR_INVALID, tip_text1),
      _s2(label2, value, lower, upper, step_inc, climb_rate, digits, SP_ATTR_INVALID, tip_text2),
      //TRANSLATORS: "Link" means to _link_ two sliders together
      _link(C_("Sliders", "Link"))
{
    signal_value_changed().connect(signal_attr_changed().make_slot());

    _s1.get_adjustment()->signal_value_changed().connect(_signal_value_changed.make_slot());
    _s2.get_adjustment()->signal_value_changed().connect(_signal_value_changed.make_slot());
    _s1.get_adjustment()->signal_value_changed().connect(sigc::mem_fun(*this, &DualSpinScale::update_linked));

    _link.signal_toggled().connect(sigc::mem_fun(*this, &DualSpinScale::link_toggled));

    Gtk::VBox* vb = Gtk::manage(new Gtk::VBox);
    vb->add(_s1);
    vb->add(_s2);
    pack_start(*vb);
    pack_start(_link, false, false);
    _link.set_active(true);

    show_all();
}

Glib::ustring DualSpinScale::get_as_attribute() const
{
    if(_link.get_active())
        return _s1.get_as_attribute();
    else
        return _s1.get_as_attribute() + " " + _s2.get_as_attribute();
}

void DualSpinScale::set_from_attribute(SPObject* o)
{
    const gchar* val = attribute_value(o);
    if(val) {
        // Split val into parts
        gchar** toks = g_strsplit(val, " ", 2);

        if(toks) {
            double v1 = 0.0, v2 = 0.0;
            if(toks[0])
                v1 = v2 = Glib::Ascii::strtod(toks[0]);
            if(toks[1])
                v2 = Glib::Ascii::strtod(toks[1]);

            _link.set_active(toks[1] == 0);

            _s1.get_adjustment()->set_value(v1);
            _s2.get_adjustment()->set_value(v2);

            g_strfreev(toks);
        }
    }
}

sigc::signal<void>& DualSpinScale::signal_value_changed()
{
    return _signal_value_changed;
}

const SpinScale& DualSpinScale::get_SpinScale1() const
{
    return _s1;
}

SpinScale& DualSpinScale::get_SpinScale1()
{
    return _s1;
}

const SpinScale& DualSpinScale::get_SpinScale2() const
{
    return _s2;
}

SpinScale& DualSpinScale::get_SpinScale2()
{
    return _s2;
}

/*void DualSpinScale::remove_scale()
{
    _s1.remove_scale();
    _s2.remove_scale();
}*/

void DualSpinScale::link_toggled()
{
    _s2.set_sensitive(!_link.get_active());
    update_linked();
}

void DualSpinScale::update_linked()
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
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

/*
 * Authors:
 *   Carl Hetherington <inkscape@carlh.net>
 *   Derek P. Moore <derekm@hackunix.org>
 *   Bryce Harrington <bryce@bryceharrington.org>
 *   Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Copyright (C) 2004-2011 authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "scalar.h"
#include "spinbutton.h"
#include <gtkmm/scale.h>

namespace Inkscape {
namespace UI {
namespace Widget {

Scalar::Scalar(Glib::ustring const &label, Glib::ustring const &tooltip,
               Glib::ustring const &suffix,
               Glib::ustring const &icon,
               bool mnemonic)
    : Labelled(label, tooltip, new SpinButton(), suffix, icon, mnemonic),
      setProgrammatically(false)
{
}

Scalar::Scalar(Glib::ustring const &label, Glib::ustring const &tooltip,
               unsigned digits,
               Glib::ustring const &suffix,
               Glib::ustring const &icon,
               bool mnemonic)
    : Labelled(label, tooltip, new SpinButton(0.0, digits), suffix, icon, mnemonic),
      setProgrammatically(false)
{
}

Scalar::Scalar(Glib::ustring const &label, Glib::ustring const &tooltip,
#if WITH_GTKMM_3_0
               Glib::RefPtr<Gtk::Adjustment> &adjust,
#else
               Gtk::Adjustment &adjust,
#endif
               unsigned digits,
               Glib::ustring const &suffix,
               Glib::ustring const &icon,
               bool mnemonic)
    : Labelled(label, tooltip, new SpinButton(adjust, 0.0, digits), suffix, icon, mnemonic),
      setProgrammatically(false)
{
}

unsigned Scalar::getDigits() const
{
    g_assert(_widget != NULL);
    return static_cast<SpinButton*>(_widget)->get_digits();
}

double Scalar::getStep() const
{
    g_assert(_widget != NULL);
    double step, page;
    static_cast<SpinButton*>(_widget)->get_increments(step, page);
    return step;
}

double Scalar::getPage() const
{
    g_assert(_widget != NULL);
    double step, page;
    static_cast<SpinButton*>(_widget)->get_increments(step, page);
    return page;
}

double Scalar::getRangeMin() const
{
    g_assert(_widget != NULL);
    double min, max;
    static_cast<SpinButton*>(_widget)->get_range(min, max);
    return min;
}

double Scalar::getRangeMax() const
{
    g_assert(_widget != NULL);
    double min, max;
    static_cast<SpinButton*>(_widget)->get_range(min, max);
    return max;
}

double Scalar::getValue() const
{
    g_assert(_widget != NULL);
    return static_cast<SpinButton*>(_widget)->get_value();
}

int Scalar::getValueAsInt() const
{
    g_assert(_widget != NULL);
    return static_cast<SpinButton*>(_widget)->get_value_as_int();
}


void Scalar::setDigits(unsigned digits)
{
    g_assert(_widget != NULL);
    static_cast<SpinButton*>(_widget)->set_digits(digits);
}

void Scalar::setIncrements(double step, double /*page*/)
{
    g_assert(_widget != NULL);
    static_cast<SpinButton*>(_widget)->set_increments(step, 0);
}

void Scalar::setRange(double min, double max)
{
    g_assert(_widget != NULL);
    static_cast<SpinButton*>(_widget)->set_range(min, max);
}

void Scalar::setValue(double value)
{
    g_assert(_widget != NULL);
    setProgrammatically = true; // callback is supposed to reset back, if it cares
    static_cast<SpinButton*>(_widget)->set_value(value);
}

void Scalar::update()
{
    g_assert(_widget != NULL);
    static_cast<SpinButton*>(_widget)->update();
}

void Scalar::addSlider()
{
#if WITH_GTKMM_3_0
    Gtk::Scale *scale = new Gtk::Scale(static_cast<SpinButton*>(_widget)->get_adjustment());
#else
    Gtk::HScale *scale = new Gtk::HScale( * static_cast<SpinButton*>(_widget)->get_adjustment() );
#endif
    scale->set_draw_value(false);
    add (*manage (scale));
}

Glib::SignalProxy0<void> Scalar::signal_value_changed()
{
    return static_cast<SpinButton*>(_widget)->signal_value_changed();
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

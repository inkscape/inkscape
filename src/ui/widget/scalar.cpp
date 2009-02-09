/**
 * \brief Scalar Widget - A labelled text box, with spin buttons and optional
 *        icon or suffix, for entering arbitrary number values.
 *
 * Authors:
 *   Carl Hetherington <inkscape@carlh.net>
 *   Derek P. Moore <derekm@hackunix.org>
 *   Bryce Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Carl Hetherington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "scalar.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Construct a Scalar Widget.
 *
 * \param label     Label.
 * \param suffix    Suffix, placed after the widget (defaults to "").
 * \param icon      Icon filename, placed before the label (defaults to "").
 * \param mnemonic  Mnemonic toggle; if true, an underscore (_) in the label
 *                  indicates the next character should be used for the
 *                  mnemonic accelerator key (defaults to false).
 */
Scalar::Scalar(Glib::ustring const &label, Glib::ustring const &tooltip,
               Glib::ustring const &suffix,
               Glib::ustring const &icon,
               bool mnemonic)
    : Labelled(label, tooltip, new Gtk::SpinButton(), suffix, icon, mnemonic),
      setProgrammatically(false)
{
    static_cast<Gtk::SpinButton*>(_widget)->set_numeric();
}

/**
 * Construct a Scalar Widget.
 *
 * \param label     Label.
 * \param digits    Number of decimal digits to display.
 * \param suffix    Suffix, placed after the widget (defaults to "").
 * \param icon      Icon filename, placed before the label (defaults to "").
 * \param mnemonic  Mnemonic toggle; if true, an underscore (_) in the label
 *                  indicates the next character should be used for the
 *                  mnemonic accelerator key (defaults to false).
 */
Scalar::Scalar(Glib::ustring const &label, Glib::ustring const &tooltip,
               unsigned digits,
               Glib::ustring const &suffix,
               Glib::ustring const &icon,
               bool mnemonic)
    : Labelled(label, tooltip, new Gtk::SpinButton(0.0, digits), suffix, icon, mnemonic),
      setProgrammatically(false)
{
    static_cast<Gtk::SpinButton*>(_widget)->set_numeric();
}

/**
 * Construct a Scalar Widget.
 *
 * \param label     Label.
 * \param adjust    Adjustment to use for the SpinButton.
 * \param digits    Number of decimal digits to display (defaults to 0).
 * \param suffix    Suffix, placed after the widget (defaults to "").
 * \param icon      Icon filename, placed before the label (defaults to "").
 * \param mnemonic  Mnemonic toggle; if true, an underscore (_) in the label
 *                  indicates the next character should be used for the
 *                  mnemonic accelerator key (defaults to true).
 */
Scalar::Scalar(Glib::ustring const &label, Glib::ustring const &tooltip,
               Gtk::Adjustment &adjust,
               unsigned digits,
               Glib::ustring const &suffix,
               Glib::ustring const &icon,
               bool mnemonic)
    : Labelled(label, tooltip, new Gtk::SpinButton(adjust, 0.0, digits), suffix, icon, mnemonic),
      setProgrammatically(false)
{
    static_cast<Gtk::SpinButton*>(_widget)->set_numeric();
}

/** Fetches the precision of the spin buton */
unsigned
Scalar::getDigits() const
{
    g_assert(_widget != NULL);
    return static_cast<Gtk::SpinButton*>(_widget)->get_digits();
}

/** Gets the current step ingrement used by the spin button */
double
Scalar::getStep() const
{
    g_assert(_widget != NULL);
    double step, page;
    static_cast<Gtk::SpinButton*>(_widget)->get_increments(step, page);
    return step;
}

/** Gets the current page increment used by the spin button */
double
Scalar::getPage() const
{
    g_assert(_widget != NULL);
    double step, page;
    static_cast<Gtk::SpinButton*>(_widget)->get_increments(step, page);
    return page;
}

/** Gets the minimum range value allowed for the spin button */
double
Scalar::getRangeMin() const
{
    g_assert(_widget != NULL);
    double min, max;
    static_cast<Gtk::SpinButton*>(_widget)->get_range(min, max);
    return min;
}

/** Gets the maximum range value allowed for the spin button */
double
Scalar::getRangeMax() const
{
    g_assert(_widget != NULL);
    double min, max;
    static_cast<Gtk::SpinButton*>(_widget)->get_range(min, max);
    return max;
}

/** Get the value in the spin_button . */
double
Scalar::getValue() const
{
    g_assert(_widget != NULL);
    return static_cast<Gtk::SpinButton*>(_widget)->get_value();
}

/** Get the value spin_button represented as an integer. */
int
Scalar::getValueAsInt() const
{
    g_assert(_widget != NULL);
    return static_cast<Gtk::SpinButton*>(_widget)->get_value_as_int();
}


/** Sets the precision to be displayed by the spin button */
void
Scalar::setDigits(unsigned digits)
{
    g_assert(_widget != NULL);
    static_cast<Gtk::SpinButton*>(_widget)->set_digits(digits);
}

/** Sets the step and page increments for the spin button
 * @todo Remove the second parameter - deprecated
 */
void
Scalar::setIncrements(double step, double /*page*/)
{
    g_assert(_widget != NULL);
    static_cast<Gtk::SpinButton*>(_widget)->set_increments(step, 0);
}

/** Sets the minimum and maximum range allowed for the spin button */
void
Scalar::setRange(double min, double max)
{
    g_assert(_widget != NULL);
    static_cast<Gtk::SpinButton*>(_widget)->set_range(min, max);
}

/** Sets the value of the spin button */
void
Scalar::setValue(double value)
{
    g_assert(_widget != NULL);
    setProgrammatically = true; // callback is supposed to reset back, if it cares
    static_cast<Gtk::SpinButton*>(_widget)->set_value(value);
}

/** Manually forces an update of the spin button */
void
Scalar::update() {
    g_assert(_widget != NULL);
    static_cast<Gtk::SpinButton*>(_widget)->update();
}



/** Signal raised when the spin button's value changes */
Glib::SignalProxy0<void>
Scalar::signal_value_changed()
{
    return static_cast<Gtk::SpinButton*>(_widget)->signal_value_changed();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

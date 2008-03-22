/**
 * \brief Point Widget - A labelled text box, with spin buttons and optional
 *        icon or suffix, for entering arbitrary coordinate values.
 *
 * Authors:
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *   Carl Hetherington <inkscape@carlh.net>
 *   Derek P. Moore <derekm@hackunix.org>
 *   Bryce Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2007 Authors
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "ui/widget/point.h"
#include "ui/widget/labelled.h"
#include "ui/widget/scalar.h"
#include <gtkmm/box.h>

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Construct a Point Widget.
 *
 * \param label     Label.
 * \param suffix    Suffix, placed after the widget (defaults to "").
 * \param icon      Icon filename, placed before the label (defaults to "").
 * \param mnemonic  Mnemonic toggle; if true, an underscore (_) in the label
 *                  indicates the next character should be used for the
 *                  mnemonic accelerator key (defaults to false).
 */
Point::Point(Glib::ustring const &label, Glib::ustring const &tooltip,
               Glib::ustring const &suffix,
               Glib::ustring const &icon,
               bool mnemonic)
    : Labelled(label, tooltip, new Gtk::VBox(), suffix, icon, mnemonic),
      xwidget("X:",""),
      ywidget("Y:","")
{
    static_cast<Gtk::VBox*>(_widget)->pack_start(xwidget, true, true);
    static_cast<Gtk::VBox*>(_widget)->pack_start(ywidget, true, true);
    static_cast<Gtk::VBox*>(_widget)->show_all_children();
}

/**
 * Construct a Point Widget.
 *
 * \param label     Label.
 * \param digits    Number of decimal digits to display.
 * \param suffix    Suffix, placed after the widget (defaults to "").
 * \param icon      Icon filename, placed before the label (defaults to "").
 * \param mnemonic  Mnemonic toggle; if true, an underscore (_) in the label
 *                  indicates the next character should be used for the
 *                  mnemonic accelerator key (defaults to false).
 */
Point::Point(Glib::ustring const &label, Glib::ustring const &tooltip,
               unsigned digits,
               Glib::ustring const &suffix,
               Glib::ustring const &icon,
               bool mnemonic)
    : Labelled(label, tooltip, new Gtk::VBox(), suffix, icon, mnemonic),
      xwidget("X:","", digits),
      ywidget("Y:","", digits)
{
    static_cast<Gtk::VBox*>(_widget)->pack_start(xwidget, true, true);
    static_cast<Gtk::VBox*>(_widget)->pack_start(ywidget, true, true);
    static_cast<Gtk::VBox*>(_widget)->show_all_children();
}

/**
 * Construct a Point Widget.
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
Point::Point(Glib::ustring const &label, Glib::ustring const &tooltip,
               Gtk::Adjustment &adjust,
               unsigned digits,
               Glib::ustring const &suffix,
               Glib::ustring const &icon,
               bool mnemonic)
    : Labelled(label, tooltip, new Gtk::VBox(), suffix, icon, mnemonic),
      xwidget("X:","", adjust, digits),
      ywidget("Y:","", adjust, digits)
{
    static_cast<Gtk::VBox*>(_widget)->pack_start(xwidget, true, true);
    static_cast<Gtk::VBox*>(_widget)->pack_start(ywidget, true, true);
    static_cast<Gtk::VBox*>(_widget)->show_all_children();
}

/** Fetches the precision of the spin buton */
unsigned
Point::getDigits() const
{
    return xwidget.getDigits();
}

/** Gets the current step ingrement used by the spin button */
double
Point::getStep() const
{
    return xwidget.getStep();
}

/** Gets the current page increment used by the spin button */
double
Point::getPage() const
{
    return xwidget.getPage();
}

/** Gets the minimum range value allowed for the spin button */
double
Point::getRangeMin() const
{
    return xwidget.getRangeMin();
}

/** Gets the maximum range value allowed for the spin button */
double
Point::getRangeMax() const
{
    return xwidget.getRangeMax();
}

/** Get the value in the spin_button . */
double
Point::getXValue() const
{
    return xwidget.getValue();
}
double
Point::getYValue() const
{
    return ywidget.getValue();
}
Geom::Point
Point::getValue() const
{
    return Geom::Point( getXValue() , getYValue() );
}

/** Get the value spin_button represented as an integer. */
int
Point::getXValueAsInt() const
{
    return xwidget.getValueAsInt();
}
int
Point::getYValueAsInt() const
{
    return ywidget.getValueAsInt();
}


/** Sets the precision to be displayed by the spin button */
void
Point::setDigits(unsigned digits)
{
    xwidget.setDigits(digits);
    ywidget.setDigits(digits);
}

/** Sets the step and page increments for the spin button */
void
Point::setIncrements(double step, double page)
{
    xwidget.setIncrements(step, page);
    ywidget.setIncrements(step, page);
}

/** Sets the minimum and maximum range allowed for the spin button */
void
Point::setRange(double min, double max)
{
    xwidget.setRange(min, max);
    ywidget.setRange(min, max);
}

/** Sets the value of the spin button */
void
Point::setValue(double xvalue, double yvalue)
{
    xwidget.setValue(xvalue);
    ywidget.setValue(yvalue);
}

/** Manually forces an update of the spin button */
void
Point::update() {
    xwidget.update();
    ywidget.update();
}

/** Check 'setProgrammatically' of both scalar widgets.   False if value is changed by user by clicking the widget. */
bool
Point::setProgrammatically() {
    return (xwidget.setProgrammatically || ywidget.setProgrammatically);
}

void
Point::clearProgrammatically() {
    xwidget.setProgrammatically = false;
    ywidget.setProgrammatically = false;
}


/** Signal raised when the spin button's value changes */
Glib::SignalProxy0<void>
Point::signal_x_value_changed()
{
    return xwidget.signal_value_changed();
}
Glib::SignalProxy0<void>
Point::signal_y_value_changed()
{
    return ywidget.signal_value_changed();
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

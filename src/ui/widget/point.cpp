/*
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

Point::Point(Glib::ustring const &label, Glib::ustring const &tooltip,
#if WITH_GTKMM_3_0
               Glib::RefPtr<Gtk::Adjustment> &adjust,
#else
               Gtk::Adjustment &adjust,
#endif
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

unsigned Point::getDigits() const
{
    return xwidget.getDigits();
}

double Point::getStep() const
{
    return xwidget.getStep();
}

double Point::getPage() const
{
    return xwidget.getPage();
}

double Point::getRangeMin() const
{
    return xwidget.getRangeMin();
}

double Point::getRangeMax() const
{
    return xwidget.getRangeMax();
}

double Point::getXValue() const
{
    return xwidget.getValue();
}

double Point::getYValue() const
{
    return ywidget.getValue();
}

Geom::Point Point::getValue() const
{
    return Geom::Point( getXValue() , getYValue() );
}

int Point::getXValueAsInt() const
{
    return xwidget.getValueAsInt();
}

int Point::getYValueAsInt() const
{
    return ywidget.getValueAsInt();
}


void Point::setDigits(unsigned digits)
{
    xwidget.setDigits(digits);
    ywidget.setDigits(digits);
}

void Point::setIncrements(double step, double page)
{
    xwidget.setIncrements(step, page);
    ywidget.setIncrements(step, page);
}

void Point::setRange(double min, double max)
{
    xwidget.setRange(min, max);
    ywidget.setRange(min, max);
}

void Point::setValue(Geom::Point const & p)
{
    xwidget.setValue(p[0]);
    ywidget.setValue(p[1]);
}

void Point::update()
{
    xwidget.update();
    ywidget.update();
}

bool Point::setProgrammatically() 
{
    return (xwidget.setProgrammatically || ywidget.setProgrammatically);
}

void Point::clearProgrammatically()
{
    xwidget.setProgrammatically = false;
    ywidget.setProgrammatically = false;
}


Glib::SignalProxy0<void> Point::signal_x_value_changed()
{
    return xwidget.signal_value_changed();
}

Glib::SignalProxy0<void> Point::signal_y_value_changed()
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

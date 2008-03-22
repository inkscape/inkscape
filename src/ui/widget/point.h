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

#ifndef INKSCAPE_UI_WIDGET_POINT_H
#define INKSCAPE_UI_WIDGET_POINT_H

#include <gtkmm/adjustment.h>
#include <gtkmm/spinbutton.h>
#include <2geom/point.h>
#include "ui/widget/labelled.h"
#include "ui/widget/scalar.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class Point : public Labelled
{
public:
    Point( Glib::ustring const &label,
           Glib::ustring const &tooltip,
           Glib::ustring const &suffix = "",
           Glib::ustring const &icon = "",
           bool mnemonic = true);
    Point( Glib::ustring const &label,
           Glib::ustring const &tooltip,
           unsigned digits,
           Glib::ustring const &suffix = "",
           Glib::ustring const &icon = "",
           bool mnemonic = true);
    Point( Glib::ustring const &label,
           Glib::ustring const &tooltip,
           Gtk::Adjustment &adjust,
           unsigned digits = 0,
           Glib::ustring const &suffix = "",
           Glib::ustring const &icon = "",
           bool mnemonic = true);

    unsigned getDigits() const;
    double  getStep() const;
    double  getPage() const;
    double  getRangeMin() const;
    double  getRangeMax() const;
    bool    getSnapToTicks() const;
    double  getXValue() const;
    double  getYValue() const;
    Geom::Point getValue() const;
    int     getXValueAsInt() const;
    int     getYValueAsInt() const;

    void    setDigits(unsigned digits);
    void    setIncrements(double step, double page);
    void    setRange(double min, double max);
    void    setValue(double xvalue, double yvalue);

    void    update();

    Glib::SignalProxy0<void> signal_x_value_changed();
    Glib::SignalProxy0<void> signal_y_value_changed();

    bool setProgrammatically(); // true if the value was set by setValue, not changed by the user; 
                                // if a callback checks it, it must reset it back to false
    void clearProgrammatically();

protected:
    Scalar xwidget, ywidget;

};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_POINT_H

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

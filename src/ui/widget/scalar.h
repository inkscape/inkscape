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

#ifndef INKSCAPE_UI_WIDGET_SCALAR_H
#define INKSCAPE_UI_WIDGET_SCALAR_H

#include <gtkmm/adjustment.h>
#include <gtkmm/spinbutton.h>

#include "labelled.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class Scalar : public Labelled
{
public:
    Scalar(Glib::ustring const &label,
           Glib::ustring const &tooltip,
           Glib::ustring const &suffix = "",
           Glib::ustring const &icon = "",
           bool mnemonic = true);
    Scalar(Glib::ustring const &label,
           Glib::ustring const &tooltip,
           unsigned digits,
           Glib::ustring const &suffix = "",
           Glib::ustring const &icon = "",
           bool mnemonic = true);
    Scalar(Glib::ustring const &label,
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
    double  getValue() const;
    int     getValueAsInt() const;

    void    setDigits(unsigned digits);
    void    setIncrements(double step, double page);
    void    setRange(double min, double max);
    void    setValue(double value);

    void    update();

    Glib::SignalProxy0<void> signal_value_changed();

    bool setProgrammatically; // true if the value was set by setValue, not changed by the user; 
                                    // if a callback checks it, it must reset it back to false
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_SCALAR_H

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

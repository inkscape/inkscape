/**
 * \brief Scalar Unit Widget - A labelled text box, with spin buttons and
 *        optional icon or suffix, for entering the values of various unit
 *        types.
 *
 * Authors:
 *   Bryce Harrington <bryce@bryceharrington.org>
 *   Derek P. Moore <derekm@hackunix.org>
 *   buliabyak@gmail.com
 *
 * Copyright (C) 2004-2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_SCALAR_UNIT_H
#define INKSCAPE_UI_WIDGET_SCALAR_UNIT_H

#include "scalar.h"
#include "unit-menu.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class ScalarUnit : public Scalar
{
public:
    ScalarUnit(Glib::ustring const &label, Glib::ustring const &tooltip,
               UnitType unit_type = UNIT_TYPE_LINEAR,
               Glib::ustring const &suffix = "",
               Glib::ustring const &icon = "",
               UnitMenu *unit_menu = NULL,
               bool mnemonic = true);

    void      initScalar(double min_value, double max_value);

    Unit      getUnit() const;
    UnitType  getUnitType() const;
    double    getValue(Glib::ustring const &units) const;

    bool      setUnit(Glib::ustring const &units);
    void      setValue(double number, Glib::ustring const &units);
    void      setValue(double number);

    void      setHundredPercent(double number);
    void      setAbsoluteIsIncrement(bool value);
    void      setPercentageIsIncrement(bool value);

    double PercentageToAbsolute(double value);
    double AbsoluteToPercentage(double value);

    double getAsPercentage();
    void setFromPercentage(double value);

    void on_unit_changed();

protected:
    UnitMenu  *_unit_menu;

    double _hundred_percent; // the length that corresponds to 100%, in px, for %-to/from-absolute conversions

    bool _absolute_is_increment; // if true, 120% with _hundred_percent=100px gets converted to/from 20px; otherwise, to/from 120px
    bool _percentage_is_increment; // if true, 120px with _hundred_percent=100px gets converted to/from 20%; otherwise, to/from 120%
                                            // if both are true, 20px is converted to/from 20% if _hundred_percent=100px

    Glib::ustring lastUnits; // previously selected unit, for conversions
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_SCALAR_UNIT_H

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

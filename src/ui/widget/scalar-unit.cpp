/**
 * \brief Scalar Unit Widget - A labelled text box, with spin buttons and
 *        optional icon or suffix, for entering the values of various unit
 *        types.
 *
 * A ScalarUnit is a control for entering, viewing, or manipulating
 * numbers with units.  This differs from ordinary numbers like 2 or
 * 3.14 because the number portion of a scalar *only* has meaning
 * when considered with its unit type.  For instance, 12 m and 12 in
 * have very different actual values, but 1 m and 100 cm have the same
 * value.  The ScalarUnit allows us to abstract the presentation of
 * the scalar to the user from the internal representations used by
 * the program.
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "scalar-unit.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Construct a ScalarUnit
 *
 * \param label      Label.
 * \param unit_type  Unit type (defaults to UNIT_TYPE_LINEAR).
 * \param suffix     Suffix, placed after the widget (defaults to "").
 * \param icon       Icon filename, placed before the label (defaults to "").
 * \param unit_menu  UnitMenu drop down; if not specified, one will be created
 *                   and displayed after the widget (defaults to NULL).
 * \param mnemonic   Mnemonic toggle; if true, an underscore (_) in the label
 *                   indicates the next character should be used for the
 *                   mnemonic accelerator key (defaults to true).
 */
ScalarUnit::ScalarUnit(Glib::ustring const &label, Glib::ustring const &tooltip,
                       UnitType unit_type,
                       Glib::ustring const &suffix,
                       Glib::ustring const &icon,
                       UnitMenu *unit_menu,
                       bool mnemonic)
    : Scalar(label, tooltip, suffix, icon, mnemonic),
      _unit_menu(unit_menu),
      _hundred_percent(0),
      _absolute_is_increment(false),
      _percentage_is_increment(false)
{
    if (_unit_menu == NULL) {
        _unit_menu = new UnitMenu();
        g_assert(_unit_menu);
        _unit_menu->setUnitType(unit_type);
        pack_start(*Gtk::manage(_unit_menu), false, false, 4);
    }
    _unit_menu->signal_changed()
            .connect_notify(sigc::mem_fun(*this, &ScalarUnit::on_unit_changed));
}

/**
 * Initializes the scalar based on the settings in _unit_menu.
 * Requires that _unit_menu has already been initialized.
 */
void
ScalarUnit::initScalar(double min_value, double max_value)
{
    g_assert(_unit_menu != NULL);
    Scalar::setDigits(_unit_menu->getDefaultDigits());
    Scalar::setIncrements(_unit_menu->getDefaultStep(),
                          _unit_menu->getDefaultPage());
    Scalar::setRange(min_value, max_value);
}

/** Sets the unit for the ScalarUnit widget */
bool
ScalarUnit::setUnit(Glib::ustring const &unit) {
    g_assert(_unit_menu != NULL);
    // First set the unit
    if (!_unit_menu->setUnit(unit)) {
        return false;
    }
    lastUnits = unit;
    return true;
}

/** Gets the object for the currently selected unit */
Unit
ScalarUnit::getUnit() const {
    g_assert(_unit_menu != NULL);
    return _unit_menu->getUnit();
}

/** Gets the UnitType ID for the unit */
UnitType
ScalarUnit::getUnitType() const {
    g_assert(_unit_menu);
    return _unit_menu->getUnitType();
}

/** Sets the number and unit system */
void
ScalarUnit::setValue(double number, Glib::ustring const &units) {
    g_assert(_unit_menu != NULL);
    _unit_menu->setUnit(units);
    Scalar::setValue(number);
}

/** Sets the number only */
void
ScalarUnit::setValue(double number) {
    Scalar::setValue(number);
}

/** Returns the value in the given unit system */
double
ScalarUnit::getValue(Glib::ustring const &unit_name) const {
    g_assert(_unit_menu != NULL);
    if (unit_name == "") {
        // Return the value in the default units
        return Scalar::getValue();
    } else {
        double conversion = _unit_menu->getConversion(unit_name);
        return conversion * Scalar::getValue();
    }
}

void
ScalarUnit::setHundredPercent(double number)
{
    _hundred_percent = number;
}

void
ScalarUnit::setAbsoluteIsIncrement(bool value)
{
    _absolute_is_increment = value;
}

void
ScalarUnit::setPercentageIsIncrement(bool value)
{
    _percentage_is_increment = value;
}

/** Convert value from % to absolute, using _hundred_percent and *_is_increment flags */
double
ScalarUnit::PercentageToAbsolute(double value)
{
    // convert from percent to absolute
    double convertedVal = 0;
    double hundred_converted = _hundred_percent / _unit_menu->getConversion("px"); // _hundred_percent is in px
    if (_percentage_is_increment) 
        value += 100;
    convertedVal = 0.01 * hundred_converted * value;
    if (_absolute_is_increment) 
        convertedVal -= hundred_converted;

    return convertedVal;
}

/** Convert value from absolute to %, using _hundred_percent and *_is_increment flags */
double
ScalarUnit::AbsoluteToPercentage(double value)
{
    double convertedVal = 0;
    // convert from absolute to percent
    if (_hundred_percent == 0) {
        if (_percentage_is_increment)
            convertedVal = 0;
        else 
            convertedVal = 100;
    } else {
        double hundred_converted = _hundred_percent / _unit_menu->getConversion("px", lastUnits); // _hundred_percent is in px
        if (_absolute_is_increment) 
            value += hundred_converted;
        convertedVal = 100 * value / hundred_converted;
        if (_percentage_is_increment) 
            convertedVal -= 100;
    }

    return convertedVal;
}

/** Assuming the current unit is absolute, get the corresponding % value */
double
ScalarUnit::getAsPercentage()
{
    double convertedVal = AbsoluteToPercentage(Scalar::getValue());
    return convertedVal;
}


/** Assuming the current unit is absolute, set the value corresponding to a given % */
void 
ScalarUnit::setFromPercentage(double value)
{
    double absolute = PercentageToAbsolute(value);
    Scalar::setValue(absolute);
}


/** Signal handler for updating the value and suffix label when unit is changed */
void
ScalarUnit::on_unit_changed()
{
    g_assert(_unit_menu != NULL);

    Glib::ustring abbr = _unit_menu->getUnitAbbr();
    _suffix->set_label(abbr);

    Inkscape::Util::UnitTable &table = _unit_menu->getUnitTable();
    Inkscape::Util::Unit new_unit = (table.getUnit(abbr));
    Inkscape::Util::Unit old_unit = (table.getUnit(lastUnits));

    double convertedVal = 0;
    if (old_unit.type == UNIT_TYPE_DIMENSIONLESS && new_unit.type == UNIT_TYPE_LINEAR) {
        convertedVal = PercentageToAbsolute(Scalar::getValue());
    } else if (old_unit.type == UNIT_TYPE_LINEAR && new_unit.type == UNIT_TYPE_DIMENSIONLESS) {
        convertedVal = AbsoluteToPercentage(Scalar::getValue());
    } else {
        double conversion = _unit_menu->getConversion(lastUnits);
        convertedVal = Scalar::getValue() / conversion;
    }
    Scalar::setValue(convertedVal);

    lastUnits = abbr;
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

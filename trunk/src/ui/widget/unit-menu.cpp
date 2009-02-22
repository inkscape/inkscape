/**
 * \brief Unit Menu Widget - A drop down menu for choosing unit types.
 *
 * Author:
 *   Bryce Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <cmath>

#include "unit-menu.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 *    Construct a UnitMenu
 *
 */
UnitMenu::UnitMenu() : _type(UNIT_TYPE_NONE)
{
    set_active(0);
}

UnitMenu::~UnitMenu() {
}

/** Adds the unit type to the widget.  This extracts the corresponding
    units from the unit map matching the given type, and appends them
    to the dropdown widget.  It causes the primary unit for the given
    unit_type to be selected.  */
bool
UnitMenu::setUnitType(UnitType unit_type) 
{
    /* Expand the unit widget with unit entries from the unit table */
    UnitTable::UnitMap m = _unit_table.units(unit_type);
    UnitTable::UnitMap::iterator iter = m.begin();
    while(iter != m.end()) {
        Glib::ustring text = (*iter).first;
        append_text(text);
        ++iter;
    }
    _type = unit_type;
    set_active_text(_unit_table.primary(unit_type));

    return true;
}

/** Returns the Unit object corresponding to the current selection
    in the dropdown widget */
Unit
UnitMenu::getUnit() const {
    if (get_active_text() == "") {
        g_assert(_type != UNIT_TYPE_NONE);
        return _unit_table.getUnit(_unit_table.primary(_type));
    }
    return _unit_table.getUnit(get_active_text());
}

/** Sets the dropdown widget to the given unit abbreviation. 
    Returns true if the unit was selectable, false if not 
    (i.e., if the unit was not present in the widget) */
bool
UnitMenu::setUnit(Glib::ustring const & unit) {
    // TODO:  Determine if 'unit' is available in the dropdown.
    //        If not, return false

    set_active_text(unit);
    return true;
}

/** Returns the abbreviated unit name of the selected unit */
Glib::ustring
UnitMenu::getUnitAbbr() const {
    if (get_active_text() == "") {
        return "";
    }
    return getUnit().abbr;
}

/** Returns the UnitType of the selected unit */
UnitType
UnitMenu::getUnitType() const {
    return getUnit().type;
}

/** Returns the unit factor for the selected unit */
double 
UnitMenu::getUnitFactor() const
{
    return getUnit().factor;
}

/** Returns the recommended number of digits for displaying
 *  numbers of this unit type.  
 */
int
UnitMenu::getDefaultDigits() const
{
    return getUnit().defaultDigits();
}

/** Returns the recommended step size in spin buttons
 *  displaying units of this type
 */
double
UnitMenu::getDefaultStep() const
{ 
    int factor_digits = -1*int(log10(getUnit().factor));
    return pow(10.0, factor_digits);
}

/** Returns the recommended page size (when hitting pgup/pgdn)
 *  in spin buttons displaying units of this type
 */
double
UnitMenu::getDefaultPage() const
{
    return 10 * getDefaultStep();
}

/**
 *  Returns the conversion factor required to convert values
 *  of the currently selected unit into units of type
 *  new_unit_abbr.
 */
double
UnitMenu::getConversion(Glib::ustring const &new_unit_abbr, Glib::ustring const &old_unit_abbr) const
{
    double old_factor = getUnit().factor;
    if (old_unit_abbr != "no_unit")
        old_factor = _unit_table.getUnit(old_unit_abbr).factor;
    Unit new_unit = _unit_table.getUnit(new_unit_abbr);

    // Catch the case of zero or negative unit factors (error!)
    if (old_factor < 0.0000001 ||
        new_unit.factor < 0.0000001) {
        // TODO:  Should we assert here?
        return 0.00;
    }

    return old_factor / new_unit.factor;
}

/** Returns true if the selected unit is not dimensionless 
 *  (false for %, true for px, pt, cm, etc) 
 */
bool
UnitMenu::isAbsolute() const {
    return getUnitType() != UNIT_TYPE_DIMENSIONLESS;
}

/** Returns true if the selected unit is radial (deg or rad)
 */
bool
UnitMenu::isRadial() const {
    return getUnitType() == UNIT_TYPE_RADIAL;
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

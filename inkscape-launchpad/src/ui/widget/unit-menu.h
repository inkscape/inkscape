/*
 * Author:
 *   Bryce Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_UNIT_H
#define INKSCAPE_UI_WIDGET_UNIT_H

#include <gtkmm/comboboxtext.h>
#include "util/units.h"

using namespace Inkscape::Util;

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * A drop down menu for choosing unit types.
 */
class UnitMenu : public Gtk::ComboBoxText
{
public:

    /**
     *    Construct a UnitMenu
     */
    UnitMenu();

    virtual ~UnitMenu();

    /**
     * Adds the unit type to the widget.  This extracts the corresponding
     * units from the unit map matching the given type, and appends them
     * to the dropdown widget.  It causes the primary unit for the given
     * unit_type to be selected.
     */
    bool          setUnitType(UnitType unit_type);

    /**
     * Removes all unit entries, then adds the unit type to the widget.
     * This extracts the corresponding
     * units from the unit map matching the given type, and appends them
     * to the dropdown widget.  It causes the primary unit for the given
     * unit_type to be selected.
     */
    bool          resetUnitType(UnitType unit_type);

    /**
     * Adds a unit, possibly user-defined, to the menu.
     */
    void          addUnit(Unit const& u);

    /**
     * Sets the dropdown widget to the given unit abbreviation. 
     * Returns true if the unit was selectable, false if not 
     * (i.e., if the unit was not present in the widget).
     */
    bool          setUnit(Glib::ustring const &unit);

    /**
     * Returns the Unit object corresponding to the current selection
     * in the dropdown widget.
     */
    Unit const *  getUnit() const;

    /**
     * Returns the abbreviated unit name of the selected unit.
     */
    Glib::ustring getUnitAbbr() const;

    /**
     * Returns the UnitType of the selected unit.
     */
    UnitType      getUnitType() const;

    /**
     * Returns the unit factor for the selected unit.
     */
    double        getUnitFactor() const;

    /**
     * Returns the recommended number of digits for displaying
     *  numbers of this unit type.  
     */
    int           getDefaultDigits() const;

    /**
     * Returns the recommended step size in spin buttons
     *  displaying units of this type.
     */
    double        getDefaultStep() const;

    /**
     * Returns the recommended page size (when hitting pgup/pgdn)
     *  in spin buttons displaying units of this type.
     */
    double        getDefaultPage() const;

    /**
     *  Returns the conversion factor required to convert values
     *  of the currently selected unit into units of type
     *  new_unit_abbr.
     */
    double        getConversion(Glib::ustring const &new_unit_abbr, Glib::ustring const &old_unit_abbr = "no_unit") const;

    /**
     * Returns true if the selected unit is not dimensionless 
     *  (false for %, true for px, pt, cm, etc).
     */
    bool          isAbsolute() const;

    /**
     * Returns true if the selected unit is radial (deg or rad).
     */
    bool          isRadial() const;

protected:
    UnitType          _type;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_UNIT_H

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

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

#ifndef INKSCAPE_UI_WIDGET_UNIT_H
#define INKSCAPE_UI_WIDGET_UNIT_H

#include "combo-text.h"
#include "util/units.h"

using namespace Inkscape::Util;

namespace Inkscape {
namespace UI {
namespace Widget {

class UnitMenu : public ComboText
{
public:
    UnitMenu();
    virtual ~UnitMenu();

    bool          setUnitType(UnitType unit_type);

    bool          setUnit(Glib::ustring const &unit);

    Unit          getUnit() const;
    Glib::ustring getUnitAbbr() const;
    UnitType      getUnitType() const;
    double        getUnitFactor() const;

    int           getDefaultDigits() const;
    double        getDefaultStep() const;
    double        getDefaultPage() const;

    double        getConversion(Glib::ustring const &new_unit_abbr, Glib::ustring const &old_unit_abbr = "no_unit") const;

    bool          isAbsolute() const;
    bool          isRadial() const;

    UnitTable     &getUnitTable() {return _unit_table;}

protected:
    UnitTable     _unit_table;
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
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

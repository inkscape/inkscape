/*
This is a rough draft of a global 'units' thingee, to allow dialogs and
the ruler to share info about unit systems...  Dunno if this is the
right kind of object though, so we may have to redo this or shift things
around later when it becomes clearer what we need.

This object is used for defining different unit systems.

This is intended to eventually replace inkscape/helper/units.*.

Need to review the Units support that's in Gtkmm already...

*/

#ifndef INKSCAPE_UTIL_UNITS_H
#define INKSCAPE_UTIL_UNITS_H

#include <map>
#include <glibmm/ustring.h>

namespace Inkscape {
namespace Util {

enum UnitType {
    UNIT_TYPE_DIMENSIONLESS,     /* Percentage */
    UNIT_TYPE_LINEAR,
    UNIT_TYPE_LINEAR_SCALED,
    UNIT_TYPE_RADIAL,
    UNIT_TYPE_TIME,
    UNIT_TYPE_FONT_HEIGHT,
    UNIT_TYPE_QTY,
    UNIT_TYPE_NONE = -1
};

class Unit {
 public:
    Glib::ustring  name;
    Glib::ustring  name_plural;
    Glib::ustring  abbr;
    Glib::ustring  description;

    UnitType       type;

    double         factor;

    bool           isAbsolute() const { return type != UNIT_TYPE_DIMENSIONLESS; }
    int            defaultDigits() const;
};

class UnitTable {
 public:
    UnitTable();
    virtual ~UnitTable();

    typedef std::map<Glib::ustring, Unit*> UnitMap;

    void    addUnit(Unit const& u, bool primary);
    Unit    getUnit(Glib::ustring const& name) const;
    bool    deleteUnit(Unit const& u);
    bool    hasUnit(Glib::ustring const &name) const;

    UnitTable::UnitMap units(UnitType type) const;

    Glib::ustring primary(UnitType type) const;

    double  getScale() const;
    void    setScale();

    bool    load(Glib::ustring const &filename);
    bool    loadText(Glib::ustring const &filename);
    bool    save(Glib::ustring const &filename);

 protected:
    UnitTable::UnitMap  _unit_map;
    Glib::ustring       _primary_unit[UNIT_TYPE_QTY];

    double              _linear_scale;

 private:
    UnitTable(UnitTable const& t);
    UnitTable operator=(UnitTable const& t);

};

} // namespace Util
} // namespace Inkscape

#endif // define INKSCAPE_UTIL_UNITS_H

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

const char DEG[] = "°";

class Unit {
 public:
    Unit();
    Unit(UnitType type,
         double factor,
         Glib::ustring const &name,
         Glib::ustring const &name_plural,
         Glib::ustring const &abbr,
         Glib::ustring const &description);

    void clear();

    bool           isAbsolute() const { return type != UNIT_TYPE_DIMENSIONLESS; }

    /**
     * Returns the suggested precision to use for displaying numbers
     * of this unit.
     */
    int            defaultDigits() const;

    UnitType       type;
    double         factor;
    Glib::ustring  name;
    Glib::ustring  name_plural;
    Glib::ustring  abbr;
    Glib::ustring  description;
};

class UnitTable {
 public:
    /**
     * Initializes the unit tables and identifies the primary unit types.
     *
     * The primary unit's conversion factor is required to be 1.00
     */
    UnitTable();
    virtual ~UnitTable();

    typedef std::map<Glib::ustring, Unit*> UnitMap;

    /** Add a new unit to the table */
    void    addUnit(Unit const& u, bool primary);

    /** Retrieve a given unit based on its string identifier */
    Unit    getUnit(Glib::ustring const& name) const;

    /** Remove a unit definition from the given unit type table */
    bool    deleteUnit(Unit const& u);

    /** Returns true if the given string 'name' is a valid unit in the table */
    bool    hasUnit(Glib::ustring const &name) const;

    /** Provides an iteratable list of items in the given unit table */
    UnitTable::UnitMap units(UnitType type) const;

    /** Returns the default unit abbr for the given type */
    Glib::ustring primary(UnitType type) const;

    double  getScale() const;

    void    setScale();

    bool    load(Glib::ustring const &filename);

    /** Loads units from a text file.
     *
     * loadText loads and merges the contents of the given file into the UnitTable,
     * possibly overwriting existing unit definitions.
     *
     * @param filename file to be loaded
     */
    bool    loadText(Glib::ustring const &filename);

    /** Saves the current UnitTable to the given file. */
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

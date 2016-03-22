/*
 * Inkscape Units
 * These classes are used for defining different unit systems.
 *
 * Authors:
 *   Matthew Petroff <matthew@mpetroff.net>
 *
 * Copyright (C) 2013 Matthew Petroff
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_UTIL_UNITS_H
#define INKSCAPE_UTIL_UNITS_H

#include <map>
#include <boost/operators.hpp>
#include <glibmm/ustring.h>
#include <2geom/coord.h>
#include "svg/svg-length.h"
#include "unordered-containers.h"

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

class Unit
    : boost::equality_comparable<Unit>
{
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

    /** Checks if a unit is compatible with the specified unit. */
    bool           compatibleWith(Unit const *u) const;
    bool           compatibleWith(Glib::ustring const &) const;
    bool           compatibleWith(char const *) const;

    UnitType       type;
    double         factor;
    Glib::ustring  name;
    Glib::ustring  name_plural;
    Glib::ustring  abbr;
    Glib::ustring  description;
    
    /** Check if units are equal. */
    bool operator==(Unit const &other) const;
    
    /** Get SVG unit code. */
    int svgUnit() const;

    /** Convert value from this unit **/
    double convert(double from_dist, Unit const *to) const;
    double convert(double from_dist, Glib::ustring const &to) const;
    double convert(double from_dist, char const *to) const;
};

class Quantity
    : boost::totally_ordered<Quantity>
{
public:
    Unit const *unit;
    double quantity;
    
    /** Initialize a quantity. */
    Quantity(double q, Unit const *u);
    Quantity(double q, Glib::ustring const &u);
    Quantity(double q, char const *u);
    
    /** Checks if a quantity is compatible with the specified unit. */
    bool compatibleWith(Unit const *u) const;
    bool compatibleWith(Glib::ustring const &u) const;
    bool compatibleWith(char const *u) const;
    
    /** Return the quantity's value in the specified unit. */
    double value(Unit const *u) const;
    double value(Glib::ustring const &u) const;
    double value(char const *u) const;
    
    /** Return a printable string of the value in the specified unit. */
    Glib::ustring string(Unit const *u) const;
    Glib::ustring string(Glib::ustring const &u) const;
    Glib::ustring string() const;
    
    /** Convert distances. 
       no NULL check is performed on the passed pointers to Unit objects!  */
    static double convert(double from_dist, Unit const *from, Unit const *to);
    static double convert(double from_dist, Glib::ustring const &from, Unit const *to);
    static double convert(double from_dist, Unit const *from, Glib::ustring const &to);
    static double convert(double from_dist, Glib::ustring const &from, Glib::ustring const &to);
    static double convert(double from_dist, char const *from, char const *to);

    /** Comparison operators. */
    bool operator<(Quantity const &rhs) const;
    bool operator==(Quantity const &other) const;
};

inline bool are_near(Quantity const &a, Quantity const &b, double eps=Geom::EPSILON)
{
    return Geom::are_near(a.quantity, b.value(a.unit), eps);
}

class UnitTable {
public:
    /**
     * Initializes the unit tables and identifies the primary unit types.
     *
     * The primary unit's conversion factor is required to be 1.00
     */
    UnitTable();
    virtual ~UnitTable();

    typedef INK_UNORDERED_MAP<Glib::ustring, Unit> UnitMap;
    typedef INK_UNORDERED_MAP<unsigned, Unit*> UnitCodeMap;

    /** Add a new unit to the table */
    void    addUnit(Unit const &u, bool primary);

    /** Retrieve a given unit based on its string identifier */
    Unit const *getUnit(Glib::ustring const &name) const;
    Unit const *getUnit(char const *name) const;

    /** Try to find a unit based on its conversion factor to the primary */
    Unit const *findUnit(double factor, UnitType type) const;
    
    /** Retrieve a given unit based on its SVGLength unit */
    Unit const *getUnit(SVGLength::Unit u) const;
    
    /** Retrieve a quantity based on its string identifier */
    Quantity parseQuantity(Glib::ustring const &q) const;

    /** Remove a unit definition from the given unit type table * /
     * DISABLED, unsafe with the current passing around pointers to Unit objects in this table */
    //bool    deleteUnit(Unit const &u);

    /** Returns true if the given string 'name' is a valid unit in the table */
    bool    hasUnit(Glib::ustring const &name) const;

    /** Provides an iteratable list of items in the given unit table */
    UnitMap units(UnitType type) const;

    /** Returns the default unit abbr for the given type */
    Glib::ustring primary(UnitType type) const;

    double  getScale() const;

    void    setScale();

    /** Load units from an XML file.
     *
     * Loads and merges the contents of the given file into the UnitTable,
     * possibly overwriting existing unit definitions.
     *
     * @param filename file to be loaded
     */
    bool    load(std::string const &filename);

    /* * Saves the current UnitTable to the given file. */
    //bool    save(std::string const &filename);

protected:
    UnitCodeMap         _unit_map;
    Glib::ustring       _primary_unit[UNIT_TYPE_QTY];

    double              _linear_scale;
    static Unit         _empty_unit;

private:
    UnitTable(UnitTable const &t);
    UnitTable operator=(UnitTable const &t);

};

extern UnitTable unit_table;

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

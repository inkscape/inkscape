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
#include <glibmm/ustring.h>
#include "svg/svg-length.h"

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

    /** Checks if a unit is compatible with the specified unit. */
    bool           compatibleWith(const Unit &u) const;
    bool           compatibleWith(const Glib::ustring) const;

    UnitType       type;
    double         factor;
    Glib::ustring  name;
    Glib::ustring  name_plural;
    Glib::ustring  abbr;
    Glib::ustring  description;
    
    /** Check if units are equal. */
    friend bool operator== (const Unit &u1, const Unit &u2);
    /** Check if units are not equal. */ 
    friend bool operator!= (const Unit &u1, const Unit &u2);
    
    /** Get SVG unit. */
    int svgUnit() const;
};

class Quantity {
public:
    const Unit *unit;
    double quantity;
    
    /** Initialize a quantity. */
    Quantity(double q, const Unit &u);          // constructor
    Quantity(double q, const Glib::ustring u);  // constructor
    
    /** Checks if a quantity is compatible with the specified unit. */
    bool compatibleWith(const Unit &u) const;
    bool compatibleWith(const Glib::ustring u) const;
    
    /** Return the quantity's value in the specified unit. */
    double value(const Unit &u) const;
    double value(const Glib::ustring u) const;
    
    /** Return a printable string of the value in the specified unit. */
    Glib::ustring string(const Unit &u) const;
    Glib::ustring string(const Glib::ustring u) const;
    Glib::ustring string() const;
    
    /** Convert distances. */
    static double convert(const double from_dist, const Unit &from, const Unit &to);
    static double convert(const double from_dist, const Glib::ustring from, const Unit &to);
    static double convert(const double from_dist, const Unit &from, const Glib::ustring to);
    static double convert(const double from_dist, const Glib::ustring from, const Glib::ustring to);
    
    /** Comparison operators. */
    friend bool operator< (const Quantity &ql, const Quantity &qr);
    friend bool operator> (const Quantity &ql, const Quantity &qr);
    friend bool operator!= (const Quantity &q1, const Quantity &q2);
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
    void    addUnit(Unit const &u, bool primary);

    /** Retrieve a given unit based on its string identifier */
    Unit    getUnit(Glib::ustring const &name) const;
    
    /** Retrieve a given unit based on its SVGLength unit */
    Unit    getUnit(SVGLength::Unit const u) const;
    
    /** Retrieve a quantity based on its string identifier */
    Quantity getQuantity(Glib::ustring const &q) const;

    /** Remove a unit definition from the given unit type table */
    bool    deleteUnit(Unit const &u);

    /** Returns true if the given string 'name' is a valid unit in the table */
    bool    hasUnit(Glib::ustring const &name) const;

    /** Provides an iteratable list of items in the given unit table */
    UnitTable::UnitMap units(UnitType type) const;

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

    /** Saves the current UnitTable to the given file. */
    bool    save(std::string const &filename);

 protected:
    UnitTable::UnitMap  _unit_map;
    Glib::ustring       _primary_unit[UNIT_TYPE_QTY];

    double              _linear_scale;

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

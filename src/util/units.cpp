/*
 * Inkscape Units
 *
 * Authors:
 *   Matthew Petroff <matthew@mpetroff.net>
 *
 * Copyright (C) 2013 Matthew Petroff
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <cmath>
#include <cerrno>
#include <iomanip>
#include <glib.h>
#include <glibmm/regex.h>
#include <glibmm/fileutils.h>
#include <glibmm/markup.h>

#include "util/units.h"
#include "path-prefix.h"
#include "streq.h"

using Inkscape::Util::UNIT_TYPE_DIMENSIONLESS;
using Inkscape::Util::UNIT_TYPE_LINEAR;
using Inkscape::Util::UNIT_TYPE_RADIAL;
using Inkscape::Util::UNIT_TYPE_FONT_HEIGHT;

namespace
{

/**
 * A std::map that gives the data type value for the string version.
 *
 * Note that we'd normally not return a reference to an internal version, but
 * for this constant case it allows us to check against getTypeMappings().end().
 */
/** @todo consider hiding map behind hasFoo() and getFoo() type functions.*/
std::map<Glib::ustring, Inkscape::Util::UnitType> &getTypeMappings()
{
    static bool init = false;
    static std::map<Glib::ustring, Inkscape::Util::UnitType> typeMap;
    if (!init)
    {
        init = true;
        typeMap["DIMENSIONLESS"] = UNIT_TYPE_DIMENSIONLESS;
        typeMap["LINEAR"] = UNIT_TYPE_LINEAR;
        typeMap["RADIAL"] = UNIT_TYPE_RADIAL;
        typeMap["FONT_HEIGHT"] = UNIT_TYPE_FONT_HEIGHT;
        // Note that code was not yet handling LINEAR_SCALED, TIME, QTY and NONE
    }
    return typeMap;
}

} // namespace

namespace Inkscape {
namespace Util {

class UnitParser : public Glib::Markup::Parser
{
public:
    typedef Glib::Markup::Parser::AttributeMap AttrMap;
    typedef Glib::Markup::ParseContext Ctx;

    UnitParser(UnitTable *table);
    virtual ~UnitParser() {}

protected:
    virtual void on_start_element(Ctx &ctx, Glib::ustring const &name, AttrMap const &attrs);
    virtual void on_end_element(Ctx &ctx, Glib::ustring const &name);
    virtual void on_text(Ctx &ctx, Glib::ustring const &text);

public:
    UnitTable *tbl;
    bool primary;
    bool skip;
    Unit unit;
};

UnitParser::UnitParser(UnitTable *table) :
    tbl(table),
    primary(false),
    skip(false)
{
}

#define BUFSIZE (255)

Unit::Unit() :
    type(UNIT_TYPE_DIMENSIONLESS), // should this or NONE be the default?
    factor(1.0),
    name(),
    name_plural(),
    abbr(),
    description()
{
}

Unit::Unit(UnitType type,
           double factor,
           Glib::ustring const &name,
           Glib::ustring const &name_plural,
           Glib::ustring const &abbr,
           Glib::ustring const &description) :
    type(type),
    factor(factor),
    name(name),
    name_plural(name_plural),
    abbr(abbr),
    description(description)
{
    g_return_if_fail(factor <= 0);
}

void Unit::clear()
{
    *this = Unit();
}

int Unit::defaultDigits() const
{
    int factor_digits = int(log10(factor));
    if (factor_digits < 0) {
        g_warning("factor = %f, factor_digits = %d", factor, factor_digits);
        g_warning("factor_digits < 0 - returning 0");
        factor_digits = 0;
    }
    return factor_digits;
}

bool Unit::compatibleWith(const Unit &u) const
{
    // Percentages
    if (type == UNIT_TYPE_DIMENSIONLESS || u.type == UNIT_TYPE_DIMENSIONLESS) {
        return true;
    }
    
    // Other units with same type
    if (type == u.type) {
        return true;
    }
    
    // Different, incompatible types
    return false;
}
bool Unit::compatibleWith(const Glib::ustring u) const
{
    static UnitTable unit_table;
    return compatibleWith(unit_table.getUnit(u));
}

bool operator== (const Unit &u1, const Unit &u2)
{
    return (u1.type == u2.type && u1.name.compare(u2.name) == 0);
}

bool operator!= (const Unit &u1, const Unit &u2)
{
    return !(u1 == u2);
}

int Unit::svgUnit() const
{
    if (!abbr.compare("px"))
        return SVGLength::PX;
    if (!abbr.compare("pt"))
        return SVGLength::PT;
    if (!abbr.compare("pc"))
        return SVGLength::PC;
    if (!abbr.compare("mm"))
        return SVGLength::MM;
    if (!abbr.compare("cm"))
        return SVGLength::CM;
    if (!abbr.compare("in"))
        return SVGLength::INCH;
    if (!abbr.compare("ft"))
        return SVGLength::FOOT;
    if (!abbr.compare("em"))
        return SVGLength::EM;
    if (!abbr.compare("ex"))
        return SVGLength::EX;
    if (!abbr.compare("%"))
        return SVGLength::PERCENT;
    return 0;
}

UnitTable::UnitTable()
{
    gchar *filename = g_build_filename(INKSCAPE_UIDIR, "units.xml", NULL);
    load(filename);
    g_free(filename);
}

UnitTable::~UnitTable()
{
    for (UnitMap::iterator iter = _unit_map.begin(); iter != _unit_map.end(); ++iter)
    {
        delete (*iter).second;
    }
}

void UnitTable::addUnit(Unit const &u, bool primary)
{
    _unit_map[u.abbr] = new Unit(u);
    if (primary) {
        _primary_unit[u.type] = u.abbr;
    }
}

Unit UnitTable::getUnit(Glib::ustring const &unit_abbr) const
{
    UnitMap::const_iterator iter = _unit_map.find(unit_abbr);
    if (iter != _unit_map.end()) {
        return *((*iter).second);
    } else {
        return Unit();
    }
}
Unit UnitTable::getUnit(SVGLength::Unit const u) const
{
    Glib::ustring u_str;
    switch(u) {
        case SVGLength::PX:
            u_str = "px"; break;
        case SVGLength::PT:
            u_str = "pt"; break;
        case SVGLength::PC:
            u_str = "pc"; break;
        case SVGLength::MM:
            u_str = "mm"; break;
        case SVGLength::CM:
            u_str = "cm"; break;
        case SVGLength::INCH:
            u_str = "in"; break;
        case SVGLength::FOOT:
            u_str = "ft"; break;
        case SVGLength::EM:
            u_str = "em"; break;
        case SVGLength::EX:
            u_str = "ex"; break;
        case SVGLength::PERCENT:
            u_str = "%"; break;
        default:
            u_str = "";
    }
    
    return getUnit(u_str);
}

Quantity UnitTable::getQuantity(Glib::ustring const& q) const
{
    Glib::MatchInfo match_info;
    
    // Extract value
    double value = 0;
    Glib::RefPtr<Glib::Regex> value_regex = Glib::Regex::create("[-+]*[\\d+]*[\\.,]*[\\d+]*[eE]*[-+]*\\d+");
    if (value_regex->match(q, match_info)) {
        std::istringstream tmp_v(match_info.fetch(0));
        tmp_v >> value;
    }
    
    // Extract unit abbreviation
    Glib::ustring abbr;
    Glib::RefPtr<Glib::Regex> unit_regex = Glib::Regex::create("[A-z%]+");
    if (unit_regex->match(q, match_info)) {
        abbr = match_info.fetch(0);
    }
    
    return Quantity(value, abbr);
}

bool UnitTable::deleteUnit(Unit const &u)
{
    bool deleted = false;
    // Cannot delete the primary unit type since it's
    // used for conversions
    if (u.abbr != _primary_unit[u.type]) {
        UnitMap::iterator iter = _unit_map.find(u.abbr);
        if (iter != _unit_map.end()) {
            delete (*iter).second;
            _unit_map.erase(iter);
            deleted = true;
        }
    }
    return deleted;
}

bool UnitTable::hasUnit(Glib::ustring const &unit) const
{
    UnitMap::const_iterator iter = _unit_map.find(unit);
    return (iter != _unit_map.end());
}

UnitTable::UnitMap UnitTable::units(UnitType type) const
{
    UnitMap submap;
    for (UnitMap::const_iterator iter = _unit_map.begin(); iter != _unit_map.end(); ++iter) {
        if (((*iter).second)->type == type) {
            submap.insert(UnitMap::value_type((*iter).first, new Unit(*((*iter).second))));
        }
    }

    return submap;
}

Glib::ustring UnitTable::primary(UnitType type) const
{
    return _primary_unit[type];
}

bool UnitTable::load(std::string const &filename) {
    UnitParser uparser(this);
    Glib::Markup::ParseContext ctx(uparser);

    try {
        Glib::ustring unitfile = Glib::file_get_contents(filename);
        ctx.parse(unitfile);
        ctx.end_parse();
    } catch (Glib::MarkupError const &e) {
        g_warning("Problem loading units file '%s': %s\n", filename.c_str(), e.what().c_str());
        return false;
    }
    return true;
}

bool UnitTable::save(std::string const &filename) {

    g_warning("UnitTable::save(): not implemented");

    return true;
}

Inkscape::Util::UnitTable unit_table;

void UnitParser::on_start_element(Ctx &ctx, Glib::ustring const &name, AttrMap const &attrs)
{
    if (name == "unit") {
        // reset for next use
        unit.clear();
        primary = false;
        skip = false;

        AttrMap::const_iterator f;
        if ((f = attrs.find("type")) != attrs.end()) {
            Glib::ustring type = f->second;
            if (getTypeMappings().find(type) != getTypeMappings().end()) {
                unit.type = getTypeMappings()[type];
            } else {
                g_warning("Skipping unknown unit type '%s'.\n", type.c_str());
                skip = true;
            }
        }
        if ((f = attrs.find("pri")) != attrs.end()) {
            primary = (f->second[0] == 'y' || f->second[0] == 'Y');
        }
    }
}

void UnitParser::on_text(Ctx &ctx, Glib::ustring const &text)
{
    Glib::ustring element = ctx.get_element();
    if (element == "name") {
        unit.name = text;
    } else if (element == "plural") {
        unit.name_plural = text;
    } else if (element == "abbr") {
        unit.abbr = text;
    } else if (element == "factor") {
        // TODO make sure we use the right conversion
        unit.factor = g_ascii_strtod(text.c_str(), NULL);
    } else if (element == "description") {
        unit.description = text;
    }
}

void UnitParser::on_end_element(Ctx &ctx, Glib::ustring const &name)
{
    if (name == "unit" && !skip) {
        tbl->addUnit(unit, primary);
    }
}

Quantity::Quantity(double q, const Unit &u)
{
    unit = new Unit(u);
    quantity = q;
}
Quantity::Quantity(double q, const Glib::ustring u)
{
    unit = new Unit(unit_table.getUnit(u));
    quantity = q;
}

bool Quantity::compatibleWith(const Unit &u) const
{
    return unit->compatibleWith(u);
}
bool Quantity::compatibleWith(const Glib::ustring u) const
{
    return compatibleWith(unit_table.getUnit(u));
}

double Quantity::value(const Unit &u) const
{
    return convert(quantity, *unit, u);
}
double Quantity::value(const Glib::ustring u) const
{
    return value(unit_table.getUnit(u));
}

Glib::ustring Quantity::string(const Unit &u) const {
    return Glib::ustring::format(std::fixed, std::setprecision(2), value(u)) + " " + unit->abbr;
}
Glib::ustring Quantity::string(const Glib::ustring u) const {
    return string(unit_table.getUnit(u));
}
Glib::ustring Quantity::string() const {
    return string(*unit);
}

double Quantity::convert(const double from_dist, const Unit &from, const Unit &to)
{
    // Percentage
    if (to.type == UNIT_TYPE_DIMENSIONLESS) {
        return from_dist * to.factor;
    }
    
    // Incompatible units
    if (from.type != to.type) {
        return -1;
    }
    
    // Compatible units
    return from_dist * from.factor / to.factor;
}
double Quantity::convert(const double from_dist, const Glib::ustring from, const Unit &to)
{
    return convert(from_dist, unit_table.getUnit(from), to);
}
double Quantity::convert(const double from_dist, const Unit &from, const Glib::ustring to)
{
    return convert(from_dist, from, unit_table.getUnit(to));
}
double Quantity::convert(const double from_dist, const Glib::ustring from, const Glib::ustring to)
{
    return convert(from_dist, unit_table.getUnit(from), unit_table.getUnit(to));
}

bool operator< (const Quantity &ql, const Quantity &qr)
{
    if (ql.unit->type != qr.unit->type) {
        g_warning("Incompatible units");
        return false;
    }
    return ql.quantity < qr.value(*ql.unit);
}
bool operator> (const Quantity &ql, const Quantity &qr)
{
    if (ql.unit->type != qr.unit->type) {
        g_warning("Incompatible units");
        return false;
    }
    return ql.quantity > qr.value(*ql.unit);
}
bool operator!= (const Quantity &q1, const Quantity &q2)
{
    return (*q1.unit != *q2.unit) || (q1.quantity != q2.quantity);
}

} // namespace Util
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

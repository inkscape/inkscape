#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <cmath>
#include <cerrno>
#include <glib.h>
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
}

void Unit::clear()
{
    *this = Unit();
}

int Unit::defaultDigits() const {
    int factor_digits = int(log10(factor));
    if (factor_digits < 0) {
        g_warning("factor = %f, factor_digits = %d", factor, factor_digits);
        g_warning("factor_digits < 0 - returning 0");
        factor_digits = 0;
    }
    return factor_digits;
}

UnitTable::UnitTable()
{
    gchar *filename = g_build_filename(INKSCAPE_UIDIR, "units.xml", NULL);
    load(filename);
    g_free(filename);
}

UnitTable::~UnitTable() {
    for (UnitMap::iterator iter = _unit_map.begin(); iter != _unit_map.end(); ++iter)
    {
        delete (*iter).second;
    }
}

void UnitTable::addUnit(Unit const &u, bool primary) {
    _unit_map[u.abbr] = new Unit(u);
    if (primary) {
        _primary_unit[u.type] = u.abbr;
    }
}

Unit UnitTable::getUnit(Glib::ustring const &unit_abbr) const {
    UnitMap::const_iterator iter = _unit_map.find(unit_abbr);
    if (iter != _unit_map.end()) {
        return *((*iter).second);
    } else {
        return Unit();
    }
}

bool UnitTable::deleteUnit(Unit const &u) {
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

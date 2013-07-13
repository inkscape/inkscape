#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <cmath>
#include <cerrno>
#include <glib.h>

#include "io/simple-sax.h"
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

class UnitsSAXHandler : public Inkscape::IO::FlatSaxHandler
{
public:
    UnitsSAXHandler(UnitTable *table);
    virtual ~UnitsSAXHandler() {}

    virtual void _startElement(xmlChar const *name, xmlChar const **attrs);
    virtual void _endElement(xmlChar const *name);

    UnitTable *tbl;
    bool primary;
    bool skip;
    Unit unit;
};

UnitsSAXHandler::UnitsSAXHandler(UnitTable *table) :
    FlatSaxHandler(),
    tbl(table),
    primary(0),
    skip(0),
    unit()
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
    // if we swich to the xml file, don't forget to force locale to 'C'
    //    load("share/ui/units.xml");  // <-- Buggy
    gchar *filename = g_build_filename(INKSCAPE_UIDIR, "units.txt", NULL);
    loadText(filename);
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

bool UnitTable::loadText(Glib::ustring const &filename)
{
    char buf[BUFSIZE] = {0};

    // Open file for reading
    FILE * f = fopen(filename.c_str(), "r");
    if (f == NULL) {
        g_warning("Could not open units file '%s': %s\n",
                  filename.c_str(), strerror(errno));
        g_warning("* INKSCAPE_DATADIR is:  '%s'\n", INKSCAPE_DATADIR);
        g_warning("* INKSCAPE_UIDIR is:  '%s'\n", INKSCAPE_UIDIR);
        return false;
    }

    /** @todo fix this to use C++ means and explicit locale to avoid need to change. */
    // bypass current locale in order to make
    // sscanf read floats with '.' as a separator
    // set locale to 'C' and keep old locale
    char *old_locale = g_strdup(setlocale(LC_NUMERIC, NULL));
    setlocale (LC_NUMERIC, "C");

    while (fgets(buf, BUFSIZE, f) != NULL) {
        char name[BUFSIZE] = {0};
        char plural[BUFSIZE] = {0};
        char abbr[BUFSIZE] = {0};
        char type[BUFSIZE] = {0};
        double factor = 0.0;
        char primary[BUFSIZE] = {0};

        int nchars = 0;
        // locale is set to C, scanning %lf should work _everywhere_
        /** @todo address %15n, which causes a warning: */
        if (sscanf(buf, "%15s %15s %15s %15s %8lf %1s %15n",
                   name, plural, abbr, type, &factor, primary, &nchars) != 6)
        {
            // Skip the line - doesn't appear to be valid
            continue;
        }

        g_assert(nchars < BUFSIZE);

        char *desc = buf;
        desc += nchars;  // buf is now only the description

        // insert into _unit_map
        if (getTypeMappings().find(type) == getTypeMappings().end())
        {
            g_warning("Skipping unknown unit type '%s' for %s.\n", type, name);
            continue;
        }
        UnitType utype = getTypeMappings()[type];

        Unit u(utype, factor, name, plural, abbr, desc);

        // if primary is 'Y', list this unit as a primary
        addUnit(u, (primary[0]=='Y' || primary[0]=='y'));
    }

    // set back the saved locale
    setlocale (LC_NUMERIC, old_locale);
    g_free (old_locale);

    // close file
    if (fclose(f) != 0) {
        g_warning("Error closing units file '%s':  %s\n", filename.c_str(), strerror(errno));
        return false;
    }

    return true;
}

bool UnitTable::load(Glib::ustring const &filename) {
    UnitsSAXHandler handler(this);

    int result = handler.parseFile( filename.c_str() );
    if ( result != 0 ) {
        // perhaps
        g_warning("Problem loading units file '%s':  %d\n", filename.c_str(), result);
        return false;
    }

    return true;
}

bool UnitTable::save(Glib::ustring const &filename) {

    // open file for writing
    FILE *f = fopen(filename.c_str(), "w");
    if (f == NULL) {
        g_warning("Could not open units file '%s': %s\n", filename.c_str(), strerror(errno));
        return false;
    }

    // write out header
    // foreach item in _unit_map, sorted alphabetically by type and then unit name
    //    sprintf a line
    //      name
    //      name_plural
    //      abbr
    //      type
    //      factor
    //      PRI - if listed in primary unit table, 'Y', else 'N'
    //      description
    //    write line to the file

    // close file
    if (fclose(f) != 0) {
        g_warning("Error closing units file '%s':  %s\n", filename.c_str(), strerror(errno));
        return false;
    }

    return true;
}


void UnitsSAXHandler::_startElement(xmlChar const *name, xmlChar const **attrs)
{
    if (streq("unit", (char const *)name)) {
        // reset for next use
        unit.clear();
        primary = false;
        skip = false;

        for ( int i = 0; attrs[i]; i += 2 ) {
            char const *const key = (char const *)attrs[i];
            if (streq("type", key)) {
                char const *type = (char const*)attrs[i+1];
                if (getTypeMappings().find(type) != getTypeMappings().end())
                {
                    unit.type = getTypeMappings()[type];
                } else {
                    g_warning("Skipping unknown unit type '%s' for %s.\n", type, name);
                    skip = true;
                }
            } else if (streq("pri", key)) {
                primary = attrs[i+1][0] == 'y' || attrs[i+1][0] == 'Y';
            }
        }
    }
}

void UnitsSAXHandler::_endElement(xmlChar const *xname)
{
    char const *const name = (char const *) xname;
    if (streq("name", name)) {
        unit.name = data;
    } else if (streq("plural", name)) {
        unit.name_plural = data;
    } else if (streq("abbr", name)) {
        unit.abbr = data;
    } else if (streq("factor", name)) {
        // TODO make sure we use the right conversion
        unit.factor = atol(data.c_str());
    } else if (streq("description", name)) {
        unit.description = data;
    } else if (streq("unit", name)) {
        if (!skip) {
            tbl->addUnit(unit, primary);
        }
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

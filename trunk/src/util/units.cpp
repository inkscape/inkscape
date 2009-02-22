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

namespace Inkscape {
namespace Util {

class UnitsSAXHandler : public Inkscape::IO::FlatSaxHandler
{
public:
    UnitsSAXHandler(UnitTable *table) : FlatSaxHandler(), tbl(table) {}
    virtual ~UnitsSAXHandler() {}

    virtual void _startElement(xmlChar const *name, xmlChar const **attrs);
    virtual void _endElement(xmlChar const *name);

    UnitTable *tbl;
    bool primary;
    bool skip;
    Unit unit;
};


#define BUFSIZE (255)

/**
 * Returns the suggested precision to use for displaying numbers
 * of this unit.
 */
int Unit::defaultDigits() const {
    int factor_digits = int(log10(factor));
    if (factor_digits < 0) {
        g_warning("factor = %f, factor_digits = %d", factor, factor_digits);
        g_warning("factor_digits < 0 - returning 0");
        return 0;
    } else {
        return factor_digits;
    }
}

/**
 * Initializes the unit tables and identifies the primary unit types.
 *
 * The primary unit's conversion factor is required to be 1.00
 */
UnitTable::UnitTable() 
{
    // if we swich to the xml file, don't forget to force locale to 'C'
    //    load("share/ui/units.xml");  // <-- Buggy 
    gchar *filename = g_build_filename(INKSCAPE_UIDIR, "units.txt", NULL);
    loadText(filename);
    g_free(filename);
}

UnitTable::~UnitTable() {
    UnitMap::iterator iter = _unit_map.begin();
    while (iter != _unit_map.end()) {
	delete (*iter).second;
	++iter;
    }
}

/** Add a new unit to the table */
void
UnitTable::addUnit(Unit const &u, bool primary) {
    _unit_map[u.abbr] = new Unit(u);
    if (primary) {
	_primary_unit[u.type] = u.abbr;
    }
}

/** Retrieve a given unit based on its string identifier */
Unit
UnitTable::getUnit(Glib::ustring const &unit_abbr) const {
    UnitMap::const_iterator iter = _unit_map.find(unit_abbr);
    if (iter != _unit_map.end()) {
	return *((*iter).second);
    } else {
	return Unit();
    }
}

/** Remove a unit definition from the given unit type table */
bool 
UnitTable::deleteUnit(Unit const &u) {
    if (u.abbr == _primary_unit[u.type]) {
	// Cannot delete the primary unit type since it's
	// used for conversions
	return false;
    }
    UnitMap::iterator iter = _unit_map.find(u.abbr);
    if (iter != _unit_map.end()) {
	delete (*iter).second;
	_unit_map.erase(iter);
	return true;
    } else {
	return false;
    }
}

/** Returns true if the given string 'name' is a valid unit in the table */
bool
UnitTable::hasUnit(Glib::ustring const &unit) const {
    UnitMap::const_iterator iter = _unit_map.find(unit);
    return (iter != _unit_map.end());
}

/** Provides an iteratable list of items in the given unit table */
UnitTable::UnitMap 
UnitTable::units(UnitType type) const
{
    UnitMap submap;
    for (UnitMap::const_iterator iter = _unit_map.begin();
         iter != _unit_map.end(); ++iter) {
	if (((*iter).second)->type == type) {
	    submap.insert(UnitMap::value_type((*iter).first, new Unit(*((*iter).second))));
	}
    }

    return submap;
}

/** Returns the default unit abbr for the given type */
Glib::ustring
UnitTable::primary(UnitType type) const {
    return _primary_unit[type];
}

/** Merges the contents of the given file into the UnitTable,
    possibly overwriting existing unit definitions.  This loads
    from a text file */
bool
UnitTable::loadText(Glib::ustring const &filename) {
    char buf[BUFSIZE];

    // Open file for reading
    FILE * f = fopen(filename.c_str(), "r");
    if (f == NULL) {
	g_warning("Could not open units file '%s': %s\n", 
		filename.c_str(), strerror(errno));
        g_warning("* INKSCAPE_DATADIR is:  '%s'\n", INKSCAPE_DATADIR);
        g_warning("* INKSCAPE_UIDIR is:  '%s'\n", INKSCAPE_UIDIR);
	return false;
    }

    // bypass current locale in order to make
    // sscanf read floats with '.' as a separator
    // set locate to 'C' and keep old locale
    char *old_locale;
    old_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
    setlocale (LC_NUMERIC, "C");

    while (fgets(buf, BUFSIZE, f) != NULL) {
	char name[BUFSIZE];
	char plural[BUFSIZE];
	char abbr[BUFSIZE];
	char type[BUFSIZE];
	double factor;
	char primary[BUFSIZE];

	int nchars = 0;
    // locate is set to C, scanning %lf should work _everywhere_
	if (sscanf(buf, "%s %s %s %s %lf %s %n", 
		   name, plural, abbr, type, &factor, 
		   primary, &nchars) != 6) {
	    // Skip the line - doesn't appear to be valid
	    continue;
	}
	g_assert(nchars < BUFSIZE);

	char *desc = buf;
	desc += nchars;  // buf is now only the description

	// insert into _unit_map
	Unit u;
	u.name = name;
	u.name_plural = plural;
	u.abbr = abbr;
	u.description = desc;
	u.factor = factor;

	if (streq(type, "DIMENSIONLESS")) {
	    u.type = UNIT_TYPE_DIMENSIONLESS;
	} else if (streq(type, "LINEAR")) {
	    u.type = UNIT_TYPE_LINEAR;
	} else if (streq(type, "RADIAL")) {
	    u.type = UNIT_TYPE_RADIAL;
	} else if (streq(type, "FONT_HEIGHT")) {
	    u.type = UNIT_TYPE_FONT_HEIGHT;
	} else {
	    g_warning("Skipping unknown unit type '%s' for %s.\n", 
		    type, name);
	    continue;
	}

	// if primary is 'Y', list this unit as a primary
	addUnit(u, (primary[0]=='Y' || primary[0]=='y'));

    }

    // set back the saved locale
    setlocale (LC_NUMERIC, old_locale);
    g_free (old_locale);

    // close file
    if (fclose(f) != 0) {
	g_warning("Error closing units file '%s':  %s\n",
		filename.c_str(), strerror(errno));
	return false;
    }

    return true;
}

bool
UnitTable::load(Glib::ustring const &filename) {
    UnitsSAXHandler handler(this);

    int result = handler.parseFile( filename.c_str() );
    if ( result != 0 ) {
        // perhaps
	g_warning("Problem loading units file '%s':  %d\n", 
		  filename.c_str(), result);
	return false;
    }

    return true;
}

/** Saves the current UnitTable to the given file. */
bool
UnitTable::save(Glib::ustring const &filename) {

    // open file for writing
    FILE *f = fopen(filename.c_str(), "w");
    if (f == NULL) {
	g_warning("Could not open units file '%s': %s\n", 
		  filename.c_str(), strerror(errno));
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
	g_warning("Error closing units file '%s':  %s\n",
		  filename.c_str(), strerror(errno));
	return false;
    }

    return true;
}


void UnitsSAXHandler::_startElement(xmlChar const *name, xmlChar const **attrs)
{
    if (streq("unit", (char const *)name)) {
        // reset for next use
        unit.name.clear();
        unit.name_plural.clear();
        unit.abbr.clear();
        unit.description.clear();
        unit.type = UNIT_TYPE_DIMENSIONLESS;
        unit.factor = 1.0;
        primary = false;
        skip = false;

        for ( int i = 0; attrs[i]; i += 2 ) {
            char const *const key = (char const *)attrs[i];
            if (streq("type", key)) {
                char const *type = (char const*)attrs[i+1];
                if (streq(type, "DIMENSIONLESS")) {
                    unit.type = UNIT_TYPE_DIMENSIONLESS;
                } else if (streq(type, "LINEAR")) {
                    unit.type = UNIT_TYPE_LINEAR;
                } else if (streq(type, "RADIAL")) {
                    unit.type = UNIT_TYPE_RADIAL;
                } else if (streq(type, "FONT_HEIGHT")) {
                    unit.type = UNIT_TYPE_FONT_HEIGHT;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

/*
 * Author:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2006 Johan Engelen, johan@shouraizou.nl
 * Copyright (C) 2004 Author
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glibmm/i18n.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include "config.h"
#include "path-prefix.h"
#include "dependency.h"
#include "db.h"
#include "extension.h"

namespace Inkscape {
namespace Extension {

// These strings are for XML attribute comparisons and should not be translated
gchar const * Dependency::_type_str[] = {
    "executable",
    "file",
    "extension",
};

// These strings are for XML attribute comparisons and should not be translated
gchar const * Dependency::_location_str[] = {
    "path",
    "extensions",
    "absolute",
};

/**
    \brief   Create a dependency using an XML definition
    \param   in_repr   XML definition of the dependency

    This function mostly looks for the 'location' and 'type' attributes
    and turns them into the enums of the same name.  This makes things
    a little bit easier to use later.  Also, a pointer to the core
    content is pulled out -- also to make things easier.
*/
Dependency::Dependency (Inkscape::XML::Node * in_repr)
{
    _type = TYPE_FILE;
    _location = LOCATION_PATH;
    _repr = in_repr;
    _string = NULL;
    _description = NULL;

    Inkscape::GC::anchor(_repr);

    if (const gchar * location = _repr->attribute("location")) {
        for (int i = 0; i < LOCATION_CNT && location != NULL; i++) {
            if (!strcmp(location, _location_str[i])) {
                _location = (location_t)i;
                break;
            }
        }
    } else if (const gchar * location = _repr->attribute("reldir")) {
        for (int i = 0; i < LOCATION_CNT && location != NULL; i++) {
            if (!strcmp(location, _location_str[i])) {
                _location = (location_t)i;
                break;
            }
        }
    }
    
    const gchar * type = _repr->attribute("type");
    for (int i = 0; i < TYPE_CNT && type != NULL; i++) {
        if (!strcmp(type, _type_str[i])) {
            _type = (type_t)i;
            break;
        }
    }

    _string = _repr->firstChild()->content();

    _description = _repr->attribute("description");
    if (_description == NULL)
        _description = _repr->attribute("_description");

    return;
}

/**
    \brief   This depenency is not longer needed

    Unreference the XML structure.
*/
Dependency::~Dependency (void)
{
    Inkscape::GC::release(_repr);
}

/**
    \brief   Check if the dependency passes.
    \return  Whether or not the dependency passes.

    This function depends largely on all of the enums.  The first level
    that is evaluted is the \c _type.

    If the type is \c TYPE_EXTENSION then the id for the extension is
    looked up in the database.  If the extension is found, and it is
    not deactivated, the dependency passes.

    If the type is \c TYPE_EXECUTABLE or \c TYPE_FILE things are getting
    even more interesting because now the \c _location variable is also
    taken into account.  First, the difference between the two is that
    the file test for \c TYPE_EXECUTABLE also tests to make sure the
    file is executable, besides checking that it exists.

    If the \c _location is \c LOCATION_EXTENSIONS then the \c INKSCAPE_EXTENSIONDIR
    is put on the front of the string with \c build_filename.  Then the
    appopriate filetest is run.

    If the \c _location is \c LOCATION_ABSOLUTE then the file test is
    run directly on the string.

    If the \c _location is \c LOCATION_PATH or not specified then the
    path is used to find the file.  Each entry in the path is stepped
    through, attached to the string, and then tested.  If the file is
    found then a TRUE is returned.  If we get all the way through the
    path then a FALSE is returned, the command could not be found.
*/
bool Dependency::check (void) const
{
    // std::cout << "Checking: " << *this << std::endl;

    if (_string == NULL) return FALSE;

    switch (_type) {
        case TYPE_EXTENSION: {
            Extension * myext = db.get(_string);
            if (myext == NULL) return FALSE;
            if (myext->deactivated()) return FALSE;
            break;
        }
        case TYPE_EXECUTABLE:
        case TYPE_FILE: {
            Glib::FileTest filetest = Glib::FILE_TEST_EXISTS;
            if (_type == TYPE_EXECUTABLE) {
                filetest |= Glib::FILE_TEST_IS_EXECUTABLE;
            }

            Glib::ustring location(_string);
            switch (_location) {
                case LOCATION_EXTENSIONS: {
                    for (unsigned int i=0; i<Inkscape::Extension::Extension::search_path.size(); i++) {
                        std::string temploc = Glib::build_filename(Inkscape::Extension::Extension::search_path[i], location);
                        if (Glib::file_test(temploc, filetest)) {
                            location = temploc;
                            break;
                        }
                    }
                } /* PASS THROUGH!!! */
                case LOCATION_ABSOLUTE: {
                    if (!Glib::file_test(location, filetest)) {
                        // std::cout << "Failing on location: " << location << std::endl;
                        return FALSE;
                    }
                    break;
                }
                /* The default case is to look in the path */
                case LOCATION_PATH:
                default: {
                    gchar * path = g_strdup(g_getenv("PATH"));

                    if (path == NULL) {
                        /* There is no `PATH' in the environment.
                           The default search path is the current directory */
                        path = g_strdup(G_SEARCHPATH_SEPARATOR_S);
                    }

                    gchar * orig_path = path;

                    for (; path != NULL;) {
                        gchar * local_path; // to have the path after detection of the separator
                        Glib::ustring final_name;

                        local_path = path;
                        path = g_utf8_strchr(path, -1, G_SEARCHPATH_SEPARATOR);
                        /* Not sure whether this is UTF8 happy, but it would seem
                           like it considering that I'm searching (and finding)
                           the ':' character */
                        if (path != NULL) {
                            path[0] = '\0';
                            path++;
                        }

                        if (*local_path == '\0') {
                            final_name = _string;
                        } else {
                            final_name = Glib::build_filename(local_path, _string);
                        }

                        if (Glib::file_test(final_name, filetest)) {
                            g_free(orig_path);
                            return TRUE;
                        }

                        // give it a 2nd try with ".exe" added
                        Glib::ustring final_name_exe = final_name + ".exe";
                        if (Glib::file_test(final_name_exe, filetest)) {
                            g_free(orig_path);
                            return TRUE;
                        }

                        // and a 3rd try with ".cmd" added (mainly for UniConvertor)
                        Glib::ustring final_name_cmd = final_name + ".cmd";
                        if (Glib::file_test(final_name_cmd, filetest)) {
                            g_free(orig_path);
                            return TRUE;
                        }
                    }

                    g_free(orig_path);
                    return FALSE; /* Reverse logic in this one */
                }
            } /* switch _location */
            break;
        } /* TYPE_FILE, TYPE_EXECUTABLE */
        default:
            return FALSE;
    } /* switch _type */

    return TRUE;
}

/**
    \brief   Accessor to the name attribute.
    \return  A string containing the name of the dependency.

    Returns the name of the dependency as found in the configuration file.
 
*/
const gchar* Dependency::get_name()
{
    return _string;
}

/**
    \brief   Print out a dependency to a string.
*/
std::ostream &
operator<< (std::ostream &out_file, const Dependency & in_dep)
{
    out_file << _("Dependency:") << '\n';
    out_file << _("  type: ") << _(in_dep._type_str[in_dep._type]) << '\n';
    out_file << _("  location: ") << _(in_dep._location_str[in_dep._location]) << '\n';
    out_file << _("  string: ") << in_dep._string << '\n';

    if (in_dep._description != NULL) {
        out_file << _("  description: ") << _(in_dep._description) << '\n';
    }

    out_file << std::flush;

    return out_file;
}

} }  /* namespace Inkscape, Extension */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

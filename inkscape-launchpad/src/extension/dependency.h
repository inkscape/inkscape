/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_EXTENSION_DEPENDENCY_H__
#define INKSCAPE_EXTENSION_DEPENDENCY_H__

#include <glibmm/ustring.h>
#include "xml/repr.h"

namespace Inkscape {
namespace Extension {

/** \brief  A class to represent a dependency for an extension.  There
            are different things that can be done in a dependency, and
            this class takes care of all of them. */
class Dependency {
    /** \brief  The XML representation of the dependency. */
    Inkscape::XML::Node * _repr;
    /** \brief  The string that is in the XML tags pulled out. */
    const gchar * _string;
    /** \brief  The description of the dependency for the users. */
    const gchar * _description;

    /** \brief  All the possible types of dependencies. */
    enum type_t {
        TYPE_EXECUTABLE, /**< Look for an executable */
        TYPE_FILE,       /**< Look to make sure a file exists */
        TYPE_EXTENSION,  /**< Make sure a specific extension is loaded and functional */
        TYPE_CNT         /**< Number of types */
    };
    /** \brief  Storing the type of this particular dependency. */
    type_t _type;

    /** \brief  All of the possible locations to look for the dependency. */
    enum location_t {
        LOCATION_PATH,       /**< Look in the PATH for this depdendency */
        LOCATION_EXTENSIONS, /**< Look in the extensions directory */
        LOCATION_ABSOLUTE,   /**< This dependency is already defined in absolute terms */
        LOCATION_CNT         /**< Number of locations to look */
    };
    /** \brief  The location to look for this particular dependency. */
    location_t _location;

    /** \brief  Strings to reperesent the different enum values in
                \c type_t in the XML */
    static gchar const * _type_str[TYPE_CNT]; 
    /** \brief  Strings to reperesent the different enum values in
                \c location_t in the XML */
    static gchar const * _location_str[LOCATION_CNT]; 

public:
    Dependency  (Inkscape::XML::Node * in_repr);
    virtual ~Dependency (void);
    bool check  (void) const;
    const gchar* get_name();
    Glib::ustring &get_help (void) const;
    Glib::ustring &get_link (void) const;

    friend std::ostream & operator<< (std::ostream &out_file, const Dependency & in_dep);
}; /* class Dependency */

std::ostream & operator<< (std::ostream &out_file, const Dependency & in_dep);

} }  /* namespace Extension, Inkscape */

#endif /* INKSCAPE_EXTENSION_DEPENDENCY_H__ */

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

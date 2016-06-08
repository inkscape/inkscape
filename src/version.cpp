/*
 * Versions
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2003 MenTaLguY
 * Copyright (C) 2012 Kris De Gussem
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <sstream>
#include "version.h"

bool sp_version_from_string(const char *string, Inkscape::Version *version)
{
    if (!string) {
        return false;
    }

    try
    {
        std::stringstream ss;

        // Throw exception if error.
        ss.exceptions(std::ios::failbit | std::ios::badbit);
        ss << string;
        ss >> version->_major;
        char tmp=0;
        ss >> tmp;
        ss >>version->_minor;

        // Don't throw exception if failbit gets set (empty string OK).
        ss.exceptions(std::ios::goodbit);
        getline(ss, version->_tail);
        return true;
    }
    catch(...)
    {
        version->_major = 0;
        version->_minor = 0;
        version->_tail.clear();
        return false;
    }
}

char *sp_version_to_string(Inkscape::Version version)
{
    return g_strdup_printf("%u.%u%s", version._major, version._minor, version._tail.c_str());
}

bool sp_version_inside_range(Inkscape::Version version,
                             unsigned major_min, unsigned minor_min,
                             unsigned major_max, unsigned minor_max)
{
    if ( version._major < major_min || version._major > major_max ) {
        return false;
    } else if ( version._major == major_min &&
                version._minor <= minor_min )
    {
        return false;
    } else if ( version._major == major_max &&
                version._minor >= minor_max )
    {
        return false;
    } else {
        return true;
    }
}

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

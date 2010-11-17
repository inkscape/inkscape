/*
 * Stub out functions when building tests
 *
 * Authors:
 *   Kees Cook <kees@outflux.net>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "svg/test-stubs.h"
#include <map>
#include <string>

std::map<std::string,long long int> int_prefs;

void
prefs_set_int_attribute(gchar const *path, gchar const *attr, long long int val)
{
    int_prefs[std::string(path) + '/' + std::string(attr)] = val;
}

long long int
prefs_get_int_attribute(gchar const *path, gchar const *attr, long long int def)
{
    std::map<std::string,long long int>::const_iterator it=int_prefs.find(std::string(path) + '/' + std::string(attr));
    long long int ret = it==int_prefs.end() ? def : it->second;
    return ret;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

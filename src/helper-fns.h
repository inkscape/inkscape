#ifndef SEEN_HELPER_FNS_H
#define SEEN_HELPER_FNS_H
/** \file
 * 
 * Some helper functions
 * 
 * Authors:
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 * 
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <sstream>

static double
helperfns_read_number(gchar const *value) {
    if (!value) return 0;
    char *end;
    double ret = g_ascii_strtod(value, &end);
    if (*end) {
        g_warning("Unable to convert \"%s\" to number", value);
        // We could leave this out, too. If strtod can't convert
        // anything, it will return zero.
        ret = 0;
    }
    return ret;
}

static bool helperfns_read_bool(gchar const *value, bool default_value){
    if (!value) return default_value;
    switch(value[0]){
        case 't':
            if (strncmp(value, "true", 4) == 0) return true;
            break;
        case 'f':
            if (strncmp(value, "false", 5) == 0) return false;
            break;
    }
    return default_value;
}

static std::vector<gdouble> helperfns_read_vector(const gchar* value, int size){
        std::vector<gdouble> v(size, (gdouble) 0);
        std::istringstream is(value);
        for(int i = 0; i < size && (is >> v[i]); i++);
        return v;
}

#endif /* !SEEN_HELPER_FNS_H */

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

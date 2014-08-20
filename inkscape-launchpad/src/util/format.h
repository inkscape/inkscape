/*
 * Inkscape::Util::format - g_strdup_printf wrapper producing shared strings
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2006 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_FORMAT_H
#define SEEN_INKSCAPE_UTIL_FORMAT_H

#include <stdarg.h>
#include <glib.h>
#include "util/share.h"

namespace Inkscape {

namespace Util {

inline ptr_shared<char> vformat(char const *format, va_list args) {
    char *temp=g_strdup_vprintf(format, args);
    ptr_shared<char> result=share_string(temp);
    g_free(temp);
    return result;
}

       // needed since G_GNUC_PRINTF can only be used on a declaration
       ptr_shared<char> format(char const *format, ...) G_GNUC_PRINTF(1,2);
inline ptr_shared<char> format(char const *format, ...) {
    va_list args;

    va_start(args, format);
    ptr_shared<char> result=vformat(format, args);
    va_end(args);

    return result;
}

}

}

#endif
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

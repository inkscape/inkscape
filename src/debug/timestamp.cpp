/*
 * Inkscape::Debug::SimpleEvent - trivial implementation of Debug::Event
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2007 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <glib.h>
#include <glibmm/ustring.h>
#include "debug/simple-event.h"
#include "timestamp.h"

namespace Inkscape {

namespace Debug {

Util::ptr_shared<char> timestamp() {
    Util::ptr_shared<char> result;
    GTimeVal timestamp;
    g_get_current_time(&timestamp);
    gchar *value = g_strdup_printf( "%d.%06d", static_cast<gint>(timestamp.tv_sec), static_cast<gint>(timestamp.tv_usec) );
    result = Util::share_string(value);
    g_free(value);
    return result;
}

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

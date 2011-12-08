/*
 * Inkscape::Util::ptr_shared<T> - like T const *, but stronger
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2006 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "util/share.h"
#include <glib.h>

namespace Inkscape {
namespace Util {

ptr_shared<char> share_string(char const *string) {
    g_return_val_if_fail(string != NULL, share_unsafe<char>(NULL));
    return share_string(string, std::strlen(string));
}

ptr_shared<char> share_string(char const *string, std::size_t length) {
    g_return_val_if_fail(string != NULL, share_unsafe<char>(NULL));
    char *new_string=new (GC::ATOMIC) char[length+1];
    std::memcpy(new_string, string, length);
    new_string[length] = 0;
    return share_unsafe(new_string);
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

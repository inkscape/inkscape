/*
 * Inkscape::Util::SharedCStringPtr - shared and immutable strings
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_SHARED_C_STRING_PTR_H
#define SEEN_INKSCAPE_UTIL_SHARED_C_STRING_PTR_H

#include <sys/types.h>
#include <glib/gtypes.h>

namespace Inkscape {

namespace Util {

class SharedCStringPtr {
public:
    SharedCStringPtr() : _str(NULL) {}

    operator char const *() const { return cString(); }

    char operator[](size_t i) const { return cString()[i]; }

    char const *cString() const { return _str; }

    static SharedCStringPtr coerce(char const *s) { return SharedCStringPtr(s); }
    static SharedCStringPtr copy(char const *s);
    static SharedCStringPtr copy(char const *s, size_t len);

    operator bool() const { return _str; }

    bool operator==(SharedCStringPtr const &other) { return _str == other._str; }
    bool operator!=(SharedCStringPtr const &other) { return _str != other._str; }

private:
    SharedCStringPtr(char const *s) : _str(s) {}

    char const *_str;
};

inline bool operator==(SharedCStringPtr const &ss, char const *s) {
    return ss.cString() == s;
}

inline bool operator==(char const *s, SharedCStringPtr const &ss) {
    return operator==(ss, s);
}

inline bool operator!=(SharedCStringPtr const &ss, char const *s) {
    return !operator==(ss, s);
}

inline bool operator!=(char const *s, SharedCStringPtr const &ss) {
    return !operator==(s, ss);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

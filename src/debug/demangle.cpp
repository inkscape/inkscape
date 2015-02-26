/*
 * Inkscape::Debug::demangle - demangle C++ symbol names
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2006 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <stdio.h>
#include <string.h>
#include <map>
#include "debug/demangle.h"
#include "util/format.h"
#include "inkgc/gc-alloc.h"

namespace Inkscape {

namespace Debug {

namespace {

char const *demangle_helper(char const *name) {
    char buffer[1024];
    char const *result;
    FILE *stream=popen(Util::format("c++filt %s", name), "r");
    if (fgets(buffer, sizeof(buffer), stream)) {
        size_t len=strlen(buffer);
        if ( buffer[len-1] == '\n' ) {
            buffer[len-1] = '\000';
        }
        result = strdup(buffer);
    } else {
        result = name;
    }
    pclose(stream);
    return result;
}

struct string_less_than {
    bool operator()(char const *a, char const *b) const {
        return ( strcmp(a, b) < 0 );
    }
};

typedef std::map<char const *, char const *, string_less_than> MangleCache;
MangleCache mangle_cache;

}

Util::ptr_shared<char> demangle(char const *name) {
    MangleCache::iterator found=mangle_cache.find(name);

    char const *result;
    if ( found != mangle_cache.end() ) {
        result = (*found).second;
    } else {
        result = demangle_helper(name);
        mangle_cache[name] = result;
    }

    return Util::share_unsafe(result);
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

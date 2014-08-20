/*
 * Inkscape::Debug::Event - event for debug tracing
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2005 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_DEBUG_EVENT_H
#define SEEN_INKSCAPE_DEBUG_EVENT_H

#include <utility>
#include "util/share.h"

namespace Inkscape {

namespace Debug {

class Event {
public:
    virtual ~Event() {}

    enum Category {
        CORE=0,
        XML,
        SPOBJECT,
        DOCUMENT,
        REFCOUNT,
        EXTENSION,
        FINALIZERS,
        INTERACTION,
        CONFIGURATION,
        OTHER
    };
    enum { N_CATEGORIES=OTHER+1 };

    struct PropertyPair {
    public:
        PropertyPair() {}
        PropertyPair(Util::ptr_shared<char> n, Util::ptr_shared<char> v)
        : name(n), value(v) {}
        PropertyPair(char const *n, Util::ptr_shared<char> v)
        : name(Util::share_string(n)), value(v) {}
        PropertyPair(Util::ptr_shared<char> n, char const *v)
        : name(n), value(Util::share_string(v)) {}
        PropertyPair(char const *n, char const *v)
        : name(Util::share_string(n)),
          value(Util::share_string(v)) {}

        Util::ptr_shared<char> name;
        Util::ptr_shared<char> value;
    };

    static Category category() { return OTHER; }

    virtual Util::ptr_shared<char> name() const=0;
    virtual unsigned propertyCount() const=0;
    virtual PropertyPair property(unsigned property) const=0;

    virtual void generateChildEvents() const=0;
};

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

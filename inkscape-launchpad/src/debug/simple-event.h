/*
 * Inkscape::Debug::SimpleEvent - trivial implementation of Debug::Event
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2005 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_DEBUG_SIMPLE_EVENT_H
#define SEEN_INKSCAPE_DEBUG_SIMPLE_EVENT_H

#include <stdarg.h>
#include <vector>
#include <glib.h> // g_assert()

#include "inkgc/gc-alloc.h"
#include "debug/event.h"

namespace Inkscape {

namespace Debug {

template <Event::Category C=Event::OTHER>
class SimpleEvent : public Event {
public:
    explicit SimpleEvent(Util::ptr_shared<char> name) : _name(name) {}
    explicit SimpleEvent(char const *name) : _name(Util::share_string(name)) {}

    // default copy
    // default assign

    static Category category() { return C; }

    Util::ptr_shared<char> name() const { return _name; }
    unsigned propertyCount() const { return _properties.size(); }
    PropertyPair property(unsigned property) const {
        return _properties[property];
    }

    void generateChildEvents() const {}

protected:
    void _addProperty(Util::ptr_shared<char> name,
                      Util::ptr_shared<char> value)
    {
        _properties.push_back(PropertyPair(name, value));
    }
    void _addProperty(Util::ptr_shared<char> name, char const *value) {
        _addProperty(name, Util::share_string(value));
    }
    void _addProperty(char const *name, Util::ptr_shared<char> value) {
        _addProperty(Util::share_string(name), value);
    }
    void _addProperty(char const *name, char const *value) {
        _addProperty(Util::share_string(name), Util::share_string(value));
    }
    void _addProperty(Util::ptr_shared<char> name, long value) {
        _addFormattedProperty(name, "%ld", value);
    }
    void _addProperty(char const *name, long value) {
        _addProperty(Util::share_string(name), value);
    }

private:
    Util::ptr_shared<char> _name;
    std::vector<PropertyPair, GC::Alloc<PropertyPair, GC::AUTO> > _properties;

    void _addFormattedProperty(Util::ptr_shared<char> name, char const *format, ...)
    {
        va_list args;
        va_start(args, format);
        gchar *value=g_strdup_vprintf(format, args);
        g_assert(value != NULL);
        va_end(args);
        _addProperty(name, value);
        g_free(value);
    }
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

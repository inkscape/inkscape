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

#include <vector>
#include "gc-alloc.h"
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

private:
    Util::ptr_shared<char> _name;
    std::vector<PropertyPair, GC::Alloc<PropertyPair, GC::AUTO> > _properties;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

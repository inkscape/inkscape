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
    SimpleEvent(char const *name) : _name(Util::share_string(name)) {}

    static Category category() { return C; }

    Util::ptr_shared<char> name() const { return _name; }
    unsigned propertyCount() const { return _properties.size(); }
    PropertyPair property(unsigned property) const {
        return _properties[property];
    }

protected:
    SimpleEvent(char const *name,
                char const *attr0, char const *value0)
    : _name(Util::share_string(name))
    {
        _addProperty(attr0, value0);
    }

    SimpleEvent(char const *name,
                char const *attr0, char const *value0,
                char const *attr1, char const *value1)
    : _name(Util::share_string(name))
    {
        _addProperty(attr0, value0);
        _addProperty(attr1, value1);
    }

    SimpleEvent(char const *name,
                char const *attr0, char const *value0,
                char const *attr1, char const *value1,
                char const *attr2, char const *value2)
    : _name(Util::share_string(name))
    {
        _addProperty(attr0, value0);
        _addProperty(attr1, value1);
        _addProperty(attr2, value2);
    }

    SimpleEvent(char const *name,
                char const *attr0, char const *value0,
                char const *attr1, char const *value1,
                char const *attr2, char const *value2,
                char const *attr3, char const *value3)
    : _name(Util::share_string(name))
    {
        _addProperty(attr0, value0);
        _addProperty(attr1, value1);
        _addProperty(attr2, value2);
        _addProperty(attr3, value3);
    }

    SimpleEvent(char const *name,
                char const *attr0, char const *value0,
                char const *attr1, char const *value1,
                char const *attr2, char const *value2,
                char const *attr3, char const *value3,
                char const *attr4, char const *value4)
    : _name(Util::share_string(name))
    {
        _addProperty(attr0, value0);
        _addProperty(attr1, value1);
        _addProperty(attr2, value2);
        _addProperty(attr3, value3);
        _addProperty(attr4, value4);
    }

    SimpleEvent(char const *name,
                char const *attr0, char const *value0,
                char const *attr1, char const *value1,
                char const *attr2, char const *value2,
                char const *attr3, char const *value3,
                char const *attr4, char const *value4,
                char const *attr5, char const *value5)
    : _name(Util::share_string(name))
    {
        _addProperty(attr0, value0);
        _addProperty(attr1, value1);
        _addProperty(attr2, value2);
        _addProperty(attr3, value3);
        _addProperty(attr4, value4);
        _addProperty(attr5, value5);
    }

    void _addProperty(char const *name, char const *value) {
        _properties.push_back(PropertyPair(name, value));
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

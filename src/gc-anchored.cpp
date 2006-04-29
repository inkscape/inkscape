/*
 * Inkscape::GC::Anchored - base class for anchored GC-managed objects
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <typeinfo>
#include "gc-anchored.h"
#include "debug/event-tracker.h"
#include "debug/event.h"
#include "util/share.h"
#include "util/format.h"

namespace Inkscape {

namespace GC {

class AnchorEvent : public Debug::Event {
public:
    enum Type { ANCHOR, RELEASE };

    AnchorEvent(Anchored const *object, Type type)
    : _base(Util::format("%p", Core::base(const_cast<Anchored *>(object)))),
      _object(Util::format("%p", object)),
      _class_name(Util::share_static_string(typeid(*object).name())),
      _refcount(Util::format("%d", ( type == ANCHOR ? object->_anchored_refcount() + 1 : object->_anchored_refcount() - 1 ))),
      _type(type)
    {}

    static Category category() { return REFCOUNT; }

    Util::ptr_shared<char> name() const {
        if ( _type == ANCHOR ) {
            return Util::share_static_string("gc-anchor");
        } else {
            return Util::share_static_string("gc-release");
        }
    }
    unsigned propertyCount() const { return 4; }
    PropertyPair property(unsigned index) const {
        switch (index) {
            case 0:
                return PropertyPair("base", _base);
            case 1:
                return PropertyPair("pointer", _object);
            case 2:
                return PropertyPair("class", _class_name);
            case 3:
                return PropertyPair("new-refcount", _refcount);
            default:
                return PropertyPair();
        }
    }

private:
    Util::ptr_shared<char> _base;
    Util::ptr_shared<char> _object;
    Util::ptr_shared<char> _class_name;
    Util::ptr_shared<char> _refcount;
    Type _type;
};

Anchored::Anchor *Anchored::_new_anchor() const {
    return new Anchor(this);
}

void Anchored::_free_anchor(Anchored::Anchor *anchor) const {
    delete anchor;
}

void Anchored::anchor() const {
    Debug::EventTracker<AnchorEvent> tracker(this, AnchorEvent::ANCHOR);
    if (!_anchor) {
        _anchor = _new_anchor();
    }
    _anchor->refcount++;
}

void Anchored::release() const {
    Debug::EventTracker<AnchorEvent> tracker(this, AnchorEvent::RELEASE);
    if (!--_anchor->refcount) {
        _free_anchor(_anchor);
        _anchor = NULL;
    }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

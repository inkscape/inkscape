#ifndef SEEN_INKSCAPE_XML_SP_REPR_ACTION_H
#define SEEN_INKSCAPE_XML_SP_REPR_ACTION_H

#include <glib/gtypes.h>
#include <glib/gquark.h>
#include <glibmm/ustring.h>

#include <iterator>
#include "util/shared-c-string-ptr.h"
#include "util/forward-pointer-iterator.h"
#include "gc-managed.h"
#include "xml/node.h"

namespace Inkscape {
namespace XML {

class Node;
class NodeObserver;

enum EventType {
	EVENT_ADD,
	EVENT_DEL,
	EVENT_CHG_ATTR,
	EVENT_CHG_CONTENT,
	EVENT_CHG_ORDER
};
		
class Event
: public Inkscape::GC::Managed<Inkscape::GC::SCANNED, Inkscape::GC::MANUAL>
{
public:
        
        virtual ~Event() {}

	Event *next;
	int serial;
	Node *repr;

	struct IteratorStrategy {
		static Event const *next(Event const *action) {
			return action->next;
		}
	};

	typedef Inkscape::Util::ForwardPointerIterator<Event, IteratorStrategy> Iterator;
	typedef Inkscape::Util::ForwardPointerIterator<Event const, IteratorStrategy> ConstIterator;

	Event *optimizeOne() { return _optimizeOne(); }
	void undoOne(NodeObserver &observer) const {
		_undoOne(observer);
	}
	void replayOne(NodeObserver &observer) const {
		_replayOne(observer);
	}

protected:
	Event(Node *r, Event *n)
	: next(n), serial(_next_serial++), repr(r) {}

	virtual Event *_optimizeOne()=0;
	virtual void _undoOne(NodeObserver &) const=0;
	virtual void _replayOne(NodeObserver &) const=0;

private:
	static int _next_serial;
};

class EventAdd : public Event {
public:
	EventAdd(Node *repr, Node *c, Node *rr, Event *next)
	: Event(repr, next), child(c), ref(rr) {}

	Node *child;
	Node *ref;

private:
	Event *_optimizeOne();
	void _undoOne(NodeObserver &observer) const;
	void _replayOne(NodeObserver &observer) const;
};

class EventDel : public Event {
public:
	EventDel(Node *repr, Node *c, Node *rr, Event *next)
	: Event(repr, next), child(c), ref(rr) {}

	Node *child;
	Node *ref;

private:
	Event *_optimizeOne();
	void _undoOne(NodeObserver &observer) const;
	void _replayOne(NodeObserver &observer) const;
};

class EventChgAttr : public Event {
public:
	EventChgAttr(Node *repr, GQuark k,
		     Inkscape::Util::SharedCStringPtr ov,
                     Inkscape::Util::SharedCStringPtr nv,
                     Event *next)
	: Event(repr, next), key(k),
	  oldval(ov), newval(nv) {}

	GQuark key;
	Inkscape::Util::SharedCStringPtr oldval;
	Inkscape::Util::SharedCStringPtr newval;

private:
	Event *_optimizeOne();
	void _undoOne(NodeObserver &observer) const;
	void _replayOne(NodeObserver &observer) const;
};

class EventChgContent : public Event {
public:
	EventChgContent(Node *repr,
                        Inkscape::Util::SharedCStringPtr ov,
                        Inkscape::Util::SharedCStringPtr nv,
                        Event *next)
	: Event(repr, next), oldval(ov), newval(nv) {}

	Inkscape::Util::SharedCStringPtr oldval;
	Inkscape::Util::SharedCStringPtr newval;

private:
	Event *_optimizeOne();
	void _undoOne(NodeObserver &observer) const;
	void _replayOne(NodeObserver &observer) const;
};

class EventChgOrder : public Event {
public:
	EventChgOrder(Node *repr, Node *c, Node *orr, Node *nrr, Event *next)
	: Event(repr, next), child(c),
	  oldref(orr), newref(nrr) {}

	Node *child;
	Node *oldref, *newref;

private:
	Event *_optimizeOne();
	void _undoOne(NodeObserver &observer) const;
	void _replayOne(NodeObserver &observer) const;
};

}
}

#endif

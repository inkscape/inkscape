/** @file
 * @brief Event object representing a change of the XML document
 */
/* Authors:
 *   Unknown author(s)
 *   Krzysztof Kosiński <tweenk.pl@gmail.com> (documentation)
 *
 * Copyright 2008 Authors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 */

#ifndef SEEN_INKSCAPE_XML_SP_REPR_ACTION_H
#define SEEN_INKSCAPE_XML_SP_REPR_ACTION_H

typedef unsigned int GQuark;
#include <glibmm/ustring.h>

#include <iterator>
#include "util/share.h"
#include "util/forward-pointer-iterator.h"
#include "inkgc/gc-managed.h"
#include "xml/node.h"

namespace Inkscape {
namespace XML {

/**
 * @brief Enumeration of all XML event types
 */
// enum EventType {
//     EVENT_ADD, ///< Child added
//     EVENT_DEL, ///< Child removed
//     EVENT_CHG_ATTR, ///< Attribute changed
//     EVENT_CHG_CONTENT, ///< Content changed
//     EVENT_CHG_ORDER ///< Order of children changed
// };

/**
 * @brief Generic XML modification event
 *
 * This is the base class for all other modification events. It is actually a singly-linked
 * list of events, called an event chain or an event log. Logs of events that happened
 * in a transaction can be obtained from Document::commitUndoable(). Events can be replayed
 * to a NodeObserver, or undone (which is equivalent to replaying opposite events in reverse
 * order).
 *
 * Event logs are built by appending to the front, so by walking the list one iterates over
 * the events in reverse chronological order.
 */
class Event
: public Inkscape::GC::Managed<Inkscape::GC::SCANNED, Inkscape::GC::MANUAL>
{
public:        
    virtual ~Event() {}

    /**
     * @brief Pointer to the next event in the event chain
     * 
     * Note that the event this pointer points to actually happened before this event.
     * This is because the event log is built by appending to the front.
     */
    Event *next;
    /**
     * @brief Serial number of the event, not used at the moment
     */
    int serial;
    /**
     * @brief Pointer to the node that was the object of the event
     *
     * Because the nodes are garbage-collected, this pointer guarantees that the node
     * will stay in memory as long as the event does. This simplifies rolling back
     * extensive deletions.
     */
    Node *repr;

    struct IteratorStrategy {
        static Event const *next(Event const *action) {
            return action->next;
        }
    };

    typedef Inkscape::Util::ForwardPointerIterator<Event, IteratorStrategy> Iterator;
    typedef Inkscape::Util::ForwardPointerIterator<Event const, IteratorStrategy> ConstIterator;

    /**
     * @brief If possible, combine this event with the next to reduce memory use
     * @return Pointer to the optimized event chain, which may have changed
     */
    Event *optimizeOne() { return _optimizeOne(); }
    /**
     * @brief Undo this event to an observer
     *
     * This method notifies the specified observer of an action opposite to the one that
     * is described by this event.
     */
    void undoOne(NodeObserver &observer) const {
        _undoOne(observer);
    }
    /**
     * @brief Replay this event to an observer
     *
     * This method notifies the specified event of the same action that it describes.
     */
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

/**
 * @brief Object representing child addition
 */
class EventAdd : public Event {
public:
    EventAdd(Node *repr, Node *c, Node *rr, Event *next)
    : Event(repr, next), child(c), ref(rr) {}

    /// The added child node
    Node *child;
    /// The node after which the child has been added, or NULL if it was added as first
    Node *ref;

private:
    Event *_optimizeOne();
    void _undoOne(NodeObserver &observer) const;
    void _replayOne(NodeObserver &observer) const;
};

/**
 * @brief Object representing child removal
 */
class EventDel : public Event {
public:
    EventDel(Node *repr, Node *c, Node *rr, Event *next)
    : Event(repr, next), child(c), ref(rr) {}

    /// The child node that was removed
    Node *child;
    /// The node after which the removed node was in the sibling order, or NULL if it was first
    Node *ref;

private:
    Event *_optimizeOne();
    void _undoOne(NodeObserver &observer) const;
    void _replayOne(NodeObserver &observer) const;
};

/**
 * @brief Object representing attribute change
 */
class EventChgAttr : public Event {
public:
    EventChgAttr(Node *repr, GQuark k,
                 Inkscape::Util::ptr_shared<char> ov,
                 Inkscape::Util::ptr_shared<char> nv,
                 Event *next)
    : Event(repr, next), key(k),
      oldval(ov), newval(nv) {}

    /// GQuark corresponding to the changed attribute's name
    GQuark key;
    /// Value of the attribute before the change
    Inkscape::Util::ptr_shared<char> oldval;
    /// Value of the attribute after the change
    Inkscape::Util::ptr_shared<char> newval;

private:
    Event *_optimizeOne();
    void _undoOne(NodeObserver &observer) const;
    void _replayOne(NodeObserver &observer) const;
};

/**
 * @brief Object representing content change
 */
class EventChgContent : public Event {
public:
    EventChgContent(Node *repr,
                    Inkscape::Util::ptr_shared<char> ov,
                    Inkscape::Util::ptr_shared<char> nv,
                    Event *next)
    : Event(repr, next), oldval(ov), newval(nv) {}

    /// Content of the node before the change
    Inkscape::Util::ptr_shared<char> oldval;
    /// Content of the node after the change
    Inkscape::Util::ptr_shared<char> newval;

private:
    Event *_optimizeOne();
    void _undoOne(NodeObserver &observer) const;
    void _replayOne(NodeObserver &observer) const;
};

/**
 * @brief Obect representing child order change
 */
class EventChgOrder : public Event {
public:
    EventChgOrder(Node *repr, Node *c, Node *orr, Node *nrr, Event *next)
    : Event(repr, next), child(c),
      oldref(orr), newref(nrr) {}

    /// The node that was relocated in sibling order
    Node *child;
    /// The node after which the relocated node was in the sibling order before the change, or NULL if it was first
    Node *oldref;
    /// The node after which the relocated node is after the change, or if it's first
    Node *newref;

private:
    Event *_optimizeOne();
    void _undoOne(NodeObserver &observer) const;
    void _replayOne(NodeObserver &observer) const;
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

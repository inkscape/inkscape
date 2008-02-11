/*
 * Inkscape::XML::CompositeNodeObserver - combine multiple observers
 *
 * Copyright 2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_XML_COMPOSITE_NODE_OBSERVER_H
#define SEEN_INKSCAPE_XML_COMPOSITE_NODE_OBSERVER_H

#include "gc-managed.h"
#include "xml/node-observer.h"
#include "util/list-container.h"

namespace Inkscape {

namespace XML {

class NodeEventVector;

class CompositeNodeObserver : public NodeObserver, public GC::Managed<> {
public:
    struct ObserverRecord : public GC::Managed<> {
        explicit ObserverRecord(NodeObserver &o) : observer(o), marked(false) {}

        NodeObserver &observer;
        bool marked; //< if marked for removal
    };
    typedef Util::ListContainer<ObserverRecord> ObserverRecordList;

    CompositeNodeObserver()
    : _iterating(0), _active_marked(0), _pending_marked(0) {}

    void add(NodeObserver &observer);
    void addListener(NodeEventVector const &vector, void *data);
    void remove(NodeObserver &observer);
    void removeListenerByData(void *data);
    
    void notifyChildAdded(Node &node, Node &child, Node *prev);

    void notifyChildRemoved(Node &node, Node &child, Node *prev);

    void notifyChildOrderChanged(Node &node, Node &child,
                                 Node *old_prev, Node *new_prev);

    void notifyContentChanged(Node &node,
                              Util::ptr_shared<char> old_content,
                              Util::ptr_shared<char> new_content);

    void notifyAttributeChanged(Node &node, GQuark name,
                                Util::ptr_shared<char> old_value,
                                Util::ptr_shared<char> new_value);

private:
    unsigned _iterating;
    ObserverRecordList _active;
    unsigned _active_marked;
    ObserverRecordList _pending;
    unsigned _pending_marked;

    void _startIteration() { ++_iterating; }
    void _finishIteration();
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

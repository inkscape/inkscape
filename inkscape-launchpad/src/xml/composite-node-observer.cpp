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

#include <cstring>
#include <glib.h>

#include "util/find-if-before.h"
#include "xml/composite-node-observer.h"
#include "xml/node-event-vector.h"
#include "debug/event-tracker.h"
#include "debug/simple-event.h"

namespace Inkscape {

namespace XML {

void CompositeNodeObserver::notifyChildAdded(Node &node, Node &child, Node *prev)
{
    _startIteration();
    for ( ObserverRecordList::iterator iter=_active.begin() ;
          iter != _active.end() ; ++iter )
    {
        if (!iter->marked) {
            iter->observer.notifyChildAdded(node, child, prev);
        }
    }
    _finishIteration();
}

void CompositeNodeObserver::notifyChildRemoved(Node &node, Node &child,
                                                           Node *prev)
{
    _startIteration();
    for ( ObserverRecordList::iterator iter=_active.begin() ;
          iter != _active.end() ; ++iter )
    {
        if (!iter->marked) {
            iter->observer.notifyChildRemoved(node, child, prev);
        }
    }
    _finishIteration();
}

void CompositeNodeObserver::notifyChildOrderChanged(Node &node, Node &child,
                                                                Node *old_prev,
                                                                Node *new_prev)
{
    _startIteration();
    for ( ObserverRecordList::iterator iter=_active.begin() ;
          iter != _active.end() ; ++iter )
    {
        if (!iter->marked) {
            iter->observer.notifyChildOrderChanged(node, child, old_prev, new_prev);
        }
    }
    _finishIteration();
}

void CompositeNodeObserver::notifyContentChanged(
    Node &node,
    Util::ptr_shared<char> old_content, Util::ptr_shared<char> new_content
) {
    _startIteration();
    for ( ObserverRecordList::iterator iter=_active.begin() ;
          iter != _active.end() ; ++iter )
    {
        if (!iter->marked) {
            iter->observer.notifyContentChanged(node, old_content, new_content);
        }
    }
    _finishIteration();
}

void CompositeNodeObserver::notifyAttributeChanged(
    Node &node, GQuark name,
    Util::ptr_shared<char> old_value, Util::ptr_shared<char> new_value
) {
    _startIteration();
    for ( ObserverRecordList::iterator iter=_active.begin() ;
          iter != _active.end() ; ++iter )
    {
        if (!iter->marked) {
            iter->observer.notifyAttributeChanged(node, name, old_value, new_value);
        }
    }
    _finishIteration();
}

void CompositeNodeObserver::add(NodeObserver &observer) {
    if (_iterating) {
        _pending.push_back(ObserverRecord(observer));
    } else {
        _active.push_back(ObserverRecord(observer));
    }
}

namespace {

class VectorNodeObserver : public NodeObserver, public GC::Managed<> {
public:
    VectorNodeObserver(NodeEventVector const &v, void *d)
    : vector(v), data(d) {}

    NodeEventVector const &vector;
    void * const data;

    void notifyChildAdded(Node &node, Node &child, Node *prev) {
        if (vector.child_added) {
            vector.child_added(&node, &child, prev, data);
        }
    }

    void notifyChildRemoved(Node &node, Node &child, Node *prev) {
        if (vector.child_removed) {
            vector.child_removed(&node, &child, prev, data);
        }
    }

    void notifyChildOrderChanged(Node &node, Node &child, Node *old_prev, Node *new_prev) {
        if (vector.order_changed) {
            vector.order_changed(&node, &child, old_prev, new_prev, data);
        }
    }

    void notifyContentChanged(Node &node, Util::ptr_shared<char> old_content, Util::ptr_shared<char> new_content) {
        if (vector.content_changed) {
            vector.content_changed(&node, old_content, new_content, data);
        }
    }

    void notifyAttributeChanged(Node &node, GQuark name, Util::ptr_shared<char> old_value, Util::ptr_shared<char> new_value) {
        if (vector.attr_changed) {
            vector.attr_changed(&node, g_quark_to_string(name), old_value, new_value, false, data);
        }
    }
};

}

void CompositeNodeObserver::addListener(NodeEventVector const &vector,
                                        void *data)
{
    Debug::EventTracker<Debug::SimpleEvent<Debug::Event::XML> > tracker("add-listener");
    add(*(new VectorNodeObserver(vector, data)));
}

namespace {

using std::find_if;
using Algorithms::find_if_before;
typedef CompositeNodeObserver::ObserverRecord ObserverRecord;
typedef CompositeNodeObserver::ObserverRecordList ObserverRecordList;

template <typename ObserverPredicate>
struct unmarked_record_satisfying {
    ObserverPredicate predicate;
    unmarked_record_satisfying(ObserverPredicate p) : predicate(p) {}
    bool operator()(ObserverRecord const &record) {
        return !record.marked && predicate(record.observer);
    }
};

template <typename Predicate>
bool mark_one(ObserverRecordList &observers, unsigned &/*marked_count*/,
              Predicate p)
{
    ObserverRecordList::iterator found=std::find_if(
        observers.begin(), observers.end(),
        unmarked_record_satisfying<Predicate>(p)
    );

    if ( found != observers.end() ) {
        found->marked = true;
        return true;
    } else {
        return false;
    }
}

template <typename Predicate>
bool remove_one(ObserverRecordList &observers, unsigned &/*marked_count*/,
                Predicate p)
{
    if (observers.empty()) {
        return false;
    }

    if (unmarked_record_satisfying<Predicate>(p)(observers.front())) {
        observers.pop_front();
        return true;
    }

    ObserverRecordList::iterator found=find_if_before(
        observers.begin(), observers.end(),
        unmarked_record_satisfying<Predicate>(p)
    );

    if ( found != observers.end() ) {
        observers.erase_after(found);
        return true;
    } else {
        return false;
    }
}

bool is_marked(ObserverRecord const &record) { return record.marked; }

void remove_all_marked(ObserverRecordList &observers, unsigned &marked_count)
{
    ObserverRecordList::iterator iter;

    g_assert( !observers.empty() || !marked_count );

    while ( marked_count && observers.front().marked ) {
        observers.pop_front();
        --marked_count;
    }

    iter = observers.begin();
    while (marked_count) {
        iter = find_if_before(iter, observers.end(), is_marked);
        observers.erase_after(iter);
        --marked_count;
    }
}

}

void CompositeNodeObserver::_finishIteration() {
    if (!--_iterating) {
        remove_all_marked(_active, _active_marked);
        remove_all_marked(_pending, _pending_marked);
        _active.insert(_active.end(), _pending.begin(), _pending.end());
        _pending.clear();
    }
}

namespace {

struct eql_observer {
    NodeObserver const &observer;
    eql_observer(NodeObserver const &o) : observer(o) {}
    bool operator()(NodeObserver const &other) {
        return &observer == &other;
    }
};

}

void CompositeNodeObserver::remove(NodeObserver &observer) {
    eql_observer p(observer);
    if (_iterating) {
        mark_one(_active, _active_marked, p) ||
        mark_one(_pending, _pending_marked, p);
    } else {
        remove_one(_active, _active_marked, p) ||
        remove_one(_pending, _pending_marked, p);
    }
}

namespace {

struct vector_data_matches {
    void * const data;
    vector_data_matches(void *d) : data(d) {}
    
    bool operator()(NodeObserver const &observer) {
        VectorNodeObserver const *vo=dynamic_cast<VectorNodeObserver const *>(&observer);
        bool OK = false;
        if (vo) {
            if (vo && vo->data == data) {
                OK = true;
            }
        }
        return OK;
    }
};

}

void CompositeNodeObserver::removeListenerByData(void *data) {
    Debug::EventTracker<Debug::SimpleEvent<Debug::Event::XML> > tracker("remove-listener-by-data");
    vector_data_matches p(data);
    if (_iterating) {
        mark_one(_active, _active_marked, p) ||
        mark_one(_pending, _pending_marked, p);
    } else {
        remove_one(_active, _active_marked, p) ||
        remove_one(_pending, _pending_marked, p);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

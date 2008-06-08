/*
 * SimpleNode - simple XML node implementation
 *
 * Copyright 2003-2005 MenTaLguY <mental@rydia.net>
 * Copyright 2003 Nathan Hurst
 * Copyright 1999-2003 Lauris Kaplinski
 * Copyright 2000-2002 Ximian Inc.
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
#include <string>
#include <glib/gstrfuncs.h>

#include "xml/simple-node.h"
#include "xml/node-event-vector.h"
#include "xml/node-fns.h"
#include "xml/repr.h"
#include "debug/event-tracker.h"
#include "debug/simple-event.h"
#include "util/share.h"
#include "util/format.h"

namespace Inkscape {

namespace XML {

namespace {

Util::ptr_shared<char> stringify_node(Node const &node) {
    gchar *string;
    switch (node.type()) {
    case ELEMENT_NODE: {
        char const *id=node.attribute("id");
        if (id) {
            string = g_strdup_printf("element(%p)=%s(#%s)", &node, node.name(), id);
        } else {
            string = g_strdup_printf("element(%p)=%s", &node, node.name());
        }
    } break;
    case TEXT_NODE:
        string = g_strdup_printf("text(%p)=%s", &node, node.content());
        break;
    case COMMENT_NODE:
        string = g_strdup_printf("comment(%p)=<!--%s-->", &node, node.content());
        break;
    case DOCUMENT_NODE:
        string = g_strdup_printf("document(%p)", &node);
        break;
    default:
        string = g_strdup_printf("unknown(%p)", &node);
    }
    Util::ptr_shared<char> result=Util::share_string(string);
    g_free(string);
    return result;
}

typedef Debug::SimpleEvent<Debug::Event::XML> DebugXML;

class DebugXMLNode : public DebugXML {
public:
    DebugXMLNode(Node const &node, Util::ptr_shared<char> name)
    : DebugXML(name)
    {
        _addProperty("node", stringify_node(node));
    }
};

class DebugAddChild : public DebugXMLNode {
public:
    DebugAddChild(Node const &node, Node const &child, Node const *prev)
    : DebugXMLNode(node, Util::share_static_string("add-child"))
    {
        _addProperty("child", stringify_node(child));
        _addProperty("position", Util::format("%d", ( prev ? prev->position() + 1 : 0 )));
    }
};

class DebugRemoveChild : public DebugXMLNode {
public:
    DebugRemoveChild(Node const &node, Node const &child)
    : DebugXMLNode(node, Util::share_static_string("remove-child"))
    {
        _addProperty("child", stringify_node(child));
    }
};

class DebugSetChildPosition : public DebugXMLNode {
public:
    DebugSetChildPosition(Node const &node, Node const &child,
                          Node const *old_prev, Node const *new_prev)
    : DebugXMLNode(node, Util::share_static_string("set-child-position"))
    {
        _addProperty("child", stringify_node(child));

        unsigned old_position = ( old_prev ? old_prev->position() : 0 );
        unsigned position = ( new_prev ? new_prev->position() : 0 );
        if ( position > old_position ) {
            --position;
        }

        _addProperty("position", Util::format("%d", position));
    }
};

class DebugSetContent : public DebugXMLNode {
public:
    DebugSetContent(Node const &node,
                    Util::ptr_shared<char> content)
    : DebugXMLNode(node, Util::share_static_string("set-content"))
    {
        _addProperty("content", content);
    }
};

class DebugClearContent : public DebugXMLNode {
public:
    DebugClearContent(Node const &node)
    : DebugXMLNode(node, Util::share_static_string("clear-content"))
    {}
};

class DebugSetAttribute : public DebugXMLNode {
public:
    DebugSetAttribute(Node const &node,
                      GQuark name,
                      Util::ptr_shared<char> value)
    : DebugXMLNode(node, Util::share_static_string("set-attribute"))
    {
        _addProperty("name", Util::share_static_string(g_quark_to_string(name)));
        _addProperty("value", value);
    }
};

class DebugClearAttribute : public DebugXMLNode {
public:
    DebugClearAttribute(Node const &node, GQuark name)
    : DebugXMLNode(node, Util::share_static_string("clear-attribute"))
    {
        _addProperty("name", Util::share_static_string(g_quark_to_string(name)));
    }
};

}

using Util::ptr_shared;
using Util::share_string;
using Util::share_unsafe;
using Util::share_static_string;
using Util::List;
using Util::MutableList;
using Util::cons;
using Util::rest;
using Util::set_rest;

SimpleNode::SimpleNode(int code, Document *document)
: Node(), _name(code), _attributes(), _child_count(0),
  _cached_positions_valid(false)
{
    this->_document = document;
    this->_parent = this->_next = NULL;
    this->_first_child = this->_last_child = NULL;

    _observers.add(_subtree_observers);
}

SimpleNode::SimpleNode(SimpleNode const &node, Document *document)
: Node(),
  _cached_position(node._cached_position),
  _name(node._name), _attributes(), _content(node._content),
  _child_count(node._child_count),
  _cached_positions_valid(node._cached_positions_valid)
{
    _document = document;
    _parent = _next = NULL;
    _first_child = _last_child = NULL;

    for ( Node *child = node._first_child ;
          child != NULL ; child = child->next() )
    {
        Node *child_copy=child->duplicate(document);

        child_copy->_setParent(this);
        if (_last_child) {
            _last_child->_setNext(child_copy);
        } else {
            _first_child = child_copy;
        }
        _last_child = child_copy;

        child_copy->release(); // release to avoid a leak
    }

    for ( List<AttributeRecord const> iter = node._attributes ;
          iter ; ++iter )
    {
        _attributes = cons(*iter, _attributes);
    }

    _observers.add(_subtree_observers);
}

gchar const *SimpleNode::name() const {
    return g_quark_to_string(_name);
}

gchar const *SimpleNode::content() const {
    return this->_content;
}

gchar const *SimpleNode::attribute(gchar const *name) const {
    g_return_val_if_fail(name != NULL, NULL);

    GQuark const key = g_quark_from_string(name);

    for ( List<AttributeRecord const> iter = _attributes ;
          iter ; ++iter )
    {
        if ( iter->key == key ) {
            return iter->value;
        }
    }

    return NULL;
}

unsigned SimpleNode::position() const {
    g_return_val_if_fail(_parent != NULL, 0);
    return _parent->_childPosition(*this);
}

unsigned SimpleNode::_childPosition(Node const &child) const {
    if (!_cached_positions_valid) {
        unsigned position=0;
        for ( Node *sibling = _first_child ;
              sibling ; sibling = sibling->next() )
        {
            sibling->_setCachedPosition(position);
            position++;
        }
        _cached_positions_valid = true;
    }
    return child._cachedPosition();
}

Node *SimpleNode::nthChild(unsigned index) {
    Node *child = _first_child;
    for ( ; index > 0 && child ; child = child->next() ) {
        index--;
    }
    return child;
}

bool SimpleNode::matchAttributeName(gchar const *partial_name) const {
    g_return_val_if_fail(partial_name != NULL, false);

    for ( List<AttributeRecord const> iter = _attributes ;
          iter ; ++iter )
    {
        gchar const *name = g_quark_to_string(iter->key);
        if (std::strstr(name, partial_name)) {
            return true;
        }
    }

    return false;
}

void SimpleNode::_setParent(Node *parent) {
   if (_parent) {
        _subtree_observers.remove(_parent->_subtreeObservers());
    }
    _parent = parent;
    if (parent) {
        _subtree_observers.add(parent->_subtreeObservers());
    }
}

void SimpleNode::setContent(gchar const *content) {
    ptr_shared<char> old_content=_content;
    ptr_shared<char> new_content = ( content ? share_string(content) : ptr_shared<char>() );

    Debug::EventTracker<> tracker;
    if (new_content) {
        tracker.set<DebugSetContent>(*this, new_content);
    } else {
        tracker.set<DebugClearContent>(*this);
    }

    _content = new_content;

    if ( _content != old_content ) {
        if (_document) {
            _document->logger()->notifyContentChanged(*this, old_content, _content);
        }

        _observers.notifyContentChanged(*this, old_content, _content);
    }
}

void
SimpleNode::setAttribute(gchar const *name, gchar const *value, bool const /*is_interactive*/)
{
    g_return_if_fail(name && *name);

    GQuark const key = g_quark_from_string(name);

    MutableList<AttributeRecord> ref;
    MutableList<AttributeRecord> existing;
    for ( existing = _attributes ; existing ; ++existing ) {
        if ( existing->key == key ) {
            break;
        }
        ref = existing;
    }

    Debug::EventTracker<> tracker;

    ptr_shared<char> old_value=( existing ? existing->value : ptr_shared<char>() );

    ptr_shared<char> new_value=ptr_shared<char>();
    if (value) {
        new_value = share_string(value);
        tracker.set<DebugSetAttribute>(*this, key, new_value);
        if (!existing) {
            if (ref) {
                set_rest(ref, MutableList<AttributeRecord>(AttributeRecord(key, new_value)));
            } else {
                _attributes = MutableList<AttributeRecord>(AttributeRecord(key, new_value));
            }
        } else {
            existing->value = new_value;
        }
    } else {
        tracker.set<DebugClearAttribute>(*this, key);
        if (existing) {
            if (ref) {
                set_rest(ref, rest(existing));
            } else {
                _attributes = rest(existing);
            }
            set_rest(existing, MutableList<AttributeRecord>());
        }
    }

    if ( new_value != old_value && (!old_value || !new_value || strcmp(old_value, new_value))) {
        if (_document) {
            _document->logger()->notifyAttributeChanged(*this, key, old_value, new_value);
        }

        _observers.notifyAttributeChanged(*this, key, old_value, new_value);
    }
}

void SimpleNode::addChild(Node *child, Node *ref) {
    g_assert(child);
    g_assert(!ref || ref->parent() == this);
    g_assert(!child->parent());

    Debug::EventTracker<DebugAddChild> tracker(*this, *child, ref);

    Node *next;
    if (ref) {
        next = ref->next();
        ref->_setNext(child);
    } else {
        next = _first_child;
        _first_child = child;
    }
    if (!next) { // appending?
        _last_child = child;
        // set cached position if possible when appending
        if (!ref) {
            // if !next && !ref, child is sole child
            child->_setCachedPosition(0);
            _cached_positions_valid = true;
        } else if (_cached_positions_valid) {
            child->_setCachedPosition(ref->_cachedPosition() + 1);
        }
    } else {
        // invalidate cached positions otherwise
        _cached_positions_valid = false;
    }

    child->_setParent(this);
    child->_setNext(next);
    _child_count++;

    if (_document) {
        child->_bindDocument(*_document);
        _document->logger()->notifyChildAdded(*this, *child, ref);
    }

    _observers.notifyChildAdded(*this, *child, ref);
}

void SimpleNode::_bindDocument(Document &document) {
    g_assert(!_document || _document == &document);

    if (!_document) {
        _document = &document;

        for ( Node *child = _first_child ; child != NULL ; child = child->next() ) {
            child->_bindDocument(document);
        }
    }
}

void SimpleNode::removeChild(Node *child) {
    g_assert(child);
    g_assert(child->parent() == this);

    Node *ref = ( child != _first_child ? previous_node(child) : NULL );

    Debug::EventTracker<DebugRemoveChild> tracker(*this, *child);

    Node *next = child->next();
    if (ref) {
        ref->_setNext(next);
    } else {
        _first_child = next;
    }
    if (!next) { // removing the last child?
        _last_child = ref;
    } else {
        // removing any other child invalidates the cached positions
        _cached_positions_valid = false;
    }

    child->_setNext(NULL);
    child->_setParent(NULL);
    _child_count--;

    if (_document) {
        _document->logger()->notifyChildRemoved(*this, *child, ref);
    }

    _observers.notifyChildRemoved(*this, *child, ref);
}

void SimpleNode::changeOrder(Node *child, Node *ref) {
    g_return_if_fail(child);
    g_return_if_fail(child->parent() == this);
    g_return_if_fail(child != ref);
    g_return_if_fail(!ref || ref->parent() == this);

    Node *const prev = previous_node(child);

    Debug::EventTracker<DebugSetChildPosition> tracker(*this, *child, prev, ref);

    if (prev == ref) { return; }

    Node *next;

    /* Remove from old position. */
    next=child->next();
    if (prev) {
        prev->_setNext(next);
    } else {
        _first_child = next;
    }
    if (!next) {
        _last_child = prev;
    }

    /* Insert at new position. */
    if (ref) {
        next = ref->next();
        ref->_setNext(child);
    } else {
        next = _first_child;
        _first_child = child;
    }
    child->_setNext(next);
    if (!next) {
        _last_child = child;
    }

    _cached_positions_valid = false;

    if (_document) {
        _document->logger()->notifyChildOrderChanged(*this, *child, prev, ref);
    }

    _observers.notifyChildOrderChanged(*this, *child, prev, ref);
}

void SimpleNode::setPosition(int pos) {
    g_return_if_fail(_parent != NULL);

    // a position beyond the end of the list means the end of the list;
    // a negative position is the same as an infinitely large position

    Node *ref=NULL;
    for ( Node *sibling = _parent->firstChild() ;
          sibling && pos ; sibling = sibling->next() )
    {
        if ( sibling != this ) {
            ref = sibling;
            pos--;
        }
    }

    _parent->changeOrder(this, ref);
}

namespace {

void child_added(Node *node, Node *child, Node *ref, void *data) {
    reinterpret_cast<NodeObserver *>(data)->notifyChildAdded(*node, *child, ref);
}

void child_removed(Node *node, Node *child, Node *ref, void *data) {
    reinterpret_cast<NodeObserver *>(data)->notifyChildRemoved(*node, *child, ref);
}

void content_changed(Node *node, gchar const *old_content, gchar const *new_content, void *data) {
    reinterpret_cast<NodeObserver *>(data)->notifyContentChanged(*node, Util::share_unsafe((const char *)old_content), Util::share_unsafe((const char *)new_content));
}

void attr_changed(Node *node, gchar const *name, gchar const *old_value, gchar const *new_value, bool /*is_interactive*/, void *data) {
    reinterpret_cast<NodeObserver *>(data)->notifyAttributeChanged(*node, g_quark_from_string(name), Util::share_unsafe((const char *)old_value), Util::share_unsafe((const char *)new_value));
}

void order_changed(Node *node, Node *child, Node *old_ref, Node *new_ref, void *data) {
    reinterpret_cast<NodeObserver *>(data)->notifyChildOrderChanged(*node, *child, old_ref, new_ref);
}

const NodeEventVector OBSERVER_EVENT_VECTOR = {
    &child_added,
    &child_removed,
    &attr_changed,
    &content_changed,
    &order_changed
};

};

void SimpleNode::synthesizeEvents(NodeEventVector const *vector, void *data) {
    if (vector->attr_changed) {
        for ( List<AttributeRecord const> iter = _attributes ;
              iter ; ++iter )
        {
            vector->attr_changed(this, g_quark_to_string(iter->key), NULL, iter->value, false, data);
        }
    }
    if (vector->child_added) {
        Node *ref = NULL;
        for ( Node *child = this->_first_child ;
              child ; child = child->next() )
        {
            vector->child_added(this, child, ref, data);
            ref = child;
        }
    }
    if (vector->content_changed) {
        vector->content_changed(this, NULL, this->_content, data);
    }
}

void SimpleNode::synthesizeEvents(NodeObserver &observer) {
    synthesizeEvents(&OBSERVER_EVENT_VECTOR, &observer);
}

Node *SimpleNode::root() {
    Node *parent=this;
    while (parent->parent()) {
        parent = parent->parent();
    }

    if ( parent->type() == DOCUMENT_NODE ) {
        for ( Node *child = _document->firstChild() ;
              child ; child = child->next() )
        {
            if ( child->type() == ELEMENT_NODE ) {
                return child;
            }
        }
        return NULL;
    } else if ( parent->type() == ELEMENT_NODE ) {
        return parent;
    } else {
        return NULL;
    }
}

void SimpleNode::mergeFrom(Node const *src, gchar const *key) {
    g_return_if_fail(src != NULL);
    g_return_if_fail(key != NULL);
    g_assert(src != this);

    setContent(src->content());

    for ( Node const *child = src->firstChild() ; child != NULL ; child = child->next() )
    {
        gchar const *id = child->attribute(key);
        if (id) {
            Node *rch=sp_repr_lookup_child(this, key, id);
            if (rch) {
                rch->mergeFrom(child, key);
            } else {
                rch = child->duplicate(_document);
                appendChild(rch);
                rch->release();
            }
        } else {
            Node *rch=child->duplicate(_document);
            appendChild(rch);
            rch->release();
        }
    }

    for ( List<AttributeRecord const> iter = src->attributeList() ;
          iter ; ++iter )
    {
        setAttribute(g_quark_to_string(iter->key), iter->value);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

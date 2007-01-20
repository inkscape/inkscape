/*
 * SimpleNode - generic XML node implementation
 *
 * Copyright 2004-2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_XML_SIMPLE_NODE_H
#define SEEN_INKSCAPE_XML_SIMPLE_NODE_H

#include "xml/node.h"
#include "xml/attribute-record.h"
#include "xml/composite-node-observer.h"
#include "util/list-container.h"

namespace Inkscape {

namespace XML {

class SimpleNode
: virtual public Node, public Inkscape::GC::Managed<>
{
public:
    gchar const *name() const;
    int code() const { return _name; }
    void setCodeUnsafe(int code) {
        g_assert(_document == NULL);
        _name = code;
    }

    Document *document() { return _document; }
    Document const *document() const {
        return const_cast<SimpleNode *>(this)->document();
    }

    Node *duplicate() const { return _duplicate(); }

    Node *root();
    Node const *root() const {
        return const_cast<SimpleNode *>(this)->root();
    }

    Node *parent() { return _parent; }
    Node const *parent() const { return _parent; }

    Node *next() { return _next; }
    Node const *next() const { return _next; }

    Node *firstChild() { return _first_child; }
    Node const *firstChild() const { return _first_child; }
    Node *lastChild() { return _last_child; }
    Node const *lastChild() const { return _last_child; }

    unsigned childCount() const { return _child_count; }
    Node *nthChild(unsigned index);
    Node const *nthChild(unsigned index) const {
        return const_cast<SimpleNode *>(this)->nthChild(index);
    }

    void addChild(Node *child, Node *ref);
    void appendChild(Node *child) {
        SimpleNode::addChild(child, _last_child);
    }
    void removeChild(Node *child);
    void changeOrder(Node *child, Node *ref);

    unsigned position() const;
    void setPosition(int pos);

    gchar const *attribute(gchar const *key) const;
    void setAttribute(gchar const *key, gchar const *value, bool is_interactive=false);
    bool matchAttributeName(gchar const *partial_name) const;

    gchar const *content() const;
    void setContent(gchar const *value);

    void mergeFrom(Node const *src, gchar const *key);

    Inkscape::Util::List<AttributeRecord const> attributeList() const {
        return _attributes;
    }

    void synthesizeEvents(NodeEventVector const *vector, void *data);
    void synthesizeEvents(NodeObserver &observer);

    void addListener(NodeEventVector const *vector, void *data) {
        g_assert(vector != NULL);
        _observers.addListener(*vector, data);
    }
    void addObserver(NodeObserver &observer) {
        _observers.add(observer);
    }
    void removeListenerByData(void *data) {
        _observers.removeListenerByData(data);
    }
    void removeObserver(NodeObserver &observer) {
        _observers.remove(observer);
    }

protected:
    SimpleNode(int code);
    SimpleNode(SimpleNode const &repr);

    virtual SimpleNode *_duplicate() const=0;

public: // ideally these should be protected somehow...
    void _setParent(Node *parent) { _parent = parent; }
    void _setNext(Node *next) { _next = next; }
    void _bindDocument(Document &document);

    unsigned _childPosition(Node const &child) const;
    unsigned _cachedPosition() const { return _cached_position; }
    void _setCachedPosition(unsigned position) const {
        _cached_position = position;
    }

private:
    void operator=(Node const &); // no assign

    Node *_parent;
    Node *_next;
    Document *_document;
    mutable unsigned _cached_position;

    int _name;

    Inkscape::Util::MutableList<AttributeRecord> _attributes;

    Inkscape::Util::ptr_shared<char> _content;

    unsigned _child_count;
    mutable bool _cached_positions_valid;
    Node *_first_child;
    Node *_last_child;

    CompositeNodeObserver _observers;
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

/** @file
 * @brief GC-managed XML node implementation
 */
/* Copyright 2004-2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 */

#ifndef SEEN_INKSCAPE_XML_NODE_H
#error  You have included xml/simple-node.h in your document, which is an implementation.  Chances are that you want xml/node.h.  Please fix that.
#endif

#ifndef SEEN_INKSCAPE_XML_SIMPLE_NODE_H
#define SEEN_INKSCAPE_XML_SIMPLE_NODE_H

#include <cassert>
#include <iostream>

#include "xml/node.h"
#include "xml/attribute-record.h"
#include "xml/composite-node-observer.h"
#include "util/list-container.h"

namespace Inkscape {

namespace XML {

/**
 * @brief Default implementation of the XML node stored in memory.
 *
 * @see Inkscape::XML::Node
 */
class SimpleNode
: virtual public Node, public Inkscape::GC::Managed<>
{
public:
    char const *name() const;
    int code() const { return _name; }
    void setCodeUnsafe(int code) {
        _name = code;
    }

    Document *document() { return _document; }
    Document const *document() const {
        return const_cast<SimpleNode *>(this)->document();
    }

    Node *duplicate(Document* doc) const { return _duplicate(doc); }

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

    char const *attribute(char const *key) const;
    void setAttribute(char const *key, char const *value, bool is_interactive=false);
    bool matchAttributeName(char const *partial_name) const;

    char const *content() const;
    void setContent(char const *value);

    void mergeFrom(Node const *src, char const *key);

    Inkscape::Util::List<AttributeRecord const> attributeList() const {
        return _attributes;
    }

    void synthesizeEvents(NodeEventVector const *vector, void *data);
    void synthesizeEvents(NodeObserver &observer);

    void addListener(NodeEventVector const *vector, void *data) {
        assert(vector != NULL);
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

    void addSubtreeObserver(NodeObserver &observer) {
        _subtree_observers.add(observer);
    }
    void removeSubtreeObserver(NodeObserver &observer) {
        _subtree_observers.remove(observer);
    }

    void recursivePrintTree(unsigned level = 0);

protected:
    SimpleNode(int code, Document *document);
    SimpleNode(SimpleNode const &repr, Document *document);

    virtual SimpleNode *_duplicate(Document *doc) const=0;

private:
    void operator=(Node const &); // no assign

    void _setParent(SimpleNode *parent);
    unsigned _childPosition(SimpleNode const &child) const;

    SimpleNode *_parent;
    SimpleNode *_next;
    Document *_document;
    mutable unsigned _cached_position;

    int _name;

    Inkscape::Util::MutableList<AttributeRecord> _attributes;

    Inkscape::Util::ptr_shared<char> _content;

    unsigned _child_count;
    mutable bool _cached_positions_valid;
    SimpleNode *_first_child;
    SimpleNode *_last_child;

    CompositeNodeObserver _observers;
    CompositeNodeObserver _subtree_observers;
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

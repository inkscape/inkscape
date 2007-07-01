/*
 * XML::Subtree - proxy for an XML subtree
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

#include "xml/node.h"
#include "xml/subtree.h"
#include "xml/node-iterators.h"

namespace Inkscape {
namespace XML {

namespace {

void recursively(void (Node::*m)(NodeObserver &observer),
                 Node &node, NodeObserver &observer)
{
    (node.*m)(observer);
    for ( NodeSiblingIterator iter = node.firstChild() ; iter ; ++iter ) {
        recursively(m, *iter, observer);
    }
}

}

Subtree::Subtree(Node &root) : _root(&root) {
    recursively(&Node::addObserver, *_root, *this);
}

Subtree::~Subtree() {
    finish();
}

void Subtree::finish() {
    if (_root) {
        recursively(&Node::removeObserver, *_root, *this);
        _root = NULL;
    }
}

void Subtree::synthesizeEvents(NodeObserver &observer) {
    if (_root) {
        recursively(&Node::synthesizeEvents, *_root, *this);
    }
}

void Subtree::addObserver(NodeObserver &observer) {
    _observers.add(observer);
}

void Subtree::removeObserver(NodeObserver &observer) {
    _observers.remove(observer); 
}

void Subtree::notifyChildAdded(Node &node, Node &child, Node *prev) {
    recursively(&Node::addObserver, child, *this);
    _observers.notifyChildAdded(node, child, prev);
}

void Subtree::notifyChildRemoved(Node &node, Node &child, Node *prev) {
    recursively(&Node::removeObserver, child, *this);
    _observers.notifyChildRemoved(node, child, prev);
}

void Subtree::notifyChildOrderChanged(Node &node, Node &child,
                                      Node *old_prev, Node *new_prev)
{
    _observers.notifyChildOrderChanged(node, child, old_prev, new_prev);
}

void Subtree::notifyContentChanged(Node &node,
                                   Util::ptr_shared<char> old_content,
                                   Util::ptr_shared<char> new_content)
{
    _observers.notifyContentChanged(node, old_content, new_content);
}

void Subtree::notifyAttributeChanged(Node &node, GQuark name,
                                     Util::ptr_shared<char> old_value,
                                     Util::ptr_shared<char> new_value)
{
    _observers.notifyAttributeChanged(node, name, old_value, new_value);
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

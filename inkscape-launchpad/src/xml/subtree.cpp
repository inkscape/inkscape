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

Subtree::Subtree(Node &root) : _root(root) {
    _root.addSubtreeObserver(_observers);
}

Subtree::~Subtree() {
    _root.removeSubtreeObserver(_observers);
}

namespace {

void synthesize_events_recursive(Node &node, NodeObserver &observer) {
    node.synthesizeEvents(observer);
    for ( NodeSiblingIterator iter = node.firstChild() ; iter ; ++iter ) {
        synthesize_events_recursive(*iter, observer);
    }
}

}

void Subtree::synthesizeEvents(NodeObserver &observer) {
    synthesize_events_recursive(_root, observer);
}

void Subtree::addObserver(NodeObserver &observer) {
    _observers.add(observer);
}

void Subtree::removeObserver(NodeObserver &observer) {
    _observers.remove(observer); 
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

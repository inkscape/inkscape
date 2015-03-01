/** @file
 * @brief Object representing a subtree of the XML document
 */
/* Copyright 2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_XML_SUBTREE_H
#define SEEN_INKSCAPE_XML_SUBTREE_H

#include "inkgc/gc-managed.h"
#include "xml/composite-node-observer.h"

namespace Inkscape {
namespace XML {

/**
 * @brief Represents a node and all its descendants
 *
 * This is a convenience object for node operations that affect all of the node's descendants.
 * Currently the only such operations are adding and removing subtree observers
 * and synthesizing events for the entire subtree.
 */
class Subtree : public GC::Managed<GC::SCANNED, GC::MANUAL> {
public:
    Subtree(Node &root);
    ~Subtree();

    /**
     * @brief Synthesize events for the entire subtree
     *
     * This method notifies the specified observer of node changes equivalent to creating
     * this subtree from scratch. The notifications recurse into the tree depth-first.
     * Currently this is the only method that provides extra functionality compared to
     * the public methods of Node.
     */
    void synthesizeEvents(NodeObserver &observer);
    /**
     * @brief Add an observer watching for subtree changes
     *
     * Equivalent to Node::addSubtreeObserver().
     */
    void addObserver(NodeObserver &observer);
    /**
     * @brief Add an observer watching for subtree changes
     *
     * Equivalent to Node::removeSubtreeObserver().
     */
    void removeObserver(NodeObserver &observer);

private:
    Node &_root;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

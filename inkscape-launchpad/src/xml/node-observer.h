/** @file
 * @brief Interface for XML node observers
 */
/* Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com> (documentation)
 *
 * Copyright 2005-2008 Authors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 */

#ifndef SEEN_INKSCAPE_XML_NODE_OBSERVER_H
#define SEEN_INKSCAPE_XML_NODE_OBSERVER_H

#include "util/share.h"
typedef unsigned int GQuark;

#ifndef INK_UNUSED
#define INK_UNUSED(x) ((void)(x))
#endif // INK_UNUSED

namespace Inkscape {
namespace XML {

class Node;

/**
 * @brief Interface for XML node observers
 *
 * This class defines an interface for objects that can receive
 * XML node state change notifications. The observer has to be registered using
 * the Node::addObserver() method to be notified of changes of this node only,
 * or using Node::addSubtreeObserver() to also receive notifications about its
 * descendants. All observer methods are called when the operations in question have
 * been completed, just before returning from the modifying methods.
 *
 * Be careful when e.g. changing an attribute of @c node in notifyAttributeChanged().
 * The method will be called again due to the XML modification performed in it. If you
 * don't take special precautions to ignore the second call, it will result in infinite
 * recursion.
 *
 * The virtual methods of this class do nothing by default, so you don't need to provide
 * stubs for things you don't use. A good idea is to make the observer register itself
 * on construction and unregister itself on destruction. This will ensure there are
 * no dangling references.
 */
class NodeObserver {
protected:
    /* the constructor is protected to prevent instantiation */
    NodeObserver() {}
public:
    virtual ~NodeObserver() {}

    // FIXME: somebody needs to learn what "pure virtual" means

    /**
     * @brief Child addition callback
     *
     * This method is called whenever a child is added to the observed node. The @c prev
     * parameter is NULL when the newly added child is first in the sibling order.
     *
     * @param node The changed XML node
     * @param child The newly added child node
     * @param prev The node after which the new child was inserted into the sibling order, or NULL
     */
    virtual void notifyChildAdded(Node &node, Node &child, Node *prev) {
        INK_UNUSED(node);
        INK_UNUSED(child);
        INK_UNUSED(prev);
    }

    /**
     * @brief Child removal callback
     *
     * This method is called whenever a child is removed from the observed node. The @c prev
     * parameter is NULL when the removed child was first in the sibling order.
     *
     * @param node The changed XML node
     * @param child The removed child node
     * @param prev The node that was before the removed node in sibling order, or NULL
     */
    virtual void notifyChildRemoved(Node &node, Node &child, Node *prev) {
        INK_UNUSED(node);
        INK_UNUSED(child);
        INK_UNUSED(prev);
    }

    /**
     * @brief Child order change callback
     *
     * This method is called whenever the order of a node's children is changed using
     * Node::changeOrder(). The @c old_prev parameter is NULL if the relocated node
     * was first in the sibling order before the order change, and @c new_prev is NULL
     * if it was moved to the first position by this operation.
     *
     * @param node The changed XML node
     * @param child The child node that was relocated in the sibling order
     * @param old_prev The node that was before @c child prior to the order change
     * @param new_prev The node that is before @c child after the order change
     */
    virtual void notifyChildOrderChanged(Node &node, Node &child,
                                         Node *old_prev, Node *new_prev) {
        INK_UNUSED(node);
        INK_UNUSED(child);
        INK_UNUSED(old_prev);
        INK_UNUSED(new_prev);
    }

    /**
     * @brief Content change callback
     *
     * This method is called whenever a node's content is changed using Node::setContent(),
     * e.g. for text or comment nodes.
     *
     * @param node The changed XML node
     * @param old_content Old content of @c node
     * @param new_content New content of @c node
     */
    virtual void notifyContentChanged(Node &node,
                                      Util::ptr_shared<char> old_content,
                                      Util::ptr_shared<char> new_content) {
        INK_UNUSED(node);
        INK_UNUSED(old_content);
        INK_UNUSED(new_content);
    }

    /**
     * @brief Attribute change callback
     *
     * This method is called whenever one of a node's attributes is changed.
     *
     * @param node The changed XML node
     * @param name GQuark corresponding to the attribute's name
     * @param old_value Old value of the modified attribute
     * @param new_value New value of the modified attribute
     */
    virtual void notifyAttributeChanged(Node &node, GQuark name,
                                        Util::ptr_shared<char> old_value,
                                        Util::ptr_shared<char> new_value) {
        INK_UNUSED(node);
        INK_UNUSED(name);
        INK_UNUSED(old_value);
        INK_UNUSED(new_value);
    }
};

} // namespace XML
} // namespace Inkscape

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

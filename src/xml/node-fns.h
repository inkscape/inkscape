/** @file
 * @brief Helper functions for XML nodes
 */
/* Authors:
 *   Unknown author
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

#ifndef SEEN_XML_NODE_FNS_H
#define SEEN_XML_NODE_FNS_H

#include "xml/node.h"

namespace Inkscape {
namespace XML {

bool id_permitted(Node const *node);

//@{
/**
 * @brief Get the next node in sibling order
 * @param node The origin node
 * @return The next node in sibling order
 * @relates Inkscape::XML::Node
 */
inline Node *next_node(Node *node) {
    return ( node ? node->next() : NULL );
}
inline Node const *next_node(Node const *node) {
    return ( node ? node->next() : NULL );
}
//@}

//@{
/**
 * @brief Get the previous node in sibling order
 *
 * This method, unlike Node::next(), is a linear search over the children of @c node's parent.
 * The return value is NULL when the node has no parent or is first in the sibling order.
 *
 * @param node The origin node
 * @return The previous node in sibling order, or NULL
 * @relates Inkscape::XML::Node
 */
Node *previous_node(Node *node);
inline Node const *previous_node(Node const *node) {
    return previous_node(const_cast<Node *>(node));
}
//@}

//@{
/**
 * @brief Get the node's parent
 * @param node The origin node
 * @return The node's parent
 * @relates Inkscape::XML::Node
 */
inline Node *parent_node(Node *node) {
    return ( node ? node->parent() : NULL );
}
inline Node const *parent_node(Node const *node) {
    return ( node ? node->parent() : NULL );
}
//@}

}
}

#endif /* !SEEN_XML_NODE_FNS_H */
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

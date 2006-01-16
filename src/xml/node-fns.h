#ifndef SEEN_REPR_GET_CHILDREN_H
#define SEEN_REPR_GET_CHILDREN_H

#include "xml/node.h"

namespace Inkscape {
namespace XML {

class Node;

bool id_permitted(Node const *node);

inline Node *next_node(Node *node) {
    return ( node ? node->next() : NULL );
}
inline Node const *next_node(Node const *node) {
    return ( node ? node->next() : NULL );
}
Node *previous_node(Node *node);
inline Node const *previous_node(Node const *node) {
    return previous_node(const_cast<Node *>(node));
}
inline Node *parent_node(Node *node) {
    return ( node ? node->parent() : NULL );
}
inline Node const *parent_node(Node const *node) {
    return ( node ? node->parent() : NULL );
}

}
}

#endif /* !SEEN_REPR_GET_CHILDREN_H */

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

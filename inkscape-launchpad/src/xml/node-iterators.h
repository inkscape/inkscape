/*
 * Node iterators
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_XML_SP_REPR_ITERATORS_H
#define SEEN_INKSCAPE_XML_SP_REPR_ITERATORS_H

#include "util/forward-pointer-iterator.h"
#include "xml/node.h"

namespace Inkscape {
namespace XML {

struct NodeSiblingIteratorStrategy {
    static Node const *next(Node const *node) {
        return ( node ? node->next() : NULL );
    }
};

struct NodeParentIteratorStrategy {
    static Node const *next(Node const *node) {
        return ( node ? node->parent() : NULL );
    }
};

typedef Inkscape::Util::ForwardPointerIterator<Node,
                                               NodeSiblingIteratorStrategy>
        NodeSiblingIterator;

typedef Inkscape::Util::ForwardPointerIterator<Node const,
                                               NodeSiblingIteratorStrategy>
        NodeConstSiblingIterator;

typedef Inkscape::Util::ForwardPointerIterator<Node,
                                               NodeParentIteratorStrategy>
        NodeParentIterator;

typedef Inkscape::Util::ForwardPointerIterator<Node const,
                                               NodeParentIteratorStrategy>
        NodeConstParentIterator;

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

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

#ifndef SEEN_INKSCAPE_XML_SUBTREE_H
#define SEEN_INKSCAPE_XML_SUBTREE_H

#include "xml/node-observer.h"
#include "xml/composite-node-observer.h"
#include "gc-finalized.h"

namespace Inkscape {
namespace XML {

class Node;

class Subtree : public GC::Managed<GC::SCANNED, GC::MANUAL>,
                private NodeObserver
{
public:
    Subtree(Node &root);
    ~Subtree();

    void synthesizeEvents(NodeObserver &observer);
    void addObserver(NodeObserver &observer);
    void removeObserver(NodeObserver &observer);

private:
    void notifyChildAdded(Node &node, Node &child, Node *prev);

    void notifyChildRemoved(Node &node, Node &child, Node *prev);

    void notifyChildOrderChanged(Node &node, Node &child,
                                 Node *old_prev, Node *new_prev);

    void notifyContentChanged(Node &node,
                              Util::ptr_shared<char> old_content,
                              Util::ptr_shared<char> new_content);

    void notifyAttributeChanged(Node &node, GQuark name,
                                Util::ptr_shared<char> old_value,
                                Util::ptr_shared<char> new_value);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

/*
 * Inkscape::XML::NodeObserver - interface implemented by observers of XML nodes
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

#ifndef SEEN_INKSCAPE_XML_NODE_OBSERVER_H
#define SEEN_INKSCAPE_XML_NODE_OBSERVER_H

#include <glib/gquark.h>
#include "util/share.h"

namespace Inkscape {
namespace XML {
class Node;
}
}


namespace Inkscape {

namespace XML {

class NodeObserver {
public:
    NodeObserver() {}
    
    virtual ~NodeObserver() {}
    
    virtual void notifyChildAdded(Node &node, Node &child, Node *prev)=0;

    virtual void notifyChildRemoved(Node &node, Node &child, Node *prev)=0;

    virtual void notifyChildOrderChanged(Node &node, Node &child,
                                         Node *old_prev, Node *new_prev)=0;

    virtual void notifyContentChanged(Node &node,
                                      Util::ptr_shared<char> old_content,
                                      Util::ptr_shared<char> new_content)=0;

    virtual void notifyAttributeChanged(Node &node, GQuark name,
                                        Util::ptr_shared<char> old_value,
                                        Util::ptr_shared<char> new_value)=0;
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

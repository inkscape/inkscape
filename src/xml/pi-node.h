/*
 * Inkscape::XML::PINode - simple XML comment implementation
 *
 * Copyright 2004-2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_XML_PI_NODE_H
#define SEEN_INKSCAPE_XML_PI_NODE_H

#include <glib/gquark.h>
#include "xml/simple-node.h"

namespace Inkscape {

namespace XML {

struct PINode : public SimpleNode {
    explicit PINode(GQuark target, Util::ptr_shared<char> content)
    : SimpleNode(target)
    {
        setContent(content);
    }

    Inkscape::XML::NodeType type() const { return Inkscape::XML::PI_NODE; }

protected:
    SimpleNode *_duplicate(Document* /*doc*/) const { return new PINode(*this); }
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

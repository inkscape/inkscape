/** @file
 * @brief Processing instruction node implementation
 */
/* Copyright 2004-2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 */

#ifndef SEEN_INKSCAPE_XML_PI_NODE_H
#define SEEN_INKSCAPE_XML_PI_NODE_H

#include "xml/simple-node.h"

namespace Inkscape {

namespace XML {

/**
 * @brief Processing instruction node, e.g. &lt;?xml version="1.0" encoding="utf-8" standalone="no"?&gt;
 */
struct PINode : public SimpleNode {
    PINode(GQuark target, Util::ptr_shared<char> content, Document *doc)
    : SimpleNode(target, doc)
    {
        setContent(content);
    }
    PINode(PINode const &other, Document *doc)
    : SimpleNode(other, doc) {}

    Inkscape::XML::NodeType type() const { return Inkscape::XML::PI_NODE; }

protected:
    SimpleNode *_duplicate(Document* doc) const { return new PINode(*this, doc); }
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

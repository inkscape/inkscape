/** @file
 * @brief Element node implementation
 */
/* Copyright 2004-2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_XML_ELEMENT_NODE_H
#define SEEN_INKSCAPE_XML_ELEMENT_NODE_H

#include "xml/simple-node.h"

namespace Inkscape {

namespace XML {

/**
 * @brief Element node, e.g. &lt;group /&gt;
 */
class ElementNode : public SimpleNode {
public:
    ElementNode(int code, Document *doc)
    : SimpleNode(code, doc) {}
    ElementNode(ElementNode const &other, Document *doc)
    : SimpleNode(other, doc) {}

    Inkscape::XML::NodeType type() const { return Inkscape::XML::ELEMENT_NODE; }

protected:
    SimpleNode *_duplicate(Document* doc) const { return new ElementNode(*this, doc); }
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

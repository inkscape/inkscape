/** @file
 * @brief Text node implementation
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

#ifndef SEEN_INKSCAPE_XML_TEXT_NODE_H
#define SEEN_INKSCAPE_XML_TEXT_NODE_H

#include <glib.h>
#include "xml/simple-node.h"

namespace Inkscape {

namespace XML {

/**
 * @brief Text node, e.g. "Some text" in &lt;group&gt;Some text&lt;/group&gt;
 */
struct TextNode : public SimpleNode {
    TextNode(Util::ptr_shared<char> content, Document *doc)
    : SimpleNode(g_quark_from_static_string("string"), doc)
    {
        setContent(content);
        _is_CData = false;
    }
    TextNode(Util::ptr_shared<char> content, Document *doc, bool is_CData)
    : SimpleNode(g_quark_from_static_string("string"), doc)
    {
        setContent(content);
        _is_CData = is_CData;
    }
    TextNode(TextNode const &other, Document *doc)
    : SimpleNode(other, doc) {
      _is_CData = other._is_CData;
    }

    Inkscape::XML::NodeType type() const { return Inkscape::XML::TEXT_NODE; }
    bool is_CData() const { return _is_CData; }

protected:
    SimpleNode *_duplicate(Document* doc) const { return new TextNode(*this, doc); }
    bool _is_CData;
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

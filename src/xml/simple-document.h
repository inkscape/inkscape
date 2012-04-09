/*
 * Inkscape::XML::SimpleDocument - generic XML document implementation
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

#ifndef SEEN_INKSCAPE_XML_SIMPLE_DOCUMENT_H
#define SEEN_INKSCAPE_XML_SIMPLE_DOCUMENT_H

#include "xml/document.h"
#include "xml/simple-node.h"
#include "xml/node-observer.h"
#include "xml/log-builder.h"

namespace Inkscape {

namespace XML {

class SimpleDocument : public SimpleNode,
                       public Document,
                       public NodeObserver
{
public:
    explicit SimpleDocument()
    : SimpleNode(g_quark_from_static_string("xml"), this),
      _in_transaction(false), _is_CData(false) {}

    NodeType type() const { return Inkscape::XML::DOCUMENT_NODE; }

    bool inTransaction() { return _in_transaction; }

    void beginTransaction();
    void rollback();
    void commit();
    Inkscape::XML::Event *commitUndoable();

    Node *createElement(char const *name);
    Node *createTextNode(char const *content);
    Node *createTextNode(char const *content, bool const is_CData);
    Node *createComment(char const *content);
    Node *createPI(char const *target, char const *content);

    void notifyChildAdded(Node &parent, Node &child, Node *prev);

    void notifyChildRemoved(Node &parent, Node &child, Node *prev);

    void notifyChildOrderChanged(Node &parent, Node &child,
                                 Node *old_prev, Node *new_prev);

    void notifyContentChanged(Node &node,
                              Util::ptr_shared<char> old_content,
                              Util::ptr_shared<char> new_content);

    void notifyAttributeChanged(Node &node, GQuark name,
                                Util::ptr_shared<char> old_value,
                                Util::ptr_shared<char> new_value);

protected:
    SimpleDocument(SimpleDocument const &doc)
    : Node(), SimpleNode(doc), Document(), NodeObserver(),
      _in_transaction(false),
      _is_CData(false){}

    SimpleNode *_duplicate(Document* /*doc*/) const
    {
        return new SimpleDocument(*this);
    }
    NodeObserver *logger() { return this; }

private:
    bool _in_transaction;
    LogBuilder _log_builder;
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

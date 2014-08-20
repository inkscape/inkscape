/**
 * @file
 * Garbage collected XML document implementation.
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

#include <glib.h> // g_assert()

#include "xml/simple-document.h"
#include "xml/event-fns.h"
#include "xml/element-node.h"
#include "xml/text-node.h"
#include "xml/comment-node.h"
#include "xml/pi-node.h"

namespace Inkscape {

namespace XML {

void SimpleDocument::beginTransaction() {
    g_assert(!_in_transaction);
    _in_transaction = true;
}

void SimpleDocument::rollback() {
    g_assert(_in_transaction);
    _in_transaction = false;
    Event *log = _log_builder.detach();
    sp_repr_undo_log(log);
    sp_repr_free_log(log);
}

void SimpleDocument::commit() {
    g_assert(_in_transaction);
    _in_transaction = false;
    _log_builder.discard();
}

Inkscape::XML::Event *SimpleDocument::commitUndoable() {
    g_assert(_in_transaction);
    _in_transaction = false;
    return _log_builder.detach();
}

Node *SimpleDocument::createElement(char const *name) {
    return new ElementNode(g_quark_from_string(name), this);
}

Node *SimpleDocument::createTextNode(char const *content) {
    return new TextNode(Util::share_string(content), this);
}

Node *SimpleDocument::createTextNode(char const *content, bool const is_CData) {
    return new TextNode(Util::share_string(content), this, is_CData);
}

Node *SimpleDocument::createComment(char const *content) {
    return new CommentNode(Util::share_string(content), this);
}

Node *SimpleDocument::createPI(char const *target, char const *content) {
    return new PINode(g_quark_from_string(target), Util::share_string(content), this);
}

void SimpleDocument::notifyChildAdded(Node &parent,
                                      Node &child,
                                      Node *prev)
{
    if (_in_transaction) {
        _log_builder.addChild(parent, child, prev);
    }
}

void SimpleDocument::notifyChildRemoved(Node &parent,
                                        Node &child,
                                        Node *prev)
{
    if (_in_transaction) {
        _log_builder.removeChild(parent, child, prev);
    }
}

void SimpleDocument::notifyChildOrderChanged(Node &parent,
                                             Node &child,
                                             Node *old_prev,
                                             Node *new_prev)
{
    if (_in_transaction) {
        _log_builder.setChildOrder(parent, child, old_prev, new_prev);
    }
}

void SimpleDocument::notifyContentChanged(Node &node,
                                          Util::ptr_shared<char> old_content,
                                          Util::ptr_shared<char> new_content)
{
    if (_in_transaction) {
        _log_builder.setContent(node, old_content, new_content);
    }
}

void SimpleDocument::notifyAttributeChanged(Node &node,
                                            GQuark name,
                                            Util::ptr_shared<char> old_value,
                                            Util::ptr_shared<char> new_value)
{
    if (_in_transaction) {
        _log_builder.setAttribute(node, name, old_value, new_value);
    }
}

}

}

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

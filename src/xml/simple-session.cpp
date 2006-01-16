/*
 * Inkscape::XML::SimpleSession - simple session/logging implementation
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

#include "xml/simple-session.h"
#include "xml/event-fns.h"
#include "xml/element-node.h"
#include "xml/text-node.h"
#include "xml/comment-node.h"

namespace Inkscape {

namespace XML {

void SimpleSession::beginTransaction() {
    g_assert(!_in_transaction);
    _in_transaction = true;
}

void SimpleSession::rollback() {
    g_assert(_in_transaction);
    _in_transaction = false;
    Event *log = _log_builder.detach();
    sp_repr_undo_log(log);
    sp_repr_free_log(log);
}

void SimpleSession::commit() {
    g_assert(_in_transaction);
    _in_transaction = false;
    _log_builder.discard();
}

Inkscape::XML::Event *SimpleSession::commitUndoable() {
    g_assert(_in_transaction);
    _in_transaction = false;
    return _log_builder.detach();
}

Node *SimpleSession::createElementNode(char const *name) {
    return new ElementNode(g_quark_from_string(name));
}

Node *SimpleSession::createTextNode(char const *content) {
    return new TextNode(Util::SharedCStringPtr::copy(content));
}

Node *SimpleSession::createCommentNode(char const *content) {
    return new CommentNode(Util::SharedCStringPtr::copy(content));
}

void SimpleSession::notifyChildAdded(Node &parent,
                                     Node &child,
                                     Node *prev)
{
    if (_in_transaction) {
        _log_builder.addChild(parent, child, prev);
    }
}

void SimpleSession::notifyChildRemoved(Node &parent,
                                       Node &child,
                                       Node *prev)
{
    if (_in_transaction) {
        _log_builder.removeChild(parent, child, prev);
    }
}

void SimpleSession::notifyChildOrderChanged(Node &parent,
                                            Node &child,
                                            Node *old_prev,
                                            Node *new_prev)
{
    if (_in_transaction) {
        _log_builder.setChildOrder(parent, child, old_prev, new_prev);
    }
}

void SimpleSession::notifyContentChanged(Node &node,
                                         Util::SharedCStringPtr old_content,
                                         Util::SharedCStringPtr new_content)
{
    if (_in_transaction) {
        _log_builder.setContent(node, old_content, new_content);
    }
}

void SimpleSession::notifyAttributeChanged(Node &node,
                                           GQuark name,
                                           Util::SharedCStringPtr old_value,
                                           Util::SharedCStringPtr new_value)
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

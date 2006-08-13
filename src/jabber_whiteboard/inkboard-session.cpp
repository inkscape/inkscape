/**
 * Inkscape::Whiteboard::InkboardSession - Whiteboard implementation of XML::Session
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm.h>
#include <glib/gmessages.h>
#include <glib/gquark.h>

#include "jabber_whiteboard/inkboard-session.h"
#include "jabber_whiteboard/inkboard-document.h"
#include "jabber_whiteboard/inkboard-node.h"
#include "jabber_whiteboard/defines.h"

#include "xml/node.h"
#include "xml/event.h"
#include "xml/element-node.h"
#include "xml/text-node.h"
#include "xml/comment-node.h"

#include "util/share.h"

namespace Inkscape {

namespace Whiteboard {

using XML::Node;

void
InkboardSession::beginTransaction()
{
    g_assert(!_in_transaction);
    _in_transaction = true;
}

void
InkboardSession::rollback()
{
    g_assert(_in_transaction);
    _in_transaction = false;
}

void 
InkboardSession::commit()
{
    g_assert(_in_transaction);
    _in_transaction = false;
}

XML::Event*
InkboardSession::commitUndoable()
{
    g_assert(_in_transaction);
    _in_transaction = false;
    return NULL;
}

XML::Node*
InkboardSession::createElementNode(char const* name)
{
    g_log(NULL, G_LOG_LEVEL_DEBUG, "InkboardSession::createElementNode");
    return new InkboardNode(g_quark_from_string(name),Inkscape::XML::ELEMENT_NODE);
}

XML::Node*
InkboardSession::createTextNode(char const* content)
{
    g_log(NULL, G_LOG_LEVEL_DEBUG, "InkboardSession::createTextNode");
    return new XML::TextNode(Util::share_string(content));
}

XML::Node*
InkboardSession::createCommentNode(char const* content)
{
    g_log(NULL, G_LOG_LEVEL_DEBUG, "InkboardSession::createCommentNode");
    return new XML::CommentNode(Util::share_string(content));
}


void InkboardSession::notifyChildAdded(Node &parent,
                                     Node &child,
                                     Node *prev)
{
    if (_in_transaction && doc->state == State::IN_WHITEBOARD) {

        InkboardNode *node = dynamic_cast< InkboardNode* >((XML::Node *)&child);
        if(node == NULL)
        {
            g_warning("non inkboard node"); 
            return;
        }

        this->doc->addNodeToTracker(node);
        Message::Message message = this->doc->composeNewMessage(node);

        this->doc->send(this->doc->getRecipient(),Message::NEW,message);
    }
}

void InkboardSession::notifyChildRemoved(Node &parent,
                                       Node &child,
                                       Node *prev)
{
    if (_in_transaction && doc->state == State::IN_WHITEBOARD) {
        g_warning("child removed");
   }
}

void InkboardSession::notifyChildOrderChanged(Node &parent,
                                            Node &child,
                                            Node *old_prev,
                                            Node *new_prev)
{
    if (_in_transaction && doc->state == State::IN_WHITEBOARD) {
        g_warning("child reordered");
    }
}

void InkboardSession::notifyContentChanged(Node &node,
                                         Util::ptr_shared<char> old_content,
                                         Util::ptr_shared<char> new_content)
{
    if (_in_transaction && doc->state == State::IN_WHITEBOARD) {
        g_warning("content changed");
    }
}

void InkboardSession::notifyAttributeChanged(Node &node,
                                           GQuark name,
                                           Util::ptr_shared<char> old_value,
                                           Util::ptr_shared<char> new_value)
{
    if (_in_transaction && doc->state == State::IN_WHITEBOARD) {

    }
}

}

}

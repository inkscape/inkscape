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
#include "jabber_whiteboard/defines.h"

#include "xml/node.h"
#include "xml/event.h"
#include "xml/element-node.h"
#include "xml/text-node.h"
#include "xml/comment-node.h"

#include "util/share.h"
#include "util/ucompose.hpp"

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
    return new XML::ElementNode(g_quark_from_string(name));
}

XML::Node*
InkboardSession::createTextNode(char const* content)
{
    return new XML::TextNode(Util::share_string(content));
}

XML::Node*
InkboardSession::createCommentNode(char const* content)
{
    return new XML::CommentNode(Util::share_string(content));
}


void InkboardSession::notifyChildAdded(Node &parent,
                                     Node &child,
                                     Node *prev)
{
    if (_in_transaction && doc->state == State::IN_WHITEBOARD) {

        XML::Node *node = (XML::Node *)&child;

        if(this->doc->tracker->get(node) == "")
        {
            this->doc->addNodeToTracker(node);
            Message::Message message = this->doc->composeNewMessage(node);

            this->doc->send(this->doc->getRecipient(),Message::NEW,message);
        }
    }
}

void InkboardSession::notifyChildRemoved(Node &parent,
                                       Node &child,
                                       Node *prev)
{
    if (_in_transaction && doc->state == State::IN_WHITEBOARD) 
    {
        XML::Node *element = (XML::Node *)&child;

        Message::Message message = String::ucompose(Vars::REMOVE_MESSAGE,
            this->doc->tracker->get(element));

        this->doc->send(this->doc->getRecipient(),Message::REMOVE,message);
   }
}

void InkboardSession::notifyChildOrderChanged(Node &parent,
                                            Node &child,
                                            Node *old_prev,
                                            Node *new_prev)
{
    if (_in_transaction && doc->state == State::IN_WHITEBOARD) 
    {
        XML::Node *element = (XML::Node *)&child;
        XML::Node *parentElement = (XML::Node *)&parent;

        unsigned int index = parentElement->_childPosition(*element);

        Message::Message message = String::ucompose(Vars::MOVE_MESSAGE,
                this->doc->tracker->get(element),index);

        this->doc->send(this->doc->getRecipient(),Message::MOVE,message);
    }
}

void InkboardSession::notifyContentChanged(Node &node,
                                         Util::ptr_shared<char> old_content,
                                         Util::ptr_shared<char> new_content)
{
    if (_in_transaction && doc->state == State::IN_WHITEBOARD) 
    {
        XML::Node *element = (XML::Node *)&node;

        if(new_content.pointer())
        {
            unsigned int version = this->doc->tracker->incrementVersion(element);

            Message::Message message = String::ucompose(Vars::CONFIGURE_TEXT_MESSAGE,
                this->doc->tracker->get(element),version,new_content.pointer());

            this->doc->send(this->doc->getRecipient(),Message::CONFIGURE,message);
        }
    }
}

void InkboardSession::notifyAttributeChanged(Node &node,
                                           GQuark name,
                                           Util::ptr_shared<char> old_value,
                                           Util::ptr_shared<char> new_value)
{
    if (_in_transaction && doc->state == State::IN_WHITEBOARD) 
    {
        XML::Node *element = (XML::Node *)&node;

        Glib::ustring value(new_value.pointer());
        Glib::ustring attribute(g_quark_to_string(name));

        Configure change = this->doc->tracker->getLastHistory(element);

        if(change.first == attribute && change.second == value)
            return;

        if(name && new_value.pointer())
        {
            unsigned int version = this->doc->tracker->incrementVersion(element);

            Message::Message message = String::ucompose(Vars::CONFIGURE_MESSAGE,
                this->doc->tracker->get(element),version,attribute.c_str(),value.c_str());

            this->doc->send(this->doc->getRecipient(),Message::CONFIGURE,message);
        }
    }
}

}

}

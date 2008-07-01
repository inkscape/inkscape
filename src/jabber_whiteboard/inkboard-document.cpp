/**
 * Inkscape::Whiteboard::InkboardDocument - Inkboard document implementation
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <glibmm.h>

#include "jabber_whiteboard/inkboard-document.h"

#include "util/ucompose.hpp"

#include "jabber_whiteboard/message-utilities.h"
#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/node-tracker.h"

#include <glibmm.h>
#include <glib/gmessages.h>
#include <glib/gquark.h>

#include "jabber_whiteboard/inkboard-document.h"
#include "jabber_whiteboard/defines.h"

#include "xml/node.h"
#include "xml/event.h"
#include "xml/element-node.h"
#include "xml/text-node.h"
#include "xml/comment-node.h"
#include "xml/pi-node.h"

#include "util/share.h"
#include "util/ucompose.hpp"

namespace Inkscape {

namespace Whiteboard {

InkboardDocument::InkboardDocument(int code, State::SessionType sessionType,
                                   Glib::ustring const& to)
: XML::SimpleNode(code, this), sessionType(sessionType), recipient(to),
  _in_transaction(false)
{
    _initBindings();
}

void
InkboardDocument::_initBindings()
{
    this->sm = &SessionManager::instance();
    this->state = State::INITIAL;
    this->tracker = new KeyNodeTable();
}

void
InkboardDocument::setRecipient(Glib::ustring const& val)
{
    this->recipient = val;
}

Glib::ustring 
InkboardDocument::getRecipient() const
{
    return this->recipient;
}

void
InkboardDocument::setSessionId(Glib::ustring const& val)
{
    this->sessionId = val;
}

Glib::ustring 
InkboardDocument::getSessionId() const
{
    return this->sessionId;
}

void
InkboardDocument::startSessionNegotiation()
{
    if(this->sessionType == State::WHITEBOARD_PEER)
        this->send(recipient, Message::PROTOCOL,Message::CONNECT_REQUEST);

    else if(this->sessionType == State::WHITEBOARD_MUC)
    {
        // Check that the MUC room is whiteboard enabled, if not no need to send 
        // anything, just set the room to be whiteboard enabled
    }
}

void
InkboardDocument::terminateSession()
{

}

void
InkboardDocument::recieve(Message::Wrapper &wrapper, Pedro::Element* data)
{
    if(this->handleIncomingState(wrapper,data))
    {
        if(wrapper == Message::PROTOCOL)
        {
            Glib::ustring message = data->getFirstChild()->getFirstChild()->getFirstChild()->getName();

            if(message == Message::CONNECT_REQUEST)
            {
                // An MUC member requesting document

            }else if(message == Message::ACCEPT_INVITATION)
            {
                // TODO : Would be nice to create the desktop here

                this->send(getRecipient(),Message::PROTOCOL, Message::CONNECTED);
                this->send(getRecipient(),Message::PROTOCOL, Message::DOCUMENT_BEGIN);

                // Send Document
                this->sendDocument(this->root());

                this->send(getRecipient(),Message::PROTOCOL, Message::DOCUMENT_END);

            }else if(message == Message::DECLINE_INVITATION)
            {
                this->sm->terminateSession(this->getSessionId());
            }
        }else if(wrapper == Message::NEW || wrapper == Message::CONFIGURE
                    || wrapper == Message::MOVE || wrapper == Message::REMOVE )
        {
            handleChange(wrapper,data->getFirstChild()->getFirstChild());
        }
    }else{
        g_warning("Recieved Message in invalid state = %d", this->state);
        data->print();
    }
}

bool
InkboardDocument::send(const Glib::ustring &destJid, Message::Wrapper &wrapper, Message::Message &message)
{
    if(this->handleOutgoingState(wrapper,message))
    {
        Glib::ustring mes;
        if(wrapper == Message::PROTOCOL)
            mes = String::ucompose(Vars::PROTOCOL_MESSAGE,wrapper,message);
        else
            mes = message;

        char *finalmessage = const_cast<char* >(String::ucompose(
            Vars::WHITEBOARD_MESSAGE, this->sessionType, this->sm->getClient().getJid(),
            destJid, Vars::INKBOARD_XMLNS, this->getSessionId(), mes).c_str());

        if (!this->sm->getClient().write("%s",finalmessage)) 
            { return false; }
        else 
            { return true; }

    }else 
    { 
        g_warning("Sending Message in invalid state message=%s , state=%d",message.c_str(),this->state);
        return false; 
    }
}

void
InkboardDocument::sendDocument(Inkscape::XML::Node* root)
{
    for(Inkscape::XML::Node *child = root->firstChild();child!=NULL;child=child->next())
    {
        Glib::ustring name(child->name());

        if(name != "svg:metadata" && name != "svg:defs" && name != "sodipodi:namedview")
        {
            Glib::ustring parentKey,tempParentKey,key;

            this->addNodeToTracker(child);
            Message::Message message = this->composeNewMessage(child);

            this->send(this->getRecipient(),Message::NEW,message);

            if(child->childCount() != 0)
            {
                sendDocument(child);
            }
        }
    }
}

bool
InkboardDocument::handleOutgoingState(Message::Wrapper &wrapper, Glib::ustring const& message)
{
    if(wrapper == Message::PROTOCOL) 
    {
        if(message == Message::CONNECT_REQUEST) 
            return this->handleState(State::INITIAL,State::AWAITING_INVITATION_REPLY);

        else if(message == Message::ACCEPT_INVITATION)
            return this->handleState(State::CONNECTING,State::AWAITING_CONNECTED);

        else if(message == Message::CONNECTED)
            return this->handleState(State::INVITATION_RECIEVED,State::CONNECTED);

        else if(message == Message::DOCUMENT_BEGIN)
            return this->handleState(State::CONNECTED,State::SYNCHRONISING);

        else if(message == Message::DOCUMENT_END) { 
            return this->handleState(State::SYNCHRONISING,State::IN_WHITEBOARD);
        }

        else 
            return false;

    } else 
        if(this->state == State::SYNCHRONISING && wrapper == Message::NEW)
            return true;

    return this->state == State::IN_WHITEBOARD;
}

bool
InkboardDocument::handleIncomingState(Message::Wrapper &wrapper, Pedro::Element* data)
{
    if(wrapper == Message::PROTOCOL) 
    {
        Glib::ustring message = data->getFirstChild()->getFirstChild()->getFirstChild()->getName();

        if(message == Message::CONNECT_REQUEST)
            return this->handleState(State::INITIAL,State::CONNECTING);
        if(message == Message::ACCEPT_INVITATION)
            return this->handleState(State::AWAITING_INVITATION_REPLY,State::INVITATION_RECIEVED);

        else if(message == Message::CONNECTED)
            return this->handleState(State::AWAITING_CONNECTED,State::AWAITING_DOCUMENT_BEGIN);

        else if(message == Message::DOCUMENT_BEGIN)
            return this->handleState(State::AWAITING_DOCUMENT_BEGIN,State::SYNCHRONISING);

        else if(message == Message::DOCUMENT_END)
            return this->handleState(State::SYNCHRONISING,State::IN_WHITEBOARD);

        else 
            return false;

    } else 
        if(this->state == State::SYNCHRONISING && wrapper == Message::NEW)
            return true;

    return this->state == State::IN_WHITEBOARD;
}

bool 
InkboardDocument::handleState(State::SessionState expectedState, State::SessionState newState)
{
    if(this->state == expectedState)
    {
        this->state = newState;
        return true;
    }

    return false;
}


void
InkboardDocument::handleChange(Message::Wrapper &wrapper, Pedro::Element* data)
{
    if(wrapper == Message::NEW)
    {
        Glib::ustring parent =  data->getTagAttribute("new","parent");
        Glib::ustring id =      data->getTagAttribute("new","id");

        signed int index = atoi
            (data->getTagAttribute("new","index").c_str());

        Pedro::Element* element = data->getFirstChild();

        if(parent.size() > 0 && id.size() > 0)
            this->changeNew(parent,id,index,element);

    }else if(wrapper == Message::CONFIGURE)
    {
        if(data->exists("text"))
        {
            Glib::ustring text =    data->getFirstChild()->getValue();
            Glib::ustring target =  data->getTagAttribute("configure","target");

            unsigned int version = atoi
                (data->getTagAttribute("configure","version").c_str());

            if(text.size() > 0 && target.size() > 0)
                this->changeConfigureText(target,version,text);

        }else 
        {
            Glib::ustring target =      data->getTagAttribute("configure","target");
            Glib::ustring attribute =   data->getTagAttribute("configure","attribute");
            Glib::ustring value =       data->getTagAttribute("configure","value");

            unsigned int version = atoi
                (data->getTagAttribute("configure","version").c_str());

            if(target.size() > 0 && attribute.size() > 0 && value.size() > 0)
                this->changeConfigure(target,version,attribute,value);
        }
    }else if(wrapper == Message::MOVE)
    {
    }else if(wrapper == Message::REMOVE) 
    {
    }
}

void
InkboardDocument::beginTransaction()
{
    g_assert(!_in_transaction);
    _in_transaction = true;
}

void
InkboardDocument::rollback()
{
    g_assert(_in_transaction);
    _in_transaction = false;
}

void 
InkboardDocument::commit()
{
    g_assert(_in_transaction);
    _in_transaction = false;
}

XML::Event*
InkboardDocument::commitUndoable()
{
    g_assert(_in_transaction);
    _in_transaction = false;
    return NULL;
}

XML::Node*
InkboardDocument::createElement(char const* name)
{
    return new XML::ElementNode(g_quark_from_string(name), this);
}

XML::Node*
InkboardDocument::createTextNode(char const* content)
{
    return new XML::TextNode(Util::share_string(content), this);
}

XML::Node*
InkboardDocument::createComment(char const* content)
{
    return new XML::CommentNode(Util::share_string(content), this);
}

XML::Node*
InkboardDocument::createPI(char const *target, char const* content)
{
    return new XML::PINode(g_quark_from_string(target), Util::share_string(content), this);
}



void InkboardDocument::notifyChildAdded(XML::Node &/*parent*/,
                                        XML::Node &child,
                                        XML::Node */*prev*/)
{
    if (_in_transaction && state == State::IN_WHITEBOARD) {

        XML::Node *node = (XML::Node *)&child;

        if(tracker->get(node) == "")
        {
            addNodeToTracker(node);
            Message::Message message = composeNewMessage(node);

            send(getRecipient(),Message::NEW,message);
        }
    }
}

void InkboardDocument::notifyChildRemoved(XML::Node &/*parent*/,
                                          XML::Node &child,
                                          XML::Node */*prev*/)
{
    if (_in_transaction && state == State::IN_WHITEBOARD) 
    {
        XML::Node *element = (XML::Node *)&child;

        Message::Message message = String::ucompose(Vars::REMOVE_MESSAGE,
            tracker->get(element));

        send(getRecipient(),Message::REMOVE,message);
   }
}

void InkboardDocument::notifyChildOrderChanged(XML::Node &parent,
                                               XML::Node &child,
                                               XML::Node */*old_prev*/,
                                               XML::Node */*new_prev*/)
{
    if (_in_transaction && state == State::IN_WHITEBOARD) 
    {
        XML::Node *element = (XML::Node *)&child;
        XML::Node *parentElement = (XML::Node *)&parent;

        unsigned int index = parentElement->_childPosition(*element);

        Message::Message message = String::ucompose(Vars::MOVE_MESSAGE,
                tracker->get(element),index);

        send(getRecipient(),Message::MOVE,message);
    }
}

void InkboardDocument::notifyContentChanged(XML::Node &node,
                                            Util::ptr_shared<char> /*old_content*/,
                                            Util::ptr_shared<char> new_content)
{
    if (_in_transaction && state == State::IN_WHITEBOARD) 
    {
        XML::Node *element = (XML::Node *)&node;

        Glib::ustring value(new_content.pointer());

        Glib::ustring change = tracker->getLastHistory(element,"text");

        if(change.size() > 0 && change == value)
            return;

        if(new_content.pointer())
        {
            unsigned int version = tracker->incrementVersion(element);

            Message::Message message = String::ucompose(Vars::CONFIGURE_TEXT_MESSAGE,
                tracker->get(element),version,new_content.pointer());

            send(getRecipient(),Message::CONFIGURE,message);
        }
    }
}

void InkboardDocument::notifyAttributeChanged(XML::Node &node,
                                              GQuark name,
                                              Util::ptr_shared<char> /*old_value*/,
                                              Util::ptr_shared<char> new_value)
{
    if (_in_transaction && state == State::IN_WHITEBOARD) 
    {
        XML::Node *element = (XML::Node *)&node;

        Glib::ustring value(new_value.pointer());
        Glib::ustring attribute(g_quark_to_string(name));

        Glib::ustring change = tracker->getLastHistory(element,attribute);

        if(change.size() > 0 && change == value)
            return;

        if(attribute.size() > 0 && value.size() > 0)
        {
            unsigned int version = tracker->incrementVersion(element);

            Message::Message message = String::ucompose(Vars::CONFIGURE_MESSAGE,
                tracker->get(element),version,attribute.c_str(),value.c_str());

            send(getRecipient(),Message::CONFIGURE,message);
        }
    }
}

}

}

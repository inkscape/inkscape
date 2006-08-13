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

#include "xml/simple-session.h"
#include "jabber_whiteboard/inkboard-session.h"
#include "jabber_whiteboard/message-utilities.h"
#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/node-tracker.h"

namespace Inkscape {

namespace Whiteboard {

InkboardDocument::InkboardDocument(int code, State::SessionType sessionType, Glib::ustring const& to) :
	XML::SimpleNode(code), sessionType(sessionType), recipient(to)
{
    _initBindings();
}

void
InkboardDocument::_initBindings()
{
    this->sm = &SessionManager::instance();
    this->state = State::INITIAL;
    _bindDocument(*this);
    _bindLogger(*(new InkboardSession(this)));
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
                this->tracker = new KeyNodeTable();
                this->sendDocument(this->root());

                this->send(getRecipient(),Message::PROTOCOL, Message::DOCUMENT_END);

            }else if(message == Message::DECLINE_INVITATION)
            {
                this->sm->terminateSession(this->getSessionId());
            }
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

        char *finalmessage = const_cast<char* >(String::ucompose(Vars::WHITEBOARD_MESSAGE,
            this->sessionType,this->sm->getClient().getJid(),destJid,
            Vars::INKBOARD_XMLNS,this->getSessionId(),mes).c_str());

        if (!this->sm->getClient().write(finalmessage)) 
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

Glib::ustring
InkboardDocument::addNodeToTracker(Inkscape::XML::Node *node)
{
    Glib::ustring key = this->tracker->generateKey(this->getRecipient());
    this->tracker->put(key,node);
    return key;
}

Message::Message
InkboardDocument::composeNewMessage(Inkscape::XML::Node *node)
{
    Glib::ustring parentKey;
    Glib::ustring key = this->tracker->get(node);
    Inkscape::XML::Node *parent = node->parent();

    Glib::ustring tempParentKey = this->tracker->get(node->parent());
    if(tempParentKey.size() < 1)
        parentKey = Vars::DOCUMENT_ROOT_NODE;
    else
        parentKey = tempParentKey;

    unsigned int index = parent->_childPosition(*node);

    Message::Message nodeMessage = MessageUtilities::objectToString(node);
    Message::Message message = String::ucompose(Vars::NEW_MESSAGE,parentKey,key,index,nodeMessage);

    return message;
}


} // namespace Whiteboard
} // namespace Inkscape


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

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
    this->tracker = new KeyNodeTable();
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

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

#include "xml/simple-session.h"
#include "jabber_whiteboard/inkboard-session.h"
#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/session-manager.h"

namespace Inkscape {

namespace Whiteboard {

InkboardDocument::InkboardDocument(int code, State::SessionType type, Glib::ustring const& to) :
	XML::SimpleNode(code), _type(type), _recipient(to)
{
    _initBindings();
}

void
InkboardDocument::_initBindings()
{
    this->_sm = &SessionManager::instance();
    this->state = State::INITIAL;
    _bindDocument(*this);
    _bindLogger(*(new XML::SimpleSession()));
}

void
InkboardDocument::setRecipient(Glib::ustring const& val)
{
    this->_recipient = val;
}

Glib::ustring 
InkboardDocument::getRecipient() const
{
    return this->_recipient;
}

void
InkboardDocument::setSessionId(Glib::ustring const& val)
{
    this->_session = val;
}

Glib::ustring 
InkboardDocument::getSessionId() const
{
    return this->_session;
}

void
InkboardDocument::startSessionNegotiation()
{
    if(_type == State::WHITEBOARD_PEER)
        sendProtocol(_recipient, Message::PROTOCOL,Message::CONNECT_REQUEST);

    else if(_type == State::WHITEBOARD_MUC)
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
InkboardDocument::processInkboardEvent(Message::Wrapper &wrapper, Pedro::Element* data)
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

                sendProtocol(getRecipient(),Message::PROTOCOL, Message::CONNECTED);
                sendProtocol(getRecipient(),Message::PROTOCOL, Message::DOCUMENT_BEGIN);

                // Send the Document

                sendProtocol(getRecipient(),Message::PROTOCOL, Message::DOCUMENT_END);

            }else if(message == Message::DECLINE_INVITATION)
            {
                this->_sm->terminateSession(this->getSessionId());
            }
        }
    }else{
        g_warning("Recieved Message in invalid state = %d", this->state);
        data->print();
    }
}

bool
InkboardDocument::sendProtocol(const Glib::ustring &destJid, Message::Wrapper &wrapper,
     Message::Message &message)
{
    if(this->handleOutgoingState(wrapper,message))
    {
        char *fmt=
            "<message type='%s' from='%s' to='%s'>"
                "<wb xmlns='%s' session='%s'>"
                    "<%s>"
                        "<%s />"
                    "</%s>"
                "</wb>"
            "</message>";

        if (!_sm->getClient().write(fmt,
                _type.c_str(),_sm->getClient().getJid().c_str(),destJid.c_str(),Vars::INKBOARD_XMLNS.c_str(),
                this->getSessionId().c_str(),wrapper.c_str(),message.c_str(),wrapper.c_str())) 
            { return false; }

        else 
            { return true; }

    }else 
    { 
        g_warning("Sending Message in invalid state message=%s , state=%d",message.c_str(),this->state);
        return false; 
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

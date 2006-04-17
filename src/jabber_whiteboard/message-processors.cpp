/**
 * Whiteboard session manager
 * Jabber received message processors
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

extern "C" {
#include <loudmouth/loudmouth.h>
}

#include <glibmm/i18n.h>

#include "xml/session.h"
#include "xml/document.h"

#include "desktop-handles.h"
#include "document.h"
#include "message-stack.h"

#include "jabber_whiteboard/undo-stack-observer.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/message-node.h"
#include "jabber_whiteboard/message-queue.h"
#include "jabber_whiteboard/message-processors.h"
#include "jabber_whiteboard/typedefs.h"

namespace Inkscape {

namespace Whiteboard {

// Message processors are here!

// TODO: Remove unnecessary status checks from processors --
// we do all of that in MessageHandler::_hasValidReceiveContext

// *********************************************************************
// ChangeHandler begin
// *********************************************************************

/**
 * MessageProcessor for document change and event commit messages.
 */
struct ChangeHandler : public MessageProcessor {
public:
	~ChangeHandler()
	{

	}

	ChangeHandler(SessionManager* sm) : MessageProcessor(sm)
	{

	}

	LmHandlerResult
	operator()(MessageType mode, JabberMessage& p)
	{
		MessageNode* msgNode;
		bool chatroom = this->_sm->session_data->status[IN_CHATROOM];

		ReceiveMessageQueue* rmq = this->_sm->session_data->receive_queues[p.sender];

		if (rmq != NULL) {
			switch (mode) {
				case CHANGE_REPEATABLE:
				case CHANGE_NOT_REPEATABLE:
				case DOCUMENT_BEGIN:
					msgNode = new MessageNode(p.sequence, p.sender, "", p.body, mode, false, chatroom);
					rmq->insert(msgNode);
					Inkscape::GC::release(msgNode);
					break;
				case DOCUMENT_END:
					this->_sm->session_data->recipients_committed_queue.push_back(p.sender);
					msgNode = new MessageNode(p.sequence, p.sender, "", p.body, mode, false, chatroom);
					rmq->insert(msgNode);
					Inkscape::GC::release(msgNode);
					break;
				case CHANGE_COMMIT:
					this->_sm->session_data->recipients_committed_queue.push_back(p.sender);
					msgNode = new MessageNode(p.sequence, p.sender, "", p.body, CHANGE_COMMIT, false, chatroom);
					rmq->insert(msgNode);
					Inkscape::GC::release(msgNode);
					break;
				case DUMMY_CHANGE:
				default:
					break;
			}
		} else {
			g_warning("Received message from unknown sender %s", p.sender.c_str());
		}
		
		return LM_HANDLER_RESULT_REMOVE_MESSAGE;
	}
};
// *********************************************************************
// ChangeHandler end
// *********************************************************************


// *********************************************************************
// ConnectRequestHandler begin
// *********************************************************************
/**
 * MessageProcessor for connection request messages.
 */
struct ConnectRequestHandler : public MessageProcessor {
public:
	~ConnectRequestHandler()
	{

	}

	ConnectRequestHandler(SessionManager* sm) : MessageProcessor(sm) 
	{

	}

	LmHandlerResult 
	operator()(MessageType mode, JabberMessage& m)
	{
		std::bitset< NUM_FLAGS >& status = this->_sm->session_data->status;
		switch(mode) {
			case CONNECT_REQUEST_USER:
				this->_sm->receiveConnectRequest(m.sender.c_str());
				break;
			case CONNECT_REQUEST_RESPONSE_USER:
				if (m.sequence == 0) {
					this->_sm->receiveConnectRequestResponse(DECLINE_INVITATION, m.sender);
				} else { // FIXME: this has got to be buggy...
					this->_sm->setRecipient(m.sender.c_str());
					this->_sm->receiveConnectRequestResponse(ACCEPT_INVITATION, m.sender);
				}
				break;
			case Inkscape::Whiteboard::CONNECTED_SIGNAL:
				if (!status[IN_CHATROOM] && !status[CONNECTING_TO_CHAT] && !status[SYNCHRONIZING_WITH_CHAT] && !status[WAITING_TO_SYNC_TO_CHAT]) {
					this->_sm->userConnectedToWhiteboard(m.sender.c_str());
					this->_sm->setRecipient(m.sender.c_str());
				} else {
					sp_desktop_message_stack(this->_sm->desktop())->flashF(Inkscape::INFORMATION_MESSAGE, _("<b>%s</b> has joined the chatroom."), m.sender.c_str());
				}
				break;
			default:
				break;
		}
		return LM_HANDLER_RESULT_REMOVE_MESSAGE;
	}
};
// *********************************************************************
// ConnectRequestHandler end
// *********************************************************************




// *********************************************************************
// ConnectErrorHandler begin
// *********************************************************************
/**
 * MessageProcessor for connection error messages.  
 */
struct ConnectErrorHandler : public MessageProcessor {
public:
	~ConnectErrorHandler()
	{

	}

	ConnectErrorHandler(SessionManager* sm) : MessageProcessor(sm) 
	{

	}

	LmHandlerResult 
	operator()(MessageType mode, JabberMessage& m)
	{
		switch(mode) {
			case CONNECT_REQUEST_REFUSED_BY_PEER:
				if (this->_sm->session_data->status[WAITING_FOR_INVITE_RESPONSE]) {
					this->_sm->receiveConnectRequestResponse(DECLINE_INVITATION, m.sender);
				}
				break;
			case Inkscape::Whiteboard::ALREADY_IN_SESSION:
				if (this->_sm->session_data->status[WAITING_FOR_INVITE_RESPONSE]) {
					this->_sm->receiveConnectRequestResponse(PEER_ALREADY_IN_SESSION, m.sender);
				}
				break;
			case Inkscape::Whiteboard::DISCONNECTED_FROM_USER_SIGNAL:
				if (!this->_sm->session_data->status[IN_CHATROOM]) {
					this->_sm->closeSession();
					this->_sm->userDisconnectedFromWhiteboard(m.sender.c_str());
				}
				break;
			default:
				break;
		}
		return LM_HANDLER_RESULT_REMOVE_MESSAGE;
	}
};
// *********************************************************************
// ConnectErrorHandler end
// *********************************************************************




// *********************************************************************
// ChatSynchronizeHandler begin
// *********************************************************************
/**
 * MessageProcessor for messages specific to chatroom synchronization.
 */
struct ChatSynchronizeHandler : public MessageProcessor {
public:
	~ChatSynchronizeHandler()
	{

	}

	ChatSynchronizeHandler(SessionManager* sm) : MessageProcessor(sm) 
	{

	}

	LmHandlerResult 
	operator()(MessageType mode, JabberMessage& m)
	{
		switch(mode) {
			case CONNECT_REQUEST_RESPONSE_CHAT:
				this->_sm->receiveConnectRequestResponseChat(m.sender.c_str());
				break;
			case CHATROOM_SYNCHRONIZE_REQUEST:
				if (this->_sm->session_data->status[IN_CHATROOM] && this->_sm->session_data->status[IN_WHITEBOARD]) {
					// Send response.  Everyone in the chatroom will do this,
					// but the client will accept only one response.
					// The response is sent privately to the client
					// <http://www.jabber.org/jeps/jep-0045.html#privatemessage>
					this->_sm->sendMessage(CHATROOM_SYNCHRONIZE_RESPONSE, this->_sm->session_data->sequence_number, "", m.sender.c_str(), false);
				}
				break;
			case CHATROOM_SYNCHRONIZE_RESPONSE:
				if (m.sequence != 0) {
					// Set sequence number
					this->_sm->session_data->sequence_number = m.sequence;

					// Set status flags
					this->_sm->session_data->status.set(WAITING_TO_SYNC_TO_CHAT, 0);
					this->_sm->session_data->status.set(SYNCHRONIZING_WITH_CHAT, 1);

					// Send document synchronization request
					this->_sm->clearDocument();
					this->_sm->setupInkscapeInterface();
					this->_sm->sendMessage(CONNECT_REQUEST_RESPONSE_CHAT, m.sequence, "", m.sender.c_str(), false);
				} else {
					this->_sm->sendMessage(CHATROOM_SYNCHRONIZE_REQUEST, 0, "", this->_sm->session_data->recipient, true);
				}
				break;
			default:
				break;
		}
		return LM_HANDLER_RESULT_REMOVE_MESSAGE;
	}
};
// *********************************************************************
// ChatSynchronizeHandler end
// *********************************************************************




// *********************************************************************
// Initializer
// *********************************************************************
void
initialize_received_message_processors(SessionManager* sm, MessageProcessorMap& mpm)
{
	MessageProcessor* ch = new ChangeHandler(sm);
	MessageProcessor* crh = new ConnectRequestHandler(sm);
	MessageProcessor* ceh = new ConnectErrorHandler(sm);
	MessageProcessor* csh = new ChatSynchronizeHandler(sm);

	mpm[CHANGE_REPEATABLE] = ch;
	mpm[CHANGE_NOT_REPEATABLE] = ch;
	mpm[DUMMY_CHANGE] = ch;
	mpm[CHANGE_COMMIT] = ch;
	mpm[DOCUMENT_BEGIN] = ch;
	mpm[DOCUMENT_END] = ch;

	mpm[CONNECT_REQUEST_USER] = crh;
	mpm[CONNECT_REQUEST_RESPONSE_USER] = crh;
	mpm[CONNECTED_SIGNAL] = crh;

	mpm[CONNECT_REQUEST_REFUSED_BY_PEER] = ceh;
	mpm[ALREADY_IN_SESSION] = ceh;
	mpm[DISCONNECTED_FROM_USER_SIGNAL] = ceh;

	mpm[CONNECT_REQUEST_RESPONSE_CHAT] = csh;
	mpm[CHATROOM_SYNCHRONIZE_REQUEST] = csh;
	mpm[CHATROOM_SYNCHRONIZE_RESPONSE] = csh;
}

/*
 * This function is provided solely for convenience and style.  You can, of course,
 * delete every MessageProcessor in the map with your own loop.
 */
void
destroy_received_message_processors(MessageProcessorMap& mpm)
{
	mpm.clear();
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

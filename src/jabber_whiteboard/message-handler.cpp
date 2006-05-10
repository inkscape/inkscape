/**
 * Whiteboard session manager
 * Jabber received message handling
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 * Steven Montgomery, Jonas Collaros (original C version)
 *
 * Copyright (c) 2004-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

extern "C" {
#include <loudmouth/loudmouth.h>
}

#include <glibmm.h>
#include <glibmm/i18n.h>
#include <glib.h>
#include <map>

#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/message-processors.h"
#include "jabber_whiteboard/message-handler.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/chat-handler.h"
#include "jabber_whiteboard/buddy-list-manager.h"

namespace Inkscape {

namespace Whiteboard {

bool message_contexts_initialized = false;
MessageContextMap _received_message_contexts;

MessageHandler::MessageHandler(SessionManager* sm) : _sm(sm)
{
	if (message_contexts_initialized == false) {
//		this->_initializeContexts();
		MessageHandler::_initializeContexts();
	}
	this->_initializeProcessors();
}

MessageHandler::~MessageHandler()
{
	this->_destructProcessors();
}

LmHandlerResult
MessageHandler::handle(LmMessage* message, HandlerMode mode)
{
	if (this->_isValidMessage(message)) {
		switch(mode) {
			case DEFAULT:
				return this->_default(message);
			case PRESENCE:
				return this->_presence(message);
			case ERROR:
				return this->_error(message);
			default:
				g_warning("Jabber message handler was asked to process a message of an unhandled type; discarding message.");
				return LM_HANDLER_RESULT_REMOVE_MESSAGE;
		}
	} else {
		return LM_HANDLER_RESULT_REMOVE_MESSAGE;
	}
}

bool
MessageHandler::_hasValidReceiveContext(LmMessage* message)
{
	MessageType type = this->_getType(message);
	std::bitset< NUM_FLAGS >& status = this->_sm->session_data->status;

	std::string s1 = status.to_string< char, std::char_traits< char >, std::allocator< char > >();


	if (type == UNKNOWN) {
		// unknown types never have a valid receive context
		return false;
	} else {
		std::bitset< NUM_FLAGS >& recvcontext = _received_message_contexts[type];
		
		// TODO: remove this debug block
		if ((status & recvcontext).to_ulong() < status.to_ulong()) {
			g_warning("Received message in incorrect context; discarding message.");

			std::string s2 = recvcontext.to_string< char, std::char_traits< char >, std::allocator< char > >();

			g_warning("current context=%s required context=%s (msgtype %s)", s1.c_str(), s2.c_str(), MessageHandler::ink_type_to_string(type));
		}

		return ((status & recvcontext).to_ulong() >= status.to_ulong());
	}
}

bool
MessageHandler::_isValidMessage(LmMessage* message)
{
	// Global sanity checks
	LmMessageNode* root;
	LmMessageNode* protocolver;
	LmMessageNode* offline;
	LmMessageType mtype;
	LmMessageSubType msubtype;
	gchar const* tmp;

	Glib::ustring sender;

	
	// 0.  The message must have a root node.
	root = lm_message_get_node(message);
	if (root == NULL) {
		g_warning("Check 0 failed (message has no root node)");
		return false;
	}
	

	// 1.  The message must be of LM_MESSAGE_TYPE_MESSAGE to continue the sanity checks.
	// If it is not, check to see if it is either
	// a presence message or an error message.  If it is either of these, then automatically
	// consider it sane.  
	//
	// FIXME:
	// (That is probably a dangerous assumption.  We should probably at least validate
	// the source for error messages.)
	// 
	// We do not handle IQ stanzas or STREAM messages (yet), and we certainly don't
	// handle unknowns.
	
	mtype = lm_message_get_type(message);
	switch(mtype) {
		case LM_MESSAGE_TYPE_PRESENCE:
		case LM_MESSAGE_TYPE_STREAM_ERROR:
			return true;
		case LM_MESSAGE_TYPE_IQ:
		case LM_MESSAGE_TYPE_STREAM:
		case LM_MESSAGE_TYPE_UNKNOWN:
			g_warning("Check 1 failed (Loudmouth reported type IQ, STREAM, or UNKNOWN)");
			return false;
		case LM_MESSAGE_TYPE_MESSAGE:
			break;
	}
	
	// 2.  The message must contain the JID of the sender.
	tmp = lm_message_node_get_attribute(root, MESSAGE_FROM);

	if (tmp == NULL) {
		g_warning("Check 2 failed (no sender attribute present)");
		return false;
	} else {
		sender = tmp;
	}

	// 3.  We do not yet handle messages from offline storage, so ensure that this is not
	// such a message.
	offline = lm_message_node_get_child(root, "x");
	if (offline != NULL) {
		gchar const* val = lm_message_node_get_value(offline);
		if (val != NULL) {
			if (strcmp(val, "Offline Storage") == 0) {
				return false;
			}
		}
	}


	// 4.  If this is a regular chat message...
	msubtype = lm_message_get_sub_type(message);

	if (msubtype == LM_MESSAGE_SUB_TYPE_CHAT) {
		// 4a.  A protocol version node must be present.
		protocolver = lm_message_node_get_child(root, MESSAGE_PROTOCOL_VER);
		if (protocolver == NULL) {
			g_warning("Check 4a failed (no protocol attribute in chat message)");
			return false;
		} else {
			tmp = lm_message_node_get_value(protocolver);
			if (tmp == NULL) {
				g_warning("Check 4a failed (no protocol attribute in chat message)");
				return false;
			}
		}

		// 5a.  The protocol version must be supported.
		if (atoi(tmp) > HIGHEST_SUPPORTED) {
			g_warning("Check 5a failed (received a message with protocol version %s, but version %s is not supported)", tmp, tmp);
			return false;
		}

	// ...otherwise, if this is a groupchat message, we may not have a protocol version 
	// (since it may be communication from the Jabber server).  In this case, we have a
	// different set of sanity checks.
	} else if (msubtype == LM_MESSAGE_SUB_TYPE_GROUPCHAT) {
		// 4b.
		// In a chatroom situation, we need to ensure that we don't process messages that
		// originated from us.
		int cutoff = sender.find_last_of('/') + 1;
		if (sender.substr(cutoff, sender.length()) == this->_sm->session_data->chat_handle) {
			return false;
		}
		// TODO: 6b.  If the message is NOT from the Jabber server, then check the protocol version.
	}

	// If all tests pass, then the message is at least valid.  
	// Correct context has not yet been established, however; that is the job of the default handler
	// and hasValidReceiveContext.

	return true;
}

MessageType
MessageHandler::_getType(LmMessage* message)
{
	LmMessageNode* root;
	LmMessageNode* typenode;

	root = lm_message_get_node(message);
	if (root != NULL) {
		typenode = lm_message_node_get_child(root, MESSAGE_TYPE);
		if (typenode != NULL) {
			return static_cast< MessageType >(atoi(lm_message_node_get_value(typenode)));
		}
	}
	return UNKNOWN;
}

JabberMessage
MessageHandler::_extractData(LmMessage* message)
{

	JabberMessage jm(message);
	LmMessageNode* root;
	LmMessageNode* sequence;
	LmMessageNode* body;
	gchar const* tmp;

	root = lm_message_get_node(message);

	if (root != NULL) {
		sequence = lm_message_node_get_child(root, MESSAGE_SEQNUM);
		body = lm_message_node_get_child(root, MESSAGE_BODY);

		jm.sender = lm_message_node_get_attribute(root, MESSAGE_FROM);

		if (sequence) {
			tmp = lm_message_node_get_value(sequence);
			if (tmp != NULL) {
				jm.sequence = atoi(tmp);
			}
		}

		if (body) {
			tmp = lm_message_node_get_value(body);
			if (tmp != NULL) {
				jm.body = tmp;
			}
		}

	} else {
		jm.sequence = 0;
		jm.sender = "";
		jm.body = "";
	}

	return jm;
}
	
LmHandlerResult
MessageHandler::_default(LmMessage* message)
{
	std::bitset< NUM_FLAGS >& status = this->_sm->session_data->status;

	// Pass groupchat messages with no Inkboard type off to the chat message handler
	if (this->_getType(message) == UNKNOWN) {
		if (lm_message_get_sub_type(message) == LM_MESSAGE_SUB_TYPE_GROUPCHAT) {
			if (status[IN_CHATROOM] || status[CONNECTING_TO_CHAT]) {
				return this->_sm->chat_handler()->parse(message);
			}
		} else {
			return LM_HANDLER_RESULT_REMOVE_MESSAGE;
		}
	}
	
	if (this->_hasValidReceiveContext(message)) {
		// Extract message data
		JabberMessage msg = this->_extractData(message);
		MessageType type = this->_getType(message);

		// Call message handler and return instruction value to Loudmouth

		return (*this->_received_message_processors[type])(type, msg);
	} else {
		g_warning("Default message handler received message in invalid receive context; discarding message.");
		return LM_HANDLER_RESULT_REMOVE_MESSAGE;
	}
}

LmHandlerResult
MessageHandler::_presence(LmMessage* message)
{
	LmMessageNode* root;
	LmMessageSubType msubtype;
	gchar const* tmp;
	std::string sender;

	SessionData* sd = this->_sm->session_data;
	std::bitset< NUM_FLAGS >& status = this->_sm->session_data->status;

	root = lm_message_get_node(message);
	if (root == NULL) {
		return LM_HANDLER_RESULT_REMOVE_MESSAGE;
	}

	tmp = lm_message_node_get_attribute(root, MESSAGE_FROM);
	if (tmp == NULL) {
		return LM_HANDLER_RESULT_REMOVE_MESSAGE;
	} else {
		sender = tmp;
	}

	msubtype = lm_message_get_sub_type(message);
	if (status[CONNECTING_TO_CHAT] || status[IN_CHATROOM]) {
		this->_sm->chat_handler()->parse(message);
	} else {
		switch(msubtype) {
			case LM_MESSAGE_SUB_TYPE_UNAVAILABLE:
				// remove buddy from online roster
				sd->buddyList.erase(sender);

				// if this buddy is in a 1-1 session with us, we need to exit
				// the whiteboard
				if (status[IN_WHITEBOARD] && !(status[IN_CHATROOM]) && strcasecmp(sender.c_str(), sd->recipient) == 0) {
					status.set(IN_WHITEBOARD, 0);
					this->_sm->userDisconnectedFromWhiteboard(sender);
					this->_sm->closeSession();
				}
				return LM_HANDLER_RESULT_REMOVE_MESSAGE;
				
			case LM_MESSAGE_SUB_TYPE_AVAILABLE:
				// we don't want to insert an entry into a buddy list
				// if it's our own presence
				if (sender != this->_sm->session_data->jid.c_str()) {
					sd->buddyList.insert(sender);
				}
				return LM_HANDLER_RESULT_REMOVE_MESSAGE;
			default:
				break;
		}
	}
	return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}

LmHandlerResult
MessageHandler::_error(LmMessage* message)
{
	LmMessageNode* root;
	LmMessageSubType msubtype;

	root = lm_message_get_node(message);
	if (root != NULL) {
		msubtype = lm_message_get_sub_type(message);
		if (msubtype == LM_MESSAGE_SUB_TYPE_ERROR) {
			gchar* error = g_strdup(lm_message_node_get_value(root));
			g_warning(error);

			// TODO: more robust error handling code
			this->_sm->disconnectFromDocument();
			this->_sm->disconnectFromServer();
			this->_sm->connectionError(error);
		}
	}

	return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}

void
MessageHandler::_initializeContexts()
{
	initialize_received_message_contexts(_received_message_contexts);	
	message_contexts_initialized = true;
}

void
MessageHandler::_initializeProcessors()
{
	initialize_received_message_processors(this->_sm, this->_received_message_processors);	
}

void
MessageHandler::_destructProcessors()
{
	destroy_received_message_processors(this->_received_message_processors);
}


char const*
MessageHandler::ink_type_to_string(gint ink_type) {
	switch(ink_type) {
		case Inkscape::Whiteboard::CHANGE_NOT_REPEATABLE:
			return "CHANGE_NOT_REPEATABLE";
		case Inkscape::Whiteboard::CHANGE_REPEATABLE:
			return "CHANGE_REPEATABLE";
		case Inkscape::Whiteboard::DUMMY_CHANGE:
			return "DUMMY_CHANGE";
		case Inkscape::Whiteboard::CHANGE_COMMIT:
			return "CHANGE_COMMIT";
		case Inkscape::Whiteboard::CONNECT_REQUEST_USER:
			return "CONNECT_REQUEST_USER";
		case Inkscape::Whiteboard::CONNECT_REQUEST_RESPONSE_USER:
			return "CONNECT_REQUEST_RESPONSE_USER";
		case Inkscape::Whiteboard::CONNECT_REQUEST_RESPONSE_CHAT:
			return "CONNECT_REQUEST_RESPONSE_CHAT";
		case Inkscape::Whiteboard::DOCUMENT_SENDER_REQUEST:
			return "DOCUMENT_SENDER_REQUEST";
		case Inkscape::Whiteboard::DOCUMENT_SENDER_REQUEST_RESPONSE:
			return "DOCUMENT_SENDER_REQUEST_RESPONSE";
		case Inkscape::Whiteboard::DOCUMENT_REQUEST:
			return "DOCUMENT_REQUEST";
		case Inkscape::Whiteboard::DOCUMENT_BEGIN:
			return "DOCUMENT_BEGIN";
		case Inkscape::Whiteboard::DOCUMENT_END:
			return "DOCUMENT_END";
		case Inkscape::Whiteboard::CONNECTED_SIGNAL:
			return "CONNECTED_SIGNAL";
		case Inkscape::Whiteboard::DISCONNECTED_FROM_USER_SIGNAL:
			return "DISCONNECTED_FROM_USER_SIGNAL";
		case Inkscape::Whiteboard::CONNECT_REQUEST_REFUSED_BY_PEER:
			return "CONNECT_REQUEST_REFUSED_BY_PEER";
		case Inkscape::Whiteboard::UNSUPPORTED_PROTOCOL_VERSION:
			return "UNSUPPORTED_PROTOCOL_VERSION";
		case Inkscape::Whiteboard::CHATROOM_SYNCHRONIZE_REQUEST:
			return "CHATROOM_SYNCHRONIZE_REQUEST";
		case Inkscape::Whiteboard::CHATROOM_SYNCHRONIZE_RESPONSE:
			return "CHATROOM_SYNCHRONIZE_RESPONSE";
		case Inkscape::Whiteboard::ALREADY_IN_SESSION:
			return "ALREADY_IN_SESSION";
		default:
			return "UNKNOWN";
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

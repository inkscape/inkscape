/**
 * Whiteboard session manager
 * Chatroom message handler
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

//#include <boost/lexical_cast.hpp>

#include "message-stack.h"
#include "desktop-handles.h"
#include "document.h"

#include "util/ucompose.hpp"

#include "xml/node.h"

#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/message-utilities.h"
#include "jabber_whiteboard/message-queue.h"
#include "jabber_whiteboard/jabber-handlers.h"
#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/node-tracker.h"
#include "jabber_whiteboard/chat-handler.h"
#include "jabber_whiteboard/error-codes.h"


namespace Inkscape {

namespace Whiteboard {

ChatMessageHandler::ChatMessageHandler(SessionManager* sm) : _sm(sm)
{ 

}

ChatMessageHandler::~ChatMessageHandler() 
{

}

LmHandlerResult
ChatMessageHandler::parse(LmMessage* message)
{
	// Retrieve the message type
	LmMessageType mtype = lm_message_get_type(message);

	// Retrieve root node of message
	LmMessageNode* root = lm_message_get_node(message);
	if (root == NULL) {
		g_warning("Received a chat message with NULL root node; discarding.");
		return LM_HANDLER_RESULT_REMOVE_MESSAGE;
	}

	LmMessageSubType msubtype;


	msubtype = lm_message_get_sub_type(message);

	switch (mtype) {
		case LM_MESSAGE_TYPE_MESSAGE:
			switch(msubtype) {
				case LM_MESSAGE_SUB_TYPE_ERROR:
				{
					LmMessageNode* error = lm_message_node_get_child(root, "error");
					if (error != NULL) {
						this->_handleError(lm_message_node_get_attribute(error, "code"));
					}
					break;
				}
				case LM_MESSAGE_SUB_TYPE_GROUPCHAT:
				{
					// FIXME: We should be checking to see if we're in a room in the presence stanzas as indicated in
					// <http://www.jabber.org/jeps/jep-0045.html#enter-pres> but current versions of mu-conference
					// don't broadcast presence in the correct order.
					//
					// Therefore we need to use some sort of hacked-up method to make this work.  We listen for 
					// the sentinel value in a groupchat message -- currently it is '[username] INKBOARD-JOINED' --
					// and begin processing that way.
					LmMessageNode* body = lm_message_node_get_child(root, "body");
					if (body != NULL) {
						gchar const* val = lm_message_node_get_value(body);
						if (strcmp(val, String::ucompose("%1 has become available", this->_sm->session_data->chat_handle).c_str()) == 0) {
							return this->_finishConnection(); break;
						} else {
							return LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS;
							break;
						}
					} else {
						return LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS;
						break;
					}
				}
												
				default:
					return LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS;
					break;
			}
			break;
		case LM_MESSAGE_TYPE_PRESENCE:
			// Retrieve the subtype.
			switch (msubtype) {
				case LM_MESSAGE_SUB_TYPE_ERROR:
				{
					g_warning("Could not connect to chatroom");
					// Extract error type
					LmMessageNode* error = lm_message_node_get_child(root, "error");
					if (error != NULL) {
						this->_handleError(lm_message_node_get_attribute(error, "code"));
					}
					// Reset status bits
					this->_sm->session_data->status.set(CONNECTING_TO_CHAT, 0);
					this->_sm->session_data->recipient = "";
					this->_sm->session_data->chat_handle = "";

					return LM_HANDLER_RESULT_REMOVE_MESSAGE;
					break;
				}

				case LM_MESSAGE_SUB_TYPE_AVAILABLE: 
				{
					// Extract the handle
					// (see JEP-0045, section 6.3.3 - <http://www.jabber.org/jeps/jep-0045.html#enter-pres>)
					Glib::ustring sender = lm_message_node_get_attribute(root, MESSAGE_FROM);
					Glib::ustring chatter = sender.substr(sender.find_last_of('/') + 1, sender.length());
					if (chatter != this->_sm->session_data->chat_handle) {
						this->_sm->session_data->chatters.insert(g_strdup(chatter.data()));
						// Make a receive queue for this chatter
						this->_sm->session_data->receive_queues[sender.raw()] = new ReceiveMessageQueue(this->_sm);
						
					} else {
						// If the presence message is from ourselves, then we know that we 
						// have successfully entered the chatroom _and_ have received the entire room roster,
						// and can therefore decide whether we need to synchronize with the rest of the room.
						// (see JEP-0045, section 6.3.3 - <http://www.jabber.org/jeps/jep-0045.html#enter-pres>)
					}
					return LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS;
				}

				case LM_MESSAGE_SUB_TYPE_UNAVAILABLE:
				{
					Glib::ustring sender = lm_message_node_get_attribute(root, MESSAGE_FROM);
					Glib::ustring chatter = sender.substr(sender.find_last_of('/') + 1, sender.length());
					this->_sm->session_data->chatters.erase(chatter.data());

					// Delete the message queue used by this sender
					this->_sm->session_data->receive_queues.erase(sender.raw());

					sp_desktop_message_stack(this->_sm->desktop())->flashF(Inkscape::INFORMATION_MESSAGE, _("<b>%s</b> has left the chatroom."), sender.c_str());
				}

				default:
					// no idea what this message is; discard it
					return LM_HANDLER_RESULT_REMOVE_MESSAGE;
					break;
			}
			break;
		default:
			break;
	}

	return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}

LmHandlerResult
ChatMessageHandler::_finishConnection()
{
	if (this->_sm->session_data->status[CONNECTING_TO_CHAT]) {
		this->_sm->session_data->status.set(CONNECTING_TO_CHAT, 0);

		if (this->_sm->session_data->chatters.empty()) {
			// We are the only one in the chatroom, so there is no
			// need for synchronization
			this->_sm->session_data->status.set(IN_CHATROOM, 1);

			// Populate node tracker
			KeyToNodeMap newids;
			NodeToKeyMap newnodes;
			NewChildObjectMessageList newchildren;

			XML::Node* root = this->_sm->document()->rroot;

			this->_sm->setupInkscapeInterface();
			this->_sm->setupCommitListener();

			for ( Inkscape::XML::Node *child = root->firstChild() ; child != NULL ; child = child->next() ) {
				MessageUtilities::newObjectMessage(NULL, newids, newnodes, newchildren, this->_sm->node_tracker(), child, true);
			}

			this->_sm->node_tracker()->put(newids, newnodes);
	//		this->_sm->node_tracker()->dump();

		} else {
			this->_sm->session_data->status.set(WAITING_TO_SYNC_TO_CHAT, 1);
			// Send synchronization request to chatroom
			this->_sm->sendMessage(CHATROOM_SYNCHRONIZE_REQUEST, 0, "", this->_sm->session_data->recipient, true);
		}
	}
	return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}

void
ChatMessageHandler::_handleError(char const* errcode)
{
//	try {
	unsigned int code = atoi(errcode);

//		unsigned int code = boost::lexical_cast< unsigned int >(errcode);

	Glib::ustring buf;
	switch (code) {
		case ErrorCodes::CHAT_HANDLE_IN_USE:
			buf = String::ucompose(_("Nickname %1 is already in use.  Please choose a different nickname."), this->_sm->session_data->chat_handle);
			this->_sm->connectionError(buf);
			break;
		case ErrorCodes::SERVER_CONNECT_FAILED:
			buf = _("An error was encountered while attempting to connect to the server.");
			this->_sm->connectionError(buf);
			break;
		default:
			break;
	}
//	} catch (boost::bad_lexical_cast&) {

//	}
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

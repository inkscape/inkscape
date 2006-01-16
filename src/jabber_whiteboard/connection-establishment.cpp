/**
 * Whiteboard session manager
 * Methods for establishing connections and notifying the user of events
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "util/ucompose.hpp"
#include <glibmm/i18n.h>
#include <gtkmm/dialog.h>
#include <gtkmm/messagedialog.h>

#include "desktop.h"
#include "file.h"
#include "document.h"

#include "xml/repr.h"

#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/node-tracker.h"
#include "jabber_whiteboard/chat-handler.h"
#include "jabber_whiteboard/message-queue.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/invitation-confirm-dialog.h"

namespace Inkscape {

namespace Whiteboard {

void
SessionManager::sendRequestToUser(std::string const& recipientJID)
{
	/*
	Glib::ustring doccopy;
	if (document != NULL) {
		doccopy = *document;
	}
	*/
	this->session_data->status.set(WAITING_FOR_INVITE_RESPONSE, 1);
	this->sendMessage(CONNECT_REQUEST_USER, 0, "", recipientJID.c_str(), false);
}

void
SessionManager::sendRequestToChatroom(Glib::ustring const& server, Glib::ustring const& chatroom, Glib::ustring const& handle, Glib::ustring const& password)
{
	// We do not yet use the Basic MUC Protocol for connection establishment etc
	// <http://www.jabber.org/jeps/jep-0045.html>.  The protocol we use is the
	// old GroupChat system; extension to MUC is TODO
	
	// Compose room@service/nick string for "to" field
	Glib::ustring dest = String::ucompose("%1@%2/%3", chatroom, server, handle);
	LmMessage* presence_req = lm_message_new(dest.data(), LM_MESSAGE_TYPE_PRESENCE);

	// Add 'from' attribute
	LmMessageNode* preq_root = lm_message_get_node(presence_req);

	lm_message_node_set_attribute(preq_root, "from", lm_connection_get_jid(this->session_data->connection));
	
	// Add <x xmlns='http://jabber.org/protocol/muc/' />
	// (Not anymore: we don't speak it! -- yipdw)
	LmMessageNode* xmlns_node = lm_message_node_add_child(preq_root, "x", "");
	lm_message_node_set_attribute(xmlns_node, "xmlns", "http://jabber.org/protocol/muc/");
	
	// If a password was supplied, add it to xmlns_node
	if (password != NULL) {
		lm_message_node_add_child(xmlns_node, "password", password.c_str());
	}

	// Create chat message handler and node tracker
	if (!this->_myChatHandler) {
		this->_myChatHandler = new ChatMessageHandler(this);
	}

	// Flag ourselves as connecting to a chatroom (but not yet connected)
	this->session_data->status.set(CONNECTING_TO_CHAT, 1);
	// Send the message
	GError *error = NULL;
	if (!lm_connection_send(this->session_data->connection, presence_req, &error)) {
		g_error("Presence message could not be sent to %s: %s", dest.data(), error->message);
		this->session_data->status.set(CONNECTING_TO_CHAT, 0);
	}

	this->session_data->chat_handle = handle;
	this->session_data->chat_server = server;
	this->session_data->chat_name = chatroom;

	this->setRecipient(String::ucompose("%1@%2", chatroom, server).data());

	lm_message_unref(presence_req);
}

void 
SessionManager::sendConnectRequestResponse(char const* recipientJID, gboolean accepted_request)
{
	if (accepted_request == TRUE) {
		this->setRecipient(recipientJID);
		this->session_data->status.set(IN_WHITEBOARD, 1);
	}

	this->sendMessage(CONNECT_REQUEST_RESPONSE_USER, accepted_request, "", recipientJID, false);
}

// When this method is invoked, it means that the user has received an invitation from another peer
// to engage in a whiteboard session (i.e. 1:1 communication).  The user may accept or reject this invitation.
void
SessionManager::receiveConnectRequest(gchar const* requesterJID)
{
	int x, y;
	Gdk::ModifierType mt;
	Gdk::Display::get_default()->get_pointer(x, y, mt);

	if (mt) {
		// Attach a polling timeout
		this->_notify_incoming_request = Glib::signal_timeout().connect(sigc::bind< 0 >(sigc::mem_fun(*this, &SessionManager::_pollReceiveConnectRequest), requesterJID), 50);
		return;
	}

	if (this->session_data->status[IN_WHITEBOARD]) {
		this->sendMessage(ALREADY_IN_SESSION, 0, "", requesterJID, false);
	}

	// Check to see if the user made any modifications to this document.  If so, 
	// we want to give them the option of (1) letting us clear their document or (2)
	// opening a new, blank document for the whiteboard session.
	Glib::ustring primary = "<span weight=\"bold\" size=\"larger\">" + String::ucompose(_("<b>%1</b> has invited you to a whiteboard session."), requesterJID) + "</span>\n\n";
	Glib::ustring title = String::ucompose(_("Incoming whiteboard invitation from %1"), requesterJID);

	if (sp_document_repr_root(this->_myDoc)->attribute("sodipodi:modified") == NULL) {
		primary += String::ucompose(_("Do you wish to accept <b>%1</b>'s whiteboard session invitation?"), requesterJID);
	} else {
		primary += String::ucompose(_("Would you like to accept %1's invitation in a new document window?\nAccepting the invitation in your current window will discard unsaved changes."), requesterJID);
	}

	// Construct confirmation dialog
	InvitationConfirmDialog dialog(primary);
	
	dialog.add_button(_("Accept invitation"), ACCEPT_INVITATION);
	dialog.add_button(_("Decline invitation"), DECLINE_INVITATION);
	dialog.add_button(_("Accept invitation in new document window"), ACCEPT_INVITATION_IN_NEW_WINDOW);

	bool undecided = true;
	InvitationResponses resp = static_cast< InvitationResponses >(dialog.run());

	while(undecided) {
		if (resp == ACCEPT_INVITATION) {
			undecided = false;
			this->clearDocument();
		
			// Create a receive queue for the initiator of this request
			this->session_data->receive_queues[requesterJID] = new ReceiveMessageQueue(this);
			
			this->setupInkscapeInterface();
			if (dialog.useSessionFile()) {
				this->session_data->sessionFile = dialog.getSessionFilePath();
				this->_tryToStartLog();
			}
			this->sendConnectRequestResponse(requesterJID, TRUE);

		} else if (resp == ACCEPT_INVITATION_IN_NEW_WINDOW) {
			SPDesktop* newdesktop = sp_file_new_default();
			if (newdesktop != NULL) {
				undecided = false;

				// Swap desktops around

				// Destroy the new desktop's session manager and add this one in
				delete newdesktop->_whiteboard_session_manager;
				newdesktop->_whiteboard_session_manager = this;

				// Assign a new session manager to our old desktop
				this->_myDesktop->_whiteboard_session_manager = new SessionManager(this->_myDesktop);

				// Reset our desktop and document pointers
				this->setDesktop(newdesktop);

				// Prepare document and send acceptance notification
				this->session_data->receive_queues[requesterJID] = new ReceiveMessageQueue(this);
				this->clearDocument();
				this->setupInkscapeInterface();
				if (dialog.useSessionFile()) {
					this->session_data->sessionFile = dialog.getSessionFilePath();
					this->_tryToStartLog();
				}
				this->sendConnectRequestResponse(requesterJID, TRUE);

			} else {
				// We could not create a new desktop; ask the user if she or he wants to 
				// replace the current document and accept the invitation, or reject the invitation.
				// TRANSLATORS: %1 is a userid here
				Glib::ustring msg = "<span weight=\"bold\" size=\"larger\">" + String::ucompose(_("A new document window could not be opened for a whiteboard session with <b>%1</b>"), requesterJID) + ".</span>\n\nWould you like to accept the whiteboard connection in the active document or refuse the invitation?";
				InvitationConfirmDialog replace_dialog(msg);
				dialog.add_button(_("Accept invitation"), ACCEPT_INVITATION);
				dialog.add_button(_("Decline invitation"), DECLINE_INVITATION);
				resp = static_cast< InvitationResponses >(dialog.run());
			}
		} else {
			undecided = false;
			this->sendMessage(CONNECT_REQUEST_REFUSED_BY_PEER, 0, "", requesterJID, false);
		}
	}
}

// When this method is invoked, it means that the other peer
// has accepted our request.
void
SessionManager::receiveConnectRequestResponse(InvitationResponses response, std::string& sender)
{
	this->session_data->status.set(WAITING_FOR_INVITE_RESPONSE, 0);

	switch(response) {
		case ACCEPT_INVITATION:
			{

			// Create a receive queue for the other peer.
			this->session_data->receive_queues[sender] = new ReceiveMessageQueue(this);
				
			KeyToNodeMap newids;
			NodeToKeyMap newnodes;
			this->_myTracker = new XMLNodeTracker(this);
			this->setupInkscapeInterface();
			this->_tryToStartLog();
			this->resendDocument(this->session_data->recipient, newids, newnodes);
			this->_myTracker->put(newids, newnodes);
//			this->_myTracker->dump();
			this->setupCommitListener();
			break;
			}

		case DECLINE_INVITATION:
			{
			// TRANSLATORS: %1 is the peer whom refused our invitation.
			Glib::ustring primary = String::ucompose(_("<span weight=\"bold\" size=\"larger\">The user <b>%1</b> has refused your whiteboard invitation.</span>\n\n"), sender);
			
			// TRANSLATORS: %1 is the peer whom refused our invitation, %2 is our Jabber identity.
			Glib::ustring secondary = String::ucompose(_("You are still connected to a Jabber server as <b>%2</b>, and may send an invitation to <b>%1</b> again, or you may send an invitation to a different user."), sender, lm_connection_get_jid(this->session_data->connection));

			Gtk::MessageDialog dialog(primary + secondary, true, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, false);
			dialog.run();
			break;

			}

		case PEER_ALREADY_IN_SESSION:
			{
			// TRANSLATORS: %1 is the peer whom we tried to contact, but is already in a whiteboard session.
			Glib::ustring primary = String::ucompose(_("<span weight=\"bold\" size=\"larger\">The user <b>%1</b> is already in a whiteboard session.</span>\n\n"), sender);

			// TRANSLATORS: %1 is the peer whom we tried to contact, but is already in a whiteboard session.
			Glib::ustring secondary = String::ucompose(_("You are still connected to a Jabber server as <b>%1</b>, and may send an invitation to a different user."), sender);
			Gtk::MessageDialog dialog(primary + secondary, true, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, false);
			dialog.run();

			break;
			}
		default:
			break;
	}
}

void
SessionManager::receiveConnectRequestResponseChat(gchar const* recipient)
{
	// When responding to connection request responses in chatrooms,
	// the responding user is already established in the whiteboard session.
	// Therefore we do not need to perform any setup of observers or dispatchers; the requesting user
	// will do that.

	KeyToNodeMap newids;
	NodeToKeyMap newnodes;
	this->resendDocument(recipient, newids, newnodes);
}

bool
SessionManager::_pollReceiveConnectRequest(Glib::ustring const recipientJID)
{
	int x, y;
	Gdk::ModifierType mt;
	Gdk::Display::get_default()->get_pointer(x, y, mt);

	if (mt) {
		return true;
	} else {
		this->receiveConnectRequest(recipientJID.c_str());
		return false;
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

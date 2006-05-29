/**
 * Whiteboard session manager
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
#include "inkscape.h"
*/

#include <cstring>

#include <glibmm/i18n.h>
#include <gtkmm/dialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/stock.h>

#include "gc-anchored.h"

#include "prefs-utils.h"

#include "xml/repr.h"
#include "xml/node-observer.h"

#include "util/ucompose.hpp"

#include "message-context.h"
#include "message-stack.h"
#include "desktop-handles.h"
#include "document.h"
#include "document-private.h"
#include "verbs.h"

#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/deserializer.h"
#include "jabber_whiteboard/message-utilities.h"
#include "jabber_whiteboard/message-handler.h"
#include "jabber_whiteboard/node-tracker.h"
#include "jabber_whiteboard/jabber-handlers.h"
#include "jabber_whiteboard/callbacks.h"
#include "jabber_whiteboard/chat-handler.h"
#include "jabber_whiteboard/session-file.h"
#include "jabber_whiteboard/session-file-player.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/message-aggregator.h"
#include "jabber_whiteboard/undo-stack-observer.h"
#include "jabber_whiteboard/serializer.h"

//#include "jabber_whiteboard/pedro/pedroxmpp.h"

#include "jabber_whiteboard/message-node.h"
#include "jabber_whiteboard/message-queue.h"

namespace Inkscape {

namespace Whiteboard {

SessionData::SessionData(SessionManager *sm)
{
	this->_sm = sm;
	this->recipient = NULL;
	this->connection = NULL;
	this->ssl = NULL;
	this->ignoreFurtherSSLErrors = false;
	this->send_queue = new SendMessageQueue(sm);
	this->sequence_number = 1;
}

SessionData::~SessionData()
{
	this->receive_queues.clear();

	if (this->send_queue) {
		delete this->send_queue;
	}
}

SessionManager::SessionManager(::SPDesktop *desktop) 
{

	// Initialize private members to NULL to facilitate deletion in destructor
	this->_myDoc = NULL;
	this->session_data = NULL;
	this->_myCallbacks = NULL;
	this->_myTracker = NULL;
	this->_myChatHandler = NULL;
	this->_mySessionFile = NULL;
	this->_mySessionPlayer = NULL;
	this->_myMessageHandler = NULL;
	this->_myUndoObserver = NULL;
	this->_mySerializer = NULL;
	this->_myDeserializer = NULL;

	this->setDesktop(desktop);

	if (this->_myDoc == NULL) {
		g_error("Initializing SessionManager on null document object!");
	}


#ifdef WIN32
    //# lm_initialize() must be called before any network code
/*
    if (!lm_initialize_called) {
        lm_initialize();
        lm_initialize_called = true;
	}
*/
#endif

	this->_setVerbSensitivity(INITIAL);
}

SessionManager::~SessionManager()
{

	if (this->session_data) {
		if (this->session_data->status[IN_WHITEBOARD]) {
			// also calls closeSession
			this->disconnectFromDocument();
		}
		this->disconnectFromServer();

		if (this->session_data->status[LOGGED_IN]) {
			// TODO: unref message handlers
		}
	}

	if (this->_mySessionFile) {
		delete this->_mySessionPlayer;
		delete this->_mySessionFile;
	}

	delete this->_myChatHandler;


	// Deletion of _myTracker is done in closeSession;
	// no need to do it here.

	// Deletion is handled separately from session teardown and server disconnection
	// because some teardown methods (e.g. closeSession) require access to members that we will
	// be deleting. Separating deletion from teardown means that we do not have
	// to worry (as much) about proper ordering of the teardown sequence.  (We still need
	// to ensure that destructors in each object being deleted have access to all the
	// members they need, though.)

	// Stop dispatchers
	if (this->_myCallbacks) {
		this->stopSendQueueDispatch();
		this->stopReceiveQueueDispatch();
		delete this->_myCallbacks;
	}

	delete this->_myMessageHandler;

	delete this->session_data;

	Inkscape::GC::release(this->_myDoc);

}

void
SessionManager::setDesktop(::SPDesktop* desktop)
{
	this->_myDesktop = desktop;
	if (this->_myDoc != NULL) {
		Inkscape::GC::release(this->_myDoc);
	}
	if (sp_desktop_document(desktop) != NULL) {
		this->_myDoc = sp_desktop_document(desktop);
		Inkscape::GC::anchor(this->_myDoc);
	}
}

int
SessionManager::initializeConnection(Glib::ustring const& server, Glib::ustring const& port, bool usessl)
{
	GError* error = NULL;

	if (!this->session_data) {
		this->session_data = new SessionData(this);
	}

	if (!this->_myMessageHandler) {
		this->_myMessageHandler = new MessageHandler(this);
	}

	// Connect to server
	// We need to check to see if this object already exists, because
	// the user may be reusing an old connection that failed due to e.g.
	// authentication failure.
	if (this->session_data->connection) {
		lm_connection_close(this->session_data->connection, &error);
		lm_connection_unref(this->session_data->connection);
	}

	this->session_data->connection = lm_connection_new(server.c_str());
	this->session_data->chat_server = server;

	lm_connection_set_port(this->session_data->connection, atoi(port.c_str()));

        g_log(NULL, G_LOG_LEVEL_DEBUG, "Opened connection to %s at port %s.  Connecting...",
                    server.c_str(), port.c_str());

	if (usessl) {
		if (lm_ssl_is_supported()) {
			this->session_data->ssl = lm_ssl_new(NULL, ssl_error_handler, reinterpret_cast< gpointer >(this), NULL);

			lm_ssl_ref(this->session_data->ssl);
		} else {
			return SSL_INITIALIZATION_ERROR;
		}
		lm_connection_set_ssl(this->session_data->connection, this->session_data->ssl);
	}

	// Send authorization
	//lm_connection_set_jid(this->session_data->connection, jid.c_str());

	// 	TODO:
	// 	Asynchronous connection and authentication would be nice,
	// 	but it's a huge mess of mixing C callbacks and C++ method calls.
	// 	I've tried to do it and only managed to severely destabilize the Jabber
	// 	server connection routines.
	//
	// 	This, of course, is an invitation to anyone more capable than me
	// 	to convert this from synchronous to asynchronous Loudmouth calls.
	if (!lm_connection_open_and_block(this->session_data->connection, &error)) {
		if (error != NULL) {
			std::cout << "Failed to open: " <<  error->message << std::endl;
		}
		return FAILED_TO_CONNECT;
	}

	//On successful connect, remember info
	prefs_set_string_attribute("whiteboard.server", "name", server.c_str());
	prefs_set_string_attribute("whiteboard.server", "port", port.c_str());
	prefs_set_int_attribute("whiteboard.server", "ssl", (usessl) ? 1 : 0);

	return CONNECT_SUCCESS;
}

std::vector<Glib::ustring>
SessionManager::getRegistrationInfo()
{
	GError* error = NULL;
	xmlDoc *doc = NULL;
    	xmlNode *root_element = NULL;
	xmlNode *cur_node = NULL;

	LmMessage *reply,*request;
	LmMessageNode  *n;

	std::vector<Glib::ustring> registerelements; 

	request = lm_message_new_with_sub_type(NULL,LM_MESSAGE_TYPE_IQ,LM_MESSAGE_SUB_TYPE_GET);
	n = lm_message_node_add_child (request->node, "query", NULL);
	lm_message_node_set_attributes (n, "xmlns", "jabber:iq:register", NULL);

	reply = lm_connection_send_with_reply_and_block(this->session_data->connection, request, &error);
	if (error != NULL) {
		return registerelements;
	}

	n = lm_message_get_node(reply);

	Glib::ustring content = static_cast< Glib::ustring >(lm_message_node_to_string(lm_message_node_get_child(n,"query")));
	doc = xmlReadMemory(content.c_str(),content.size(), "noname.xml", NULL, 0);

	if (doc == NULL) {
        	g_warning("Failed to parse document\n");
		return registerelements;
    	}

	root_element = xmlDocGetRootElement(doc);

    	for (cur_node = root_element->children; cur_node; cur_node = cur_node->next) {
		Glib::ustring name = static_cast< Glib::ustring >((char const *)cur_node->name);
        	if (cur_node->type == XML_ELEMENT_NODE && name != "instructions" 
			&& name != "username" && name != "password" ) {
			registerelements.push_back(name);
        	}
	}

	xmlFreeDoc(doc);
	xmlCleanupParser();

	return registerelements;
}

int
SessionManager::registerWithServer(Glib::ustring const& username, Glib::ustring const& pw, 
				std::vector<Glib::ustring> key, std::vector<Glib::ustring> val)
{

	GError* error = NULL;
	
	LmMessage *request,*reply;
	LmMessageNode  *n;

	request = lm_message_new_with_sub_type(NULL,LM_MESSAGE_TYPE_IQ,LM_MESSAGE_SUB_TYPE_SET);
	n = lm_message_node_add_child (request->node, "query", NULL);
	lm_message_node_set_attributes (n, "xmlns", "jabber:iq:register", NULL);

	lm_message_node_add_child(n,"username",username.c_str());
	lm_message_node_add_child(n,"password",pw.c_str());

	for(unsigned i=0;i<key.size();i++)
	{	
		lm_message_node_add_child(n, (key[i]).c_str(),(val[i]).c_str());
	}
	
	
	reply = lm_connection_send_with_reply_and_block(this->session_data->connection, request, &error);
	if (error != NULL || lm_message_get_type(reply) != LM_MESSAGE_SUB_TYPE_RESULT) {
		return INVALID_AUTH;
	}

	this->session_data->jid = username + "@" + (this->session_data->chat_server.c_str()) + "/" + RESOURCE_NAME;

	prefs_set_string_attribute("whiteboard.server", "username", username.c_str());

	return this->finaliseConnection();
}

int
SessionManager::connectToServer(Glib::ustring const& server, Glib::ustring const& port, 
				Glib::ustring const& entered_username, Glib::ustring const& pw, bool usessl)
{
	GError* error = NULL;
	Glib::ustring username;
	Glib::ustring jid;

	initializeConnection(server,port,usessl);

	Glib::ustring::size_type atPos = entered_username.find('@');

     if (atPos != Glib::ustring::npos) {
 		jid += entered_username;
 		username = entered_username.substr(0, atPos);
 	} else {
 		jid += entered_username + "@" + server + "/" + RESOURCE_NAME;
 		username = entered_username;
 	}
 
	this->session_data->jid = jid;

	if (!lm_connection_authenticate_and_block(this->session_data->connection, username.c_str(), pw.c_str(), RESOURCE_NAME, &error)) {
		if (error != NULL) {
			g_warning("Failed to authenticate: %s", error->message);
		}
		lm_connection_close(this->session_data->connection, NULL);
		lm_connection_unref(this->session_data->connection);
		this->session_data->connection = NULL;
		return INVALID_AUTH;
	}

        g_log(NULL, G_LOG_LEVEL_DEBUG, "Successfully authenticated.");

	return this->finaliseConnection();
}

int 
SessionManager::finaliseConnection()
{

	GError* error = NULL;
	LmMessage* m;
	LmMessageHandler* mh;

	// Register message handler for presence messages
	mh = lm_message_handler_new((LmHandleMessageFunction)presence_handler, reinterpret_cast< gpointer >(this->_myMessageHandler), NULL);
	lm_connection_register_message_handler(this->session_data->connection, mh, LM_MESSAGE_TYPE_PRESENCE, LM_HANDLER_PRIORITY_NORMAL);

	// Register message handler for stream error messages
	mh = lm_message_handler_new((LmHandleMessageFunction)stream_error_handler, reinterpret_cast< gpointer >(this->_myMessageHandler), NULL);
	lm_connection_register_message_handler(this->session_data->connection, mh, LM_MESSAGE_TYPE_STREAM_ERROR, LM_HANDLER_PRIORITY_NORMAL);

	// Register message handler for chat messages
	mh = lm_message_handler_new((LmHandleMessageFunction)default_handler, reinterpret_cast< gpointer >(this->_myMessageHandler), NULL);
	lm_connection_register_message_handler(this->session_data->connection, mh, LM_MESSAGE_TYPE_MESSAGE, LM_HANDLER_PRIORITY_NORMAL);

	// Send presence message to server
	m = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_NOT_SET);
	if (!lm_connection_send(this->session_data->connection, m, &error)) {
		if (error != NULL) {
			g_warning("Presence message could not be sent: %s", error->message);
		}
		lm_connection_close(this->session_data->connection, NULL);
		lm_connection_unref(this->session_data->connection);
		this->session_data->connection = NULL;
		return FAILED_TO_CONNECT;
	}

	this->session_data->status.set(LOGGED_IN, 1);

	this->_myCallbacks = new Callbacks(this);

	lm_message_unref(m);

	this->_setVerbSensitivity(ESTABLISHED_CONNECTION);

	return CONNECT_SUCCESS;
}

LmSSLResponse
SessionManager::handleSSLError(LmSSL* ssl, LmSSLStatus status)
{
	if (this->session_data->ignoreFurtherSSLErrors) {
		return LM_SSL_RESPONSE_CONTINUE;
	}

	Glib::ustring msg;

	// TODO: It'd be nice to provide the user with additional information in some cases,
	// like fingerprints, hostname, etc.
	switch(status) {
		case LM_SSL_STATUS_NO_CERT_FOUND:
			msg = _("No SSL certificate was found.");
			break;
		case LM_SSL_STATUS_UNTRUSTED_CERT:
			msg = _("The SSL certificate provided by the Jabber server is untrusted.");
			break;
		case LM_SSL_STATUS_CERT_EXPIRED:
			msg = _("The SSL certificate provided by the Jabber server is expired.");
			break;
		case LM_SSL_STATUS_CERT_NOT_ACTIVATED:
			msg = _("The SSL certificate provided by the Jabber server has not been activated.");
			break;
		case LM_SSL_STATUS_CERT_HOSTNAME_MISMATCH:
			msg = _("The SSL certificate provided by the Jabber server contains a hostname that does not match the Jabber server's hostname.");
			break;
		case LM_SSL_STATUS_CERT_FINGERPRINT_MISMATCH:
			msg = _("The SSL certificate provided by the Jabber server contains an invalid fingerprint.");
			break;
		case LM_SSL_STATUS_GENERIC_ERROR:
			msg = _("An unknown error occurred while setting up the SSL connection.");
			break;
	}

	// TRANSLATORS: %1 is the message that describes the specific error that occurred when
	// establishing the SSL connection.
	Glib::ustring mainmsg = String::ucompose(_("<span weight=\"bold\" size=\"larger\">%1</span>\n\nDo you wish to continue connecting to the Jabber server?"), msg);

	Gtk::MessageDialog dlg(mainmsg, true, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE, false);
	dlg.add_button(_("Continue connecting and ignore further errors"), 0);
	dlg.add_button(_("Continue connecting, but warn me of further errors"), 1);
	dlg.add_button(_("Cancel connection"), 2);

	switch(dlg.run()) {
		case 0:
			this->session_data->ignoreFurtherSSLErrors = true;
                        /* FALL-THROUGH */
		case 1:
			return LM_SSL_RESPONSE_CONTINUE;

		default:
			return LM_SSL_RESPONSE_STOP;
	}
}

void
SessionManager::disconnectFromServer()
{
	if (this->session_data->connection) 
	{
		GError* error = NULL;

		LmMessage *m;
		this->disconnectFromDocument();
		m = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_UNAVAILABLE);
		if (!lm_connection_send(this->session_data->connection, m, &error)) {
			g_warning("Could not send unavailable presence message: %s", error->message);
		}

		lm_message_unref(m);
		lm_connection_close(this->session_data->connection, NULL);
		lm_connection_unref(this->session_data->connection);
		if (this->session_data->ssl) {
			lm_ssl_unref(this->session_data->ssl);
		}

		this->session_data->connection = NULL;
		this->session_data->ssl = NULL;
		this->_setVerbSensitivity(INITIAL);
	}
}

void
SessionManager::disconnectFromDocument()
{
	if (this->session_data->status[IN_WHITEBOARD] || !this->session_data->status[IN_CHATROOM]) {
		this->sendMessage(DISCONNECTED_FROM_USER_SIGNAL, 0, "", this->session_data->recipient, false);
	}
	this->closeSession();
	this->_setVerbSensitivity(DISCONNECTED_FROM_SESSION);
}

void
SessionManager::closeSession()
{

	if (this->session_data->status[IN_WHITEBOARD]) {
		this->session_data->status.set(IN_WHITEBOARD, 0);
		this->session_data->receive_queues.clear();
		this->session_data->send_queue->clear();
		this->stopSendQueueDispatch();
		this->stopReceiveQueueDispatch();
	}

	if (this->_myUndoObserver) {
		this->_myDoc->removeUndoObserver(*this->_myUndoObserver);
	}

	delete this->_myUndoObserver;
	delete this->_mySerializer;
	delete this->_myDeserializer;

	this->_myUndoObserver = NULL;
	this->_mySerializer = NULL;
	this->_myDeserializer = NULL;

	if (this->_myTracker) {
		delete this->_myTracker;
		this->_myTracker = NULL;
	}


	this->setRecipient(NULL);
}

void
SessionManager::setRecipient(char const* recipientJID)
{
	if (this->session_data->recipient) {
		free(const_cast< gchar* >(this->session_data->recipient));
	}

	if (recipientJID == NULL) {
		this->session_data->recipient = NULL;
	} else {
		this->session_data->recipient = g_strdup(recipientJID);
	}
}

void
SessionManager::sendChange(Glib::ustring const& msg, MessageType type, std::string const& recipientJID, bool chatroom)
{
	if (!this->session_data->status[IN_WHITEBOARD]) {
		return;
	}

	std::string& recipient = const_cast< std::string& >(recipientJID);
	if (recipient.empty()) {
		recipient = this->session_data->recipient;
	}
		

	switch (type) {
		case DOCUMENT_BEGIN:
		case DOCUMENT_END:
		case CHANGE_NOT_REPEATABLE:
		case CHANGE_REPEATABLE:
		case CHANGE_COMMIT:
		{
			MessageNode *newNode = new MessageNode(this->session_data->sequence_number++, this->session_data->jid, recipient, msg, type, false, chatroom);
			this->session_data->send_queue->insert(newNode);
			Inkscape::GC::release(newNode);
			break;
		}
		default:
			g_warning("Cannot insert MessageNode with unknown change type into send queue; discarding message.  This may lead to desynchronization!");
			break;
	}
}


// FIXME:
// This method needs a massive, massive, massive overhaul.
int
SessionManager::sendMessage(MessageType msgtype, unsigned int sequence, Glib::ustring const& msg, char const* recipientJID, bool chatroom)
{
	LmMessage* m;
	GError* error = NULL;
	char* type, * seq;

	if (recipientJID == NULL || recipientJID == "") {
		g_warning("Null recipient JID specified; not sending message.");
		return NO_RECIPIENT_JID;
	} else {
	}

	// create message
	m = lm_message_new(recipientJID, LM_MESSAGE_TYPE_MESSAGE);

	// add sender
	lm_message_node_set_attribute(m->node, "from", this->session_data->jid.c_str());

	// set message subtype according to whether or not this is
	// destined for a chatroom
	if (chatroom) {
		lm_message_node_set_attribute(m->node, "type", "groupchat");
	} else {
		lm_message_node_set_attribute(m->node, "type", "chat");
	}

	// set protocol version;
	// we are currently fixed at version 1
	lm_message_node_add_child(m->node, MESSAGE_PROTOCOL_VER, MESSAGE_PROTOCOL_V1);

	// add message type
	type = (char *)calloc(TYPE_FIELD_SIZE, sizeof(char));
	snprintf(type, TYPE_FIELD_SIZE, "%i", msgtype);
	lm_message_node_add_child(m->node, MESSAGE_TYPE, type);
	free(type);

	// add message body
	if (!msg.empty()) {
		lm_message_node_add_child(m->node, MESSAGE_BODY, msg.c_str());
	} else {
	}

	// add sequence number
	switch(msgtype) {
		case CHANGE_REPEATABLE:
		case CHANGE_NOT_REPEATABLE:
		case DUMMY_CHANGE:
		case CHANGE_COMMIT:
		case DOCUMENT_BEGIN:
		case DOCUMENT_END:
		case CONNECT_REQUEST_RESPONSE_CHAT:
		case CONNECT_REQUEST_RESPONSE_USER:
		case CHATROOM_SYNCHRONIZE_RESPONSE:
			seq = (char* )calloc(SEQNUM_FIELD_SIZE, sizeof(char));
			sprintf(seq, "%u", sequence);
			lm_message_node_add_child(m->node, MESSAGE_SEQNUM, seq);
			free(seq);
			break;

		case CONNECT_REQUEST_USER:
		case CONNECTED_SIGNAL:
		case DISCONNECTED_FROM_USER_SIGNAL:
			break;

		// Error messages and synchronization requests do not need a sequence number
		case CHATROOM_SYNCHRONIZE_REQUEST:
		case CONNECT_REQUEST_REFUSED_BY_PEER:
		case UNSUPPORTED_PROTOCOL_VERSION:
		case ALREADY_IN_SESSION:
			break;

		default:
			g_warning("Outgoing message type not recognized; not sending.");
			lm_message_unref(m);
			return UNKNOWN_OUTGOING_TYPE;
	}

	// We want to log messages even if they were not successfully sent,
	// since the user may opt to re-synchronize a session using the session
	// file record.
	if (!msg.empty()) {
		this->_log(msg);
		this->_commitLog();
	}

	// send message

	if (!lm_connection_send(this->session_data->connection, m, &error)) {
		g_warning("Send failed: %s", error->message);
		lm_message_unref(m);
		return CONNECTION_ERROR;
	}

	lm_message_unref(m);
	return SEND_SUCCESS;
}

void
SessionManager::connectionError(Glib::ustring const& errmsg)
{
	Gtk::MessageDialog dlg(errmsg, true, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
	dlg.run();
//	sp_whiteboard_connect_dialog(const_cast< gchar* >(errmsg));
}

void
SessionManager::resendDocument(char const* recipientJID, KeyToNodeMap& newidsbuf, NodeToKeyMap& newnodesbuf)
{
	Glib::ustring docbegin = MessageUtilities::makeTagWithContent(MESSAGE_DOCBEGIN, "");
	this->sendChange(docbegin, DOCUMENT_BEGIN, recipientJID, false);

	Inkscape::XML::Node* root = sp_document_repr_root(this->_myDoc);

	if(root == NULL) {
		return;
    }

	NewChildObjectMessageList newchildren;
	MessageAggregator& agg = MessageAggregator::instance();
	Glib::ustring buf;

    for ( Inkscape::XML::Node *child = root->firstChild() ; child != NULL ; child = child->next() ) {
		// TODO: replace with Serializer methods
		MessageUtilities::newObjectMessage(&buf, newidsbuf, newnodesbuf, newchildren, this->_myTracker, child);

		NewChildObjectMessageList::iterator j = newchildren.begin();
		Glib::ustring aggbuf;

		for(; j != newchildren.end(); j++) {
			if (!agg.addOne(*j, aggbuf)) {
				this->sendChange(aggbuf, CHANGE_REPEATABLE, recipientJID, false);
				aggbuf.clear();
				agg.addOne(*j, aggbuf);
			}
		}

		// send remaining changes
		if (!aggbuf.empty()) {
			this->sendChange(aggbuf, CHANGE_REPEATABLE, recipientJID, false);
			aggbuf.clear();
		}

		newchildren.clear();
		buf.clear();
    }

	Glib::ustring commit = MessageUtilities::makeTagWithContent(MESSAGE_COMMIT, "");
	this->sendChange(commit, CHANGE_COMMIT, recipientJID, false);
	Glib::ustring docend = MessageUtilities::makeTagWithContent(MESSAGE_DOCEND, "");
	this->sendChange(docend, DOCUMENT_END, recipientJID, false);
}

void
SessionManager::receiveChange(Glib::ustring const& changemsg)
{

	struct Node part;

	Glib::ustring msgcopy = changemsg.c_str();


	while(MessageUtilities::getFirstMessageTag(part, msgcopy) != false) {
		// TODO:
		// Yikes.  This is ugly.
		if (part.tag == MESSAGE_CHANGE) {
			this->_myDeserializer->deserializeEventChgAttr(part.data);
			msgcopy.erase(0, part.next_pos);

		} else if (part.tag == MESSAGE_NEWOBJ) {
			this->_myDeserializer->deserializeEventAdd(part.data);
			msgcopy.erase(0, part.next_pos);

		} else if (part.tag == MESSAGE_DELETE) {
			this->_myDeserializer->deserializeEventDel(part.data);
			msgcopy.erase(0, part.next_pos);

		} else if (part.tag == MESSAGE_DOCUMENT) {
			// no special handler, just keep going with the rest of the message
			msgcopy.erase(0, part.next_pos);

		} else if (part.tag == MESSAGE_NODECONTENT) {
			this->_myDeserializer->deserializeEventChgContent(part.data);
			msgcopy.erase(0, part.next_pos);

		} else if (part.tag == MESSAGE_ORDERCHANGE) {
			this->_myDeserializer->deserializeEventChgOrder(part.data);
			msgcopy.erase(0, part.next_pos);

		} else if (part.tag == MESSAGE_COMMIT) {
			// Retrieve the deserialized event log, node actions, and nodes with updated attributes
			XML::Event* log = this->_myDeserializer->getEventLog();
			KeyToNodeActionList& node_changes = this->_myDeserializer->getNodeTrackerActions();
			AttributesUpdatedSet& updated = this->_myDeserializer->getUpdatedAttributeNodeSet();

			// Make document insensitive to undo
			gboolean saved = sp_document_get_undo_sensitive(this->_myDoc);
			sp_document_set_undo_sensitive(this->_myDoc, FALSE);

			// Replay the log and push it onto the undo stack
			sp_repr_replay_log(log);

			// Call updateRepr on changed nodes
			// This is required for some tools to function properly, i.e. text tool
			// (TODO: we don't need to update _all_ changed nodes, just their parents)
			AttributesUpdatedSet::iterator i = updated.begin();
			for(; i != updated.end(); i++) {
				SPObject* updated = this->_myDoc->getObjectByRepr(*i);
				if (updated) {
					updated->updateRepr();
				}
			}

			// merge the events generated by updateRepr
			sp_repr_coalesce_log(this->_myDoc->priv->partial, log);
			this->_myDoc->priv->partial = NULL;

			this->_myDoc->priv->undo = g_slist_prepend(this->_myDoc->priv->undo, log);

			// Restore undo sensitivity
			sp_document_set_undo_sensitive(this->_myDoc, saved);

			// Add or delete nodes to/from the tracker
			this->_myTracker->process(node_changes);

			// Reset deserializer state
			this->_myDeserializer->reset();
			break;

		} else if (part.tag == MESSAGE_UNDO) {
			this->_myUndoObserver->lockObserverFromSending(UndoStackObserver::UNDO_EVENT);
			sp_document_undo(this->_myDoc);
			this->_myUndoObserver->unlockObserverFromSending(UndoStackObserver::UNDO_EVENT);
			msgcopy.erase(0, part.next_pos);

		} else if (part.tag == MESSAGE_REDO) {
			this->_myUndoObserver->lockObserverFromSending(UndoStackObserver::REDO_EVENT);
			sp_document_redo(this->_myDoc);
			this->_myUndoObserver->unlockObserverFromSending(UndoStackObserver::REDO_EVENT);
			msgcopy.erase(0, part.next_pos);

		} else if (part.tag == MESSAGE_DOCBEGIN) {
			msgcopy.erase(0, part.next_pos);

		} else if (part.tag == MESSAGE_DOCEND) {
			// Set this to be the new original state of the document
			sp_document_done(this->document());
			sp_document_clear_redo(this->document());
			sp_document_clear_undo(this->document());
			this->setupCommitListener();
			msgcopy.erase(0, part.next_pos);

		} else {
			msgcopy.erase(0, part.next_pos);

		}
	}

	this->_log(changemsg);

	this->_commitLog();
}

bool
SessionManager::isPlayingSessionFile()
{
	return this->session_data->status[PLAYING_SESSION_FILE];
}

void
SessionManager::loadSessionFile(Glib::ustring filename)
{
	if (!this->session_data || !this->session_data->status[IN_WHITEBOARD]) {
		try {
			if (this->_mySessionFile) {
				delete this->_mySessionFile;
			}
			this->_mySessionFile = new SessionFile(filename, true, false);

			// Initialize objects needed for session playback
			if (this->_mySessionPlayer == NULL) {
				this->_mySessionPlayer = new SessionFilePlayer(16, this);
			} else {
				this->_mySessionPlayer->load(this->_mySessionFile);
			}

			if (this->_myTracker == NULL) {
				this->_myTracker = new XMLNodeTracker(this);
			} else {
				this->_myTracker->reset();
			}

			if (!this->session_data) {
				this->session_data = new SessionData(this);
			}

			if (!this->_myDeserializer) {
				this->_myDeserializer = new Deserializer(this->node_tracker());
			}

			if (!this->_myUndoObserver) {
				this->setupCommitListener();
			}

			this->session_data->status.set(PLAYING_SESSION_FILE, 1);


		} catch (Glib::FileError e) {
			g_warning("Could not load session file: %s", e.what().data());
		}
	}
}

void
SessionManager::userConnectedToWhiteboard(gchar const* JID)
{
	sp_desktop_message_stack(this->_myDesktop)->flashF(Inkscape::INFORMATION_MESSAGE, _("Established whiteboard session with <b>%s</b>."), JID);
}


void
SessionManager::userDisconnectedFromWhiteboard(std::string const& JID)
{

	sp_desktop_message_stack(this->_myDesktop)->flashF(Inkscape::INFORMATION_MESSAGE, _("<b>%s</b> has <b>left</b> the whiteboard session."), JID.c_str());

	// Inform the user
	// TRANSLATORS: %1 is the name of the user that disconnected, %2 is the name of the user whom the disconnected user disconnected from.
	// This message is not used in a chatroom context.
	Glib::ustring primary = String::ucompose(_("<span weight=\"bold\" size=\"larger\">The user <b>%1</b> has left the whiteboard session.</span>\n\n"), JID);
	// TRANSLATORS: %1 and %2 are userids
	Glib::ustring secondary = String::ucompose(_("You are still connected to a Jabber server as <b>%2</b>, and may establish a new session to <b>%1</b> or a different user."), JID, this->session_data->jid);

	// TODO: parent this dialog to the active desktop
	Gtk::MessageDialog dialog(primary + secondary, true, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, false);
	/*
	dialog.set_message(primary, true);
	dialog.set_secondary_text(secondary, true);
	*/
	dialog.run();
}

void
SessionManager::startSendQueueDispatch()
{
	this->_send_queue_dispatcher = Glib::signal_timeout().connect(sigc::mem_fun(*(this->_myCallbacks), &Callbacks::dispatchSendQueue), SEND_TIMEOUT);
}

void
SessionManager::stopSendQueueDispatch()
{
	if (this->_send_queue_dispatcher) {
		this->_send_queue_dispatcher.disconnect();
	}
}

void
SessionManager::startReceiveQueueDispatch()
{
	this->_receive_queue_dispatcher = Glib::signal_timeout().connect(sigc::mem_fun(*(this->_myCallbacks), &Callbacks::dispatchReceiveQueue), SEND_TIMEOUT);
}

void
SessionManager::stopReceiveQueueDispatch()
{
	if (this->_receive_queue_dispatcher) {
		this->_receive_queue_dispatcher.disconnect();
	}
}

void
SessionManager::clearDocument()
{
	// clear all layers, definitions, and metadata
	XML::Node* rroot = this->_myDoc->rroot;

	// clear definitions
	XML::Node* defs = SP_OBJECT_REPR((SPDefs*)SP_DOCUMENT_DEFS(this->_myDoc));
	g_assert(SP_ROOT(this->_myDoc->root)->defs);

	for(XML::Node* child = defs->firstChild(); child; child = child->next()) {
		defs->removeChild(child);
	}

	// clear layers
	for(XML::Node* child = rroot->firstChild(); child; child = child->next()) {
		if (strcmp(child->name(), "svg:g") == 0) {
			rroot->removeChild(child);
		}
	}

	// clear metadata
	for(XML::Node* child = rroot->firstChild(); child; child = child->next()) {
		if (strcmp(child->name(), "svg:metadata") == 0) {
			rroot->removeChild(child);
		}
	}

//	sp_document_done(this->_myDoc);
}

void
SessionManager::setupInkscapeInterface()
{
	this->session_data->status.set(IN_WHITEBOARD, 1);
	this->startSendQueueDispatch();
	this->startReceiveQueueDispatch();
	if (!this->_myTracker) {
		this->_myTracker = new XMLNodeTracker(this);
	}

	this->_mySerializer = new Serializer(this->node_tracker());
	this->_myDeserializer = new Deserializer(this->node_tracker());

	// We're in a whiteboard session now, so set verb sensitivity accordingly
	this->_setVerbSensitivity(ESTABLISHED_SESSION);
}

void
SessionManager::setupCommitListener()
{
	this->_myUndoObserver = new Whiteboard::UndoStackObserver(this);
	this->_myDoc->addUndoObserver(*this->_myUndoObserver);
}

::SPDesktop*
SessionManager::desktop()
{
	return this->_myDesktop;
}

::SPDocument*
SessionManager::document()
{
	return this->_myDoc;
}

Callbacks*
SessionManager::callbacks()
{
	return this->_myCallbacks;
}

Whiteboard::UndoStackObserver*
SessionManager::undo_stack_observer()
{
	return this->_myUndoObserver;
}

Serializer*
SessionManager::serializer()
{
	return this->_mySerializer;
}

XMLNodeTracker*
SessionManager::node_tracker()
{
	return this->_myTracker;
}


ChatMessageHandler*
SessionManager::chat_handler()
{
	return this->_myChatHandler;
}

SessionFilePlayer*
SessionManager::session_player()
{
	return this->_mySessionPlayer;
}

SessionFile*
SessionManager::session_file()
{
	return this->_mySessionFile;
}

void
SessionManager::_log(Glib::ustring const& message)
{
	if (this->_mySessionFile && !this->_mySessionFile->isReadOnly()) {
		this->_mySessionFile->addMessage(message);
	}
}

void
SessionManager::_commitLog()
{
	if (this->_mySessionFile && !this->_mySessionFile->isReadOnly()) {
		this->_mySessionFile->commit();
	}
}

void
SessionManager::_closeLog()
{
	if (this->_mySessionFile) {
		this->_mySessionFile->close();
	}
}

void
SessionManager::startLog(Glib::ustring filename)
{
	try {
		this->_mySessionFile = new SessionFile(filename, false, false);
	} catch (Glib::FileError e) {
		g_warning("Caught I/O error %s while attemping to open file %s for session recording.", e.what().c_str(), filename.c_str());
		throw;
	}
}

void
SessionManager::_tryToStartLog()
{
	if (this->session_data) {
		if (!this->session_data->sessionFile.empty()) {
			bool undecided = true;
			while(undecided) {
				try {
					this->startLog(this->session_data->sessionFile);
					undecided = false;
				} catch (Glib::FileError e) {
					undecided = true;
					Glib::ustring msg = String::ucompose(_("Could not open file %1 for session recording.\nThe error encountered was: %2.\n\nYou may select a different location to record the session, or you may opt to not record this session."), this->session_data->sessionFile, e.what());
					Gtk::MessageDialog dlg(msg, true, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_NONE, false);
					dlg.add_button(_("Choose a different location"), 0);
					dlg.add_button(_("Skip session recording"), 1);
					switch (dlg.run()) {
						case 0:
						{
							Gtk::FileChooserDialog sessionfiledlg(_("Select a location and filename"), Gtk::FILE_CHOOSER_ACTION_SAVE);
							sessionfiledlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
							sessionfiledlg.add_button(_("Set filename"), Gtk::RESPONSE_OK);
							int result = sessionfiledlg.run();
							switch (result) {
								case Gtk::RESPONSE_OK:
								{
									this->session_data->sessionFile = sessionfiledlg.get_filename();
									break;
								}
								case Gtk::RESPONSE_CANCEL:
								default:
									undecided = false;
									break;
							}
							break;
						}
						case 1:
						default:
							undecided = false;
							break;
					}
				}
			}
		}
	}
}

void
SessionManager::_setVerbSensitivity(SensitivityMode mode)
{
	return;

	switch (mode) {
		case ESTABLISHED_CONNECTION:
			// Upon successful connection, we can disconnect from the server.
			// We can also start sharing a document with a user or chatroom.
			// We cannot, however, connect to a new server without first disconnecting.
			Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_CONNECT)->sensitive(this->_myDoc, false);
			Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_DISCONNECT_FROM_SERVER)->sensitive(this->_myDoc, true);
			Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_SHAREWITHUSER)->sensitive(this->_myDoc, true);
			Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_SHAREWITHCHAT)->sensitive(this->_myDoc, true);
			break;

		case ESTABLISHED_SESSION:
			// When we have established a session, we should not permit the user to go and
			// establish another session from the same document without first disconnecting.
			//
			// TODO: Well, actually, we probably _should_, but there's no real reconnection logic just yet.
			Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_SHAREWITHUSER)->sensitive(this->_myDoc, false);
			Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_SHAREWITHCHAT)->sensitive(this->_myDoc, false);
			Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_DISCONNECT_FROM_SESSION)->sensitive(this->_myDoc, true);
			break;
		case DISCONNECTED_FROM_SESSION:
			// Upon disconnecting from a session, we can establish a new session and disconnect
			// from the server, but we cannot disconnect from a session (since we just left it.)
			Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_DISCONNECT_FROM_SESSION)->sensitive(this->_myDoc, false);
			Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_SHAREWITHUSER)->sensitive(this->_myDoc, true);
			Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_SHAREWITHCHAT)->sensitive(this->_myDoc, true);

		case INITIAL:
		default:
			// Upon construction, there is no active connection, so we cannot do the following:
			// (1) disconnect from a session
			// (2) disconnect from a server
			// (3) share with a user 
			// (4) share with a chatroom
			Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_CONNECT)->sensitive(this->_myDoc, true);
			Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_SHAREWITHUSER)->sensitive(this->_myDoc, false);
			Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_SHAREWITHCHAT)->sensitive(this->_myDoc, false);
			Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_DISCONNECT_FROM_SESSION)->sensitive(this->_myDoc, false);
			Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_DISCONNECT_FROM_SERVER)->sensitive(this->_myDoc, false);
			break;
	};
}

/*
void
SessionManager::Listener::processXmppEvent(Pedro::XmppEvent const& event)
{
	int type = event.getType();

	switch (type) {
		case Pedro::XmppEvent::EVENT_STATUS:
			break;
		case Pedro::XmppEvent::EVENT_ERROR:
			break;
		case Pedro::XmppEvent::EVENT_CONNECTED:
			break;
		case Pedro::XmppEvent::EVENT_DISCONNECTED:
			break;
		case Pedro::XmppEvent::EVENT_MESSAGE:
			break;
		case Pedro::XmppEvent::EVENT_PRESENCE:
			break;
		case Pedro::XmppEvent::EVENT_MUC_MESSAGE:
			break;
		case Pedro::XmppEvent::EVENT_MUC_JOIN:
			break;
		case Pedro::XmppEvent::EVENT_MUC_LEAVE:
			break;
		case Pedro::XmppEvent::EVENT_MUC_PRESENCE:
			break;
	}
}
*/

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

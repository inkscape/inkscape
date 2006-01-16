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

#ifndef __SESSION_MANAGER_H__
#define __SESSION_MANAGER_H__

#include <glibmm.h>
#include <set>
#include <bitset>

extern "C" {
#include <loudmouth/loudmouth.h>
}

#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/buddy-list-manager.h"

#include "gc-alloc.h"

struct SPDesktop;
struct SPDocument;

namespace Inkscape {
namespace XML {
class Node;
}
}

namespace Inkscape {

namespace Whiteboard {

class ReceiveMessageQueue;
class SendMessageQueue;
class XMLNodeTracker;
class SessionManager;
class MessageHandler;
class ChatMessageHandler;
class Callbacks;
class SessionFile;
class SessionFilePlayer;
class UndoStackObserver;
class Serializer;
class Deserializer;

/// Jabber resource name
#define RESOURCE_NAME	"Inkboard"

/// connectToServer return values
#define CONNECT_SUCCESS		0
#define FAILED_TO_CONNECT	1
#define INVALID_AUTH		2
#define SSL_INITIALIZATION_ERROR	3

/// sendMessage return values
#define SEND_SUCCESS			0
#define CONNECTION_ERROR		1
#define UNKNOWN_OUTGOING_TYPE	2
#define NO_RECIPIENT_JID		3

/**
 * Structure grouping data items pertinent to a whiteboard session.
 *
 * SessionData holds all session data for both 1:1 and chatroom conferences.
 * Access to members should be controlled by first querying the status bitset
 * to see if useful data will actually exist in that member -- i.e. checking
 * status[IN_CHATROOM] to see if the chatters set will contain anything.
 * It usually won't hurt to do a straight query -- there are very few members
 * that remain uninitialized for very long -- but it's a good idea to check.
 */
struct SessionData {
public:
	/**
	 * Constructor.
	 *
	 * \param sm The SessionManager with which a SessionData instance should be
	 * associated with.
	 */
	SessionData(SessionManager *sm);

	~SessionData();

	/**
	 * The JID of the recipient: either another user JID or the JID of a chatroom.
	 */
	gchar const* recipient;

	/**
	 * Pointer to Loudmouth connection structure.
	 * Used for Loudmouth calls that require it.
	 */
	LmConnection* connection;

	/**
	 * SSL information structure for SSL connections.
	 */
	LmSSL* ssl;

	/**
	 * Flag indicating whether or not we should ignore further SSL errors for a given session.
	 */
	bool ignoreFurtherSSLErrors;


	/**
	 * A user's handle in a Jabber chatroom.
	 */
	Glib::ustring chat_handle;

	/**
	 * Name of the chatroom that a user in a chatroom is connected to.
	 */
	Glib::ustring chat_name;

	/**
	 * Name of the conference server.
	 */
	Glib::ustring chat_server;

	// Message queues
	
	/**
	 * Map associating senders to receive queues.
	 */
	RecipientToReceiveQueueMap receive_queues;

	/**
	 * Map associating senders to commit events sent by those committers.
	 */
	CommitsQueue recipients_committed_queue;

	/**
	 * Pointer to queue for messages to be sent.
	 */
	SendMessageQueue* send_queue;

	// Message sequence numbers
	
	/**
	 * The sequence number of the latest message sent by this client in a given session.
	 * Used for determining the sequence number of the next message.
	 */
	unsigned int sequence_number;

	//unsigned int latest_sent_transaction;
	//RecipientToLatestTransactionMap latest_processed_transactions;


	// Status tracking
	/**
	 * Session state and status flags.
	 */
	std::bitset< NUM_FLAGS > status;
	
	/**
	 * Jabber buddy list data.
	 */
	BuddyListManager buddyList;

	/**
	 * List of participants in a Jabber chatroom.
	 */
	ChatterList chatters;

	/**
	 * Session file filename; blank if no session file is to be
	 * recorded.
	 */
	Glib::ustring sessionFile;

private:
	// access to containing class
	SessionManager *_sm;

	// noncopyable, nonassignable
	SessionData(SessionData const&);
	SessionData& operator=(SessionData const&);
};


// TODO: This class is huge.  It might be best to refactor it into smaller,
// more coherent chunks.
//
// TODO: convert to pass-by-reference where appropriate.  In particular, a lot of the
// string buffers passed to methods in the argument list can be made into references
// appropriately and easily.

/**
 * Session management class for Inkboard.
 *
 * By "session management", we refer to the management of all events that an Inkboard
 * session may need to handle: negotiating a connection to a Jabber server, negotiating
 * sessions with users and chatrooms, sending, receiving, and parsing messages, and so
 * forth.
 *
 * SessionManager instances are associated with Inkscape desktop objects on a 1:1 basis.
 */
class SessionManager {
public:
	/**
	 * Constructor.
	 *
	 * \param desktop The desktop with which this SessionManager is associated.  */
	SessionManager(::SPDesktop *desktop);
	~SessionManager();

	// Session tracking data
	
	/** 
	 * Pointer to SessionData structure.
	 */
	struct SessionData *session_data;

	// Inkscape interface
	
	/**
	 * Set the desktop with which this SessionManager is associated.
	 *
	 * @param desktop the desktop with which this SessionManager should be associated
	 */
	void setDesktop(::SPDesktop* desktop);
	
	// Session management
	
	/**
	 * Connect to a Jabber server.
	 *
	 * @param server Jabber server URL
	 * @param username Jabber username
	 * @param pw password for Jabber account
	 * @param usessl use SSL for connection
	 *
	 * @return CONNECT_SUCCESS if connection successful; FAILED_TO_CONNECT if connection failed or INVALID_AUTH
	 * if authentication invalid
	 */
	int connectToServer(Glib::ustring const& server, Glib::ustring const& port, Glib::ustring const& username, Glib::ustring const& pw, bool usessl);

	/**
	 * Handle an SSL error by prompting the user for feedback, and continuing or aborting the connection
	 * process based on that feedback.
	 *
	 * @param ssl pointer to LmSSL structure
	 * @param status The error message
	 *
	 * @return LM_SSL_RESPONSE_CONTINUE if user wishes to continue establishing the connection or LM_SSL_RESPONSE_STOP if user wishes to abort connection
	 */
	LmSSLResponse handleSSLError(LmSSL* ssl, LmSSLStatus status);

	/**
	 * Disconnect from a Jabber server.  
	 *
	 * This invokes disconnectFromDocument().
	 *
	 * \see Inkscape::Whiteboard::SessionManager::disconnectFromDocument
	 */
	void disconnectFromServer();

	/**
	 * Disconnect from a document session.  The connection to the Jabber server is not 
	 * broken, and may be reused to connect to a new document session.
	 *
	 */
	void disconnectFromDocument();

	/**
	 * Perform session teardown.  This method by itself does not disconnect from a document or 
	 * a Jabber server.
	 *
	 */
	void closeSession();

	/**
	 * Set the recipient for Inkboard messages.
	 *
	 * @param recipientJID the recipient's JID
	 */
	void setRecipient(char const* recipientJID);

	// Message sending utilities
	
	/**
	 * Put an Inkboard message into the send queue.
	 * This method does not actually send anything to an Inkboard client.
	 *
	 * \see Inkscape::Whiteboard::SessionManager::sendMessage
	 *
	 *
	 * @param msg the message to send
	 * @param type the type of message (only CHANGE_* types permitted)
	 * @param chatroom whether or not this message is destined for a chatroom
	 */
	void sendChange(Glib::ustring const& msg, MessageType type, std::string const& recipientJID, bool chatroom);

	/**
	 * Send a message to an Inkboard client.
	 *
	 *
	 * @param msgtype the type of message to send
	 * @param sequence message sequence number
	 * @param msg the message to send
	 * @param recipientJID the JID of the recipient
	 * @param chatroom whether or not this message is destined for a chatroom
	 *
	 * @return SEND_SUCCESS if successful; otherwise: UNKNOWN_OUTGOING_TYPE if msgtype is not recognized, NO_RECIPIENT_JID if recipientJID is NULL or blank, CONNECTION_ERROR if Jabber connection error occurred
	 */
	int sendMessage(MessageType msgtype, unsigned int sequence, Glib::ustring const& msg, char const* recipientJID, bool chatroom);

	/**
	 * Inform the user of a connection error via a Gtk::MessageDialog.
	 *
	 * @param errmsg message to display
	 */
	void connectionError(Glib::ustring const& errmsg);

	/**
	 * Stream the contents of the document with which this SessionManager is associated with to the given recipient.
	 * 
	 * @param recipientJID the JID of the recipient
	 * @param newidsbuf buffer to store IDs of new nodes 
	 * @param newnodesbuf buffer to store address of new nodes 
	 */
	void resendDocument(char const* recipientJID, KeyToNodeMap& newidsbuf, NodeToKeyMap& newnodesbuf);
	
	
	/**
	 * Send a connection request to another Inkboard client.
	 *
	 *
	 * @param recipientJID the JID to connect to
	 * @param document document message to send
	 */
	void sendRequestToUser(std::string const& recipientJID);

	/**
	 * Send a connection request to chatroom.
	 * 
	 * @param server server to connect to
	 * @param chatroom name of chatroom
	 * @param handle chatroom handle to use
	 * @param password chatroom password; leave NULL if no password
	 */
	void sendRequestToChatroom(Glib::ustring const& server, Glib::ustring const& chatroom, Glib::ustring const& handle, Glib::ustring const& password);

	/**
	 * Send a connection request response to a user who requested to connect to us.
	 *
	 * @param requesterJID the JID of the user whom sent us the request
	 * @param accepted_request whether or not we accepted the request
	 */
	void sendConnectRequestResponse(char const* requesterJID, gboolean accepted_request); 

	/**
	 * Method called when a connection request is received.  This method produces a dialog
	 * that asks the user whether or not s/he would like to accept the request.
	 *
	 *
	 * @param requesterJID the JID of the user whom sent us the request
	 * @param msg the message associated with this request
	 */
	void receiveConnectRequest(gchar const* requesterJID);

	/**
	 * Method called when a response to a connection request is received.
	 * This method performs any necessary session setup/teardown and user notification
	 * depending on the response received.
	 *
	 *
	 * @param msg the message associated with this request
	 * @param response the response code
	 * @param sender the JID of the user whom responded to our request
	 */
	void receiveConnectRequestResponse(InvitationResponses response, std::string& sender);

	/**
	 * Method called when a document synchronization request is received from a new conference
	 * member in a chatroom.
	 *
	 * \param recipient the recipient JID
	 */
	void receiveConnectRequestResponseChat(gchar const* recipient);

	// Message parsing and passing
	
	/**
	 * Processes a group of document change messages.
	 *
	 * \param changemsg The change message group to process.
	 */
	void receiveChange(Glib::ustring const& changemsg);

	// Logging and session file handling
	/**
	 * Start a session log with the given filename.
	 *
	 * \param filename Full path to the file that the session log should be written to.
	 * \throw Glib::FileError Thrown if an exception is thrown during session file creation.
	 */
	void startLog(Glib::ustring filename);

	/**
	 * Load a session file for playback.
	 *
	 * \param filename Full path to the session file that is to be loaded.
	 */
	void loadSessionFile(Glib::ustring filename);

	/**
	 * Returns whether or not the session is in session file playback mode.
	 *
	 * \return Whether or not the session is in session file playback mode.
	 */
	bool isPlayingSessionFile();

	// User event notification
	
	/**
	 * Method to notify the user that a whiteboard session to another user has been successfully
	 * established.
	 *
	 * \param JID The JID with whom the user established a session.
	 */
	void userConnectedToWhiteboard(gchar const* JID);

	/**
	 * Method to notify the user that the other user in a user-to-user whiteboard session
	 * has disconnected.
	 *
	 * \param JID The JID of the user who left the whiteboard session.
	 */
	void userDisconnectedFromWhiteboard(std::string const& JID);

	// Queue dispatching and UI setup
	
	/**
	 * Start the send queue for this session.
	 */
	void startSendQueueDispatch();

	/**
	 * Stop the send queue for this session.
	 */
	void stopSendQueueDispatch();

	/**
	 * Start the receive queue for this session.
	 */
	void startReceiveQueueDispatch();

	/**
	 * Stop the receive queue for this session.
	 */
	void stopReceiveQueueDispatch();

	/**
	 * Clear all layers, definitions, and metadata from the document with which a
	 * SessionManager instance is associated. 
	 *
	 * Documents are cleared to assist synchronization between two clients
	 * or a client and a chatroom.
	 */
	void clearDocument();

	/**
	 * Set up objects for handling actions generated by the user interacting with
	 * Inkscape.  This includes marking the active session as being in a whiteboard session,
	 * starting send and receive queues, and creating an event serializer and deserializer.
	 *
	 * \see Inkscape::Whiteboard::SendMessageQueue
	 * \see Inkscape::Whiteboard::ReceiveMessageQueue
	 * \see Inkscape::Whiteboard::Serializer
	 * \see Inkscape::Whiteboard::Deserializer
	 */
	void setupInkscapeInterface();

	/**
	 * Reset whiteboard verbs to INITIAL state.
	 */
	void setInitialVerbSensitivity() {
		this->_setVerbSensitivity(INITIAL);
	}

	/**
	 * Set up the event commit listener.
	 *
	 * The event commit listener watches for events that are committed to the document's undo log,
	 * serializes those events, and then adds them to the message send queue.
	 *
	 * \see Inkscape::Whiteboard::SendMessageQueue
	 * \see Inkscape::Whiteboard::UndoStackObserver
	 */
	void setupCommitListener();

	// Private object retrieval
	::SPDesktop* desktop();
	::SPDocument* document();
	Callbacks* callbacks();
	Whiteboard::UndoStackObserver* undo_stack_observer();
	Serializer* serializer();
	XMLNodeTracker* node_tracker();
	Deserializer* deserializer();
	ChatMessageHandler* chat_handler();
	SessionFilePlayer* session_player();
	SessionFile* session_file();

private:
	// Internal logging methods
	void _log(Glib::ustring const& message);
	void _commitLog();
	void _closeLog();
	void _tryToStartLog();

	enum SensitivityMode {
			INITIAL,
			ESTABLISHED_CONNECTION,
			ESTABLISHED_SESSION,
			DISCONNECTED_FROM_SESSION
	};

	void _setVerbSensitivity(SensitivityMode mode);

	bool _pollReceiveConnectRequest(Glib::ustring const recipient);

	::SPDesktop* _myDesktop;
	::SPDocument* _myDoc;
	Whiteboard::UndoStackObserver* _myUndoObserver;
	XMLNodeTracker* _myTracker;
	ChatMessageHandler* _myChatHandler;
	Callbacks* _myCallbacks;
	SessionFile* _mySessionFile;
	SessionFilePlayer* _mySessionPlayer;
	MessageHandler* _myMessageHandler;
	Serializer* _mySerializer;
	Deserializer* _myDeserializer; 

	sigc::connection _send_queue_dispatcher;
	sigc::connection _receive_queue_dispatcher;
	sigc::connection _notify_incoming_request;

	// noncopyable, nonassignable
	SessionManager(SessionManager const&);
	SessionManager& operator=(SessionManager const&);
};

}

}

#endif

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

/**
 * Whiteboard session manager
 * Definitions
 * 
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_DEFINES_H__
#define __WHITEBOARD_DEFINES_H__

#include "jabber_whiteboard/message-tags.h"
#include "jabber_whiteboard/internal-constants.h"

namespace Inkscape {

namespace Whiteboard {

// message types
// explicitly numbered to aid protocol description later on
enum MessageType {
    // image and internal data
    CHANGE_NOT_REPEATABLE = 0,
	CHANGE_REPEATABLE = 1,
	DUMMY_CHANGE = 2,
	CHANGE_COMMIT = 3,
	DOCUMENT_BEGIN = 4,
	DOCUMENT_END = 5,
    // 1-1 connections
    CONNECT_REQUEST_USER = 6,
	CONNECT_REQUEST_RESPONSE_USER = 7, 
    // chat connections
    CONNECT_REQUEST_RESPONSE_CHAT = 8,
	// chatroom document synchronization
	CHATROOM_SYNCHRONIZE_REQUEST = 9, 
	CHATROOM_SYNCHRONIZE_RESPONSE = 10,
    // requests
    DOCUMENT_SENDER_REQUEST = 11,
	DOCUMENT_SENDER_REQUEST_RESPONSE = 12, 
	DOCUMENT_REQUEST = 13,
    // notifications
    CONNECTED_SIGNAL = 14,
	DISCONNECTED_FROM_USER_SIGNAL = 15,
	// error responses
	CONNECT_REQUEST_REFUSED_BY_PEER = 16, 
	UNSUPPORTED_PROTOCOL_VERSION = 17,
	ALREADY_IN_SESSION = 18,
	
	// error cases, i.e. garbled messages or bad clients.  These should
	// never actually be transmitted
	UNKNOWN = 21
};

// Responses to whiteboard invitations
enum InvitationResponses {
	ACCEPT_INVITATION,
	ACCEPT_INVITATION_IN_NEW_WINDOW,
	DECLINE_INVITATION,
	PEER_ALREADY_IN_SESSION
};

// Message handler modes
enum HandlerMode {
	DEFAULT,
	PRESENCE,
	ERROR
};

// Actions to pass to the node tracker when we modify a node in 
// the document tree upon event serialization
enum NodeTrackerAction {
	NODE_ADD,
	NODE_REMOVE,
	NODE_UNKNOWN
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

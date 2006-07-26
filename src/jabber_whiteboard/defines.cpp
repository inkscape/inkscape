/**
 * Whiteboard session manager
 * Definitions
 * 
 * Authors:
 * Dale Harvey <harveyd@gmail.com>
 *
 * Copyright (c) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __INKSCAPE_WHITEBOARD_DEFINES_CPP__
#define __INKSCAPE_WHITEBOARD_DEFINES_CPP__

#include "jabber_whiteboard/defines.h"

namespace Inkscape {

namespace Whiteboard {

char const * const MessageString[] = {
    // image and internal data
        "CHANGE_NOT_REPEATABLE",
	"CHANGE_REPEATABLE",
	"DUMMY_CHANGE",
	"CHANGE_COMMIT",
	"DOCUMENT_BEGIN",
	"DOCUMENT_END",

    // 1-1 connections
        "connect-request",
	"CONNECT_REQUEST_RESPONSE_USER", 
    // chat connections
        "CONNECT_REQUEST_RESPONSE_CHAT",

    // chatroom document synchronization
	"CHATROOM_SYNCHRONIZE_REQUEST", 
	"CHATROOM_SYNCHRONIZE_RESPONSE",

    // requests
        "DOCUMENT_SENDER_REQUEST",
	"DOCUMENT_SENDER_REQUEST_RESPONSE", 
	"DOCUMENT_REQUEST",

    // notifications
        "CONNECTED_SIGNAL",
	"DISCONNECTED_FROM_USER_SIGNAL",

    // error responses
	"CONNECT_REQUEST_REFUSED_BY_PEER", 
	"UNSUPPORTED_PROTOCOL_VERSION",
	"ALREADY_IN_SESSION",
	
    // error cases, i.e. garbled messages or bad clients.  These should
    // never actually be transmitted
	"UNKNOWN"
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

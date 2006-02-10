/**
 * Whiteboard session manager
 * Inkboard message context definitions
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/message-contexts.h"

#include <bitset>

namespace Inkscape {

namespace Whiteboard {

void
initialize_received_message_contexts(MessageContextMap& mcm)
{

	// Each Inkboard message has a context of validity according to an Inkboard 
	// client's state.  That is, certain messages should be acknowledged and processed
	// in some states, whereas other messages should not.
	// For instance, a whiteboard invitation should be acknowledged if an Inkboard
	// client is not in a whiteboard session, but should be refused if said client
	// _is_ in a whiteboard session.

	// Only the flags that are required to be set must be set; all flags, by default,
	// are zero.  Explicit definition doesn't hurt, though.

	// The general format of message context creation and registration is
	// std::bitset< NUM_FLAGS > m_
	// m_.set( );
	// m_.set( );
	// mcm[ ] = m_;


	// Begin

	// Special bitsets
	std::bitset< NUM_FLAGS > all_contexts;
	all_contexts.flip();

	// Messages: CHANGE_NOT_REPEATABLE, CHANGE_REPEATABLE, DUMMY_CHANGE, CHANGE_COMMIT
	std::bitset< NUM_FLAGS > m1;
	m1.set(LOGGED_IN);
	m1.set(IN_WHITEBOARD);
	m1.set(IN_CHATROOM);
	m1.set(SYNCHRONIZING_WITH_CHAT);
	mcm[CHANGE_NOT_REPEATABLE] = m1;
	mcm[CHANGE_REPEATABLE] = m1;
	mcm[DUMMY_CHANGE] = m1;
	mcm[CHANGE_COMMIT] = m1;

	// Messages: DOCUMENT_BEGIN, DOCUMENT_END
	std::bitset< NUM_FLAGS > m4;
	m4.set(LOGGED_IN);
	m4.set(IN_WHITEBOARD);
	m4.set(SYNCHRONIZING_WITH_CHAT);
	mcm[DOCUMENT_BEGIN] = m4;
	mcm[DOCUMENT_END] = m4;

	// Message: CONNECT_REQUEST_USER
	std::bitset< NUM_FLAGS > m5;
	m5.set(LOGGED_IN);

	// We should still _accept_ CONNECT_REQUEST_USER messages even if we're already
	// waiting for a response.  It is up to the higher-level handler (i.e. the connection
	// request handler) to properly handle it.
	m5.set(WAITING_FOR_INVITE_RESPONSE);
	mcm[CONNECT_REQUEST_USER] = m5;

	// Message: CONNECT_REQUEST_RESPONSE_USER
	std::bitset< NUM_FLAGS > m6;
	m6.set(LOGGED_IN);
	m6.set(WAITING_FOR_INVITE_RESPONSE);
	mcm[CONNECT_REQUEST_RESPONSE_USER] = m6;

	// Message: CHATROOM_SYNCHRONIZE_REQUEST
	std::bitset< NUM_FLAGS > m7;
	m7.set(LOGGED_IN);
	m7.set(IN_CHATROOM);
	m7.set(IN_WHITEBOARD);
	mcm[CHATROOM_SYNCHRONIZE_REQUEST] = m7;

	// Message: CHATROOM_SYNCHRONIZE_RESPONSE
	std::bitset< NUM_FLAGS > m8;
	m8.set(LOGGED_IN);
	m8.set(WAITING_TO_SYNC_TO_CHAT);
	mcm[CHATROOM_SYNCHRONIZE_RESPONSE] = m8;
	
	// Message: CONNECT_REQUEST_RESPONSE_CHAT
	std::bitset< NUM_FLAGS > m9;
	m9.set(LOGGED_IN);
	m9.set(IN_CHATROOM);
	m9.set(IN_WHITEBOARD);
	mcm[CONNECT_REQUEST_RESPONSE_CHAT] = m9;

	// Message: CONNECTED_SIGNAL
	mcm[CONNECTED_SIGNAL] = all_contexts;

	// Message: DISCONNECTED_FROM_USER_SIGNAL
	std::bitset< NUM_FLAGS > m11;
	m11.set(LOGGED_IN);
	m11.set(IN_WHITEBOARD);
	mcm[DISCONNECTED_FROM_USER_SIGNAL] = m11;

	// Messages: CONNECT_REQUEST_REFUSED_BY_PEER, ALREADY_IN_SESSION
	std::bitset< NUM_FLAGS > m12;
	m12.set(LOGGED_IN);
	m12.set(WAITING_FOR_INVITE_RESPONSE);
	mcm[CONNECT_REQUEST_REFUSED_BY_PEER] = m12;
	mcm[ALREADY_IN_SESSION] = m12;

	// Message: UNSUPPORTED_PROTOCOL_VERSION
	std::bitset< NUM_FLAGS > m14;
	m14.set(LOGGED_IN);
	mcm[UNSUPPORTED_PROTOCOL_VERSION] = m14;
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

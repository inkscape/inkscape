/**
 * Whiteboard session manager
 * Internal constants
 * 
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_INTERNAL_CONSTANTS_H__
#define __WHITEBOARD_INTERNAL_CONSTANTS_H__

namespace Inkscape {

namespace Whiteboard {

// TODO: breaking these up into namespaces would be nice, but it's too much typing
// for now

// Protocol versions
extern char const* MESSAGE_PROTOCOL_V1;
extern int const HIGHEST_SUPPORTED;

// Node types (as strings)
extern char const* NODETYPE_DOCUMENT_STR;
extern char const* NODETYPE_ELEMENT_STR;
extern char const* NODETYPE_TEXT_STR;
extern char const* NODETYPE_COMMENT_STR;

// Number of chars to allocate for type field (in SessionManager::sendMessage)
extern int const TYPE_FIELD_SIZE;

// Number of chars to allocate for sequence number field (in SessionManager::sendMessage)
extern int const SEQNUM_FIELD_SIZE;

// Designators for certain "special" nodes in the document
// These nodes are "special" because they are generally present in all documents
extern char const* DOCUMENT_ROOT_NODE;
extern char const* DOCUMENT_NAMEDVIEW_NODE;

// Names of these special nodes
extern char const* DOCUMENT_ROOT_NAME;
extern char const* DOCUMENT_NAMEDVIEW_NAME;

// Inkboard client states
extern int const IN_WHITEBOARD;
extern int const LOGGED_IN;
extern int const IN_CHATROOM;
extern int const WAITING_FOR_INVITE_RESPONSE;
extern int const CONNECTING_TO_CHAT;
extern int const WAITING_TO_SYNC_TO_CHAT;
extern int const SYNCHRONIZING_WITH_CHAT;
extern int const OPEN_FOR_DOC;
extern int const PLAYING_SESSION_FILE;

// update this if any other status flags are added
#define NUM_FLAGS	9

// TODO: make this user-configurable, within sane limits
// ("sane" limits being roughly in the range (10, 100], from personal testing)
// Based on discussions with Ted, it seems that we're going to make the Jabber guys
// accomodate Inkscape, not the other way around...
// Dispatch interval (in milliseconds)
extern int const SEND_TIMEOUT;

}

}

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

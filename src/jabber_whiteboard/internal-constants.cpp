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

#include "jabber_whiteboard/internal-constants.h"

namespace Inkscape {

namespace Whiteboard {
// Protocol versions
char const* MESSAGE_PROTOCOL_V1 =	"1";
char const* MESSAGE_PROTOCOL_V2	=	"2";
int const HIGHEST_SUPPORTED =		1;

// Node types (as strings)
char const* NODETYPE_DOCUMENT_STR =	"document";
char const* NODETYPE_ELEMENT_STR =	"element";
char const* NODETYPE_TEXT_STR =		"text";
char const* NODETYPE_COMMENT_STR = 	"comment";

// Number of chars to allocate for type field (in SessionManager::sendMessage)
int const TYPE_FIELD_SIZE =			5;

// Number of chars to allocate for sequence number field (in SessionManager::sendMessage)
int const SEQNUM_FIELD_SIZE	=	70;

// Designators for certain "special" nodes in the document
// These nodes are "special" because they are generally present in all documents,
// and we generally only want one copy of them
char const* DOCUMENT_ROOT_NODE =		"ROOT";
char const* DOCUMENT_NAMEDVIEW_NODE =	"NAMEDVIEW";

// Names of these special nodes
char const* DOCUMENT_ROOT_NAME =		"svg:svg";
char const* DOCUMENT_NAMEDVIEW_NAME =	"sodipodi:namedview";

// Inkboard client states
int const IN_WHITEBOARD =				0;
int const LOGGED_IN	=					1;
int const IN_CHATROOM =					2;
int const WAITING_FOR_INVITE_RESPONSE =	3;
int const CONNECTING_TO_CHAT =			4;
int const WAITING_TO_SYNC_TO_CHAT =		5;
int const SYNCHRONIZING_WITH_CHAT =		6;
int const OPEN_FOR_DOC =				7;
int const PLAYING_SESSION_FILE =		8;

// TODO: make this user-configurable, within sane limits
// ("sane" limits being roughly in the range (10, 100], from personal testing)
// Based on discussions with Ted, it seems that we're going to make the Jabber guys
// accomodate Inkscape, not the other way around...
// Dispatch interval (in milliseconds)
int const SEND_TIMEOUT = 35;
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

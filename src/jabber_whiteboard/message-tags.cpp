/**
 * Whiteboard session manager
 * Message tags
 * 
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "jabber_whiteboard/message-tags.h"

namespace Inkscape {

namespace Whiteboard {

const char* MESSAGE_CHANGE =		"inkboard:change";
const char* MESSAGE_NEWOBJ =		"inkboard:new";
const char* MESSAGE_DELETE =		"inkboard:delete";
const char* MESSAGE_DOCUMENT =		"inkboard:document";
const char* MESSAGE_NODECONTENT =	"inkboard:node-content";
const char* MESSAGE_ORDERCHANGE	=	"inkboard:order-change";
const char* MESSAGE_COMMIT = 		"inkboard:commit";
const char* MESSAGE_UNDO = 			"inkboard:undo";
const char* MESSAGE_REDO = 			"inkboard:redo";
const char* MESSAGE_DOCBEGIN = 		"inkboard:document-begin";
const char* MESSAGE_DOCEND = 		"inkboard:document-end";
const char* MESSAGE_OBJKEY =		"objid";
const char* MESSAGE_ID =			"id";
const char* MESSAGE_KEY	=			"key";
const char* MESSAGE_OLDVAL =		"old";
const char* MESSAGE_NEWVAL =		"new";
const char* MESSAGE_NAME =			"name";
const char* MESSAGE_ISINTERACTIVE =	"interactive";
const char* MESSAGE_DATA =			"data";
const char* MESSAGE_PARENT =		"parent";
const char* MESSAGE_CHILD =			"child";
const char* MESSAGE_REF =			"ref";
const char* MESSAGE_CONTENT	=		"content";
const char* MESSAGE_REPEATABLE =	"repeatable";
const char* MESSAGE_CHATROOM =		"chatroom";

const char* MESSAGE_TYPE =			"inkboard-type";
const char* MESSAGE_NODETYPE =		"node-type";
const char* MESSAGE_FROM =			"from";
const char* MESSAGE_TO =			"to";
const char* MESSAGE_BODY =			"body";
const char* MESSAGE_QUEUE =			"queue";
const char* MESSAGE_SEQNUM =		"sequence-number";
const char* MESSAGE_PROTOCOL_VER =	"inkboard-protocol";

}

}

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

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

#ifndef __WHITEBOARD_MESSAGE_TAGS_H__
#define __WHITEBOARD_MESSAGE_TAGS_H__

namespace Inkscape {

namespace Whiteboard {
/**
 *
 * These message tags are <b>not</b> used in all messages.
 * They are confined to messages of type CHANGE_* and DOCUMENT_*;
 * they define the tags that are used inside those messages' bodies.
 */
// TODO: breaking these up into namespaces would be nice, but it's too much typing
// for now
//
// TODO: Some of these message tags are obsolete, and should be removed...

/// Message tag signaling an attribute change on a node.
extern char const* MESSAGE_CHANGE;

/// Message tag signaling a new node.
extern char const* MESSAGE_NEWOBJ;

/// Message tag signaling a node to remove.
extern char const* MESSAGE_DELETE;

/// Message tag signaling the beginning of a document synchronization.
extern char const* MESSAGE_DOCUMENT;

/// Message tag signaling a change in node content.
extern char const* MESSAGE_NODECONTENT;

/// Message tag signaling a change in node order.
extern char const* MESSAGE_ORDERCHANGE;

/// Message tag signaling a commit.
extern char const* MESSAGE_COMMIT;

/// Message tag signaling an undo.
extern char const* MESSAGE_UNDO;

/// Message tag signaling a redo.
extern char const* MESSAGE_REDO;

/// Message tag signaling the beginning of a document synchronization.
extern char const* MESSAGE_DOCBEGIN;

/// Message tag signaling the end of a document synchronization.
extern char const* MESSAGE_DOCEND;

/// Message tag used to identify an object's key.
extern char const* MESSAGE_OBJKEY;

/// Message tag used to identify a node ID.
extern char const* MESSAGE_ID;

/// Message tag used to identify an attribute key.
extern char const* MESSAGE_KEY;

/// Message tag used to identify an old value (attribute or content).
extern char const* MESSAGE_OLDVAL;

/// Message tag used to identify a new value (attribute or content).
extern char const* MESSAGE_NEWVAL;

/// Message tag used to identify a node name.
extern char const* MESSAGE_NAME;

extern char const* MESSAGE_ISINTERACTIVE;
extern char const* MESSAGE_DATA;

/// Message tag used to identify the parent of a node by string key.
extern char const* MESSAGE_PARENT;

/// Message tag used to identify a child node by string key.
extern char const* MESSAGE_CHILD;

/// Message tag used to identify the node previous to a child node by string key.
extern char const* MESSAGE_REF;

/// Message tag used to identify the content in a node.  
extern char const* MESSAGE_CONTENT;
extern char const* MESSAGE_REPEATABLE;
extern char const* MESSAGE_CHATROOM;

/**
 * These message tags are used in all messages.
 */

/// Message tag used to identify the message type.
extern char const* MESSAGE_TYPE;

/// Message tag used to identify the type of node being operated on.
extern char const* MESSAGE_NODETYPE;

/// Message tag used to identify the sender.
extern char const* MESSAGE_FROM;

/// Message tag used to identify the recipient.
extern char const* MESSAGE_TO;

/// Message tag used to identify the body portion of the message.
extern char const* MESSAGE_BODY;
extern char const* MESSAGE_QUEUE;

/// Message tag used to identify the sequence number of the message.
extern char const* MESSAGE_SEQNUM;

/// Message tag used to identify the Inkboard protocol version being used.
extern char const* MESSAGE_PROTOCOL_VER;

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

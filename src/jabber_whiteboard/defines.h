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

#ifndef __INKSCAPE_WHITEBOARD_DEFINES_H__
#define __INKSCAPE_WHITEBOARD_DEFINES_H__

#include "jabber_whiteboard/message-tags.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <map>
#include <set>
#include <bitset>
#include <vector>

#include <glibmm.h>
#include <sigc++/sigc++.h>

#include "jabber_whiteboard/keynode.h"

#include "gc-alloc.h"

// Various specializations of std::less for XMLNodeTracker maps.
namespace std {
using Inkscape::XML::Node;

/**
 * Specialization of std::less<> for pointers to XML::Nodes.a
 *
 * \see Inkscape::XML::Node
 */
template<>
struct less< Node* > : public binary_function < Node*, Node*, bool >
{
	bool operator()(Node* _x, Node* _y) const
	{
		return _x < _y;
	}

};

}

namespace Inkscape {

namespace XML {
    class Node;
}

namespace Util {
    template< typename T > class ListContainer;
}

namespace Whiteboard {

#define NUM_FLAGS	9

namespace Message {

    typedef char const* Wrapper;
    typedef char const* Message;

    extern Wrapper PROTOCOL;
    extern Wrapper NEW;
    extern Wrapper REMOVE;
    extern Wrapper CONFIGURE;
    extern Wrapper MOVE;

    extern Message CONNECT_REQUEST;
    extern Message CONNECTED;
    extern Message ACCEPT_INVITATION;
    extern Message DECLINE_INVITATION;
    extern Message DOCUMENT_BEGIN;
    extern Message DOCUMENT_END;

}

namespace Vars {

    extern char const* INKBOARD_XMLNS; 

}

namespace State {

    typedef char const* SessionType;

    extern SessionType WHITEBOARD_MUC; 
    extern SessionType WHITEBOARD_PEER;

    typedef char const* SessionState;
}

// message types
// explicitly numbered to aid protocol description later on


// Responses to whiteboard invitations
enum InvitationResponses {
	ACCEPT_INVITATION,
	DECLINE_INVITATION,
	PEER_ALREADY_IN_SESSION,
	UNSUPPORTED_PROTOCOL
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

// I am assuming that std::string (which will not properly represent Unicode data) will
// suffice for associating (integer, Jabber ID) identifiers with nodes. 
// We do not need to preserve all semantics handled by Unicode; we just need to have 
// the byte representation.  std::string is good enough for that.
//
// The reason for this is that comparisons with std::string are much faster than 
// comparisons with Glib::ustring (simply because the latter is using significantly
// more complex text-handling algorithms), and we need speed here.  We _could_ use
// Glib::ustring::collate_key() here and therefore get the best of both worlds,
// but collation keys are rather big.
//
// XML node tracker maps

/// Associates node keys to pointers to XML::Nodes.
/// \see Inkscape::Whiteboard::XMLNodeTracker
typedef std::map< std::string, XML::Node*, std::less< std::string >, GC::Alloc< std::pair< std::string, XML::Node* >, GC::MANUAL > > KeyToTrackerNodeMap;

/// Associates pointers to XML::Nodes with node keys.
/// \see Inkscape::Whiteboard::XMLNodeTracker
typedef std::map< XML::Node*, std::string, std::less< XML::Node* >, GC::Alloc< std::pair< XML::Node*, std::string >, GC::MANUAL > > TrackerNodeToKeyMap;


// TODO: Clean up these typedefs.  I'm sure quite a few of these aren't used anymore; additionally,
// it's probably possible to consolidate a few of these types into one.

// Temporary storage of new object messages and new nodes in said messages
typedef std::list< Glib::ustring > NewChildObjectMessageList;

typedef std::pair< KeyNodePair, NodeTrackerAction > SerializedEventNodeAction;

typedef std::list< SerializedEventNodeAction > KeyToNodeActionList;

//typedef std::map< std::string, SerializedEventNodeAction > KeyToNodeActionMap;

typedef std::set< std::string > AttributesScannedSet;
typedef std::set< XML::Node* > AttributesUpdatedSet;

typedef std::map< std::string, XML::Node const* > KeyToNodeMap;
typedef std::map< XML::Node const*, std::string > NodeToKeyMap;


// Message context verification and processing
struct MessageProcessor;
class ReceiveMessageQueue;

typedef std::map< std::string, ReceiveMessageQueue*, std::less< std::string >, GC::Alloc< std::pair< std::string, ReceiveMessageQueue* >, GC::MANUAL > > RecipientToReceiveQueueMap;
typedef std::map< std::string, unsigned int > ReceipientToLatestTransactionMap;

typedef std::string ReceivedCommitEvent;
typedef std::list< ReceivedCommitEvent > CommitsQueue;

// Message serialization
typedef std::list< Glib::ustring > SerializedEventList;

// Error handling -- someday
// TODO: finish and integrate this
//typedef boost::function< LmHandlerResult (unsigned int code) > ErrorHandlerFunctor;
//typedef std::map< unsigned int, ErrorHandlerFunctor > ErrorHandlerFunctorMap;

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
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

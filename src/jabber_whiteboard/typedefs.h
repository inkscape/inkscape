/**
 * Whiteboard session manager
 * Typedef declarations and template specializations
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_TYPEDEFS_H__
#define __WHITEBOARD_TYPEDEFS_H__

extern "C" {
#include <loudmouth/loudmouth.h>
}

#include <algorithm>
#include <cstring>
#include <string>
#include <map>
#include <set>
#include <bitset>

#include <glibmm.h>
#include <sigc++/sigc++.h>

#include "jabber_whiteboard/defines.h"

#include "gc-alloc.h"

namespace Inkscape {

namespace XML {

class Node;

}

namespace Util {

template< typename T >
class ListContainer;

}

}

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

namespace Whiteboard {
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
typedef std::map< std::string, XML::Node*, std::less< std::string >, GC::Alloc< std::pair< const std::string, XML::Node* >, GC::MANUAL > > KeyToTrackerNodeMap;

/// Associates pointers to XML::Nodes with node keys.
/// \see Inkscape::Whiteboard::XMLNodeTracker
typedef std::map< XML::Node*, std::string, std::less< XML::Node* >, GC::Alloc< std::pair< XML::Node* const, std::string >, GC::MANUAL > > TrackerNodeToKeyMap;


// TODO: Clean up these typedefs.  I'm sure quite a few of these aren't used anymore; additionally,
// it's probably possible to consolidate a few of these types into one.

// Temporary storage of new object messages and new nodes in said messages
typedef std::list< Glib::ustring > NewChildObjectMessageList;

typedef std::pair< std::string, XML::Node* > KeyNodePair;
typedef std::pair< KeyNodePair, NodeTrackerAction > SerializedEventNodeAction;

typedef std::list< SerializedEventNodeAction > KeyToNodeActionList;

//typedef std::map< std::string, SerializedEventNodeAction > KeyToNodeActionMap;

typedef std::set< std::string > AttributesScannedSet;
typedef std::set< XML::Node* > AttributesUpdatedSet;

typedef std::map< std::string, XML::Node const* > KeyToNodeMap;
typedef std::map< XML::Node const*, std::string > NodeToKeyMap;

// Buddy list management
typedef std::set< std::string > BuddyList;
typedef sigc::signal< void, std::string const& > BuddyListSignal;
typedef sigc::slot< void, std::string const& > BuddyListListener;

// Chatroom list participants
typedef std::set< char const* > ChatterList;

// Message context verification and processing
struct MessageProcessor;
class ReceiveMessageQueue;

typedef std::map< MessageType, std::bitset< NUM_FLAGS > > MessageContextMap;
typedef std::map< MessageType, MessageProcessor*, std::less< MessageType >, GC::Alloc< std::pair< const MessageType, MessageProcessor* >, GC::MANUAL > > MessageProcessorMap;

typedef std::map< std::string, ReceiveMessageQueue*, std::less< std::string >, GC::Alloc< std::pair< const std::string, ReceiveMessageQueue* >, GC::MANUAL > > RecipientToReceiveQueueMap;
typedef std::map< std::string, unsigned int > ReceipientToLatestTransactionMap;

typedef std::string ReceivedCommitEvent;
typedef std::list< ReceivedCommitEvent > CommitsQueue;

// Message serialization
typedef std::list< Glib::ustring > SerializedEventList;

// Error handling -- someday
// TODO: finish and integrate this
//typedef boost::function< LmHandlerResult (unsigned int code) > ErrorHandlerFunctor;
//typedef std::map< unsigned int, ErrorHandlerFunctor > ErrorHandlerFunctorMap;
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

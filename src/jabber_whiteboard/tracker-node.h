/**
 * Whiteboard session manager
 * XML node tracking facility
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_TRACKER_NODE_H__
#define __WHITEBOARD_TRACKER_NODE_H__

#include "xml/node.h"

#include "gc-managed.h"
#include "gc-finalized.h"

#include <glibmm.h>
#include <bitset>

namespace Inkscape {

namespace Whiteboard {

// set _size in TrackerNode private members if you add or delete
// any more listeners
enum ListenerType {
	ATTR_CHANGED,
	CHILD_ADDED,
	CHILD_REMOVED,
	CHILD_ORDER_CHANGED,
	CONTENT_CHANGED
};

struct TrackerNode : public GC::Managed<> {
public:
	TrackerNode(XML::Node const* n) : _node(n)
	{
	}

    virtual ~TrackerNode()
	{
	}
	
	void lock(ListenerType listener)
	{
		if (listener < _size) {
			this->_listener_locks.set(listener, true);
		}
	}
	
	void unlock(ListenerType listener)
	{
		if (listener < _size) {
			this->_listener_locks.set(listener, false);
		}
	}
	
	bool isLocked(ListenerType listener) 
	{
		return (this->_listener_locks[listener]);
	}

	XML::Node const* _node;

private:
	// change this if any other flags are added
	static unsigned short const _size = 5;
	std::bitset< _size > _listener_locks;

	// noncopyable, nonassignable
	TrackerNode(TrackerNode const&);
	TrackerNode& operator=(TrackerNode const&);
};

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

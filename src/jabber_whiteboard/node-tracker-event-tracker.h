/**
 * Tracks node add/remove events to an XMLNodeTracker, and eliminates cases such as
 * consecutive add/remove.
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_NODE_TRACKER_EVENT_TRACKER_H__
#define __WHITEBOARD_NODE_TRACKER_EVENT_TRACKER_H__

#include <map>

#include "jabber_whiteboard/typedefs.h"

namespace Inkscape {

namespace Whiteboard {

typedef std::pair< XML::Node*, std::string > NodeKeyPair;
typedef std::map< XML::Node*, NodeTrackerAction > NodeActionMap;

class NodeTrackerEventTracker {
public:
	NodeTrackerEventTracker() { }
	~NodeTrackerEventTracker() { }
	bool tryToTrack(XML::Node* node, NodeTrackerAction action);

	NodeTrackerAction getAction(XML::Node const* node)
	{
		NodeActionMap::iterator i = this->_actions.find(const_cast< XML::Node* >(node));
		if (i != this->_actions.end()) {
			return i->second;
		} else {
			return NODE_UNKNOWN;
		}
	}

	void clear()
	{
		this->_actions.clear();
	}
private:
	NodeActionMap _actions;
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

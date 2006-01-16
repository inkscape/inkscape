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

#include "xml/node.h"

#include "jabber_whiteboard/node-tracker-event-tracker.h"

namespace Inkscape {

namespace Whiteboard {

bool
NodeTrackerEventTracker::tryToTrack(XML::Node* node, NodeTrackerAction action)
{
	// 1.  Check if node is being tracked.
	NodeActionMap::iterator i = this->_actions.find(node);
	if (i != this->_actions.end()) {
		// 2a.  Check the action.  If it is the same as the action we are registering,
		// return false.  Otherwise, register the action with the actions map
		// and return true.
		if (i->second == action) {
			return false;
		} else {
			this->_actions[node] = action;
			return true;
		}
	} else {
		// 2b.  If we aren't tracking this node, insert it with the given action.
		this->_actions[node] = action;
		return true;
	}
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

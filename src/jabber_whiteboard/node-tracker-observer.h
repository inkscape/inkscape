/**
 * Convenience base class for XML::NodeObservers that need to extract data
 * from an XMLNodeTracker and queue up added or removed nodes for later
 * processing
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_NODE_TRACKER_OBSERVER_H__
#define __WHITEBOARD_NODE_TRACKER_OBSERVER_H__

#include "xml/node-observer.h"

#include "jabber_whiteboard/node-tracker-event-tracker.h"
#include "jabber_whiteboard/node-tracker.h"
#include "jabber_whiteboard/typedefs.h"

namespace Inkscape {

namespace XML {

class Node;

}

namespace Whiteboard {

class NodeTrackerObserver : public XML::NodeObserver {
public:
	NodeTrackerObserver(XMLNodeTracker* xnt) : _xnt(xnt) { }
	virtual ~NodeTrackerObserver() { }

	// just reinforce the fact that we don't implement any of the 
	// notification methods here
    virtual void notifyChildAdded(XML::Node &node, XML::Node &child, XML::Node *prev)=0;

    virtual void notifyChildRemoved(XML::Node &node, XML::Node &child, XML::Node *prev)=0;

    virtual void notifyChildOrderChanged(XML::Node &node, XML::Node &child,
                                         XML::Node *old_prev, XML::Node *new_prev)=0;

    virtual void notifyContentChanged(XML::Node &node,
                                      Util::shared_ptr<char> old_content,
                                      Util::shared_ptr<char> new_content)=0;

    virtual void notifyAttributeChanged(XML::Node &node, GQuark name,
                                        Util::shared_ptr<char> old_value,
                                        Util::shared_ptr<char> new_value)=0;


	// ...but we do provide node tracking facilities
	KeyToNodeActionList& getNodeTrackerActions()
	{
		return this->newnodes;
	}

	KeyToNodeActionList getNodeTrackerActionsCopy()
	{
		return this->newnodes;
	}

	void clearNodeBuffers()
	{
		this->newnodes.clear();
		this->newkeys.clear();
		this->actions.clear();
	}

protected:
	std::string _findOrGenerateNodeID(XML::Node& node)
	{
		NodeToKeyMap::iterator i = newkeys.find(&node);
		if (i != newkeys.end()) {
//			g_log(NULL, G_LOG_LEVEL_DEBUG, "Found key for %p (local): %s", &node, i->second.c_str());
			return i->second;
		} else {
			std::string nodeid = this->_xnt->get(node);
			if (nodeid.empty()) {
//				g_log(NULL, G_LOG_LEVEL_DEBUG, "Generating key for %p", &node);
				return this->_xnt->generateKey();
			} else {
//				g_log(NULL, G_LOG_LEVEL_DEBUG, "Found key for %p (tracker): %s", &node, nodeid.c_str());
				return nodeid;
			}
		}
	}

	KeyToNodeActionList newnodes;
	NodeTrackerEventTracker actions;
	NodeToKeyMap newkeys;
	XMLNodeTracker* _xnt;

private:
	// noncopyable, nonassignable
	NodeTrackerObserver(NodeTrackerObserver const& other);
	NodeTrackerObserver& operator=(NodeTrackerObserver const& other);

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

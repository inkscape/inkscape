/**
 * Inkboard message -> XML::Event* serializer
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_MESSAGE_SERIALIZER_H__
#define __WHITEBOARD_MESSAGE_SERIALIZER_H__

#include "xml/node-observer.h"

#include "util/share.h"

#include "jabber_whiteboard/node-tracker.h"
#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/node-tracker-observer.h"

#include <map>

namespace Inkscape {

namespace Whiteboard {

class Serializer : public NodeTrackerObserver {
public:
	Serializer(XMLNodeTracker* xnt) : NodeTrackerObserver(xnt) { }
	~Serializer() { }

    void notifyChildAdded(XML::Node &node, XML::Node &child, XML::Node *prev);

    void notifyChildRemoved(XML::Node &node, XML::Node &child, XML::Node *prev);

    void notifyChildOrderChanged(XML::Node &node, XML::Node &child,
                                         XML::Node *old_prev, XML::Node *new_prev);

    void notifyContentChanged(XML::Node &node,
                                      Util::shared_ptr<char> old_content,
                                      Util::shared_ptr<char> new_content);

    void notifyAttributeChanged(XML::Node &node, GQuark name,
                                        Util::shared_ptr<char> old_value,
                                        Util::shared_ptr<char> new_value);

	void synthesizeChildNodeAddEvents();

	SerializedEventList& getEventList()
	{
		return this->_events;
	}

	SerializedEventList getEventListCopy()
	{
		return this->_events;
	}

	SerializedEventList getAndClearEventList()
	{
		SerializedEventList ret = this->_events;
		this->_events.clear();
		return ret;
	}

	void clearEventList()
	{
		this->_events.clear();
	}

	void clearAttributesScannedBuffer()
	{
		this->_attributes_scanned.clear();
	}

	// Convenience method for resetting all stateful aspects of the serializer
	void reset()
	{	
		g_log(NULL, G_LOG_LEVEL_DEBUG, "Clearing serializer buffers");
		this->clearEventList();
		this->_parent_child_map.clear();
		this->_nn.clear();
		this->clearNodeBuffers();
		this->clearAttributesScannedBuffer();
	}

private:
	typedef std::set< XML::Node* > _New_nodes_type;
	typedef std::map< XML::Node*, XML::Node* > _pc_map_type;

	SerializedEventList _events;
	AttributesScannedSet _attributes_scanned;

	_New_nodes_type _nn;
	_pc_map_type _parent_child_map;

	void _newObjectEventHelper(XML::Node& parent, XML::Node& child, XML::Node* prev, bool recurse);
	void _recursiveMarkAsRemoved(XML::Node& node);
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

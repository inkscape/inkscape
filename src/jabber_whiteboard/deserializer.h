/**
 * Inkboard message -> XML::Event* deserializer
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_MESSAGE_DESERIALIZER_H__
#define __WHITEBOARD_MESSAGE_DESERIALIZER_H__

#include "xml/log-builder.h"
#include "xml/event.h"

#include "jabber_whiteboard/node-tracker-event-tracker.h"
#include "jabber_whiteboard/node-tracker.h"
#include "jabber_whiteboard/typedefs.h"

#include <functional>
#include <algorithm>
#include <glibmm.h>

namespace Inkscape {

namespace Whiteboard {

/**
 * A stateful XML::Event deserializer.
 *
 * The Deserializer class is meant to deserialize XML::Events serialized by 
 * Inkscape::Whiteboard::Serializer or a serializer that serializes
 * XML::Events into the same format.
 *
 * Usage is as follows:
 * <ol>
 * 	<li>For each serialized event called, call the appropriate deserialization method.</li>
 * 	<li>Detach the deserialized event.</li>
 * </ol>
 *
 * The deserializer does not actually modify any aspect of the document or node-tracking systems.
 * Methods are provided to provide the information necessary to perform the modifications outside
 * of the deserializer.
 */
class Deserializer {
public:
	/**
	 * Constructor.
	 *
	 * \param xnt The XMLNodeTracker that a Deserializer should use for retrieving 
	 * XML::Nodes based on string keys.
	 */
	Deserializer(XMLNodeTracker* xnt) : _xnt(xnt)
	{
		this->clearEventLog();
	}

	~Deserializer() { }

	/**
	 * Deserialize a node add event.
	 *
	 * \see XML::EventAdd
	 * \param msg The message that describes the event.
	 */
	void deserializeEventAdd(Glib::ustring const& msg);

	/**
	 * Deserialize a node remove event.
	 *
	 * \see XML::EventDel
	 * \param msg The message that describes the event.
	 */
	void deserializeEventDel(Glib::ustring const& msg);

	/**
	 * Deserialize a node order change event.
	 *
	 * \see XML::EventChgOrder
	 * \param msg The message that describes the event.
	 */
	void deserializeEventChgOrder(Glib::ustring const& msg);

	/**
	 * Deserialize a node content change event.
	 *
	 * \see XML::EventChgContent
	 * \param msg The message that describes the event.
	 */
	void deserializeEventChgContent(Glib::ustring const& msg);

	/**
	 * Deserialize a node attribute change event.
	 *
	 * \see XML::EventChgAttr
	 * \param msg The message that describes the event.
	 */
	void deserializeEventChgAttr(Glib::ustring const& msg);

	/**
	 * Retrieve the deserialized event log.
	 * This method does <b>not</b> clear the internal event log kept by the deserializer.
	 * To do that, use detachEventLog.
	 *
	 * \return The deserialized event log.
	 */
	XML::Event* getEventLog()
	{
		return this->_log;
	}

	/**
	 * Retrieve the deserialized event log and clear the internal event log kept by the deserializer.
	 *
	 * \return The deserialized event log.
	 */
	XML::Event* detachEventLog()
	{
		XML::Event* ret = this->_log;
		this->clearEventLog();
		return ret;
	}

	/**
	 * Clear the internal event log.
	 */
	void clearEventLog()
	{
		g_log(NULL, G_LOG_LEVEL_DEBUG, "Clearing event log");
		this->_log = NULL;
	}

	/**
	 * Retrieve a list of node entry actions (add node entry, remove node entry)
	 * that need to be performed on the XMLNodeTracker.
	 *
	 * Because this method returns a reference to a list, it is not safe for use 
	 * across multiple invocations of this Deserializer.
	 *
	 * \return A reference to a list of node entry actions generated while deserializing.
	 */
	KeyToNodeActionList& getNodeTrackerActions()
	{
		return this->_actions;
	}

	/**
	 * Retrieve a list of node entry actions (add node entry, remove node entry)
	 * that need to be performed on the XMLNodeTracker.
	 *
	 * \return A list of node entry actions generated while deserializing.
	 */
	KeyToNodeActionList getNodeTrackerActionsCopy()
	{
		return this->_actions;
	}

	/**
	 * Retrieve a set of nodes for which an EventChgAttr was deserialized.
	 *
	 * For some actions (i.e. text tool) it is necessary to call updateRepr() on
	 * the updated nodes.  This method provides the information required to perform
	 * that action.
	 *
	 * Because this method returns a reference to a set, it is not safe for use 
	 * across multiple invocations of this Deserializer.
	 *
	 * \return A reference to a set of nodes for which an EventChgAttr was deserialized.
	 */
	AttributesUpdatedSet& getUpdatedAttributeNodeSet()
	{
		return this->_updated;
	}

	/**
	 * Retrieve a set of nodes for which an EventChgAttr was deserialized.
	 *
	 * For some actions (i.e. text tool) it is necessary to call updateRepr() on
	 * the updated nodes.  This method provides the information required to perform
	 * that action.
	 *
	 * \return A set of nodes for which an EventChgAttr was deserialized.
	 */
	AttributesUpdatedSet getUpdatedAttributeNodeSetCopy()
	{
		return this->_updated;
	}

	/**
	 * Clear all internal node buffers.
	 */
	void clearNodeBuffers()
	{
		g_log(NULL, G_LOG_LEVEL_DEBUG, "Clearing deserializer node buffers");
		this->_newnodes.clear();
		this->_actions.clear();
		this->_newkeys.clear();
		this->_parent_child_map.clear();
		this->_updated.clear();
	}

	/**
	 * Clear all internal state.
	 */
	void reset() 
	{
		this->clearEventLog();
		this->clearNodeBuffers();
	}

private:
	XML::Node* _getNodeByID(std::string const& id)
	{
		KeyToNodeMap::iterator i = this->_newnodes.find(id);
		if (i != this->_newnodes.end()) {
			return const_cast< XML::Node* >(i->second);
		} else {
			if (this->_xnt->isTracking(id)) {
				return this->_xnt->get(id);
			} else {
				return NULL;
			}
		}
	}

	void _addOneEvent(XML::Event* event)
	{
		if (this->_log == NULL) {
			this->_log = event;
		} else {
			event->next = this->_log;
			this->_log = event;
		}
	}

	void _recursiveMarkForRemoval(XML::Node* node);

	// internal states with accessors:
	
	// node tracker actions (add node, remove node)
	KeyToNodeActionList _actions;

	// nodes that have had their attributes updated
	AttributesUpdatedSet _updated;

	// the deserialized event log
	XML::Event* _log;


	// for internal use:
	
	// These maps only store information on a single node.  That's fine, though;
	// all we care about is the ability to do key <-> node association.  The NodeTrackerEventTracker
	// and KeyToNodeActionList keep track of the actual actions we need to perform 
	// on the node tracker.
	NodeToKeyMap _newkeys;
	KeyToNodeMap _newnodes;
	NodeTrackerEventTracker _node_action_tracker;

	typedef std::map< XML::Node*, XML::Node* > _pc_map_type;
	_pc_map_type _parent_child_map;

	XMLNodeTracker* _xnt;

	XML::LogBuilder _builder;
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

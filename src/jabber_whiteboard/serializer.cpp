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

#include "xml/attribute-record.h"

#include "jabber_whiteboard/serializer.h"

#include "util/list.h"
#include "util/shared-c-string-ptr.h"

#include "jabber_whiteboard/message-utilities.h"
#include "jabber_whiteboard/message-tags.h"
#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/node-tracker.h"
#include "jabber_whiteboard/node-utilities.h"
#include "jabber_whiteboard/node-tracker-observer.h"

namespace Inkscape {

namespace Whiteboard {

void
Serializer::notifyChildAdded(XML::Node& node, XML::Node& child, XML::Node* prev)
{
	// do not recurse upon initial notification
	this->_newObjectEventHelper(node, child, prev, false);
	this->_nn.insert(&child);
}

void
Serializer::_newObjectEventHelper(XML::Node& node, XML::Node& child, XML::Node* prev, bool recurse)
{
	// 1.  Check if we are tracking the parent node,
	// and issue it an ID if we are not.
	std::string parentid = this->_findOrGenerateNodeID(node);

	// 2.  Check if the child node is a special node.
	// Special nodes are nodes that should appear only once in a document.
	// If it is, we do not want to generate a new ID for the child; we will use
	// the existing ID.  Otherwise, we will generate a new ID for it, since we 
	// have not yet seen it.
	std::string childid;
	if (this->_xnt->isSpecialNode(child.name())) {
		childid = this->_xnt->get(child);
	} else {
		// If the child id already exists in the new node buffer, then we've already seen it.
		if (!this->actions.tryToTrack(&child, NODE_ADD)) {
				return;
		} else {
			childid = this->_xnt->generateKey();
//			childid = this->_findOrGenerateNodeID(child);
		}
	}

	// 3.  Find this node's previous node, and, if it has one, retrieve its ID.
	std::string previd;
	if (prev) {
		previd = this->_findOrGenerateNodeID(*prev);
	}

	// 4.  Serialize.
	Glib::ustring childmsg = MessageUtilities::makeTagWithContent(MESSAGE_CHILD, childid);
	Glib::ustring parentmsg = MessageUtilities::makeTagWithContent(MESSAGE_PARENT, parentid);
	Glib::ustring namemsg = MessageUtilities::makeTagWithContent(MESSAGE_NAME, child.name());
	Glib::ustring nodetype = MessageUtilities::makeTagWithContent(MESSAGE_NODETYPE, NodeUtilities::nodeTypeToString(child));

	Glib::ustring prevmsg;
	if (!previd.empty()) {
		prevmsg = MessageUtilities::makeTagWithContent(MESSAGE_REF, previd);
	}

	Glib::ustring buf = MessageUtilities::makeTagWithContent(MESSAGE_NEWOBJ, childmsg + parentmsg + prevmsg + namemsg + nodetype);


	this->_events.push_back(buf);

	// 5.  Add the child node to the new nodes buffers.
	this->newnodes.push_back(SerializedEventNodeAction(KeyNodePair(childid, &child), NODE_ADD));
	this->newkeys[&child] = childid;
	this->_parent_child_map[&child] = &node;

	// 6.  Scan attributes and content.
	Inkscape::Util::List<Inkscape::XML::AttributeRecord const> attrlist = child.attributeList();

	for(; attrlist; attrlist++) {
		this->notifyAttributeChanged(child, attrlist->key, Util::SharedCStringPtr(), attrlist->value);
	}
	
	if (child.content()) {
		this->notifyContentChanged(child, Util::SharedCStringPtr(), Util::SharedCStringPtr::copy(child.content()));
	}

	this->_attributes_scanned.insert(childid);

	// 7.  Repeat this process for each child of this child.
	if (recurse && child.childCount() > 0) {
		XML::Node* prev = child.firstChild();
		for(XML::Node* ch = child.firstChild(); ch; ch = ch->next()) {
			if (ch == child.firstChild()) {
				// No prev node in this case.
				this->_newObjectEventHelper(child, *ch, NULL, true);
			} else {
				this->_newObjectEventHelper(child, *ch, prev, true);
				prev = ch;
			}
		}
	}

	return;
}


void
Serializer::notifyChildRemoved(XML::Node& node, XML::Node& child, XML::Node* prev)
{
	// 1.  Get the ID of the child.
	std::string childid;

	_pc_map_type::iterator i = this->_parent_child_map.find(&child);
	if (i != this->_parent_child_map.end() && i->second != &node) {
		// Don't look in local! Go for the tracker.
		childid = this->_xnt->get(child);
	} else if (i == this->_parent_child_map.end()) {
		childid = this->_findOrGenerateNodeID(child);
	} else if (i->second == &node) {
		childid = this->_findOrGenerateNodeID(child);
		this->_parent_child_map.erase(i);
	} else {
		childid = this->_findOrGenerateNodeID(child);
	}
	
	// 2.  Double-deletes don't make any sense.  If we've seen this node already and if it's
	// marked for deletion, return.
	if (!this->actions.tryToTrack(&child, NODE_REMOVE)) {
			return;
	} else {
		// 2a.  Although we do not have to remove all child nodes of this subtree,
		// we _do_ have to mark each child node as deleted.
		this->_recursiveMarkAsRemoved(child);
	}

	// 2.  Mark this node as deleted.  We don't want to be faced with the possibility of 
	// generating a new key for this deleted node, so insert it into both maps.
	this->newnodes.push_back(SerializedEventNodeAction(KeyNodePair(childid, &child), NODE_REMOVE));
	this->newkeys[&child] = childid;
	this->_nn.erase(&child);
	std::string parentid = this->_findOrGenerateNodeID(node);


	// 4.  Serialize the event.
	this->_attributes_scanned.erase(childid);
	Glib::ustring childidmsg = MessageUtilities::makeTagWithContent(MESSAGE_CHILD, childid);
	Glib::ustring parentidmsg = MessageUtilities::makeTagWithContent(MESSAGE_PARENT, parentid);
	this->_events.push_back(MessageUtilities::makeTagWithContent(MESSAGE_DELETE, childidmsg + parentidmsg));
}


void
Serializer::notifyChildOrderChanged(XML::Node& node, XML::Node& child, XML::Node* old_prev, XML::Node* new_prev)
{
	// 1.  Find the ID of the node, or generate it if it does not exist.
	std::string nodeid = this->_findOrGenerateNodeID(child);
	
	// 2.  Find the ID of the parent of this node, or generate it if it does not exist.
	std::string parentid = this->_findOrGenerateNodeID(*(child.parent()));

	// 3.  Get the ID for the new child reference node, or generate it if it does not exist.
	std::string newprevid = this->_findOrGenerateNodeID(*new_prev);

	// 4.  Get the ID for the old child reference node, or generate it if it does not exist.
	std::string oldprevid = this->_findOrGenerateNodeID(*old_prev);

	// 5.  Serialize the event.
	Glib::ustring nodeidmsg = MessageUtilities::makeTagWithContent(MESSAGE_ID, nodeid);
	Glib::ustring parentidmsg = MessageUtilities::makeTagWithContent(MESSAGE_PARENT, parentid);
	Glib::ustring oldprevidmsg = MessageUtilities::makeTagWithContent(MESSAGE_OLDVAL, oldprevid);
	Glib::ustring newprevidmsg = MessageUtilities::makeTagWithContent(MESSAGE_NEWVAL, newprevid);

	this->_events.push_back(MessageUtilities::makeTagWithContent(MESSAGE_ORDERCHANGE, nodeidmsg + parentidmsg + oldprevidmsg + newprevidmsg));
}

void
Serializer::notifyContentChanged(XML::Node& node, Util::SharedCStringPtr old_content, Util::SharedCStringPtr new_content)
{
	// 1.  Find the ID of the node, or generate it if it does not exist.
	std::string nodeid = this->_findOrGenerateNodeID(node);

	std::string oldvalmsg, newvalmsg;

	// 2.  If the old and new content are identical, don't send out this change.
	// (identical meaning "same string" or "same string content")
	if (old_content == new_content) {
		return;
	}

	if (old_content.cString() != NULL && new_content.cString() != NULL) {
		if (strcmp(old_content.cString(), new_content.cString()) == 0) {
			return;
		}
	}

	// 3.  Serialize the event.
	if (old_content.cString() != NULL) {
		oldvalmsg = MessageUtilities::makeTagWithContent(MESSAGE_OLDVAL, old_content.cString());
	}

	if (new_content.cString() != NULL) {
		newvalmsg = MessageUtilities::makeTagWithContent(MESSAGE_NEWVAL, new_content.cString());
	}

	Glib::ustring nodeidmsg = MessageUtilities::makeTagWithContent(MESSAGE_ID, nodeid);
	this->_events.push_back(MessageUtilities::makeTagWithContent(MESSAGE_NODECONTENT, nodeidmsg + oldvalmsg + newvalmsg));
}

void
Serializer::notifyAttributeChanged(XML::Node& node, GQuark name, Util::SharedCStringPtr old_value, Util::SharedCStringPtr new_value)
{
	// 1.  Find the ID of the node that has had an attribute modified, or generate it if it
	// does not exist.
	std::string nodeid = this->_findOrGenerateNodeID(node);

	// Proceed with 2-4 if the node has not already been scanned by notifyChildAdded.
	if (this->_attributes_scanned.find(nodeid) == this->_attributes_scanned.end()) {
		// 2.  Convert the key to a string.
		Glib::ustring key = g_quark_to_string(name);

		// 3.  If oldval == newval, don't echo this change.
		if (new_value.cString() != NULL && old_value.cString() != NULL) {
			if (strcmp(new_value.cString(), old_value.cString()) == 0) {
				return;
			}
		}

		// 4.  Serialize the event.
		Glib::ustring keymsg = MessageUtilities::makeTagWithContent(MESSAGE_KEY, key);
		Glib::ustring oldvalmsg, newvalmsg;

		if (old_value.cString() != NULL) {
			oldvalmsg = MessageUtilities::makeTagWithContent(MESSAGE_OLDVAL, old_value.cString());
		}

		if (new_value.cString() != NULL) {
			newvalmsg = MessageUtilities::makeTagWithContent(MESSAGE_NEWVAL, new_value.cString());
		}

		Glib::ustring nodeidmsg = MessageUtilities::makeTagWithContent(MESSAGE_ID, nodeid);

		this->_events.push_back(MessageUtilities::makeTagWithContent(MESSAGE_CHANGE, nodeidmsg + keymsg + oldvalmsg + newvalmsg));
	}
}

void
Serializer::synthesizeChildNodeAddEvents()
{
	_New_nodes_type::iterator i = this->_nn.begin();
	for(; i != this->_nn.end(); ++i) {
		XML::Node* parent = *i;
		// The root of the subtree defined by parent has already been considered; now,
		// recursively look at the rest of the tree.
		XML::Node* prev = parent->firstChild();
		for(XML::Node* ch = parent->firstChild(); ch; ch = ch->next()) {
			if (ch == parent->firstChild()) {
				this->_newObjectEventHelper(*parent, *ch, NULL, true);
			} else {
				this->_newObjectEventHelper(*parent, *ch, prev, true);
				prev = ch;
			}
		}
	}
}


void
Serializer::_recursiveMarkAsRemoved(XML::Node& node)
{
	this->actions.tryToTrack(&node, NODE_REMOVE);

	for(XML::Node* ch = node.firstChild(); ch; ch = ch->next()) {
		this->_recursiveMarkAsRemoved(*ch);
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

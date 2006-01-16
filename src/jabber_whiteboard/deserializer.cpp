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

#include "xml/log-builder.h"
#include "xml/event.h"
#include "xml/node.h"
#include "xml/repr.h"

#include "util/shared-c-string-ptr.h"

#include "gc-anchored.h"

#include "jabber_whiteboard/message-utilities.h"
#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/node-utilities.h"
#include "jabber_whiteboard/node-tracker.h"
#include "jabber_whiteboard/deserializer.h"
#include <glibmm.h>

namespace Inkscape {

namespace Whiteboard {

void
Deserializer::deserializeEventAdd(Glib::ustring const& msg)
{
	// 1.  Extract required attributes: parent, child, node name, and node type.
	// If any of these cannot be found, return.
	std::string parent, child, prev;
	Glib::ustring name, type;

	struct Node buf;

	buf.tag = MESSAGE_PARENT;
	if (!MessageUtilities::findTag(buf, msg)) {
		return;
	}
	parent = buf.data.c_str();

	buf.tag = MESSAGE_CHILD;
	if (!MessageUtilities::findTag(buf, msg)) {
		return;
	}
	child = buf.data.c_str();

	KeyToNodeMap::iterator i = this->_newnodes.find(child);
	if (i != this->_newnodes.end()) {
		if (this->_node_action_tracker.getAction(i->second) == NODE_ADD) {
			return;
		}
	}

	buf.tag = MESSAGE_NAME;
	if (!MessageUtilities::findTag(buf, msg)) {
		return;
	} else {
		name = buf.data;
	}

	buf.tag = MESSAGE_NODETYPE;
	if (!MessageUtilities::findTag(buf, msg)) {
		return;
	} else {
		type = buf.data;
	}

	// 2.  Extract optional attributes: the node previous to the new child node.
	buf.tag = MESSAGE_REF;
	if (MessageUtilities::findTag(buf, msg)) {
		prev = buf.data.c_str();
	}

	// 3. Check if the received child node is a special node.  If it is, and we are already 
	// tracking it, then we should not add the node.
	if (this->_xnt->isSpecialNode(name)) {
		if (this->_xnt->isTracking(this->_xnt->getSpecialNodeKeyFromName(name))) {
			return;
		}
	}

	// 4.  Look up the parent node.  If we cannot find it, return.
	XML::Node* parentRepr = this->_getNodeByID(parent);
	if (parentRepr == NULL) {
		g_warning("Cannot find parent node identified by %s", parent.c_str());
		return; 
	}

	// 5.  Look up the node previous to the child, if it exists.
	// If we cannot find it, we may be in a change conflict situation.
	// In that case, just append the node.
	XML::Node* prevRepr = NULL;
	if (!prev.empty()) {
		prevRepr = this->_getNodeByID(prev);
		if (prevRepr == NULL) {
			g_warning("Prev node %s could not be found; appending incoming node.  Document may not be synchronized.", prev.c_str());
			prevRepr = parentRepr->lastChild();
		}
	}

	if (prevRepr) {
		if (prevRepr->parent() != parentRepr) {
			if (this->_parent_child_map[prevRepr] != parentRepr) {
				g_warning("ref mismatch on node %s: child=%s ref=%p parent=%p ref->parent()=%p", prev.c_str(), child.c_str(), prevRepr, parentRepr, prevRepr->parent());
				g_warning("parent_child_map[%p (%s)] = %p (%s)", prevRepr, this->_xnt->get(*prevRepr).c_str(), this->_parent_child_map[prevRepr], this->_xnt->get(*(this->_parent_child_map[prevRepr])).c_str());
				return;
			}
		}
	}

	XML::Node* childRepr = NULL;
	
	// 6.  Create the child node.

	switch (NodeUtilities::stringToNodeType(type)) {
		case XML::TEXT_NODE:
			buf.tag = MESSAGE_CONTENT;
			if (!MessageUtilities::findTag(buf, msg)) {
				childRepr = sp_repr_new_text("");
			} else {
				childRepr = sp_repr_new_text(buf.data.c_str());
			}
			break;
		case XML::DOCUMENT_NODE:
			// TODO
		case XML::COMMENT_NODE:
			buf.tag = MESSAGE_CONTENT;
			if (!MessageUtilities::findTag(buf, msg)) {
				childRepr = sp_repr_new_comment("");
			} else {
				childRepr = sp_repr_new_comment(buf.data.c_str()); 
			}
			break;
		case XML::ELEMENT_NODE: 
		default:
			childRepr = sp_repr_new(name.data());
			break;
	}


	this->_actions.push_back(SerializedEventNodeAction(KeyNodePair(child, childRepr), NODE_ADD));
	this->_newnodes[child] = childRepr;
	this->_newkeys[childRepr] = child;


	// 8.  Deserialize the event.
	this->_builder.addChild(*parentRepr, *childRepr, prevRepr);
	this->_parent_child_map.erase(childRepr);
	this->_parent_child_map[childRepr] = parentRepr;
	this->_addOneEvent(this->_builder.detach());
	Inkscape::GC::release(childRepr);
}

void
Deserializer::deserializeEventDel(Glib::ustring const& msg)
{
	// 1.  Extract required attributes: parent, child.  If we do not know these,
	// return.
	std::string parent, child, prev;

	struct Node buf;

	buf.tag = MESSAGE_PARENT;
	if (!MessageUtilities::findTag(buf, msg)) {
		return;
	}
	parent = buf.data.c_str();

	buf.tag = MESSAGE_CHILD;
	if (!MessageUtilities::findTag(buf, msg)) {
		return;
	}
	child = buf.data.c_str();

	KeyToNodeMap::iterator i = this->_newnodes.find(child);
	if (i != this->_newnodes.end()) {
		if (this->_node_action_tracker.getAction(i->second) == NODE_REMOVE) {
			return;
		}
	}

	// 2.  Extract optional attributes: previous node.
	buf.tag = MESSAGE_REF;
	if (MessageUtilities::findTag(buf, msg)) {
		prev = buf.data.c_str(); 
	}

	// 3.  Retrieve nodes.  If we cannot find all nodes involved, return.
	XML::Node* parentRepr = this->_getNodeByID(parent);
	XML::Node* childRepr = this->_getNodeByID(child);
	XML::Node* prevRepr = NULL;
	if (!prev.empty()) {
		prevRepr = this->_getNodeByID(prev);
		if (prevRepr == NULL) {
			return;
		}
	}

	// 4.  Deserialize the event.
	if (parentRepr && childRepr) {
		if (childRepr->parent() == parentRepr || this->_parent_child_map[childRepr] == parentRepr) {
//			this->_actions.push_back(SerializedEventNodeAction(KeyNodePair(child, childRepr), NODE_REMOVE));
			this->_builder.removeChild(*parentRepr, *childRepr, prevRepr);
			this->_parent_child_map.erase(childRepr);
			this->_addOneEvent(this->_builder.detach());

			// 5.  Mark the removed node and all its children for removal from the tracker.
			this->_recursiveMarkForRemoval(childRepr);
		} else {
			g_warning("child->parent() == parent mismatch on child=%s (%p), parent=%s: parent=%p child->parent()=%p", child.c_str(), childRepr, parent.c_str(), parentRepr, childRepr->parent());
			g_warning("parent_child_map[%p] = %p", childRepr, this->_parent_child_map[childRepr]);
		}
	} else {
		g_warning("Missing parentRepr and childRepr: parent=%p, child=%p", parentRepr, childRepr);
	}
}

void
Deserializer::deserializeEventChgOrder(Glib::ustring const& msg)
{
	// 1.  Extract required attributes: node ID, parent ID, new previous node ID.
	// If we do not know these, return.
	std::string id, parentid, oldprevid, newprevid;
	Node buf;

	buf.tag = MESSAGE_ID;
	if (MessageUtilities::findTag(buf, msg)) {
		id = buf.data.raw();
	} else {
		return;
	}

	buf.tag = MESSAGE_PARENT;
	if (MessageUtilities::findTag(buf, msg)) {
		parentid = buf.data.raw();
	} else {
		return;
	}

	// 2.  Extract optional attributes: old previous node, new previous node.
	buf.tag = MESSAGE_OLDVAL;
	if (MessageUtilities::findTag(buf, msg)) {
		oldprevid = buf.data.raw();
	}

	buf.tag = MESSAGE_NEWVAL;
	if (MessageUtilities::findTag(buf, msg)) {
		newprevid = buf.data.raw();
	} else {
		return;
	}

	// 3.  Find the identified nodes.  If we do not know about the parent and child, return.
	XML::Node* node = this->_getNodeByID(id);
	XML::Node* parent = this->_getNodeByID(parentid);
	XML::Node* oldprev = NULL;
	XML::Node* newprev = NULL;
	if (!oldprevid.empty()) {
		oldprev = this->_getNodeByID(oldprevid);
	}

	if (!newprevid.empty()) {
		newprev = this->_getNodeByID(newprevid);
	}

	if (parent && node) {
		// 4.  Deserialize the event.
		this->_builder.setChildOrder(*parent, *node, oldprev, newprev);
		this->_addOneEvent(this->_builder.detach());
	} else {
		return;
	}
}

void
Deserializer::deserializeEventChgContent(Glib::ustring const& msg)
{
	// 1.  Extract required attributes: node ID.  If we do not know these, return.
	std::string id;
	Util::SharedCStringPtr oldval, newval;
	Node buf;

	buf.tag = MESSAGE_ID;
	if (!MessageUtilities::findTag(buf, msg)) {
		return;
	} else {
		id = buf.data.raw();
	}

	// 2.  Extract optional attributes: old value, new value.
	buf.tag = MESSAGE_OLDVAL;
	if (MessageUtilities::findTag(buf, msg)) {
		oldval = Util::SharedCStringPtr::copy(buf.data.c_str());
	} else {
		oldval = Util::SharedCStringPtr::copy("");
	}

	buf.tag = MESSAGE_NEWVAL;
	if (MessageUtilities::findTag(buf, msg)) {
		newval = Util::SharedCStringPtr::copy(buf.data.c_str());
	} else {
		newval = Util::SharedCStringPtr::copy("");
	}

	// 3.  Find the node identified by the ID.  If we cannot find it, return.
	XML::Node* node = this->_getNodeByID(id);
	if (node == NULL) {
		return;
	}

	// 4.  Deserialize the event.
	this->_builder.setContent(*node, oldval, newval);
	this->_addOneEvent(this->_builder.detach());
}

void
Deserializer::deserializeEventChgAttr(Glib::ustring const& msg)
{
	// 1.  Extract required attributes: node ID, attribute key.  If we do not know these,
	// return.

	struct Node buf;
	buf.tag = MESSAGE_ID;
	if (!MessageUtilities::findTag(buf, msg)) {
		return;
	}

	std::string id = buf.data.data();

	buf.tag = MESSAGE_KEY;
	if (!MessageUtilities::findTag(buf, msg)) {
		return;
	}

	Glib::ustring key = buf.data;

	// 2.  Extract optional attributes: new value.  If we do not find it in the message,
	// assume there is no new value.
	buf.tag = MESSAGE_NEWVAL;
	Util::SharedCStringPtr newval;
	if (MessageUtilities::findTag(buf, msg)) {
		newval = Util::SharedCStringPtr::copy(buf.data.c_str());
	} else {
		newval = Util::SharedCStringPtr::copy("");
	}

	// 3.  Extract optional attributes: old value.  If we do not find it in the message,
	// assume that there is no old value.
	buf.tag = MESSAGE_OLDVAL;
	Util::SharedCStringPtr oldval;
	if (MessageUtilities::findTag(buf, msg)) {
		oldval = Util::SharedCStringPtr::copy(buf.data.c_str());
	} else {
		oldval = Util::SharedCStringPtr::copy("");
	}

	// 4.  Look up this node in the local node database and external tracker.
	// If it cannot be found, return.
	XML::Node* node = this->_getNodeByID(id);
	if (node == NULL) {
		g_warning("Could not find node %s on which to change attribute", id.c_str());
		return;
	}

	// 5.  If this node is in the actions queue and is marked as "new", we need to apply
	// _all_ received attributes to it _before_ adding it to the document tree.
	if (this->_newnodes.find(id) != this->_newnodes.end()) {
		node->setAttribute(key.c_str(), newval.cString());	
	}

	// 6.  Deserialize the event.
	this->_builder.setAttribute(*node, g_quark_from_string(key.c_str()), oldval, newval);
	this->_updated.insert(node);
	this->_addOneEvent(this->_builder.detach());
}

void 
Deserializer::_recursiveMarkForRemoval(XML::Node* node)
{
	if (node != NULL) {
		NodeToKeyMap::iterator i = this->_newkeys.find(node);
		if (i == this->_newkeys.end()) {
			std::string id = this->_xnt->get(*node);
			if (!id.empty()) {
				this->_actions.push_back(SerializedEventNodeAction(KeyNodePair(id, node), NODE_REMOVE));
			}
		} else {
			this->_actions.push_back(SerializedEventNodeAction(KeyNodePair((*i).second, node), NODE_REMOVE));
		}

		for (XML::Node* child = node->firstChild(); child; child = child->next()) {
			this->_recursiveMarkForRemoval(child);
		}
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

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

#include "sp-object.h"
#include "sp-item-group.h"
#include "document.h"
#include "document-private.h"

#include "xml/node.h"

#include "util/compose.hpp"

#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/node-tracker.h"


// TODO: remove redundant calls to isTracking(); it's a rather unnecessary
// performance burden. 
namespace Inkscape {

namespace Whiteboard {

// Lookup tables

/**
 * Keys for special nodes.
 *
 * A special node is a node that can only appear once in a document.
 */
char const* specialnodekeys[] = {
	DOCUMENT_ROOT_NODE,
	DOCUMENT_NAMEDVIEW_NODE,
};

/**
 * Names of special nodes.
 *
 * A special node is a node that can only appear once in a document.
 */
char const* specialnodenames[] = {
	DOCUMENT_ROOT_NAME,
	DOCUMENT_NAMEDVIEW_NAME,
};

XMLNodeTracker::XMLNodeTracker(SessionManager* sm) :
	_rootKey(DOCUMENT_ROOT_NODE),
	_namedviewKey(DOCUMENT_NAMEDVIEW_NODE)
{
	this->_sm = sm;
	this->_counter = 0;

	// Construct special node maps
	this->createSpecialNodeTables();
	this->reset();
}

XMLNodeTracker::~XMLNodeTracker()
{
	this->_clear();
}

void
XMLNodeTracker::put(std::string key, XML::Node const& node)
{
	this->put(key, const_cast< XML::Node& >(node));
}

void 
XMLNodeTracker::put(std::string key, XML::Node& node)
{	
	KeyToTrackerNodeMap::iterator i = this->_keyToNode.find(key);
	if (i != this->_keyToNode.end()) {
		this->_keyToNode.erase(i);
	}
	this->_keyToNode.insert(std::make_pair< std::string, XML::Node* >(key, &node));

	TrackerNodeToKeyMap::iterator j = this->_nodeToKey.find(&node);
	if (j != this->_nodeToKey.end()) {
		this->_nodeToKey.erase(j);
	}
	this->_nodeToKey.insert(std::make_pair< XML::Node*, std::string >(&node, key));
}

void
XMLNodeTracker::put(KeyToNodeMap& newids, NodeToKeyMap& newnodes)
{
	// TODO: redo
	KeyToNodeMap::iterator i = newids.begin();

	for(; i != newids.end(); i++) {
		this->put((*i).first, *((*i).second));
	}
}

void
XMLNodeTracker::process(KeyToNodeActionList& actions)
{
	KeyToNodeActionList::iterator i = actions.begin();
	for(; i != actions.end(); i++) {
		// Get the action to perform.
		SerializedEventNodeAction action = *i;
		switch(action.second) {
			case NODE_ADD:
				this->put(action.first.first, *action.first.second);
				break;
			case NODE_REMOVE:
		//		this->remove(const_cast< XML::Node& >(*action.first.second));
				break;
			default:
				break;
		}
	}
}

XML::Node*
XMLNodeTracker::get(std::string& key)
{
	KeyToTrackerNodeMap::iterator i = this->_keyToNode.find(key);
	if (i != this->_keyToNode.end()) {
		return (*i).second;
	} else {
		g_warning("Key %s is not being tracked!", key.c_str());
		return NULL;
	}
}

XML::Node*
XMLNodeTracker::get(std::string const& key)
{
	return this->get(const_cast< std::string& >(key));
}


std::string const
XMLNodeTracker::get(XML::Node& node)
{
	TrackerNodeToKeyMap::iterator i = this->_nodeToKey.find(&node);
	if (i != this->_nodeToKey.end()) {
		return (*i).second;
	} else {
		return "";
	}
}

std::string const
XMLNodeTracker::get(XML::Node const& node)
{
	return this->get(const_cast< XML::Node& >(node));
}

bool
XMLNodeTracker::isTracking(std::string& key)
{
	return (this->_keyToNode.find(key) != this->_keyToNode.end());
}

bool
XMLNodeTracker::isTracking(std::string const& key)
{
	return this->isTracking(const_cast< std::string& >(key));
}

bool
XMLNodeTracker::isTracking(XML::Node& node)
{
	return (this->_nodeToKey.find(&node) != this->_nodeToKey.end());
}

bool
XMLNodeTracker::isTracking(XML::Node const& node)
{
	return this->isTracking(const_cast< XML::Node& >(node));
}

bool
XMLNodeTracker::isRootNode(XML::Node& node)
{
	XML::Node* docroot = sp_document_repr_root(this->_sm->document());
	return (docroot == &node);
}


void
XMLNodeTracker::remove(std::string& key)
{
	if (this->isTracking(key)) {
		XML::Node* element = this->get(key);
		this->_keyToNode.erase(key);
		this->_nodeToKey.erase(element);
	} 
}

void
XMLNodeTracker::remove(XML::Node& node)
{
	if (this->isTracking(node)) {
		std::string const element = this->get(node);
		this->_nodeToKey.erase(&node);
		this->_keyToNode.erase(element);
	}
}

bool 
XMLNodeTracker::isSpecialNode(char const* name)
{
	return (this->_specialnodes.find(name) != this->_specialnodes.end());	
}

bool 
XMLNodeTracker::isSpecialNode(std::string const& name)
{
	return (this->_specialnodes.find(name.data()) != this->_specialnodes.end());	
}

std::string const
XMLNodeTracker::getSpecialNodeKeyFromName(Glib::ustring const& name)
{
	return this->_specialnodes[name.data()];
}

std::string const
XMLNodeTracker::getSpecialNodeKeyFromName(Glib::ustring const* name)
{
	return this->_specialnodes[name->data()];	
}

std::string
XMLNodeTracker::generateKey(gchar const* JID)
{
	return String::compose("%1;%2", this->_counter++, JID);
}

std::string 
XMLNodeTracker::generateKey()
{
	SessionData* sd = this->_sm->session_data;
	std::bitset< NUM_FLAGS >& status = sd->status;
	if (status[IN_CHATROOM]) {
		// This is not strictly required for chatrooms: chatrooms will
		// function just fine with the user-to-user ID scheme.  However,
		// the user-to-user scheme can lead to loss of anonymity
		// in anonymous chat rooms, since it contains the real JID
		// of a user.
		return String::compose("%1;%2@%3/%4", this->_counter++, sd->chat_name, sd->chat_server, sd->chat_handle);
	} else {
		return String::compose("%1;%2", this->_counter++, sd->jid);
	}
}

void
XMLNodeTracker::createSpecialNodeTables()
{
	int const sz = sizeof(specialnodekeys) / sizeof(char const*);
	for(int i = 0; i < sz; i++) {
		this->_specialnodes[specialnodenames[i]] = specialnodekeys[i];
	}
}


// rather nasty and crufty debugging function
void 
XMLNodeTracker::dump()
{
	g_log(NULL, G_LOG_LEVEL_DEBUG, "XMLNodeTracker dump for %s", this->_sm->session_data->jid.c_str());
	KeyToTrackerNodeMap::iterator i = this->_keyToNode.begin();
	TrackerNodeToKeyMap::iterator j = this->_nodeToKey.begin();
	std::map< char const*, char const* >::iterator k = this->_specialnodes.begin();
	

	g_log(NULL, G_LOG_LEVEL_DEBUG, "%u entries in keyToNode, %u entries in nodeToKey", this->_keyToNode.size(), this->_nodeToKey.size());

	if (this->_keyToNode.size() != this->_nodeToKey.size()) {
		g_warning("Map sizes do not match!");
	}

	g_log(NULL, G_LOG_LEVEL_DEBUG, "XMLNodeTracker keyToNode dump");
	while(i != this->_keyToNode.end()) {
		if (!((*i).first.empty())) {
			if ((*i).second != NULL) {
				g_log(NULL, G_LOG_LEVEL_DEBUG, "%s\t->\t%p (%s) (%s)", (*i).first.c_str(), (*i).second, (*i).second->name(), (*i).second->content());
			} else {
				g_log(NULL, G_LOG_LEVEL_DEBUG, "%s\t->\t(null)", (*i).first.c_str());
			}
		} else {
			g_log(NULL, G_LOG_LEVEL_DEBUG, "(null)\t->\t%p (%s)", (*i).second, (*i).second->name());
		}
		i++;
	}

	g_log(NULL, G_LOG_LEVEL_DEBUG, "XMLNodeTracker nodeToKey dump");
	while(j != this->_nodeToKey.end()) {
		if (!((*j).second.empty())) {
			if ((*j).first) {
				g_log(NULL, G_LOG_LEVEL_DEBUG, "%p\t->\t%s (parent %p)", (*j).first, (*j).second.c_str(), (*j).first->parent());
			} else {
				g_log(NULL, G_LOG_LEVEL_DEBUG, "(null)\t->\t%s", (*j).second.c_str());
			}
		} else {
			g_log(NULL, G_LOG_LEVEL_DEBUG, "%p\t->\t(null)", (*j).first);
		}
		j++;
	}

	g_log(NULL, G_LOG_LEVEL_DEBUG, "_specialnodes dump");
	while(k != this->_specialnodes.end()) {
		g_log(NULL, G_LOG_LEVEL_DEBUG, "%s\t->\t%s", (*k).first, (*k).second);
		k++;
	}
}

void
XMLNodeTracker::reset()
{
	this->_clear();

	// Find and insert special nodes
	// root node
	this->put(this->_rootKey, *(sp_document_repr_root(this->_sm->document())));

	// namedview node
	SPObject* namedview = sp_item_group_get_child_by_name((SPGroup *)this->_sm->document()->root, NULL, DOCUMENT_NAMEDVIEW_NAME);
	if (!namedview) {
		g_warning("namedview node does not exist; it will be created during synchronization");
	} else {
		this->put(this->_namedviewKey, *(SP_OBJECT_REPR(namedview)));
	}
}

void
XMLNodeTracker::_clear()
{
	// Remove all keys in both trackers, and delete each key.
	this->_keyToNode.clear();
	this->_nodeToKey.clear();

	/*
	TrackerNodeToKeyMap::iterator j = this->_nodeToKey.begin();
	for(; j != this->_nodeToKey.end(); j++) {
		this->_nodeToKey.erase(j);
	}
	*/
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

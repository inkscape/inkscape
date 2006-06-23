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
    _sm = sm;
   init();
}

XMLNodeTracker::XMLNodeTracker() :
                         _rootKey(DOCUMENT_ROOT_NODE),
                         _namedviewKey(DOCUMENT_NAMEDVIEW_NODE)
{
    _sm = NULL;
    init();
}

XMLNodeTracker::~XMLNodeTracker()
{
    _clear();
}


void
XMLNodeTracker::init()
{
    _counter = 0;

    // Construct special node maps
    createSpecialNodeTables();
    if (_sm)
        reset();
}

void
XMLNodeTracker::setSessionManager(const SessionManager *val)
{
    _sm = (SessionManager *)val;
    if (_sm)
        reset();
}

void 
XMLNodeTracker::put(const Glib::ustring &key, const XML::Node &nodeArg)
{	
    keyNodeTable.put(key, &nodeArg);
}


void
XMLNodeTracker::process(const KeyToNodeActionList &actions)
{
    KeyToNodeActionList::const_iterator iter = actions.begin();
    for(; iter != actions.end(); iter++) {
        // Get the action to perform.
        SerializedEventNodeAction action = *iter;
        switch(action.second) {
            case NODE_ADD:
                //g_log(NULL, G_LOG_LEVEL_DEBUG, 
                //"NODE_ADD event: key %s, node %p", 
                //action.first.first.c_str(), action.first.second);
                put(action.first.key, *action.first.node);
                break;
            case NODE_REMOVE:
                //g_log(NULL, G_LOG_LEVEL_DEBUG,
                //"NODE_REMOVE event: key %s, node %p",
                // action.first.first.c_str(), action.first.second);
                //remove(const_cast< XML::Node& >(*action.first.second));
                break;
            default:
                break;
        }
    }
}

XML::Node*
XMLNodeTracker::get(const Glib::ustring &key)
{
    XML::Node *node = keyNodeTable.get(key);
    if (node)
        return node;

    g_warning("Key %s is not being tracked!", key.c_str());
    return NULL;
}

Glib::ustring
XMLNodeTracker::get(const XML::Node &nodeArg)
{
    Glib::ustring key = keyNodeTable.get((XML::Node *)&nodeArg);
    return key;
}

bool
XMLNodeTracker::isTracking(const Glib::ustring &key)
{
    return (get(key)!=NULL);
}

bool
XMLNodeTracker::isTracking(const XML::Node &node)
{
    return (get(node).size()>0);
}


bool
XMLNodeTracker::isRootNode(const XML::Node &node)
{
    XML::Node* docroot = sp_document_repr_root(_sm->getDocument());
    return (docroot == &node);
}


void
XMLNodeTracker::remove(const Glib::ustring& key)
{
    g_log(NULL, G_LOG_LEVEL_DEBUG, "Removing node with key %s", key.c_str());
    keyNodeTable.remove(key);
}

void
XMLNodeTracker::remove(const XML::Node &nodeArg)
{
    //g_log(NULL, G_LOG_LEVEL_DEBUG, "Removing node %p", &node);
    keyNodeTable.remove((XML::Node *)&nodeArg);
}


bool 
XMLNodeTracker::isSpecialNode(const Glib::ustring &name)
{
    return (_specialnodes.find(name.data()) != _specialnodes.end());	
}

Glib::ustring
XMLNodeTracker::getSpecialNodeKeyFromName(Glib::ustring const& name)
{
    return _specialnodes[name.data()];
}

Glib::ustring
XMLNodeTracker::generateKey(gchar const* JID)
{
    return String::compose("%1;%2", _counter++, JID);
}

Glib::ustring 
XMLNodeTracker::generateKey()
{
    std::bitset< NUM_FLAGS >& status = _sm->getStatus();
    Glib::ustring ret;
    if (status[IN_CHATROOM]) {
        // This is not strictly required for chatrooms: chatrooms will
        // function just fine with the user-to-user ID scheme.  However,
        // the user-to-user scheme can lead to loss of anonymity
        // in anonymous chat rooms, since it contains the real JID
        // of a user.
        /*
        ret = String::compose("%1;%2@%3/%4",
                 _counter++,
                 _sm->getClient().getUsername(),
                 _sm->getClient().getHost(),
                 sd->chat_handle);
       */
       //We need to work on this since Pedro allows multiple chatrooms
        ret = String::compose("%1;%2",
                 _counter++, 
                 _sm->getClient().getJid());
    } else {
        ret = String::compose("%1;%2",
                 _counter++, 
                 _sm->getClient().getJid());
    }
    return  ret;
}

void
XMLNodeTracker::createSpecialNodeTables()
{
    int const sz = sizeof(specialnodekeys) / sizeof(char const*);
    for(int i = 0; i < sz; i++)
         _specialnodes[specialnodenames[i]] = specialnodekeys[i];
}


// rather nasty and crufty debugging function
void 
XMLNodeTracker::dump()
{
    g_log(NULL, G_LOG_LEVEL_DEBUG, "XMLNodeTracker dump for %s",
           _sm->getClient().getJid().c_str());



    g_log(NULL, G_LOG_LEVEL_DEBUG, "%u entries in keyNodeTable", 
                         keyNodeTable.size());

    g_log(NULL, G_LOG_LEVEL_DEBUG, "XMLNodeTracker keyNodeTable dump");
    for (unsigned int i=0 ; i<keyNodeTable.size() ; i++)
        {
        KeyNodePair pair = keyNodeTable.item(i);
        Glib::ustring key = pair.key;
        XML::Node *node = pair.node;
        char *name    = "none";
        char *content = "none";
        if (node)
            {
            name = (char *)node->name();
            content = (char *)node->content();
            }
        g_log(NULL, G_LOG_LEVEL_DEBUG, "%s\t->\t%p (%s) (%s)",
                  key.c_str(), node, name, content);
        }

    g_log(NULL, G_LOG_LEVEL_DEBUG, "_specialnodes dump");
    std::map< char const*, char const* >::iterator k = _specialnodes.begin();
    while(k != _specialnodes.end()) {
            g_log(NULL, G_LOG_LEVEL_DEBUG, "%s\t->\t%s", (*k).first, (*k).second);
            k++;
    }
}

void
XMLNodeTracker::reset()
{
    _clear();

    // Find and insert special nodes
    // root node
    put(_rootKey, *(sp_document_repr_root(_sm->getDocument())));

    // namedview node
    SPObject* namedview = sp_item_group_get_child_by_name(
                 (SPGroup *)_sm->getDocument()->root,
                  NULL, DOCUMENT_NAMEDVIEW_NAME);
    if (!namedview) {
            g_warning("namedview node does not exist; it will be created during synchronization");
    } else {
            put(_namedviewKey, *(SP_OBJECT_REPR(namedview)));
    }
}

void
XMLNodeTracker::_clear()
{
    // Remove all keys in both trackers, and delete each key.
    keyNodeTable.clear();
}

}  // namespace Whiteboard

}  // namespace Inkscape



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

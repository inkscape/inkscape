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

#ifndef __WHITEBOARD_XML_NODE_TRACKER_H__
#define __WHITEBOARD_XML_NODE_TRACKER_H__

#include "jabber_whiteboard/tracker-node.h"
#include "jabber_whiteboard/defines.h"

#include <bitset>
#include <cstring>
#include <map>
#include <glibmm.h>

namespace Inkscape {

namespace Whiteboard { 

class SessionManager;

/**
 * std::less-like functor for C-style strings.
 */
struct strcmpless : public std::binary_function< char const*, char const*, bool >
{
	bool operator()(char const* _x, char const* _y) const
	{
		return (strcmp(_x, _y) < 0);
	}
};


// TODO: This is a pretty heinous mess of methods that accept
// both pointers and references -- a lot of it has to do with
// XML::Node& in the node observer and XML::Node* elsewhere,
// although some of it (like Glib::ustring const& vs. 
// Glib::ustring const*) is completely mea culpa. When possible
// it'd be good to thin this class out.

/**
 * XMLNodeTracker generates and watches unique IDs for XML::Nodes for use in 
 * document event serialization and deserialization.
 *
 * More specifically, it has three tasks:
 * <ol>
 * 	<li>Association XML::Nodes with string IDs, and vice versa.</li>
 *  <li>Facilitation of lookup of a string ID or XML::Node given the other key.</li>
 * 	<li>Generation of new string IDs for XML::Nodes.</li>
 * </ol>
 *
 * XML::Nodes are assigned an ID that follows one of two forms:
 * <ol>
 * 	<li>unsigned integer;user JID</li>
 * 	<li>unsigned integer;chatroom@conference server/handle</li>
 * </ol>
 * 
 * Form 1 is used in user-to-user sessions; form 2 is used in chatroom sessions.
 */
class XMLNodeTracker  {
public:
	/**
	 * Constructor.
       	 */
	XMLNodeTracker();

	/**
	 * Constructor.
	 *
	 * \param sm The SessionManager with which an XMLNodeTracker instance is to be associated with.
	 */
	XMLNodeTracker(SessionManager* sm);

    virtual ~XMLNodeTracker();

        void setSessionManager(const SessionManager *val);

	/** 
	 * Insert a (key,node) pair into the tracker.
	 *
	 * \param key The key to associate with the node.
	 * \param node The node to associate with the key.
	 */
	void put(const Glib::ustring &key, const XML::Node &node);

	/**
	 * Process a list of node actions to add and remove nodes from the tracker.
	 *
	 * \param actions The action list to process.
	 */
	void process(const KeyToNodeActionList& actions);

	/**
	 * Retrieve an XML::Node given a key.
	 *
	 * \param key Reference to a const string key.
	 * \return Pointer to an XML::Node, or NULL if no associated node could be found.
	 */
	XML::Node* get(const Glib::ustring &key);

	/**
	 * Retrieve a string key given a reference to an XML::Node.
	 *
	 * \param node Reference to a const XML::Node.
	 * \return The associated string key, or an empty string if no associated key could be found.
	 */
	Glib::ustring get(const XML::Node &node);

	/**
	 * Remove an entry from the tracker based on key.
	 *
	 * \param The key of the entry to remove.
	 */
	void remove(const Glib::ustring& key);

	/**
	 * Remove an entry from the tracker based on XML::Node.
	 *
	 * \param A reference to the XML::Node associated with the entry to remove.
	 */
	void remove(const XML::Node& node);

	/**
	 * Return whether or not a (key,node) pair is being tracked, given a string key.
	 *
	 * \param The key associated with the pair to check.
	 * \return Whether or not the pair is being tracked.
	 */
	bool isTracking(const Glib::ustring &key);

	/**
	 * Return whether or not a (key,node) pair is being tracked, given a node.
	 *
	 * \param The node associated with the pair to check.
	 * \return Whether or not the pair is being tracked.
	 */
	bool isTracking(const XML::Node & node);

	/**
	 * Return whether or not a node identified by a given name is a special node.
	 *
	 * \see Inkscape::Whiteboard::specialnodekeys
	 * \see Inkscape::Whiteboard::specialnodenames
	 *
	 * \param The name associated with the node.
	 * \return Whether or not the node is a special node.
	 */
	bool isSpecialNode(Glib::ustring const& name);

	/**
	 * Retrieve the key of a special node given the name of a special node.
	 *
	 * \see Inkscape::Whiteboard::specialnodekeys
	 * \see Inkscape::Whiteboard::specialnodenames
	 *
	 * \param The name associated with the node.
	 * \return The key of the special node.
	 */
	Glib::ustring getSpecialNodeKeyFromName(
                               const Glib::ustring &name);

	/**
	 * Returns whether or not the given node is the root node of the Document associated
	 * with an XMLNodeTracker's SessionManager.
	 *
	 * \param Reference to an XML::Node to test.
	 * \return Whether or not the given node is the document root node.
	 */
	bool isRootNode(const XML::Node& node);

	/** 
	 * Generate a node key given a JID.
	 *
	 * \param The JID to use in the key.
	 * \return A node string key.
	 */
	Glib::ustring generateKey(gchar const* JID);

	/** 
	 * Generate a node key given the JID specified in the SessionData structure associated
	 * with an XMLNodeTracker's SessionManager.
	 *
	 * \return A node string key.
	 */
	Glib::ustring generateKey();

	// TODO: remove debugging function
	void dump();
	void reset();

private:
        //common code called by constructors
        void init();

	void createSpecialNodeTables();
	void _clear();
	
	unsigned int _counter;
	SessionManager* _sm;

        //KeyNodeTable keyNodeTable;

	std::map< char const*, char const*, strcmpless > _specialnodes;

	// Keys for special nodes
	Glib::ustring _rootKey;
	Glib::ustring _defsKey;
	Glib::ustring _namedviewKey;
	Glib::ustring _metadataKey;

	// noncopyable, nonassignable
	XMLNodeTracker(XMLNodeTracker const&);
	XMLNodeTracker& operator=(XMLNodeTracker const&);
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

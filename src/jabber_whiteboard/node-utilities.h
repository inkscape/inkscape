/**
 * Whiteboard session manager
 * XML node manipulation / retrieval utilities
 * 
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_NODE_UTILITIES_H__
#define __WHITEBOARD_NODE_UTILITIES_H__

#include "jabber_whiteboard/typedefs.h"
#include "xml/node.h"
#include <glibmm.h>

namespace Inkscape {

namespace XML {

class Node;

}

namespace Whiteboard {

class XMLNodeTracker;

class NodeUtilities {
public:
	// Node utilities
//	static Inkscape::XML::Node* lookupReprByValue(Inkscape::XML::Node* root, gchar const* key, ustring const* value);
	static Glib::ustring const nodeTypeToString(XML::Node const& node);
	static XML::NodeType stringToNodeType(Glib::ustring const& type);

	// Node key search utility method
	static std::string const findNodeID(XML::Node const& node, XMLNodeTracker* tracker, NodeToKeyMap const& newnodes);

private:
	// noncopyable, nonassignable
	NodeUtilities(NodeUtilities const&);
	NodeUtilities& operator=(NodeUtilities const&);
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

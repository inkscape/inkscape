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

#include "jabber_whiteboard/defines.h"
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
        //static Inkscape::XML::Node* lookupReprByValue(Inkscape::XML::Node* root,
        //                      gchar const* key, Glib::ustring const* value);
	static Glib::ustring nodeTypeToString(XML::Node const& node);
	static XML::NodeType stringToNodeType(Glib::ustring const& type);

	// Node key search utility method
	static Glib::ustring findNodeID(XML::Node const& node, 
                              XMLNodeTracker* tracker, 
                              KeyNodeTable const& newnodes);

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
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

/**
 * Whiteboard session manager
 * XML node manipulation / retrieval utilities
 * 
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "util/shared-c-string-ptr.h"
#include "util/list.h"

#include "xml/node-observer.h"
#include "xml/attribute-record.h"
#include "xml/repr.h"

#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/node-utilities.h"
#include "jabber_whiteboard/node-tracker.h"
//#include "jabber_whiteboard/node-observer.h"

namespace Inkscape {

namespace Whiteboard {

/*
Inkscape::XML::Node*
NodeUtilities::lookupReprByValue(Inkscape::XML::Node* root, gchar const* key, Glib::ustring const* value)
{
	GQuark const quark = g_quark_from_string(key);
	if (root == NULL) {
		return NULL;
	}

	Inkscape::Util::List<Inkscape::XML::AttributeRecord const> attrlist = root->attributeList();
	for( ; attrlist ; attrlist++) {
		if ((attrlist->key == quark) && (strcmp(attrlist->value, value->data()) == 0)) {
			return root;
		}
	}
	Inkscape::XML::Node* result;
    for ( Inkscape::XML::Node* child = root->firstChild() ; child != NULL ; child = child->next() ) {
		result = NodeUtilities::lookupReprByValue(child, key, value);
        if(result != NULL) {
			return result;
		}
    }

    return NULL;
}
*/

Glib::ustring
NodeUtilities::nodeTypeToString(XML::Node const& node)
{
	switch(node.type()) {
		case XML::DOCUMENT_NODE:
			return NODETYPE_DOCUMENT_STR;
		case XML::ELEMENT_NODE:
			return NODETYPE_ELEMENT_STR;
		case XML::TEXT_NODE:
			return NODETYPE_TEXT_STR;
		case XML::COMMENT_NODE:
		default:
			return NODETYPE_COMMENT_STR;
	}
}

XML::NodeType
NodeUtilities::stringToNodeType(Glib::ustring const& type)
{	
	if (type == NODETYPE_DOCUMENT_STR) {
		return XML::DOCUMENT_NODE;
	} else if (type == NODETYPE_ELEMENT_STR) {
		return XML::ELEMENT_NODE;
	} else if (type == NODETYPE_TEXT_STR) {
		return XML::TEXT_NODE;
	} else {
		return XML::COMMENT_NODE;
	}
}

Glib::ustring
NodeUtilities::findNodeID(XML::Node       const& node, 
                          XMLNodeTracker* tracker,
                          KeyNodeTable    const& newnodes)
{
        //g_log(NULL, G_LOG_LEVEL_DEBUG, "Attempting to locate id for %p", &node);
        Glib::ustring key = newnodes.get((XML::Node *)&node);
        if (key.size()>0)
            return key;

	if (tracker->isTracking(node)) {
                 //g_log(NULL, G_LOG_LEVEL_DEBUG, "Located id for %p (in tracker): %s", &node, tracker->get(node).c_str());
		return tracker->get(node);
	} else {
                //g_log(NULL, G_LOG_LEVEL_DEBUG, "Failed to locate id");
		return "";
	}
}

}

}

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

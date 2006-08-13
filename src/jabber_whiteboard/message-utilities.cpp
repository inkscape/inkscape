/**
 * Message generation utilities
 * 
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 * Jonas Collaros, Stephen Montgomery
 *
 * Copyright (c) 2004-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "util/share.h"
#include "util/list.h"
#include "util/ucompose.hpp"

#include "xml/node.h"
#include "xml/attribute-record.h"
#include "xml/repr.h"

#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/node-utilities.h"
#include "jabber_whiteboard/message-utilities.h"
#include "jabber_whiteboard/node-tracker.h"

#include <iostream>

namespace Inkscape {

namespace Whiteboard {

// This method can be instructed to not build a message string but only collect nodes that _would_ be transmitted
// and subsequently added to the tracker.  This can be useful in the case where an Inkboard user is the only one
// in a chatroom and therefore needs to fill out the node tracker, but does not need to build the message string.
// This can be controlled with the only_collect_nodes flag, which will only create pointers to new XML::Nodes 
// in the maps referenced by newidsbuf and newnodesbuf.  Passing NULL as the message buffer has the same effect.
//
// only_collect_nodes defaults to false because most invocations of this method also use the message string.

Glib::ustring
MessageUtilities::objectToString(Inkscape::XML::Node *element)
{
    if(element->type() == Inkscape::XML::TEXT_NODE)
        return String::ucompose("<text>%1</text>",element->content());

    Glib::ustring attributes;

    for ( Inkscape::Util::List<Inkscape::XML::AttributeRecord const> 
        iter = element->attributeList() ; iter ; ++iter )
    {
        attributes.append(g_quark_to_string(iter->key));
        attributes.append("=\"");
        attributes.append(iter->value);
        attributes.append("\" ");
    }

    return String::ucompose("<%1 %2/>",element->name(),attributes);
}
/*
void
MessageUtilities::newObjectMessage(Glib::ustring &msgbuf, 
                       KeyNodeTable& newnodesbuf,
                       NewChildObjectMessageList& childmsgbuf, 
                       XMLNodeTracker* xmt, 
                       Inkscape::XML::Node const* node,
                       bool only_collect_nodes,
                       bool collect_children)
{
	// Initialize pointers
	Glib::ustring id, refid, parentid;

	gchar const* name = NULL;
	XML::Node* parent = NULL;
	XML::Node* ref = NULL;

	bool only_add_children = false;

        //g_log(NULL, G_LOG_LEVEL_DEBUG, "newObjectMessage: processing node %p of type %s", node, NodeUtilities::nodeTypeToString(*node).data());

	if (node != NULL) {
		parent = sp_repr_parent(node);
		if (parent != NULL) {
                        //g_log(NULL, G_LOG_LEVEL_DEBUG, "Attempting to find ID for parent node %p (on node %p)", parent, node);
			parentid = NodeUtilities::findNodeID(*parent, xmt, newnodesbuf);
			if (parentid.empty()) {
				g_warning("Parent %p is not being tracked, creating new ID", parent);
				parentid = xmt->generateKey();
				newnodesbuf.put(parentid, parent);
			}

			if ( node != parent->firstChild() && parent != NULL ) {
				ref = parent->firstChild();
				while (ref->next() != node) {
					ref = ref->next();
				}
			}	
		}

		if (ref != NULL) {
                        //g_log(NULL, G_LOG_LEVEL_DEBUG, "Attempting to find ID for ref node %p (on %p)", ref, node);
			refid = NodeUtilities::findNodeID(*ref, xmt, newnodesbuf);
			if (refid.empty() && ref != NULL) {
				g_warning("Ref %p is not being tracked, creating new ID", ref);
				refid = xmt->generateKey();
				newnodesbuf.put(refid, ref);
			}
		}

		name = static_cast< gchar const* >(node->name());
	}

	// Generate an id for this object and append it onto the list, if 
	// it's not already in the tracker
	if (!xmt->isSpecialNode(node->name())) {
		if (!xmt->isTracking(*node)) {
			id = xmt->generateKey();	
                        //g_log(NULL, G_LOG_LEVEL_DEBUG, "Inserting %p with id %s", node, id.c_str());
			newnodesbuf.put(id, node);
		} else {
			id = xmt->get(*node);
                //g_log(NULL, G_LOG_LEVEL_DEBUG, "Found id %s for node %p; not inserting into new nodes buffers.", id.c_str(), node);
		}
	} else {
                //g_log(NULL, G_LOG_LEVEL_DEBUG, "Processing special node; not generating key");
		id = xmt->get(*node);
		if (id.empty()) {
			g_warning("Node %p (name %s) is a special node, but it could not be found in the node tracker (possible unexpected duplicate?)  Generating unique ID anyway.", node, node->name());
			id = xmt->generateKey();
			newnodesbuf.put(id, node);
		}
		only_add_children = true;
	}

	// If we're only adding children (i.e. this is a special node)
	// don't process the given node.
	if( !only_add_children && !id.empty() && msgbuf != NULL && !only_collect_nodes ) {
		// <MESSAGE_NEWOBJ>
		msgbuf = msgbuf + "<" + MESSAGE_NEWOBJ + ">";

		// <MESSAGE_PARENT>
		msgbuf = msgbuf + "<" + MESSAGE_PARENT + ">";

		if(!parentid.empty()) {
			msgbuf += parentid;
		}

		// </MESSAGE_NEWOBJ><MESSAGE_CHILD>id</MESSAGE_CHILD>
		msgbuf = msgbuf + "</" + MESSAGE_PARENT + ">";

		msgbuf = msgbuf + "<" + MESSAGE_CHILD + ">";
		msgbuf += id;

		msgbuf = msgbuf + "</" + MESSAGE_CHILD + ">";

		if(!refid.empty()) {
			// <MESSAGE_REF>refid</MESSAGE_REF>
			msgbuf = msgbuf + "<" + MESSAGE_REF + ">";
			
			msgbuf += refid;

			msgbuf = msgbuf + "</" + MESSAGE_REF + ">";
		}

		// <MESSAGE_NODETYPE>*node.type()</MESSAGE_NODETYPE>
		msgbuf = msgbuf + "<" + MESSAGE_NODETYPE + ">" + NodeUtilities::nodeTypeToString(*node);
		msgbuf = msgbuf + "</" + MESSAGE_NODETYPE + ">";

		if (node->content() != NULL) {
			// <MESSAGE_CONTENT>node->content()</MESSAGE_CONTENT>
			msgbuf = msgbuf + "<" + MESSAGE_CONTENT + ">" + node->content();
			msgbuf = msgbuf + "</" + MESSAGE_CONTENT + ">";
		}

		// <MESSAGE_NAME>name</MESSAGE_NAME>
		msgbuf = msgbuf + "<" + MESSAGE_NAME + ">";

		if( name != NULL )
			msgbuf += name;

		msgbuf = msgbuf + "</" + MESSAGE_NAME + ">";

		// </MESSAGE_NEWOBJ>
		msgbuf = msgbuf + "</" + MESSAGE_NEWOBJ + ">";
	} else if (id.empty()) {
		// if ID is NULL, then we have a real problem -- we were not able to find a key
		// nor generate one.  The only thing we can really do here is abort, since we have
		// no way to let the other client(s) uniquely identify this object.
		g_warning(_("ID for new object is NULL even after generation and lookup attempts: the new object will NOT be sent, nor will any of its child objects!"));
		return;
	} else {

	}

        //g_log(NULL, G_LOG_LEVEL_DEBUG, "Generated message");

	if (!only_collect_nodes && msgbuf != NULL && !id.empty()) {
		// Collect new object's attributes and append them onto the msgbuf
		Inkscape::Util::List<Inkscape::XML::AttributeRecord const> attrlist = node->attributeList();

		for(; attrlist; attrlist++) {
			MessageUtilities::objectChangeMessage(msgbuf,
                         xmt, id, g_quark_to_string(attrlist->key),
                          NULL, attrlist->value, false);
		}
	}

	if (!only_collect_nodes)
		childmsgbuf.push_back(msgbuf);

	if (!id.empty() && collect_children) {
		Glib::ustring childbuf;
		// Collect any child objects of this new object
		for ( Inkscape::XML::Node const *child = node->firstChild(); child != NULL; child = child->next() ) {
			childbuf.clear();
			MessageUtilities::newObjectMessage(childbuf,
                        newnodesbuf, childmsgbuf, xmt, child, only_collect_nodes);
			if (!only_collect_nodes) {
				// we're recursing down the tree, so we're picking up child nodes first
				// and parents afterwards
//				childmsgbuf.push_front(childbuf);
			}

		}
	}
}

void
MessageUtilities::objectChangeMessage(Glib::ustring &msgbuf, 
                                    XMLNodeTracker* xmt, 
                                    const Glib::ustring &id, 
                                    gchar const* key, 
                                    gchar const* oldval, 
                                    gchar const* newval, 
                                    bool is_interactive)
{
    // Construct message

    // <MESSAGE_CHANGE><MESSAGE_ID>id</MESSAGE_ID>
    msgbuf = msgbuf + "<" + MESSAGE_CHANGE + ">";
    msgbuf = msgbuf + "<" + MESSAGE_ID + ">";
    msgbuf += id;
    msgbuf = msgbuf + "</" + MESSAGE_ID + ">";

    // <MESSAGE_KEY>key</MESSAGE_KEY>
    msgbuf = msgbuf + "<" + MESSAGE_KEY + ">";
    if (key != NULL) {
            msgbuf += key;
    }
    msgbuf = msgbuf + "</" + MESSAGE_KEY + ">";

    // <MESSAGE_OLDVAL>oldval</MESSAGE_OLDVAL>
    msgbuf = msgbuf + "<" + MESSAGE_OLDVAL + ">";
    if (oldval != NULL) {
            msgbuf += oldval;
    }
    msgbuf = msgbuf + "</" + MESSAGE_OLDVAL + ">";

    // <MESSAGE_NEWVAL>newval</MESSAGE_NEWVAL>
    msgbuf = msgbuf + "<" + MESSAGE_NEWVAL + ">";
    if (newval != NULL) {
            msgbuf += newval;
    }
    msgbuf = msgbuf + "</" + MESSAGE_NEWVAL + ">";

    // <MESSAGE_ISINTERACTIVE>is_interactive</MESSAGE_ISINTERACTIVE>
    msgbuf = msgbuf + "<" + MESSAGE_ISINTERACTIVE + ">";
    if (is_interactive) {
            msgbuf += "true";
    } else {
            msgbuf += "false";
    }
    msgbuf = msgbuf + "</" + MESSAGE_ISINTERACTIVE + ">";

    // </MESSAGE_CHANGE>
    msgbuf = msgbuf + "</" + MESSAGE_CHANGE + ">";
}

void
MessageUtilities::objectDeleteMessage(Glib::ustring &msgbuf, 
                            XMLNodeTracker* xmt,
                            Inkscape::XML::Node const& parent,
                            Inkscape::XML::Node const& child,
                            Inkscape::XML::Node const* prev)
{
    /*
    gchar const* parentid = NULL;
    gchar const* previd = NULL;
    gchar const* childid = NULL;

    childid = child.attribute("id");
    parentid = parent.attribute("id");
    if (prev != NULL) {
            previd = prev->attribute("id");
    }

    Glib::ustring parentid, previd, childid;

    childid = xmt->get(child);
    parentid = xmt->get(parent);
    previd = xmt->get(*prev);

    if (childid.empty())
        return;
            
 
     // <MESSAGE_DELETE><MESSAGE_PARENT>parentid</MESSAGE_PARENT>
     msgbuf = msgbuf + "<" + MESSAGE_DELETE + ">" + "<" + MESSAGE_PARENT + ">";
     if (!parentid.empty()) {
             msgbuf += parentid;
     }
     msgbuf = msgbuf + "</" + MESSAGE_PARENT + ">";

     // <MESSAGE_CHILD>childid</MESSAGE_CHILD>
     msgbuf = msgbuf + "<" + MESSAGE_CHILD + ">";
     if (!childid.empty()) {
             msgbuf += childid;
     }
     msgbuf = msgbuf + "</" + MESSAGE_CHILD + ">";

     // <MESSAGE_REF>previd</MESSAGE_REF>
     msgbuf = msgbuf + "<" + MESSAGE_REF + ">";
     if (!previd.empty()) {
             msgbuf += previd;
     }
     msgbuf = msgbuf + "</" + MESSAGE_REF + ">";

     // </MESSAGE_DELETE>
     msgbuf = msgbuf + "</" + MESSAGE_DELETE + ">";
}

void
MessageUtilities::contentChangeMessage(Glib::ustring& msgbuf, 
                       const Glib::ustring &nodeid, 
                       Util::ptr_shared<char> old_value,
                       Util::ptr_shared<char> new_value)
{
    if (nodeid.empty())
        return;
        
     // <MESSAGE_NODECONTENT>
     msgbuf = msgbuf + "<" + MESSAGE_NODECONTENT + ">";

     // <MESSAGE_ID>nodeid</MESSAGE_ID>
     msgbuf = msgbuf + "<" + MESSAGE_ID + ">";
     msgbuf += nodeid;
     msgbuf = msgbuf + "</" + MESSAGE_ID + ">";

     // <MESSAGE_OLDVAL>old_value</MESSAGE_OLDVAL>
     msgbuf = msgbuf + "<" + MESSAGE_OLDVAL + ">";
     msgbuf += old_value.pointer();
     msgbuf = msgbuf + "</" + MESSAGE_OLDVAL + ">";

     // <MESSAGE_NEWVAL>new_value</MESSAGE_NEWVAL>
     msgbuf = msgbuf + "<" + MESSAGE_NEWVAL + ">";
     msgbuf += new_value.pointer();
     msgbuf = msgbuf + "</" + MESSAGE_NEWVAL + ">";

     // </MESSAGE_NODECONTENT>
     msgbuf = msgbuf + "</" + MESSAGE_NODECONTENT + ">";
}

void
MessageUtilities::childOrderChangeMessage(Glib::ustring& msgbuf, 
                          const Glib::ustring &childid, 
                          const Glib::ustring &oldprevid, 
                          const Glib::ustring &newprevid)
{
    if (childid.empty())
        return;
        
    // <MESSAGE_ORDERCHANGE>
    msgbuf = msgbuf + "<" + MESSAGE_ORDERCHANGE + ">";

    // <MESSAGE_ID>nodeid</MESSAGE_ID>
    msgbuf = msgbuf + "<" + MESSAGE_CHILD + ">";
    msgbuf += childid;
    msgbuf = msgbuf + "</" + MESSAGE_CHILD + ">";

    // <MESSAGE_OLDVAL>oldprevid</MESSAGE_OLDVAL>
    /*
    msgbuf = msgbuf + "<" + MESSAGE_OLDVAL + ">";
    msgbuf += (*oldprevid);
    msgbuf = msgbuf + "</" + MESSAGE_OLDVAL + ">";
    

    // <MESSAGE_NEWVAL>newprevid</MESSAGE_NEWVAL>
    msgbuf = msgbuf + "<" + MESSAGE_NEWVAL + ">";
    msgbuf += newprevid;
    msgbuf = msgbuf + "</" + MESSAGE_NEWVAL + ">";

    // </MESSAGE_ORDERCHANGE>
    msgbuf = msgbuf + "</" + MESSAGE_ORDERCHANGE + ">";

}


bool
MessageUtilities::getFirstMessageTag(struct Node& buf, const Glib::ustring &msg)
{
    if (msg.empty())
        return false;

    // See if we have a valid start tag, i.e. < ... >.  If we do,
    // continue; if not, stop and return NULL.
    //
    // find_first_of returns ULONG_MAX when it cannot find the first
    // instance of the given character.

    Glib::ustring::size_type startDelim = msg.find_first_of('<');
    if (startDelim != ULONG_MAX) {
            Glib::ustring::size_type endDelim = msg.find_first_of('>');
            if (endDelim != ULONG_MAX) {
                    if (endDelim > startDelim) {
                            buf.tag = msg.substr(startDelim+1, (endDelim-startDelim)-1);
                            if (buf.tag.find_first_of('/') == ULONG_MAX) { // start tags should not be end tags


                                    // construct end tag (</buf.data>)
                                    Glib::ustring endTag(buf.tag);
                                    endTag.insert(0, "/");

                                    Glib::ustring::size_type endTagLoc = msg.find(endTag, endDelim);
                                    if (endTagLoc != ULONG_MAX) {
                                            buf.data = msg.substr(endDelim+1, ((endTagLoc - 1) - (endDelim + 1)));
                                            buf.next_pos = endTagLoc + endTag.length() + 1;

                                            return true;
                                    }
                            }
                    }
            }
    }

    return false;
}

bool
MessageUtilities::findTag(struct Node& buf, const Glib::ustring &msg)
{
    if (msg.empty())
        return false;

    // Read desired tag type out of buffer, and append
    // < > to it 

    Glib::ustring searchterm("<");
    searchterm += buf.tag;
    searchterm + ">";

    Glib::ustring::size_type tagStart = msg.find(searchterm, 0);
    if (tagStart != ULONG_MAX) {
            // Find ending tag starting at the point at the end of
            // the start tag.
            searchterm.insert(1, "/");
            Glib::ustring::size_type tagEnd = msg.find(searchterm, tagStart + searchterm.length());
            if (tagEnd != ULONG_MAX) {
                    Glib::ustring::size_type start = tagStart + searchterm.length();
                    buf.data = msg.substr(start, tagEnd - start);
                    return true;
            }
    }
    return false;
}

Glib::ustring
MessageUtilities::makeTagWithContent(const Glib::ustring &tagname,
                                     const Glib::ustring &content)
{
    Glib::ustring buf = "<" + tagname + ">";
    buf += content;
    buf += "</" + tagname + ">";
    return buf;
}
*/

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

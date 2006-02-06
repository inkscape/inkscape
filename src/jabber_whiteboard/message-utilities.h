/**
 * Whiteboard session manager
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

#ifndef __WHITEBOARD_MESSAGE_UTILITIES_H__
#define __WHITEBOARD_MESSAGE_UTILITIES_H__

#include <glibmm.h>
#include "xml/repr.h"

#include "xml/node.h"
#include "jabber_whiteboard/typedefs.h"

using Glib::ustring;

namespace Inkscape {

namespace Util {

class shared_ptr<char>;

}

namespace Whiteboard {

struct Node {
	ustring tag;
	ustring data;
	ustring::size_type next_pos;
};

class XMLNodeTracker;

class MessageUtilities {
public:
	// Message generation utilities
	static void newObjectMessage(ustring* msgbuf, KeyToNodeMap& newidsbuf, NodeToKeyMap& newnodesbuf, NewChildObjectMessageList& childmsgbuf, XMLNodeTracker* xmt, Inkscape::XML::Node const* node, bool only_collect_nodes = false, bool collect_children = true);	
	static void objectChangeMessage(ustring* msgbuf, XMLNodeTracker* xmt, std::string const id, gchar const* key, gchar const* oldval, gchar const* newval, bool is_interactive);
	static void objectDeleteMessage(ustring* msgbuf, XMLNodeTracker* xmt, Inkscape::XML::Node const& parent, Inkscape::XML::Node const& child, Inkscape::XML::Node const* prev);
	static void contentChangeMessage(ustring& msgbuf, std::string const nodeid, Util::shared_ptr<char> old_value, Util::shared_ptr<char> new_value);
	static void childOrderChangeMessage(ustring& msgbuf, std::string const childid, std::string const oldprevid, std::string const newprevid);

	// Message parsing utilities
	static bool getFirstMessageTag(struct Node& buf, ustring const& msg);
	static bool findTag(struct Node& buf, ustring const& msg);

	// Message tag generation utilities
	static Glib::ustring makeTagWithContent(Glib::ustring tagname, Glib::ustring content);

private:
	// noncopyable, nonassignable
	MessageUtilities(MessageUtilities const&);
	MessageUtilities& operator=(MessageUtilities const&);

};

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
#endif

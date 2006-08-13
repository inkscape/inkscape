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
#include "jabber_whiteboard/defines.h"

using Glib::ustring;

namespace Inkscape {

namespace Util {

template< typename T >
class ptr_shared;

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
    static Glib::ustring objectToString(Inkscape::XML::Node *element);
/*
    static void newObjectMessage(Glib::ustring &msgbuf, 
                           KeyNodeTable& newnodesbuf, 
                           NewChildObjectMessageList& childmsgbuf, 
                           XMLNodeTracker* xmt,
                           Inkscape::XML::Node const* node, 
                           bool only_collect_nodes = false,
                           bool collect_children = true); 
    static void objectChangeMessage(Glib::ustring &msgbuf, 
                           XMLNodeTracker* xmt,
                           const Glib::ustring &id, 
                           gchar const* key,
                           gchar const* oldval, 
                           gchar const* newval,
                           bool is_interactive);
    static void objectDeleteMessage(Glib::ustring &msgbuf,
                           XMLNodeTracker* xmt, 
                           Inkscape::XML::Node const& parent,
                           Inkscape::XML::Node const& child,
                           Inkscape::XML::Node const* prev);
    static void contentChangeMessage(Glib::ustring &msgbuf,
                           const Glib::ustring &nodeid,
                           Util::ptr_shared<char> old_value,
                           Util::ptr_shared<char> new_value);
    static void childOrderChangeMessage(Glib::ustring &msgbuf,
                           const Glib::ustring &childid,
                           const Glib::ustring &oldprevid,
                           const Glib::ustring &newprevid);

    // Message parsing utilities
    static bool getFirstMessageTag(struct Node& buf,
                          const Glib::ustring &msg);
    static bool findTag(struct Node& buf, 
                          const Glib::ustring &msg);

    // Message tag generation utilities
    static Glib::ustring makeTagWithContent(const Glib::ustring &tagname,
                                            const Glib::ustring &content);
*/
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

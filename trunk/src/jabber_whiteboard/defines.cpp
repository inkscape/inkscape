/**
 * Whiteboard session manager
 * Definitions
 * 
 * Authors:
 * Dale Harvey <harveyd@gmail.com>
 *
 * Copyright (c) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __INKSCAPE_WHITEBOARD_DEFINES_CPP__
#define __INKSCAPE_WHITEBOARD_DEFINES_CPP__

#include "jabber_whiteboard/defines.h"

namespace Inkscape {

namespace Whiteboard {

namespace Message {

    Wrapper PROTOCOL       ("protocol");
    Wrapper NEW            ("new");
    Wrapper REMOVE         ("remove");
    Wrapper CONFIGURE      ("configure");
    Wrapper MOVE           ("move");

    Message CONNECT_REQUEST       ("connect-request");
    Message CONNECTED             ("connected");
    Message ACCEPT_INVITATION     ("accept-invitation");
    Message DECLINE_INVITATION    ("decline-invitation");
    Message DOCUMENT_BEGIN        ("document-begin");
    Message DOCUMENT_END          ("document-end");
}

namespace Vars {

    const std::string DOCUMENT_ROOT_NODE("ROOT"); 
    const std::string INKBOARD_XMLNS("http://inkscape.org/inkboard"); 

    const std::string WHITEBOARD_MESSAGE(
        "<message type='%1' from='%2' to='%3'>"
            "<wb xmlns='%4' session='%5'>%6</wb>"
        "</message>"); 

    const std::string PROTOCOL_MESSAGE(
        "<%1><%2 /></%1>");

    const std::string NEW_MESSAGE(
        "<new parent=\"%1\" id=\"%2\" index=\"%3\" version=\"%4\">%5</new>");

    const std::string CONFIGURE_MESSAGE(
        "<configure target=\"%1\" version=\"%2\" attribute=\"%3\" value=\"%4\" />");

    const std::string CONFIGURE_TEXT_MESSAGE(
        "<configure target=\"%1\" version=\"%2\"><text>%3</text></configure>");

    const std::string MOVE_MESSAGE(
        "<move target=\"%1\" n=\"%2\" />");

    const std::string REMOVE_MESSAGE(
        "<remove target=\"%1\" />");
}

namespace State {

    SessionType WHITEBOARD_MUC      ("groupchat"); 
    SessionType WHITEBOARD_PEER     ("chat");

}

// Protocol versions
char const* MESSAGE_PROTOCOL_V1 =	"1";
char const* MESSAGE_PROTOCOL_V2	=	"2";
int const HIGHEST_SUPPORTED =		1;

// Node types (as strings)
char const* NODETYPE_DOCUMENT_STR =	"document";
char const* NODETYPE_ELEMENT_STR =	"element";
char const* NODETYPE_TEXT_STR =		"text";
char const* NODETYPE_COMMENT_STR = 	"comment";

// Number of chars to allocate for type field (in SessionManager::sendMessage)
int const TYPE_FIELD_SIZE =			5;

// Number of chars to allocate for sequence number field (in SessionManager::sendMessage)
int const SEQNUM_FIELD_SIZE	=	70;

// Designators for certain "special" nodes in the document
// These nodes are "special" because they are generally present in all documents,
// and we generally only want one copy of them
char const* DOCUMENT_ROOT_NODE =		"ROOT";
char const* DOCUMENT_NAMEDVIEW_NODE =	"NAMEDVIEW";

// Names of these special nodes
char const* DOCUMENT_ROOT_NAME =		"svg:svg";
char const* DOCUMENT_NAMEDVIEW_NAME =	"sodipodi:namedview";


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

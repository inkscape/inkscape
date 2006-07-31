/**
 * Inkscape::Whiteboard::InkboardDocument - Inkboard document implementation
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <glibmm.h>

#include "jabber_whiteboard/inkboard-document.h"

#include "xml/simple-session.h"
#include "jabber_whiteboard/inkboard-session.h"
#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/session-manager.h"

namespace Inkscape {

namespace Whiteboard {

InkboardDocument::InkboardDocument(int code, State::SessionType type, Glib::ustring const& to) :
	XML::SimpleNode(code), _type(type), _recipient(to)
{
    _initBindings();
}

void
InkboardDocument::_initBindings()
{
    this->_sm = &SessionManager::instance();
    _bindDocument(*this);
    _bindLogger(*(new XML::SimpleSession()));
}

void
InkboardDocument::setRecipient(Glib::ustring const& val)
{
    this->_recipient = val;
}

Glib::ustring 
InkboardDocument::getRecipient() const
{
    return this->_recipient;
}

void
InkboardDocument::setSessionIdent(Glib::ustring const& val)
{
    this->_session = val;
}

Glib::ustring 
InkboardDocument::getSessionIdent() const
{
    return this->_session;
}

void
InkboardDocument::startSessionNegotiation()
{
    sendProtocol(_recipient, Message::PROTOCOL,Message::CONNECT_REQUEST);
}

void
InkboardDocument::terminateSession()
{

}

void
InkboardDocument::processInkboardEvent(Message::Wrapper mtype, unsigned int seqnum, Glib::ustring const& data)
{
    g_log(NULL, G_LOG_LEVEL_DEBUG, "Processing Inkboard event: mtype=%s seqnum=%d data=%s\n", mtype, seqnum, data.c_str());
}

bool
InkboardDocument::sendProtocol(const Glib::ustring &destJid, Message::Wrapper wrapper,
     Message::Message message)
{
    char *fmt=
        "<message type='%s' from='%s' to='%s'>"
            "<wb xmlns='%s' session='%s'>"
                "<%s>"
                    "%s"
                "</%s>"
            "</wb>"
        "</message>";
    if (!_sm->getClient().write(
        fmt,_type,_sm->getClient().getJid().c_str(),destJid.c_str(),
        Vars::INKBOARD_XMLNS,this->getSessionIdent().c_str(),wrapper,message,wrapper))
        return false;

    return true;
}

} // namespace Whiteboard
} // namespace Inkscape


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

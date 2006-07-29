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

InkboardDocument::InkboardDocument(int code, SessionType type, Glib::ustring const& to) :
	XML::SimpleNode(code), _type(type), _recipient(to)
{
	_initBindings();
}

void
InkboardDocument::_initBindings()
{
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
InkboardDocument::startSessionNegotiation()
{
	SessionManager& sm = SessionManager::instance();
        switch (_type) {
            case INKBOARD_MUC:
                sm.sendGroup(_recipient, Message::PROTOCOL, " ");
                break;
            case INKBOARD_PRIVATE:
            default:
                sm.send(_recipient, Message::PROTOCOL," ");
                break;
        }
}

void
InkboardDocument::terminateSession()
{

}

void
InkboardDocument::processInkboardEvent(Message::Wrapper mtype, unsigned int seqnum, Glib::ustring const& data)
{
    g_log(NULL, G_LOG_LEVEL_DEBUG, "Processing Inkboard event: mtype=%d seqnum=%d data=%s\n", mtype, seqnum, data.c_str());
}

bool
InkboardDocument::send(const Glib::ustring &destJid, Message::Wrapper wrapper, Message::Message message)
{
    char *fmt=
        "<message type='chat' from='%s' to='%s'>"
            "<wb xmlns='%s'>"
                "<protocol>"
                    "%s"
                "</protocol>"
            "</wb>"
        "</message>";
    //if (!getClient().write(fmt,getClient().getJid().c_str(),destJid.c_str(),Vars::INKBOARD_XMLNS,message))
    //    return false;

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

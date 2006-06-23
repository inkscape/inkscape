/**
 * Whiteboard session manager
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 * Bob Jamison (Pedro port)
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <functional>
#include <algorithm>
#include <iostream>

#include <gtkmm.h>
#include <glibmm/i18n.h>

#include "xml/node-observer.h"

#include "document.h"
#include "desktop.h"
#include "desktop-handles.h"

#include "jabber_whiteboard/message-verifier.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/inkboard-document.h"
#include "jabber_whiteboard/new-inkboard-document.h"

#define INKBOARD_XMLNS "http://inkscape.org/inkboard"

namespace Inkscape {

namespace Whiteboard {

//#########################################################################
//# S E S S I O N    M A N A G E R
//#########################################################################

SessionManager *sessionManagerInstance = NULL;

void SessionManager::showClient()
{
	SessionManager::instance().gui.show();
}

SessionManager&
SessionManager::instance()
{
    if (!sessionManagerInstance)
        sessionManagerInstance = new SessionManager();
	return *sessionManagerInstance;
}

SessionManager::SessionManager() 
{
    sequenceNumber = 0L;
    getClient().addXmppEventListener(*this);

	this->_check_pending_invitations = Glib::signal_timeout().connect(sigc::mem_fun(*this, &SessionManager::_checkInvitationQueue), 50);
	this->_check_invitation_responses = Glib::signal_timeout().connect(sigc::mem_fun(*this, &SessionManager::_checkInvitationResponseQueue), 50);
}

SessionManager::~SessionManager()
{
    getClient().removeXmppEventListener(*this);
    getClient().disconnect();
}

unsigned long SessionManager::getSequenceNumber()
{
    return sequenceNumber++;
}

bool
SessionManager::send(const Glib::ustring &destJid, 
					 const MessageType type,
                     const Glib::ustring &data)
{
    Pedro::DOMString xmlData = Pedro::Parser::encode(data);
    char *fmt=
    "<message type='chat' from='%s' to='%s' id='ink_%d'>"
    "<inkboard xmlns='%s' "
    "protocol='%d' type='%d' seq='%d'><x:inkboard-data>%s</x:inkboard-data></inkboard>"
    "<body></body>"
    "</message>";
    if (!getClient().write(fmt, 
                           getClient().getJid().c_str(),
                           destJid.c_str(),
                           getClient().getMsgId(),
                           INKBOARD_XMLNS,
                           2,
                           (MessageType)type,
                           getSequenceNumber(),
                           xmlData.c_str()
                           ))
        {
        return false;
        }
        
    return true;
}

bool
SessionManager::sendGroup(const Glib::ustring &groupJid,
						  const MessageType type,
                          const Glib::ustring &data)
{
    Pedro::DOMString xmlData = Pedro::Parser::encode(data);
    char *fmt=
    "<message type='groupchat' from='%s' to='%s' id='ink_%d'>"
    "<inkboard xmlns='%s' "
    "protocol='%d' type='%d' seq='%d'><x:inkboard-data>%s</x:inkboard-data></inkboard>"
    "<body></body>"
    "</message>";
    if (!getClient().write(fmt,
                           getClient().getJid().c_str(),
                           groupJid.c_str(),
                           getClient().getMsgId(),
                           INKBOARD_XMLNS,
                           2,
                           type,
                           getSequenceNumber(),
                           xmlData.c_str()
                           ))
        {
        return false;
        }
        
    return true;
}

void
SessionManager::processXmppEvent(const Pedro::XmppEvent &event)
{
    int type = event.getType();

    switch (type) {
        case Pedro::XmppEvent::EVENT_STATUS:
            {
            break;
            }
        case Pedro::XmppEvent::EVENT_ERROR:
            {
            break;
            }
        case Pedro::XmppEvent::EVENT_CONNECTED:
            {
            break;
            }
        case Pedro::XmppEvent::EVENT_DISCONNECTED:
            {
            break;
            }
        case Pedro::XmppEvent::EVENT_MESSAGE:
            {
            printf("## SM message:%s\n", event.getFrom().c_str());
            Pedro::Element *root = event.getDOM();

            if (root)
                {
                if (root->getTagAttribute("inkboard", "xmlns") ==
                               INKBOARD_XMLNS)
                    {
                        _processInkboardEvent(event);
                    }
                }
            break;
            }
        case Pedro::XmppEvent::EVENT_PRESENCE:
            {
            break;
            }
        case Pedro::XmppEvent::EVENT_MUC_MESSAGE:
            {
            printf("## SM MUC message:%s\n", event.getFrom().c_str());
            Pedro::Element *root = event.getDOM();
            if (root)
                {
                if (root->getTagAttribute("inkboard", "xmlns") ==
                               INKBOARD_XMLNS)
                    {
                        _processInkboardEvent(event);
                    }
                }
            break;
            }
        case Pedro::XmppEvent::EVENT_MUC_JOIN:
            {
            break;
            }
        case Pedro::XmppEvent::EVENT_MUC_LEAVE:
            {
            break;
            }
        case Pedro::XmppEvent::EVENT_MUC_PRESENCE:
            {
            break;
            }
        default:
            {
            break;
            }
    }
}

/**
 * Initiates a shared session with a user or conference room.
 * 
 * \param to The recipient to which this desktop will be linked, specified as a JID.
 * \param type Type of the session; i.e. private message or group chat.
 */
void
SessionManager::doShare(Glib::ustring const& to, SessionType type)
{
    SPDesktop* dt = createInkboardDesktop(to, type);
	if (dt != NULL) {
		InkboardDocument* doc = dynamic_cast< InkboardDocument* >(sp_desktop_document(dt)->rdoc);
		if (doc != NULL) {
			doc->startSessionNegotiation();
		}
	}
}

/**
 * Clone of sp_file_new and all related subroutines and functions,
 * with appropriate modifications to use the Inkboard document class.
 *
 * \param to The JID to which this Inkboard document will be connected.
 * \return A pointer to the created desktop, or NULL if a new desktop could not be created.
 */
SPDesktop*
SessionManager::createInkboardDesktop(Glib::ustring const& to, SessionType type)
{
// Create document (sp_repr_document_new clone)
    SPDocument* doc = makeInkboardDocument(g_quark_from_static_string("xml"), "svg:svg", type, to);
    g_return_val_if_fail(doc != NULL, NULL);

    InkboardDocument* inkdoc = dynamic_cast< InkboardDocument* >(doc->rdoc);
    if (inkdoc == NULL) { // this shouldn't ever happen...
        return NULL;
    }

// Create desktop and attach document
    SPDesktop *dt = makeInkboardDesktop(doc);
    _inkboards.push_back(Inkboard_record_type(to, inkdoc));
    return dt;
}

void
SessionManager::terminateInkboardSession(Glib::ustring const& to)
{
	std::cout << "Terminating Inkboard session to " << to << std::endl;
    Inkboards_type::iterator i = _inkboards.begin();
    for(; i != _inkboards.end(); ++i) {
        if ((*i).first == to) {
            break;
        }
    }

    if (i != _inkboards.end()) {
		std::cout << "Erasing Inkboard session to " << to << std::endl;
        (*i).second->terminateSession();
        _inkboards.erase(i);
    }
}

InkboardDocument*
SessionManager::getInkboardSession(Glib::ustring const& to)
{
    Inkboards_type::iterator i = _inkboards.begin();
    for(; i != _inkboards.end(); ++i) {
        if ((*i).first == to) {
            return (*i).second;
        }
    }
    return NULL;
}

void
SessionManager::_processInkboardEvent(Pedro::XmppEvent const& event)
{
    Pedro::Element* root = event.getDOM();

	if (root == NULL) {
		g_warning("Received null DOM; ignoring message.");
		return;
	}

    Pedro::DOMString type = root->getTagAttribute("inkboard", "type");
    Pedro::DOMString seq = root->getTagAttribute("inkboard", "seq");
    Pedro::DOMString protover = root->getTagAttribute("inkboard", "protocol");

    if (type.empty() || seq.empty() || protover.empty()) {
        g_warning("Received incomplete Inkboard message (missing type, protocol, or sequence number); ignoring message.");
        return;
    }

    MessageType mtype = static_cast< MessageType >(atoi(type.c_str()));

	// Messages that deal with the creation and destruction of sessions should be handled
	// here in the SessionManager.  
	//
	// These events are listed below, along with rationale.
	//
	// - CONNECT_REQUEST_USER: when we begin to process this message, we will not have an 
	//   Inkboard session available to send the message to.  Therefore, this message needs
	//   to be handled by the SessionManager.
	// 
	// - CONNECT_REQUEST_REFUSED_BY_PEER: this message means that the recipient of a 
	//   private invitation refused the invitation.  In this case, we need to destroy the
	//   Inkboard desktop, document, and session associated with that invitation.
	//   Destruction of these components seems to be more naturally done in the SessionManager
	//   than in the Inkboard document itself (especially since the document may be associated
	//   with multiple desktops).
	//
	// - UNSUPPORTED_PROTOCOL_VERSION: this message means that the recipient of an invitation
	//   does not support the version of the Inkboard protocol we are using.  In this case,
	//   we have to destroy the Inkboard desktop, document, and session associated with that
	//   invitation.  The rationale for doing it in the SessionManager is the same as that
	//   given above.
	//
	// - ALREADY_IN_SESSION: similar rationale to above.
	//
	// - DISCONNECTED_FROM_USER_SIGNAL: similar rationale to above.
	//
	//
        // All other events can be handled inside an Inkboard session.
	
	// The message we are handling will have come from some Jabber ID.  We need to verify
	// that the Inkboard session associated with that JID is in the correct state for the
	// incoming message (or, in some cases, that the session correctly exists / does not
	// exist).
	InkboardDocument* doc = getInkboardSession(event.getFrom());

//      NOTE: This line refers to a class that hasn't been written yet
//	MessageValidityTestResult res = MessageVerifier::verifyMessageValidity(event, mtype, doc);

	MessageValidityTestResult res = RESULT_INVALID;

	switch (res) {
		case RESULT_VALID:
		{
			switch (mtype) {
				case CONNECT_REQUEST_USER:
				case CONNECT_REQUEST_REFUSED_BY_PEER:
				case UNSUPPORTED_PROTOCOL_VERSION:
				case ALREADY_IN_SESSION:
					_handleSessionEvent(mtype, event);
					break;
				case DISCONNECTED_FROM_USER_SIGNAL:
					break;
				default:
					if (doc != NULL) {
						unsigned int seqnum = atoi(seq.c_str());
						doc->processInkboardEvent(mtype, seqnum, event.getData());
					}
					break;
			}
			break;
		}
		case RESULT_INVALID:
		default:
			// FIXME: better warning message
			g_warning("Received message in invalid context.");
			break;
	}
}

void
SessionManager::_handleSessionEvent(MessageType mtype, Pedro::XmppEvent const& event)
{
	switch (mtype) {
		case CONNECT_REQUEST_USER:
			_handleIncomingInvitation(event.getFrom());
			break;
		case CONNECT_REQUEST_REFUSED_BY_PEER:
			_handleInvitationResponse(event.getFrom(), DECLINE_INVITATION);
			break;
		case ALREADY_IN_SESSION:
			_handleInvitationResponse(event.getFrom(), PEER_ALREADY_IN_SESSION);
			break;
		case UNSUPPORTED_PROTOCOL_VERSION:
			_handleInvitationResponse(event.getFrom(), UNSUPPORTED_PROTOCOL);
			break;
		default:
			break;
	}
}

void
SessionManager::_handleIncomingInvitation(Glib::ustring const& from)
{
	// don't insert duplicate invitations
	if (std::find(_pending_invitations.begin(), _pending_invitations.end(), from) != _pending_invitations.end()) {
		return;
	}

	// We need to do the invitation confirm/deny dialog elsewhere --
	// when this method is called, we're still executing in Pedro's context,
	// which causes issues when we run a dialog main loop.
	//
	// The invitation handling is done in a poller timeout that executes approximately
	// every 50ms.  It calls _checkInvitationQueue.
	_pending_invitations.push_back(from);

}

void
SessionManager::_handleInvitationResponse(Glib::ustring const& from, InvitationResponses resp)
{
	// only handle one response per invitation sender
	//
	// TODO: this could have one huge bug: say that Alice sends an invite to Bob, but
	// Bob is doing something else at the moment and doesn't want to get in an Inkboard
	// session.  Eve intercepts Bob's "reject invitation" message and passes a
	// "accept invitation" message to Alice that comes before Bob's "reject invitation"
	// message. 
	//
	// Does XMPP prevent this sort of attack?  Need to investigate that.
	if (std::find_if(_invitation_responses.begin(), _invitation_responses.end(), CheckInvitationSender(from)) != _invitation_responses.end()) {
		return;
	}

	// We need to do the invitation confirm/deny dialog elsewhere --
	// when this method is called, we're still executing in Pedro's context,
	// which causes issues when we run a dialog main loop.
	//
	// The invitation handling is done in a poller timeout that executes approximately
	// every 50ms.  It calls _checkInvitationResponseQueue.
	_invitation_responses.push_back(Invitation_response_type(from, resp));

}

}  // namespace Whiteboard
 
}  // namespace Inkscape


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

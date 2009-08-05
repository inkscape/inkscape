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
#include <time.h>

#include <gtkmm.h>
#include <glibmm/i18n.h>

#include "xml/node.h"
#include "xml/repr.h"

#include "util/ucompose.hpp"

#include "xml/node-observer.h"

#include "pedro/pedrodom.h"

#include "ui/view/view-widget.h"

#include "application/application.h"
#include "application/editor.h"

#include "document-private.h"
#include "interface.h"
#include "sp-namedview.h"
#include "document.h"
#include "desktop.h"
#include "desktop-handles.h"

#include "jabber_whiteboard/invitation-confirm-dialog.h"
#include "jabber_whiteboard/message-verifier.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/inkboard-document.h"
#include "jabber_whiteboard/defines.h"

#include "jabber_whiteboard/dialog/choose-desktop.h"

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
    getClient().addXmppEventListener(*this);

    this->CheckPendingInvitations = 
        Glib::signal_timeout().connect(sigc::mem_fun(
            *this, &SessionManager::checkInvitationQueue), 50);
}

SessionManager::~SessionManager()
{
    getClient().removeXmppEventListener(*this);
    getClient().disconnect();
}

/**
 * Initiates a shared session with a user or conference room.
 * 
 * \param to The recipient to which this desktop will be linked, specified as a JID.
 * \param type Type of the session; i.e. private message or group chat.
 */
void
SessionManager::initialiseSession(Glib::ustring const& to, State::SessionType type)
{

    Document* doc = makeInkboardDocument(g_quark_from_static_string("xml"), "svg:svg", type, to);
    InkboardDocument* inkdoc = dynamic_cast< InkboardDocument* >(doc->rdoc);
    if(inkdoc == NULL) return;

    if(type == State::WHITEBOARD_PEER) 
    {
        ChooseDesktop dialog;
        int result = dialog.run();

        if(result == Gtk::RESPONSE_OK)
        {
            SPDesktop *desktop = dialog.getDesktop();

            if(desktop != NULL)
            {
                Inkscape::XML::Document *old_doc =
                    sp_desktop_document(desktop)->rdoc;
                inkdoc->root()->mergeFrom(old_doc->root(),"id");
            }
        }else { return; }
    }

    char * sessionId = createSessionId(10);

    inkdoc->setSessionId(sessionId);

    makeInkboardDesktop(doc);
    addSession(WhiteboardRecord(sessionId, inkdoc));

    inkdoc->startSessionNegotiation();


}

void
SessionManager::terminateSession(Glib::ustring const& sessionId)
{
    WhiteboardList::iterator i = whiteboards.begin();
    for(; i != whiteboards.end(); ++i) {
        if ((*i).first == sessionId) 
            break;
    }

    if (i != whiteboards.end()) {
        (*i).second->terminateSession();
        whiteboards.erase(i);
    }
}

void
SessionManager::addSession(WhiteboardRecord whiteboard)
{
    whiteboards.push_back(whiteboard);
}

InkboardDocument*
SessionManager::getInkboardSession(Glib::ustring const& sessionId)
{
    WhiteboardList::iterator i = whiteboards.begin();
    for(; i != whiteboards.end(); ++i) {
        if ((*i).first == sessionId) {
            return (*i).second;
        }
    }
    return NULL;
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
        case Pedro::XmppEvent::EVENT_MUC_MESSAGE:
        case Pedro::XmppEvent::EVENT_MESSAGE:
            {
            Pedro::Element *root = event.getDOM();

            if (root && root->getTagAttribute("wb", "xmlns") == Vars::INKBOARD_XMLNS)
                processWhiteboardEvent(event);

            break;
            }
        case Pedro::XmppEvent::EVENT_PRESENCE:
            {
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
 * Handles all incoming messages from pedro within a valid namespace, CONNECT_REQUEST messages
 * are handled here, as they have no InkboardDocument to be handled from, all other messages
 * are passed to their appropriate Inkboard document, which is identified by the 'session' 
 * attribute of the 'wb' element
 *
 */
void
SessionManager::processWhiteboardEvent(Pedro::XmppEvent const& event)
{
    Pedro::Element* root = event.getDOM();
    if (root == NULL) {
	g_warning("Received null DOM; ignoring message.");
	return;
    }

    Pedro::DOMString session = root->getTagAttribute("wb", "session");
    Pedro::DOMString type = root->getTagAttribute("message", "type");
    Pedro::DOMString domwrapper = root->getFirstChild()->getFirstChild()->getFirstChild()->getName();

    if (session.empty()) {
        g_warning("Received incomplete Whiteboard message, missing session identifier; ignoring message.");
        return;
    }

    if(root->exists(Message::CONNECT_REQUEST) && type == State::WHITEBOARD_PEER)
    {
        handleIncomingInvitation(Invitation(event.getFrom(),session));

    }else
    { 
        Message::Wrapper wrapper = static_cast< Message::Wrapper >(domwrapper);
        InkboardDocument* doc = getInkboardSession(session);

        if(doc != NULL)
            doc->recieve(wrapper, root->getFirstChild());
    }
}

char*
SessionManager::createSessionId(int size)
{
    // Create a random session identifier
    char * randomString = (char*) malloc (size);
    for (int n=0; n<size; n++)
        randomString[n]=rand()%26+'a';
    randomString[size+1]='\0';

    return randomString;
}

/**
 * Adds an invitation to a queue to be executed in SessionManager::_checkInvitationQueue()
 * as when this method is called, we're still executing in Pedro's context, which causes 
 * issues when we run a dialog main loop.
 *
 */
void
SessionManager::handleIncomingInvitation(Invitation invitation)
{
    // don't insert duplicate invitations
    if (std::find(invitations.begin(),invitations.end(),invitation) != invitations.end())
        return;

    invitations.push_back(invitation);

}

bool
SessionManager::checkInvitationQueue()
{
    // The user is currently busy with an action.  Defer invitation processing 
    // until the user is free.
    int x, y;
    Gdk::ModifierType mt;
    Gdk::Display::get_default()->get_pointer(x, y, mt);
    if (mt & GDK_BUTTON1_MASK) 
        return true;

    if (invitations.size() > 0) 
    {
        // There's an invitation to process; process it.
	Invitation invitation = invitations.front();
        Glib::ustring from = invitation.first;
        Glib::ustring sessionId = invitation.second;

	Glib::ustring primary = 
            "<span weight=\"bold\" size=\"larger\">" + 
            String::ucompose(_("<b>%1</b> has invited you to a whiteboard session."), from) + 
            "</span>\n\n" + 
            String::ucompose(_("Do you wish to accept <b>%1</b>'s whiteboard session invitation?"), from);

        InvitationConfirmDialog dialog(primary);

        dialog.add_button(_("Accept invitation"), Dialog::ACCEPT_INVITATION);
        dialog.add_button(_("Decline invitation"), Dialog::DECLINE_INVITATION);

        Dialog::DialogReply reply = static_cast< Dialog::DialogReply >(dialog.run());


        Document* doc = makeInkboardDocument(g_quark_from_static_string("xml"), "svg:svg", State::WHITEBOARD_PEER, from);

        InkboardDocument* inkdoc = dynamic_cast< InkboardDocument* >(doc->rdoc);
        if(inkdoc == NULL) return true;

        inkdoc->handleState(State::INITIAL,State::CONNECTING);
        inkdoc->setSessionId(sessionId);
        addSession(WhiteboardRecord(sessionId, inkdoc));

        switch (reply) {

            case Dialog::ACCEPT_INVITATION:{
                inkdoc->send(from, Message::PROTOCOL,Message::ACCEPT_INVITATION);
                makeInkboardDesktop(doc);
                break; }

            case Dialog::DECLINE_INVITATION: default: {
                inkdoc->send(from, Message::PROTOCOL,Message::DECLINE_INVITATION);
                terminateSession(sessionId);
                break; }
        }

        invitations.pop_front();

    }

    return true;
}

//#########################################################################
//# HELPER FUNCTIONS
//#########################################################################

Document*
makeInkboardDocument(int code, gchar const* rootname, State::SessionType type, Glib::ustring const& to)
{
    Document* doc;

    InkboardDocument* rdoc = new InkboardDocument(g_quark_from_static_string("xml"), type, to);
    rdoc->setAttribute("version", "1.0");
    rdoc->setAttribute("standalone", "no");
    XML::Node *comment = rdoc->createComment(" Created with Inkscape (http://www.inkscape.org/) ");
    rdoc->appendChild(comment);
    GC::release(comment);

    XML::Node* root = rdoc->createElement(rootname);
    rdoc->appendChild(root);
    GC::release(root);

    Glib::ustring name = String::ucompose(
        _("Inkboard session (%1 to %2)"), SessionManager::instance().getClient().getJid(), to);

    doc = sp_document_create(rdoc, NULL, NULL, name.c_str(), TRUE);
    g_return_val_if_fail(doc != NULL, NULL);

    return doc;
}

// TODO: When the switchover to the new GUI is complete, this function should go away
// and be replaced with a call to Inkscape::NSApplication::Editor::createDesktop.  
// It currently only exists to correctly mimic the desktop creation functionality
// in file.cpp.
//
// \see sp_file_new
SPDesktop*
makeInkboardDesktop(Document* doc)
{
    SPDesktop* dt;

    if (NSApplication::Application::getNewGui()) 
        dt = NSApplication::Editor::createDesktop(doc);

    else 
    {
        SPViewWidget *dtw = sp_desktop_widget_new(sp_document_namedview(doc, NULL));
        g_return_val_if_fail(dtw != NULL, NULL);
        sp_document_unref(doc);

        sp_create_window(dtw, TRUE);
        dt = static_cast<SPDesktop*>(dtw->view);
        sp_namedview_window_from_document(dt);
        sp_namedview_update_layers_from_document(dt);
    }

    return dt;
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

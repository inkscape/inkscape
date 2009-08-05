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

#ifndef __INKSCAPE_WHITEBOARD_SESSION_MANAGER_H__
#define __INKSCAPE_WHITEBOARD_SESSION_MANAGER_H__

#include <glibmm.h>

#include <list>
#include <bitset>

#include "desktop.h"

#include "jabber_whiteboard/pedrogui.h"
#include "jabber_whiteboard/message-queue.h"
#include "jabber_whiteboard/defines.h"

#include "gc-alloc.h"

class Document;
class SPDesktop;


namespace Inkscape {

namespace Whiteboard {

class InkboardDocument;

typedef Glib::ustring from,sessionId;
typedef std::pair< Glib::ustring, InkboardDocument* > WhiteboardRecord;
typedef std::vector< WhiteboardRecord, GC::Alloc< WhiteboardRecord, GC::MANUAL > > WhiteboardList;

typedef std::pair< from, sessionId > Invitation;
typedef std::list< Invitation > InvitationList;

class SessionManager : public Pedro::XmppEventListener 
{

public:

    SessionManager();

    virtual ~SessionManager();

    static void             showClient();
    static SessionManager&  instance();

    virtual Pedro::XmppClient &getClient()
        { return gui.client; }

    /**
     * Handles all incoming XMPP events associated with this document
     * apart from CONNECT_REQUEST, which is handled in SessionManager
     */
    virtual void processXmppEvent(const Pedro::XmppEvent &event);


    /**
     * Initiates a shared session with a user or conference room.
     * 
     * \param to The recipient to which this desktop will be linked, specified as a JID.
     * \param type Type of the session; i.e. private message or group chat.
     */
    virtual void initialiseSession(Glib::ustring const& to, State::SessionType type);

    /**
     * Terminates an Inkboard session to a given recipient.  If the session to be
     * terminated does not exist, does nothing.
     *
     * \param sessionId The session identifier to be terminated.
     */
    virtual void terminateSession(Glib::ustring const& sessionId);

    /**
     * Adds a session to whiteboard
     *
     * \param sessionId The session identifier to be terminated.
     */
    virtual void addSession(WhiteboardRecord whiteboard);

    /**
     * Locates an Inkboard session by recipient JID.  
     *
     * \param to The recipient JID identifying the session to be located.
     * \return A pointer to the InkboardDocument associated with the Inkboard session,
     * or NULL if no such session exists.
     */
    InkboardDocument* getInkboardSession(Glib::ustring const& to);


    void operator=(XmppEventListener const& /*other*/)
	    {}

private:

    Pedro::PedroGui     gui;
    WhiteboardList      whiteboards;
    InvitationList      invitations;
    sigc::connection    CheckPendingInvitations;

    void    processWhiteboardEvent(Pedro::XmppEvent const& event);

    void    handleIncomingInvitation(Invitation invitation);

    bool    checkInvitationQueue();

    char*   createSessionId(int size);

};

Document* makeInkboardDocument(int code, gchar const* rootname, 
    State::SessionType type, Glib::ustring const& to);

SPDesktop*  makeInkboardDesktop(Document* doc);

}  // namespace Whiteboard

}  // namespace Inkscape

#endif  /* __SESSION_MANAGER_H__ */

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

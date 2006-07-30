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

#include "jabber_whiteboard/pedrogui.h"
#include "jabber_whiteboard/message-queue.h"
#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/inkboard-session.h"

#include "gc-alloc.h"

class SPDesktop;

namespace Inkscape {

namespace Whiteboard {

enum SessionType {
    INKBOARD_PRIVATE,
    INKBOARD_MUC
};

class SessionManager;
class InkboardDocument;

class SessionManager : public Pedro::XmppEventListener {
public:

    /**
     *
     */
    SessionManager();

    /**
     *
     */
    virtual ~SessionManager();


    static void showClient();
    static SessionManager& instance();

    /**
     *
     */
    virtual Pedro::XmppClient &getClient()
        { return gui.client; }

    /**
     *
     */
    virtual void processXmppEvent(const Pedro::XmppEvent &event);

    /**
     * The session handling has been broken out into a subclass of XML::Session.  See
     * Whiteboard::InkboardSession and Whiteboard::InkboardDocument, both of which
     * are still under heavy construction.  Don't expect them to work just yet.
     *
     * This (should) allow us to substitute an InkboardDocument for an
     * XML::SimpleDocument (InkboardDocument uses InkboardSession), which, I think, should 
     * lead to a very clean way of integrating Inkboard.  It should also
     * give us a cleaner way of handling node tracking.  Finally,
     * it'll let us experiment with data models that may be more appropriate for Inkboard.
     *
     * SessionManager still manages sessions insofar as it receives XMPP data from Pedro and
     * sends it off to the correct InkboardSession.  I'd like to have it still manage
     * user sessions, which is what the user sees; however, that can be confusing from
     * a programmer's standpoint.  This class might therefore need to be renamed 
     * or something.
     *
     * Creation of an Inkboard session should be handled in some fashion similar
     * to this:
     *
     * (1) The user chooses a chat room or user with which she wants to share a whiteboard.
     *
     * (2) This invokes the appropriate doShare() method in Pedro's GUI.
     *
     * (3) doShare() invokes code to create a new Inkscape desktop with a blank document.
     * This desktop will use InkboardDocument (and hence InkboardSession) for transaction
     * management.
	 *
     * (4) Inkboard setup and operation proceeds as it did in the Loudmouth-based code.
     *
     * (3) and (4) will probably need tweaking (for example, it's probably a bad idea to
     * open up a new desktop if an invitation is refused).   Actually, really, nothing
     * here is set in stone.  Feel free to modify.		-- dwyip 2005-11-21
     * 
     */

    /**
     * Initiates a shared session with a user or conference room.
     * 
     * \param to The recipient to which this desktop will be linked, specified as a JID.
     * \param type Type of the session; i.e. private message or group chat.
     */
    virtual void doShare(Glib::ustring const& to, State::SessionType type);

    /**
     * Creates a new desktop with an InkboardDocument.
     *
     * An InkboardDocument (and hence desktop)
     * is identified by the recipient of the document, be it a 
     * conference room or another user.  If an existing document is found, it will be
     * returned.
     *
     * \param to The recipient to which this desktop will be linked, specified as a JID.
     * \param type Type of the session; i.e. private message or group chat.
     * \return A pointer to the created SPDesktop.
     */
    virtual SPDesktop* createInkboardDesktop(Glib::ustring const& to, State::SessionType type);

    /**
     * Terminates an Inkboard session to a given recipient.  If the session to be
     * terminated does not exist, does nothing.
     *
     * \param to The recipient JID identifying the session to be terminated.
     */
    virtual void terminateInkboardSession(Glib::ustring const& to);

    /**
     * Locates an Inkboard session by recipient JID.  
     *
     * \param to The recipient JID identifying the session to be located.
     * \return A pointer to the InkboardDocument associated with the Inkboard session,
     * or NULL if no such session exists.
     */
    InkboardDocument* getInkboardSession(Glib::ustring const& to);

	void operator=(XmppEventListener const& other) {

	}
private:
	// types
    typedef std::pair< Glib::ustring, InkboardDocument* > Inkboard_record_type;
    typedef std::vector< Inkboard_record_type, GC::Alloc< Inkboard_record_type, GC::MANUAL > > Inkboards_type;

	typedef std::list< Glib::ustring > Pending_invitations_type;

	typedef std::pair< Glib::ustring, InvitationResponses > Invitation_response_type;
	typedef std::list< Invitation_response_type > Invitation_responses_type;

	// functors
	struct CheckInvitationSender {
	public:
		CheckInvitationSender(Glib::ustring const& x) : x(x) { }
		~CheckInvitationSender() { }

		bool operator()(SessionManager::Invitation_response_type const& y) const {
			return (x == y.first);
		}
	private:
		Glib::ustring const& x;
	};

   	// objects 
    Pedro::PedroGui gui;
    SendMessageQueue sendMessageQueue;
    ReceiveMessageQueue receiveMessageQueue;

	// members
    unsigned long sequenceNumber;
    Inkboards_type _inkboards;
	Pending_invitations_type _pending_invitations;
	Invitation_responses_type _invitation_responses;

	sigc::connection _check_pending_invitations;
	sigc::connection _check_invitation_responses;

	// methods
    void _processInkboardEvent(Pedro::XmppEvent const& event);
    void _handleSessionEvent(Message::Wrapper mtype, Pedro::XmppEvent const& event);
    void _handleIncomingInvitation(Glib::ustring const& from);
	void _handleInvitationResponse(Glib::ustring const& from, InvitationResponses resp);

	// methods handled externally
	bool _checkInvitationQueue();
	bool _checkInvitationResponseQueue();
};

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

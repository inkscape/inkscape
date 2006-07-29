/**
 * Whiteboard session manager - invitation handling methods
 *
 * Authors: 
 * David Yip <yipdw@rose-hulman.edu>
 * Bob Jamison (Pedro port)
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm.h>
#include <glibmm/i18n.h>

#include "util/ucompose.hpp"

#include "document.h"
#include "desktop-handles.h"

#include "jabber_whiteboard/inkboard-document.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/invitation-confirm-dialog.h"
#include "jabber_whiteboard/defines.h"

namespace Inkscape {

namespace Whiteboard {

bool
SessionManager::_checkInvitationQueue()
{
	int x, y;
	Gdk::ModifierType mt;
	Gdk::Display::get_default()->get_pointer(x, y, mt);
	if (mt & GDK_BUTTON1_MASK) {
		// The user is currently busy with an action.  Defer invitation processing 
		// until the user is free.
		return true;
	}

	if (_pending_invitations.size() > 0) {
		// There's an invitation to process; process it.
		Glib::ustring from = _pending_invitations.front();

		Glib::ustring primary = "<span weight=\"bold\" size=\"larger\">" + String::ucompose(_("<b>%1</b> has invited you to a whiteboard session."), from) + "</span>\n\n";
		primary += String::ucompose(_("Do you wish to accept <b>%1</b>'s whiteboard session invitation?"), from);

		InvitationConfirmDialog dialog(primary);
		
		dialog.add_button(_("Accept invitation"), ACCEPT_INVITATION);
		dialog.add_button(_("Decline invitation"), DECLINE_INVITATION);

		InvitationResponses resp = static_cast< InvitationResponses >(dialog.run());

		switch (resp) {
			case ACCEPT_INVITATION:
			{
				SPDesktop* dt = createInkboardDesktop(from, INKBOARD_PRIVATE);
				InkboardDocument* idoc = dynamic_cast< InkboardDocument* >(sp_desktop_document(dt)->rdoc);
				send(from, Message::PROTOCOL, " ");
				break;
			}
			case DECLINE_INVITATION:
			{
				break;
			}
			default:
				send(from, Message::PROTOCOL, " ");
				break;
		}

		_pending_invitations.pop_front();
	}

	return true;
}


bool
SessionManager::_checkInvitationResponseQueue()
{
	int x, y;
	Gdk::ModifierType mt;
	Gdk::Display::get_default()->get_pointer(x, y, mt);
	if (mt & GDK_BUTTON1_MASK) {
		// The user is currently busy with an action.  Defer invitation response processing 
		// until the user is free.
		return true;
	}

	if (_invitation_responses.size() > 0) {
		Invitation_response_type response = _invitation_responses.front();

		switch (response.second) {
			case ACCEPT_INVITATION:
			{
				break;
			}
			case DECLINE_INVITATION:
			{
				Glib::ustring primary = String::ucompose(_("<span weight=\"bold\" size=\"larger\">The user <b>%1</b> has refused your whiteboard invitation.</span>\n\n"), response.first);

				// TRANSLATORS: %1 is the peer whom refused our invitation, %2 is our Jabber identity.
				Glib::ustring secondary = String::ucompose(_("You are still connected to a Jabber server as <b>%2</b>, and may send an invitation to <b>%1</b> again, or you may send an invitation to a different user."), response.first, this->getClient().getJid());

				Gtk::MessageDialog dialog(primary + secondary, true, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, false);
				dialog.run();
				terminateInkboardSession(response.first);
				break;
			}
			case PEER_ALREADY_IN_SESSION:
				break;

			case UNSUPPORTED_PROTOCOL:
			{
				Glib::ustring primary = String::ucompose(_("<span weight=\"bold\" size=\"larger\">The user <b>%1</b> is using an incompatible version of Inkboard.</span>\n\n"), response.first);

				// TRANSLATORS: %1 is the peer whom refused our invitation, %2 is our Jabber identity.
				Glib::ustring secondary = String::ucompose(_("Inkscape cannot connect to <b>%1</b>.\n\nYou are still connected to a Jabber server as <b>%2</b>."), response.first, this->getClient().getJid());

				Gtk::MessageDialog dialog(primary + secondary, true, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, false);
				dialog.run();
				terminateInkboardSession(response.first);
				break;
			}
			default:
				break;
		}

		_invitation_responses.pop_front();
	}

	return true;
}

}

}

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

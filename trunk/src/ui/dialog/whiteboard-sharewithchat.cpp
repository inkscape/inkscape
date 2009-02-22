/** @file
 * @brief Whiteboard share with chatroom dialog - implementation
 */
/* Authors:
 *   David Yip <yipdw@rose-hulman.edu>
 *   Jason Segal
 *   Jonas Collaros
 *   Stephen Montgomery
 *   Brandi Soggs
 *   Matthew Weinstock (original C/Gtk version)
 *
 * Copyright (c) 2004-2005 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include <sigc++/sigc++.h>
#include <gtk/gtkdialog.h>

#include "message-stack.h"
#include "message-context.h"
#include "inkscape.h"
#include "desktop.h"

#include "preferences.h"

#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/buddy-list-manager.h"

#include "jabber_whiteboard/session-file-selector.h"

#include "ui/dialog/whiteboard-sharewithchat.h"

#include "util/ucompose.hpp"

namespace Inkscape {
namespace UI {
namespace Dialog {

WhiteboardShareWithChatroomDialog* 
WhiteboardShareWithChatroomDialog::create()
{
	return new WhiteboardShareWithChatroomDialogImpl();
}

WhiteboardShareWithChatroomDialogImpl::WhiteboardShareWithChatroomDialogImpl() :
	_layout(4, 2, false)
{
	this->setSessionManager();
	this->_construct();
	this->get_vbox()->show_all_children();
}

WhiteboardShareWithChatroomDialogImpl::~WhiteboardShareWithChatroomDialogImpl()
{

}

void
WhiteboardShareWithChatroomDialogImpl::setSessionManager()
{
    this->_desktop = this->getDesktop();
	this->_sm = this->_desktop->whiteboard_session_manager();

}


void
WhiteboardShareWithChatroomDialogImpl::_construct()
{
	Gtk::VBox* main = this->get_vbox();

	// Construct labels
	this->_labels[0].set_markup_with_mnemonic(_("Chatroom _name:"));
	this->_labels[1].set_markup_with_mnemonic(_("Chatroom _server:"));
	this->_labels[2].set_markup_with_mnemonic(_("Chatroom _password:"));
	this->_labels[3].set_markup_with_mnemonic(_("Chatroom _handle:"));

	this->_labels[0].set_mnemonic_widget(this->_roomname);
	this->_labels[1].set_mnemonic_widget(this->_confserver);
	this->_labels[2].set_mnemonic_widget(this->_roompass);
	this->_labels[3].set_mnemonic_widget(this->_handle);

	Inkscape::Preferences *prefs = Inkscape::Preferences::get();
	this->_roomname.set_text(prefs->getString("/whiteboard/room/name"));
	this->_confserver.set_text(prefs->getString("/whiteboard/room/server"));
	this->_handle.set_text(prefs->getString("/whiteboard/server/username"));

	// Pack table
	this->_layout.attach(this->_labels[0], 0, 1, 0, 1);
	this->_layout.attach(this->_labels[1], 0, 1, 1, 2);
	this->_layout.attach(this->_labels[2], 0, 1, 2, 3);
	this->_layout.attach(this->_labels[3], 0, 1, 3, 4);

	this->_layout.attach(this->_roomname, 1, 2, 0, 1);
	this->_layout.attach(this->_confserver, 1, 2, 1, 2);
	this->_layout.attach(this->_roompass, 1, 2, 2, 3);
	this->_layout.attach(this->_handle, 1, 2, 3, 4);

	// Button setup and callback registration
	this->_share.set_label(_("Connect to chatroom"));
	this->_cancel.set_label(_("Cancel"));
	this->_share.set_use_underline(true);
	this->_cancel.set_use_underline(true);

	this->_share.signal_clicked().connect(sigc::bind< 0 >(sigc::mem_fun(*this, &WhiteboardShareWithChatroomDialogImpl::_respCallback), WhiteboardShareWithChatroomDialogImpl::SHARE));
	this->_cancel.signal_clicked().connect(sigc::bind< 0 >(sigc::mem_fun(*this, &WhiteboardShareWithChatroomDialogImpl::_respCallback), WhiteboardShareWithChatroomDialogImpl::CANCEL));

	// Pack buttons
	this->_buttonsbox.pack_start(this->_cancel);
	this->_buttonsbox.pack_start(this->_share);

	// Set default values
	Glib::ustring jid = this->_sm->session_data->jid;
	Glib::ustring nick = jid.substr(0, jid.find_first_of('@'));
	this->_handle.set_text(nick);
	this->_roomname.set_text("inkboard");

	// Pack into main box
	main->pack_start(this->_layout);
	main->pack_end(this->_buttonsbox);
}

void
WhiteboardShareWithChatroomDialogImpl::_respCallback(int resp)
{
	switch (resp) {
		case SHARE:
		{
			Glib::ustring chatroom, server, handle, password;
			chatroom = this->_roomname.get_text();
			server = this->_confserver.get_text();
			password = this->_roompass.get_text();
			handle = this->_handle.get_text();

			Glib::ustring msg = String::ucompose(_("Synchronizing with chatroom <b>%1@%2</b> using the handle <b>%3</b>"), chatroom, server, handle);

			this->_desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, msg.data());

			this->_desktop->whiteboard_session_manager()->sendRequestToChatroom(server, chatroom, handle, password);
		}
		case CANCEL:
		default:
			this->hide();
			break;
	}
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

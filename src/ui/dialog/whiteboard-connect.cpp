/**
 * Whiteboard connection establishment dialog
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 * Jason Segal, Jonas Collaros, Stephen Montgomery, Brandi Soggs, Matthew Weinstock (original C/Gtk version)
 *
 * Copyright (c) 2004-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>
#include <gtk/gtkdialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/table.h>

#include "inkscape.h"
#include "desktop.h"
#include "message-stack.h"
#include "prefs-utils.h"

#include "jabber_whiteboard/session-manager.h"

#include "message-context.h"
#include "ui/dialog/whiteboard-connect.h"

#include "util/ucompose.hpp"

namespace Inkscape {

namespace UI {

namespace Dialog {

WhiteboardConnectDialog* 
WhiteboardConnectDialog::create()
{
	return new WhiteboardConnectDialogImpl();
}

WhiteboardConnectDialogImpl::WhiteboardConnectDialogImpl() :
	_layout(4, 4, false), _usessl(_("_Use SSL"), true)
{
	this->setSessionManager();
	this->_construct();
	this->get_vbox()->show_all_children();
}

WhiteboardConnectDialogImpl::~WhiteboardConnectDialogImpl()
{
}

void
WhiteboardConnectDialogImpl::setSessionManager()
{
	this->_desktop = SP_ACTIVE_DESKTOP;
	this->_sm = SP_ACTIVE_DESKTOP->whiteboard_session_manager();

}

void
WhiteboardConnectDialogImpl::_construct()
{
	Gtk::VBox* main = this->get_vbox();

	// Construct dialog interface
	this->_labels[0].set_markup_with_mnemonic(_("_Server:"));
	this->_labels[1].set_markup_with_mnemonic(_("_Username:"));
	this->_labels[2].set_markup_with_mnemonic(_("_Password:"));
	this->_labels[3].set_markup_with_mnemonic(_("P_ort:"));

	this->_labels[0].set_mnemonic_widget(this->_server);
	this->_labels[1].set_mnemonic_widget(this->_username);
	this->_labels[2].set_mnemonic_widget(this->_password);
	this->_labels[3].set_mnemonic_widget(this->_port);

	this->_port.set_text("5222");

	this->_layout.attach(this->_labels[0], 0, 1, 0, 1);
	this->_layout.attach(this->_labels[3], 2, 3, 0, 1);
	this->_layout.attach(this->_labels[1], 0, 1, 1, 2);
	this->_layout.attach(this->_labels[2], 0, 1, 2, 3);

	this->_layout.attach(this->_server, 1, 2, 0, 1);
	this->_layout.attach(this->_port, 3, 4, 0, 1);
	this->_layout.attach(this->_username, 1, 4, 1, 2);
	this->_layout.attach(this->_password, 1, 4, 2, 3);

	this->_layout.attach(this->_usessl, 1, 4, 3, 4);

	this->_layout.set_col_spacings(1);
	this->_layout.set_row_spacings(1);

	this->_password.set_visibility(false);
	this->_password.set_invisible_char('*');

	// Buttons
	this->_ok.set_label(_("Connect"));
	this->_cancel.set_label(_("Cancel"));
	this->_ok.signal_clicked().connect(sigc::bind< 0 >(sigc::mem_fun(*this, &WhiteboardConnectDialogImpl::_respCallback), GTK_RESPONSE_OK));
	this->_cancel.signal_clicked().connect(sigc::bind< 0 >(sigc::mem_fun(*this, &WhiteboardConnectDialogImpl::_respCallback), GTK_RESPONSE_CANCEL));
	this->_usessl.signal_clicked().connect(sigc::mem_fun(*this, &WhiteboardConnectDialogImpl::_useSSLClickedCallback));

	this->_buttons.pack_start(this->_cancel, true, true, 0);
	this->_buttons.pack_end(this->_ok, true, true, 0);

	// Pack widgets into main vbox
	main->pack_start(this->_layout);
	main->pack_end(this->_buttons);
}

void
WhiteboardConnectDialogImpl::_respCallback(int resp)
{
	if (resp == GTK_RESPONSE_OK) {
		Glib::ustring server, port, username, password;
		bool usessl;

		server = this->_server.get_text();
		port = this->_port.get_text();
		username = this->_username.get_text();
		password = this->_password.get_text();
		usessl = this->_usessl.get_active();

		Glib::ustring msg = String::ucompose(_("Establishing connection to Jabber server <b>%1</b> as user <b>%2</b>"), server, username);
		this->_desktop->messageStack()->flash(INFORMATION_MESSAGE, msg.data());

		switch (this->_sm->connectToServer(server, port, username, password, usessl)) {
			case FAILED_TO_CONNECT:
				msg = String::ucompose(_("Failed to establish connection to Jabber server <b>%1</b>"), server);
				this->_desktop->messageStack()->flash(WARNING_MESSAGE, msg.data());
				this->_sm->connectionError(msg);
				break;
			case INVALID_AUTH:
				msg = String::ucompose(_("Authentication failed on Jabber server <b>%1</b> as <b>%2</b>"), server, username);
				this->_desktop->messageStack()->flash(WARNING_MESSAGE, msg.data());
				this->_sm->connectionError(msg);
				break;
			case SSL_INITIALIZATION_ERROR:
				msg = String::ucompose(_("SSL initialization failed when connecting to Jabber server <b>%1</b>"), server);
				this->_desktop->messageStack()->flash(WARNING_MESSAGE, msg.data());
				this->_sm->connectionError(msg);
				break;
				
			case CONNECT_SUCCESS:
				msg = String::ucompose(_("Connected to Jabber server <b>%1</b> as <b>%2</b>"), server, username);
				this->_desktop->messageStack()->flash(INFORMATION_MESSAGE, msg.data());

				// Save preferences
				prefs_set_string_attribute(this->_prefs_path, "server", this->_server.get_text().c_str());
				break;
			default:
				break;
		}
	}

	this->_password.set_text("");
	this->hide();
}

void
WhiteboardConnectDialogImpl::_useSSLClickedCallback()
{
	if (this->_usessl.get_active()) {
		this->_port.set_text("5223");
	
		// String::ucompose seems to format numbers according to locale; unfortunately,
		// I'm not yet sure how to turn that off
		//this->_port.set_text(String::ucompose("%1", LM_CONNECTION_DEFAULT_PORT_SSL));
	} else {
		this->_port.set_text("5222");
	}
}

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

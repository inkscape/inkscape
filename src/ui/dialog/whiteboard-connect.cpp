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
#include "preferences.h"

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
	_layout(4, 4, false), _usessl(_("_Use SSL"), true), _register(_("_Register"), true)
{
	this->setSessionManager();
	this->_construct();
	//this->set_resize_mode(Gtk::RESIZE_IMMEDIATE);
	this->set_resizable(false);
	this->get_vbox()->show_all_children();
}

WhiteboardConnectDialogImpl::~WhiteboardConnectDialogImpl()
{
}

void WhiteboardConnectDialogImpl::present()
{
    Dialog::present();
}

void
WhiteboardConnectDialogImpl::setSessionManager()
{
	this->_desktop = this->getDesktop();
	this->_sm = this->_desktop->whiteboard_session_manager();
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

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
	this->_server.set_text(prefs->getString("whiteboard.server", "name"));
	/// @todo Convert port to an integer preference?
	this->_port.set_text(prefs->getString("whiteboard.server", "port"));
	this->_username.set_text(prefs->getString("whiteboard.server", "username"));
	this->_usessl.set_active(prefs->getBool("whiteboard.server", "ssl", false);

	this->_layout.attach(this->_labels[0], 0, 1, 0, 1);
	this->_layout.attach(this->_labels[1], 0, 1, 1, 2);
	this->_layout.attach(this->_labels[2], 0, 1, 2, 3);
	this->_layout.attach(this->_labels[3], 2, 3, 0, 1);

	this->_layout.attach(this->_server, 1, 2, 0, 1);
	this->_layout.attach(this->_port, 3, 4, 0, 1);
	this->_layout.attach(this->_username, 1, 4, 1, 2);
	this->_layout.attach(this->_password, 1, 4, 2, 3);

	this->_checkboxes.attach(this->_blank,0,1,0,1);
	this->_checkboxes.attach(this->_blank,0,1,1,2);

	this->_checkboxes.attach(this->_usessl, 1, 4, 0, 1);
	this->_checkboxes.attach(this->_register, 1, 5, 1, 2);

	this->_layout.set_col_spacings(1);
	this->_layout.set_row_spacings(1);

	this->_password.set_visibility(false);
	this->_password.set_invisible_char('*');

	// Buttons
	this->_ok.set_label(_("Connect"));
	this->_cancel.set_label(_("Cancel"));

	this->_ok.signal_clicked().connect(sigc::bind< 0 >(sigc::mem_fun(*this, &WhiteboardConnectDialogImpl::_respCallback), GTK_RESPONSE_OK));
	this->_cancel.signal_clicked().connect(sigc::bind< 0 >(sigc::mem_fun(*this, &WhiteboardConnectDialogImpl::_respCallback), GTK_RESPONSE_CANCEL));
	
	this->_register.signal_clicked().connect(sigc::mem_fun(*this, &WhiteboardConnectDialogImpl::_registerCallback));
	this->_usessl.signal_clicked().connect(sigc::mem_fun(*this, &WhiteboardConnectDialogImpl::_useSSLClickedCallback));

	this->_buttons.pack_start(this->_cancel, true, true, 0);
	this->_buttons.pack_end(this->_ok, true, true, 0);
	
	// Pack widgets into main vbox
	main->pack_start(this->_layout,Gtk::PACK_SHRINK);
	main->pack_start(this->_checkboxes,Gtk::PACK_SHRINK);
	main->pack_end(this->_buttons,Gtk::PACK_SHRINK);
}


void
WhiteboardConnectDialogImpl::_registerCallback()
{
	if (this->_register.get_active()) 
	{
		Glib::ustring server, port;
		bool usessl;

		server = this->_server.get_text();
		port = this->_port.get_text();
		usessl = this->_usessl.get_active();

		Glib::ustring msg = String::ucompose(_("Establishing connection to Jabber server <b>%1</b>"), server);
		this->_desktop->messageStack()->flash(INFORMATION_MESSAGE, msg.data());

		if(this->_sm->initializeConnection(server,port,usessl) == CONNECT_SUCCESS)
		{

			std::vector<Glib::ustring> entries = this->_sm->getRegistrationInfo();

			for(unsigned i = 0; i<entries.size();i++)
			{

				Gtk::Entry *entry = manage (new Gtk::Entry);
				Gtk::Label *label = manage (new Gtk::Label);

				Glib::ustring::size_type zero=0,one=1;
				Glib::ustring LabelText = entries[i].replace(zero,one,one,Glib::Unicode::toupper(entries[i].at(0)));

				(*label).set_markup_with_mnemonic(LabelText.c_str());
				(*label).set_mnemonic_widget(*entry);

				this->_layout.attach (*label, 0, 1, i+3, i+4, Gtk::FILL|Gtk::EXPAND|Gtk::SHRINK, (Gtk::AttachOptions)0,0,0);		
				this->_layout.attach (*entry, 1, 4, i+3, i+4, Gtk::FILL|Gtk::EXPAND|Gtk::SHRINK, (Gtk::AttachOptions)0,0,0);

				this->registerlabels.push_back(label);
				this->registerentries.push_back(entry);
			}
		}else{
			Glib::ustring msg = String::ucompose(_("Failed to establish connection to Jabber server <b>%1</b>"), server);
			this->_desktop->messageStack()->flash(WARNING_MESSAGE, msg.data());
			this->_sm->connectionError(msg);
		}

	}else{

		for(unsigned i = 0; i<registerlabels.size();i++)
		{
			this->_layout.remove(*registerlabels[i]);
			this->_layout.remove(*registerentries[i]);

			delete registerlabels[i];
			delete registerentries[i];
		}

		registerentries.erase(registerentries.begin(), registerentries.end());
		registerlabels.erase(registerlabels.begin(), registerlabels.end());
	}

	this->get_vbox()->show_all_children();
	//this->reshow_with_initial_size();
}

void
WhiteboardConnectDialogImpl::_respCallback(int resp)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
	if (resp == GTK_RESPONSE_OK) 
	{
		Glib::ustring server, port, username, password;
		bool usessl;

		server = this->_server.get_text();
		port = this->_port.get_text();
		username = this->_username.get_text();
		password = this->_password.get_text();
		usessl = this->_usessl.get_active();

		Glib::ustring msg = String::ucompose(_("Establishing connection to Jabber server <b>%1</b> as user <b>%2</b>"), server, username);
		this->_desktop->messageStack()->flash(INFORMATION_MESSAGE, msg.data());

		if (!this->_register.get_active()) 
		{
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
					prefs->setString(this->_prefs_path, "server", this->_server.get_text());
					break;
				default:
					break;
			}
		}else{

			std::vector<Glib::ustring> key,val;	

			for(unsigned i = 0; i<registerlabels.size();i++)
			{
				key.push_back((*registerlabels[i]).get_text());
				val.push_back((*registerentries[i]).get_text());
			}

			switch (this->_sm->registerWithServer(username, password, key, val)) 
			{
				case FAILED_TO_CONNECT:
					msg = String::ucompose(_("Failed to establish connection to Jabber server <b>%1</b>"), server);
					this->_desktop->messageStack()->flash(WARNING_MESSAGE, msg.data());
					this->_sm->connectionError(msg);
					break;
				case INVALID_AUTH:
					msg = String::ucompose(_("Registration failed on Jabber server <b>%1</b> as <b>%2</b>"), server, username);
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
					prefs->setString(this->_prefs_path, "server", this->_server.get_text());
					break;
				default:
					break;
			}
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

} // namespace Dialog

} // namespace UI

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

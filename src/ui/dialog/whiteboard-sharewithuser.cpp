/** @file
 * Whiteboard share with user dialog - implementation
 */
/* Authors:
 * David Yip <yipdw@rose-hulman.edu>
 * Jason Segal, Jonas Collaros, Stephen Montgomery, Brandi Soggs, Matthew Weinstock (original C/Gtk version)
 *
 * Copyright (c) 2004-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include <sigc++/sigc++.h>
#include <gtk/gtkdialog.h>

#include "message-stack.h"
#include "message-context.h"
#include "inkscape.h"
#include "desktop.h"

#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/buddy-list-manager.h"

#include "jabber_whiteboard/session-file-selector.h"

#include "ui/dialog/whiteboard-sharewithuser.h"

#include "util/ucompose.hpp"

namespace Inkscape {

namespace UI {

namespace Dialog {

WhiteboardShareWithUserDialog* 
WhiteboardShareWithUserDialog::create()
{
	return new WhiteboardShareWithUserDialogImpl();
}

WhiteboardShareWithUserDialogImpl::WhiteboardShareWithUserDialogImpl() 
{
	this->setSessionManager();
	this->_construct();
	this->get_vbox()->show_all_children();

	this->_sm->session_data->buddyList.addInsertListener(sigc::mem_fun(this, &WhiteboardShareWithUserDialogImpl::_insertBuddy));
	this->_sm->session_data->buddyList.addEraseListener(sigc::mem_fun(this, &WhiteboardShareWithUserDialogImpl::_eraseBuddy));
	
}

WhiteboardShareWithUserDialogImpl::~WhiteboardShareWithUserDialogImpl()
{

}

void
WhiteboardShareWithUserDialogImpl::setSessionManager()
{
        this->_desktop = this->getDesktop();
	this->_sm = this->_desktop->whiteboard_session_manager();

}


void
WhiteboardShareWithUserDialogImpl::_construct()
{
	Gtk::VBox* main = this->get_vbox();

	// Construct dialog interface
	this->_labels[0].set_markup_with_mnemonic(_("_User's Jabber ID:"));
	this->_labels[0].set_mnemonic_widget(this->_jid);
	
	// Buttons
	this->_share.set_label(_("_Invite user"));
	this->_cancel.set_label(_("_Cancel"));
	this->_share.set_use_underline(true);
	this->_cancel.set_use_underline(true);

	// Button callbacks
	this->_share.signal_clicked().connect(sigc::bind< 0 >(sigc::mem_fun(*this, &WhiteboardShareWithUserDialogImpl::_respCallback), SHARE));
	this->_cancel.signal_clicked().connect(sigc::bind< 0 >(sigc::mem_fun(*this, &WhiteboardShareWithUserDialogImpl::_respCallback), CANCEL));

	// Construct ListStore for buddy list information
	this->_buddylistdata = Gtk::ListStore::create(this->_blm);
	this->_buddylist.set_model(this->_buddylistdata);
	this->_buddylist.append_column(_("Buddy List"), this->_blm.jid);

	// Fill buddy list
	this->_fillBuddyList();

	// Buddy list onclick callback
	this->_buddylist.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &WhiteboardShareWithUserDialogImpl::_listCallback));

	// Pack widgets into boxes
	this->_listwindow.add(this->_buddylist);
	this->_listwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	this->_buddylistbox.pack_start(this->_listwindow);

	this->_connecttojidbox.pack_start(this->_labels[0]);
	this->_connecttojidbox.pack_end(this->_jid);

	this->_buttons.pack_start(this->_cancel);
	this->_buttons.pack_end(this->_share);

	// Pack boxes into main box
	main->pack_start(this->_buddylistbox);
	main->pack_start(this->_connecttojidbox);
	main->pack_start(this->_sfsbox);
	main->pack_end(this->_buttons);
}


void
WhiteboardShareWithUserDialogImpl::_fillBuddyList()
{
	Whiteboard::BuddyList& bl = this->_sm->session_data->buddyList.getList();

	for(Whiteboard::BuddyList::iterator i = bl.begin(); i != bl.end(); i++) {
		this->_insertBuddy(*i);
	}
//	std::for_each(bl.begin(), bl.end(), std::mem_fun(&WhiteboardShareWithUserDialogImpl::_insertBuddy));
}

void
WhiteboardShareWithUserDialogImpl::_insertBuddy(std::string const& jid)
{
	// FIXME: need a better way to avoid inserting duplicate rows in the case 
	// of duplicate Jabber presence messages
	typedef Gtk::TreeModel::Children type_children;
	type_children children = this->_buddylistdata->children();
	for(type_children::iterator i = children.begin(); i != children.end(); i++) {
		if ((*i).get_value(this->_blm.jid) == jid) {
			return;
		}
	}

	Gtk::TreeModel::Row row = *(this->_buddylistdata->append());
	row[this->_blm.jid] = jid;
}

void
WhiteboardShareWithUserDialogImpl::_eraseBuddy(std::string const& jid)
{
	// FIXME: Doesn't gtkmm provide a better way to erase rows from a ListStore?
	typedef Gtk::TreeModel::Children type_children;
	type_children children = this->_buddylistdata->children();
	for(type_children::iterator i = children.begin(); i != children.end(); i++) {
		if ((*i).get_value(this->_blm.jid) == jid) {
			this->_buddylistdata->erase(i);
			return;
		}
	}
}

void
WhiteboardShareWithUserDialogImpl::_respCallback(int resp)
{
	switch (resp) {
		case SHARE:
		{
			Glib::ustring jid = this->_jid.get_text();

			// Check that the JID is in the format user@host/resource
			if (jid.find("@", 0) == Glib::ustring::npos) {
				jid += "@";
				jid += lm_connection_get_server(this->_sm->session_data->connection);
			} 

			if (jid.find("/", 0) == Glib::ustring::npos) {
				jid += "/" + static_cast< Glib::ustring >(RESOURCE_NAME);
			}

			g_log(NULL, G_LOG_LEVEL_DEBUG, "Full JID is %s", jid.c_str());

			Glib::ustring msg = String::ucompose(_("Sending whiteboard invitation to <b>%1</b>"), jid);
			this->_sm->desktop()->messageStack()->flash(Inkscape::NORMAL_MESSAGE, msg.data());
			if (this->_sfsbox.isSelected()) {
				this->_sm->session_data->sessionFile = this->_sfsbox.getFilename();
			} else {
				this->_sm->session_data->sessionFile.clear();
			}
			this->_sm->sendRequestToUser(jid);
			this->hide();
			break;
		}

		case CANCEL:
			this->hide();
			break;

		default:
			break;
	}
}

void
WhiteboardShareWithUserDialogImpl::_listCallback()
{
	Glib::RefPtr< Gtk::TreeSelection > sel = this->_buddylist.get_selection();
	
	typedef Gtk::TreeModel::Children type_children;
	type_children::iterator row = sel->get_selected();
	this->_jid.set_text((*row).get_value(this->_blm.jid));
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

/** @file
 * @brief Whiteboard share with user dialog
 */
/* Authors:
 * David Yip <yipdw@rose-hulman.edu>
 * Jason Segal, Jonas Collaros, Stephen Montgomery, Brandi Soggs, Matthew Weinstock (original C/Gtk version)
 *
 * Copyright (c) 2004-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_SHAREWITHUSER_DIALOG_H__
#define __WHITEBOARD_SHAREWITHUSER_DIALOG_H__

#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/scrolledwindow.h>

#include "verbs.h"
#include "ui/dialog/dialog.h"
#include "jabber_whiteboard/session-file-selector.h"


struct SPDesktop;

namespace Inkscape {
    namespace Whiteboard {
        class SessionManager;
    }
    namespace UI {
        namespace Dialog {

class WhiteboardShareWithUserDialog : public Dialog {
public:
	WhiteboardShareWithUserDialog() : Dialog("/dialogs/whiteboard_sharewithuser", SP_VERB_DIALOG_WHITEBOARD_SHAREWITHUSER)
	{

	}

	static WhiteboardShareWithUserDialog* create();

	virtual ~WhiteboardShareWithUserDialog()
	{

	}
};

class WhiteboardShareWithUserDialogImpl : public WhiteboardShareWithUserDialog {
public:
	WhiteboardShareWithUserDialogImpl();
	~WhiteboardShareWithUserDialogImpl();
	void setSessionManager();

private:
	// Response flags
	static unsigned int const SHARE = 0;
	static unsigned int const CANCEL = 2;

	// GTK+ widgets
	Gtk::HBox _connecttojidbox;
	Gtk::HBox _buddylistbox;
	Gtk::HBox _buttons;

	Whiteboard::SessionFileSelectorBox _sfsbox;

	Gtk::Entry _jid;

	// more or less shamelessly stolen from gtkmm tutorial book
	Glib::RefPtr< Gtk::ListStore > _buddylistdata;
	Gtk::TreeView _buddylist;
	class BuddyListModel : public Gtk::TreeModel::ColumnRecord {
	public:
		BuddyListModel()
		{
			add(jid);
		}

		Gtk::TreeModelColumn< std::string > jid;
	};
	BuddyListModel _blm;

	Gtk::Label _labels[2];	
	Gtk::ScrolledWindow _listwindow;

	Gtk::Button _share, _cancel;

	// Construction and callback
	void _construct();
	void _respCallback(int resp);
	void _listCallback();

	// Buddy list management
	void _fillBuddyList();
	void _insertBuddy(std::string const& jid);
	void _eraseBuddy(std::string const& jid);

	// SessionManager and SPDesktop pointers
	::SPDesktop* _desktop;
	Whiteboard::SessionManager* _sm;
};


}

}

}

#endif

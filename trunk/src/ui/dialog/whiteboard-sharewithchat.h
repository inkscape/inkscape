/** @file
 * @brief Whiteboard share with chatroom dialog
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

#ifndef __WHITEBOARD_SHAREWITHCHAT_DIALOG_H__
#define __WHITEBOARD_SHAREWITHCHAT_DIALOG_H__

#include "verbs.h"
#include "dialog.h"

#include <gtkmm/table.h>
#include "jabber_whiteboard/session-file-selector.h"

struct SPDesktop;

namespace Inkscape {

namespace Whiteboard {

class SessionManager;

}

namespace UI {

namespace Dialog {

class WhiteboardShareWithChatroomDialog : public Dialog {
public:
	WhiteboardShareWithChatroomDialog() : Dialog("/dialogs/whiteboard_sharewithuser", SP_VERB_DIALOG_WHITEBOARD_SHAREWITHUSER)
	{

	}

	static WhiteboardShareWithChatroomDialog* create();

	virtual ~WhiteboardShareWithChatroomDialog()
	{

	}
};


class WhiteboardShareWithChatroomDialogImpl : public WhiteboardShareWithChatroomDialog {
public:
	WhiteboardShareWithChatroomDialogImpl();
	~WhiteboardShareWithChatroomDialogImpl();
	void setSessionManager();

private:
	// Response flags
	static unsigned int const SHARE = 0;
	static unsigned int const CANCEL = 2;

	// GTK+ widgets
	Gtk::Table _layout;

	Gtk::HBox _buttonsbox;
	Whiteboard::SessionFileSelectorBox _sfsbox;

	Gtk::Entry _roomname;
	Gtk::Entry _roompass;
	Gtk::Entry _confserver;
	Gtk::Entry _handle;

	Gtk::Label _labels[4];	

	Gtk::Button _share, _cancel;

	// Construction and callback
	void _construct();
	void _respCallback(int resp);

	// SessionManager and SPDesktop pointers
	::SPDesktop* _desktop;
	Whiteboard::SessionManager* _sm;
};

}

}

}

#endif

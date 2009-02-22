/**
 * Whiteboard invitation confirmation dialog --
 * quick subclass of Gtk::MessageDialog
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_INVITATION_CONFIRM_DIALOG_H__
#define __WHITEBOARD_INVITATION_CONFIRM_DIALOG_H__

#include <gtkmm.h>
#include <glibmm.h>

#include "session-file-selector.h"

namespace Inkscape {

namespace Whiteboard {

class InvitationConfirmDialog : public Gtk::MessageDialog {
public:
	InvitationConfirmDialog(Glib::ustring const& msg);
    virtual ~InvitationConfirmDialog();

	Glib::ustring const& getSessionFilePath();
	bool useSessionFile();

private:
	static unsigned int const SELECT_FILE = 0;

	void _respCallback(int resp);

	Gtk::HBox _filesel;

	SessionFileSelectorBox _sfsbox;
	Gtk::CheckButton _usesessionfile;
	Gtk::Entry _sessionfile;
	Gtk::Button _getfilepath;

	void _construct();
	Glib::ustring _selectedpath;

	// noncopyable, nonassignable
	InvitationConfirmDialog(InvitationConfirmDialog const&);
	InvitationConfirmDialog& operator=(InvitationConfirmDialog const&);
};

}

}
#endif

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

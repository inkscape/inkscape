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

#include <gtkmm.h>
#include <glibmm.h>
#include <glibmm/i18n.h>

#include "invitation-confirm-dialog.h"
#include "session-file-selector.h"

namespace Inkscape {

namespace Whiteboard {

InvitationConfirmDialog::InvitationConfirmDialog(Glib::ustring const& msg) :
	Gtk::MessageDialog(msg, true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE, false),
	_usesessionfile(_("_Write session file:"), true)
{
	this->_construct();
	this->get_vbox()->show_all_children();
}

InvitationConfirmDialog::~InvitationConfirmDialog()
{

}

Glib::ustring const&
InvitationConfirmDialog::getSessionFilePath()
{
	return this->_sfsbox.getFilename();
}

bool
InvitationConfirmDialog::useSessionFile()
{
	return this->_sfsbox.isSelected();
}

void
InvitationConfirmDialog::_construct()
{
	this->get_vbox()->pack_end(this->_sfsbox);
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

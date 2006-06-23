/**
 * Session file selector widget
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm.h>
#include <gtkmm.h>

#include "session-file-selector.h"

#include <glibmm/i18n.h>

namespace Inkscape {

namespace Whiteboard {

SessionFileSelectorBox::SessionFileSelectorBox() :
	_usesessionfile(_("_Write session file:"), true)
{
	this->_construct();
}

SessionFileSelectorBox::~SessionFileSelectorBox()
{

}

bool
SessionFileSelectorBox::isSelected()
{
	return this->_usesessionfile.get_active();
}

Glib::ustring const&
SessionFileSelectorBox::getFilename()
{
	return this->_filename;
}

void
SessionFileSelectorBox::_construct()
{
	this->_getfilepath.set_label("...");

	this->pack_start(this->_usesessionfile);
	this->pack_start(this->_sessionfile);
	this->pack_end(this->_getfilepath);

	this->_getfilepath.signal_clicked().connect(sigc::mem_fun(*this, &SessionFileSelectorBox::_callback));
}

void
SessionFileSelectorBox::_callback() {
	Gtk::FileChooserDialog sessionfiledlg(_("Select a location and filename"), Gtk::FILE_CHOOSER_ACTION_SAVE);
	sessionfiledlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	sessionfiledlg.add_button(_("Set filename"), Gtk::RESPONSE_OK);
	int result = sessionfiledlg.run();
	switch (result) {
		case Gtk::RESPONSE_OK:
		{
			this->_usesessionfile.set_active();
			this->_sessionfile.set_text(sessionfiledlg.get_filename());
			this->_filename = sessionfiledlg.get_filename();
			break;
		}
		case Gtk::RESPONSE_CANCEL:
		default:
			break;
	}
}

}

}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/

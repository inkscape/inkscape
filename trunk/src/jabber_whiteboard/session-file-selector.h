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

#ifndef __WHITEBOARD_SESSION_FILE_SELECTOR_BOX_H__
#define __WHITEBOARD_SESSION_FILE_SELECTOR_BOX_H__

#include <glibmm.h>
#include <gtkmm.h>

namespace Inkscape {

namespace Whiteboard {

class SessionFileSelectorBox : public Gtk::HBox {
public:
	SessionFileSelectorBox();
    virtual ~SessionFileSelectorBox();

	bool isSelected();
	Glib::ustring const& getFilename();

private:
	// Construction
	void _construct();
	void _callback();

	// GTK+ widgets
	Gtk::CheckButton _usesessionfile;
	Gtk::Entry _sessionfile;
	Gtk::Button _getfilepath;

	// Internal state
	Glib::ustring _filename;
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

/** @file
 * @brief Whiteboard session playback control dialog
 */
/* Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __SESSION_PLAYBACK_DIALOG_H__
#define __SESSION_PLAYBACK_DIALOG_H__

#include "verbs.h"
#include "dialog.h"

#include "gtkmm/toolbutton.h"
#include "gtkmm/toolbar.h"
#include "gtkmm/expander.h"

#include "ui/widget/icon-widget.h"

struct SPDesktop;

namespace Inkscape {

namespace Whiteboard {

class SessionManager;
class SessionFilePlayer;

}

namespace UI {

namespace Dialog {

class SessionPlaybackDialog : public Dialog {
public:
	SessionPlaybackDialog() : Dialog("/dialogs/session_playback", SP_VERB_DIALOG_WHITEBOARD_SESSIONPLAYBACK)
	{

	}

	static SessionPlaybackDialog* create();

	virtual ~SessionPlaybackDialog()
	{

	}
private:
	SessionPlaybackDialog(SessionPlaybackDialog const& dlg);		// no copy
	void operator=(SessionPlaybackDialog const& dlg);	// no assign
};

class SessionPlaybackDialogImpl : public SessionPlaybackDialog {
public:
	SessionPlaybackDialogImpl();
	~SessionPlaybackDialogImpl();

private:
	// GTK+ widgets
	Gtk::HBox _filebox;
	Gtk::HBox _filebuttons;
	Gtk::HBox _toolbarbox;
	Gtk::HBox _delaybox;

	Gtk::Entry _openfile;

	Gtk::Label _labels[2];
	Gtk::ToolButton _controls[5];

	Gtk::Button _close, _open, _setdelay;

	Gtk::Tooltips _tooltips;
	Gtk::Toolbar _playbackcontrols;
	Gtk::Adjustment _delay;
	Gtk::SpinButton _delayentry;

	Gtk::Frame _filemanager;
	Gtk::VBox _fm;

	Gtk::Frame _playback;

	Gtk::Expander _currentmsgbox;
	Glib::RefPtr<Gtk::TextBuffer> _currentmsgbuffer;
	Gtk::TextView _currentmsgview;
	Gtk::ScrolledWindow _currentmsgscroller;

	// Construction and callback
	void _construct();
	void _respCallback(int resp);

	// SessionManager and SPDesktop pointers
	::SPDesktop* _desktop;
	Whiteboard::SessionManager* _sm;
	Whiteboard::SessionFilePlayer* _sfp;

	// button values
	static unsigned short const CLOSE_FILE = 0;
	static unsigned short const OPEN_FILE = 1;
	static unsigned short const RESET_DELAY = 2;

	static unsigned short const TOOLBAR_BASE = 10;
	static unsigned short const REWIND = TOOLBAR_BASE + 0; 
	static unsigned short const STEP_REWIND = TOOLBAR_BASE + 1;
	static unsigned short const PAUSE = TOOLBAR_BASE + 2;
	static unsigned short const STEP_PLAY = TOOLBAR_BASE + 3;
	static unsigned short const PLAY = TOOLBAR_BASE + 4;


	// noncopyable
	SessionPlaybackDialogImpl(SessionPlaybackDialogImpl const& dlg);		// no copy
	void operator=(SessionPlaybackDialogImpl const& dlg);	// no assign
};

}

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

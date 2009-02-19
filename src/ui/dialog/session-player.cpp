/** @file
 * @brief Whiteboard session playback control dialog - implementation
 */
/* Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtk/gtkdialog.h>
#include <gtkmm.h>

#include "inkscape.h"
#include "path-prefix.h"

#include "desktop.h"
#include "desktop-handles.h"
#include "document.h"

#include "jabber_whiteboard/node-tracker.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/session-file-player.h"

#include "ui/dialog/session-player.h"

#include "util/ucompose.hpp"

namespace Inkscape {

namespace UI {

namespace Dialog {

SessionPlaybackDialog*
SessionPlaybackDialog::create()
{
	return new SessionPlaybackDialogImpl();
}

SessionPlaybackDialogImpl::SessionPlaybackDialogImpl() 
	: _delay(100, 1, 5000, 10, 100), _delayentry(_delay)
{
        this->_desktop = this->getDesktop();
	this->_sm = this->_desktop->whiteboard_session_manager();
	this->_sfp = this->_sm->session_player();
	this->_openfile.set_text(this->_sfp->filename());

	this->_construct();
	this->get_vbox()->show_all_children();
}

SessionPlaybackDialogImpl::~SessionPlaybackDialogImpl()
{

}

void
SessionPlaybackDialogImpl::_construct()
{
	Gtk::VBox* main = this->get_vbox();

	// Dialog organization
	this->_filemanager.set_label(_("Session file"));
	this->_playback.set_label(_("Playback controls"));
	this->_currentmsgbox.set_label(_("Message information"));

	this->_filemanager.set_border_width(4);
	this->_playback.set_border_width(4);
	this->_fm.set_border_width(4);
	this->_toolbarbox.set_border_width(4);

	// Active session file display
	// fixme: Does this mean the active file for the session, or the file for the active session?
	// Please indicate which with a TRANSLATORS comment.
	this->_labels[0].set_text(_("Active session file:"));
	this->_labels[1].set_text(_("Delay (milliseconds):"));

	this->_openfile.set_editable(false);

	this->_filebox.pack_start(this->_labels[0], true, false, 8);
	this->_filebox.pack_end(this->_openfile, true, true, 0);

	// Unload/load buttons
	this->_close.set_label(_("Close file"));
	this->_open.set_label(_("Open new file"));
	this->_setdelay.set_label(_("Set delay"));

	// Attach callbacks
	this->_close.signal_clicked().connect(sigc::bind< 0 >(sigc::mem_fun(*this, &SessionPlaybackDialogImpl::_respCallback), CLOSE_FILE));
	this->_open.signal_clicked().connect(sigc::bind< 0 >(sigc::mem_fun(*this, &SessionPlaybackDialogImpl::_respCallback), OPEN_FILE));
	this->_setdelay.signal_clicked().connect(sigc::bind< 0 >(sigc::mem_fun(*this, &SessionPlaybackDialogImpl::_respCallback), RESET_DELAY));

	// Button box
	this->_filebuttons.pack_start(this->_close, true, false, 0);
	this->_filebuttons.pack_start(this->_open, true, false, 0);

	// Message information box
	this->_currentmsgbuffer = Gtk::TextBuffer::create();
	this->_currentmsgview.set_buffer(this->_currentmsgbuffer);
	this->_currentmsgview.set_editable(false);
	this->_currentmsgview.set_cursor_visible(false);
	this->_currentmsgview.set_wrap_mode(Gtk::WRAP_WORD);
	this->_currentmsgscroller.add(this->_currentmsgview);
	this->_currentmsgbox.add(this->_currentmsgscroller);
	this->_sfp->setMessageOutputWidget(this->_currentmsgbuffer);
	
	// Delay setting
	// parameters: initial lower upper single-incr page-incr
	this->_delayentry.set_numeric(true);

	// Playback controls
	this->_playbackcontrols.set_show_arrow(false);

	this->_controls[0].set_label("Rewind");
	this->_controls[1].set_label("Go back one");
	this->_controls[2].set_label("Pause");
	this->_controls[3].set_label("Go forward one");
	this->_controls[4].set_label("Play");

	this->_controls[0].set_tooltip(this->_tooltips, _("Rewind"));
	this->_controls[1].set_tooltip(this->_tooltips, _("Go back one change"));
	this->_controls[2].set_tooltip(this->_tooltips, _("Pause"));
	this->_controls[3].set_tooltip(this->_tooltips, _("Go forward one change"));
	this->_controls[4].set_tooltip(this->_tooltips, _("Play"));

	for(int i = 0; i < 5; i++) {
		this->_playbackcontrols.append(this->_controls[i], sigc::bind< 0 >(sigc::mem_fun(*this, &SessionPlaybackDialogImpl::_respCallback), TOOLBAR_BASE + i));
	}

	this->_delaybox.pack_start(this->_labels[1]);
	this->_delaybox.pack_start(this->_delayentry);
	this->_delaybox.pack_end(this->_setdelay);

	this->_toolbarbox.pack_start(this->_delaybox);
	this->_toolbarbox.pack_end(this->_playbackcontrols);

	// Pack widgets into frames
	this->_fm.pack_start(this->_filebox);
	this->_fm.pack_end(this->_filebuttons);

	this->_filemanager.add(this->_fm);
	this->_playback.add(this->_toolbarbox);
			
	// Pack widgets into main vbox
	main->pack_start(this->_filemanager);
	main->pack_start(this->_playback);
	main->pack_end(this->_currentmsgbox);
}

void
SessionPlaybackDialogImpl::_respCallback(int resp)
{
	g_log(NULL, G_LOG_LEVEL_DEBUG, "_respCallback: %u", resp);
	switch(resp) {
		case CLOSE_FILE:
			this->_sfp->unload();
			break;
		case OPEN_FILE: {
			Gtk::FileChooserDialog sessionfiledlg(_("Open session file"), Gtk::FILE_CHOOSER_ACTION_OPEN);
			sessionfiledlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
			sessionfiledlg.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

			int result = sessionfiledlg.run();
			switch (result) {
				case Gtk::RESPONSE_OK:
					this->_sm->clearDocument();
					sp_document_done(sp_desktop_document(this->_desktop), SP_VERB_NONE, 
							 /* TODO: annotate */ "session-player.cpp:186");
					this->_sm->loadSessionFile(sessionfiledlg.get_filename());
					this->_openfile.set_text(this->_sfp->filename());
					break;
				default:
					break;
			}
			break;
						}
		case RESET_DELAY:
			this->_sfp->setDelay(this->_delayentry.get_value_as_int());
			break;
		case REWIND:
			if (this->_sfp->_playing) {
				this->_sfp->stop();
			}
			this->_sfp->_curdir = Whiteboard::SessionFilePlayer::BACKWARD;
			this->_sfp->start();
			break;
		case STEP_REWIND:
			this->_sfp->step(Whiteboard::SessionFilePlayer::BACKWARD);
			break;
		case PAUSE:
			this->_sfp->stop();
			break;
		case STEP_PLAY:
			this->_sfp->step(Whiteboard::SessionFilePlayer::FORWARD);
			break;
		case PLAY:
			if (this->_sfp->_playing) {
				this->_sfp->stop();
			}
			this->_sfp->_curdir = Whiteboard::SessionFilePlayer::FORWARD;
			this->_sfp->start();
			break;
		default:
			break;
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

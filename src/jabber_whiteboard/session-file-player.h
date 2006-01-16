/**
 * Whiteboard session file playback mechanism
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_SESSION_FILE_PLAYER_H__
#define __WHITEBOARD_SESSION_FILE_PLAYER_H__

#include <list>
#include <glibmm/refptr.h>
#include <glibmm/ustring.h>
#include <gtkmm/textbuffer.h>
#include <sigc++/sigc++.h>

struct SPDesktop;

namespace Inkscape {
    namespace UI {
       namespace Dialog {
           class SessionPlaybackDialogImpl;
       }
    }
    namespace Whiteboard {

class SessionFile;
class SessionManager;

class SessionFilePlayer {
friend class UI::Dialog::SessionPlaybackDialogImpl;

public:
	SessionFilePlayer(unsigned int bufsz, SessionManager* sm);
	~SessionFilePlayer();

	void load(SessionFile* sm);
	SessionFile* unload();

	Glib::ustring const& filename();
	Glib::ustring const& curmsg();

	void start();
	void stop();
	void setDelay(unsigned int delay);
	void setMessageOutputWidget(Glib::RefPtr<Gtk::TextBuffer> const& widgetptr);

	static unsigned short const BACKWARD = 0;
	static unsigned short const FORWARD = 1;

private:
	bool step(unsigned short dir);
	void _outputMessageToWidget();

	SessionManager* _sm;
	SessionFile* _sf;

	// Output widget refptr
	Glib::RefPtr<Gtk::TextBuffer> _outputwidget;

	// Playback signal
	sigc::connection _playback_dispatcher;

	// trackers
	unsigned int _delay;
	unsigned short _curdir;
	bool _playing;
	Glib::ustring _curmsg;

	// (position, msgsize)
	std::list< std::pair< gint64, gint64 > > _visited;
	gint64 _current;
	gint64 _next;

	// noncopyable, nonassignable
	SessionFilePlayer(SessionFilePlayer const&);
	void operator=(SessionFilePlayer const&);
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

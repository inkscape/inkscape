/**
 * Whiteboard session manager
 * Message dispatch devices and timeout triggers
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_CALLBACKS_H__
#define __WHITEBOARD_CALLBACKS_H__

#include <glibmm.h>

namespace Inkscape {

namespace Whiteboard {

class SessionManager;
class SessionData;

/**
 * Callback methods used in timers to dispatch MessageNodes from message queues.
 */
class Callbacks {
public:
	/**
	 * Constructor.
	 *
	 * \param sm The SessionManager to associate with.
	 */
	Callbacks(SessionManager* sm);
	~Callbacks();

	/**
	 * Dispatch a message from the send queue to the associated SessionManager object.
	 *
	 * The SessionManager object handles the task of actually sending out a Jabber message.
	 *
	 * \see Inkscape::Whiteboard::SessionManager::sendMessage
	 * \return Whether or not this callback should be called again by the timer routine.
	 */
	bool dispatchSendQueue();

	/**
	 * Dispatch a message from the receive queue to the associated SessionManager object.
	 *
	 * The SessionManager object handles the task of actually processing a Jabber message.
	 *
	 * \see Inkscape::Whiteboard::SessionManager::receiveChange
	 * \return Whether or not this callback should be called again by the timer routine.
	 */
	bool dispatchReceiveQueue();

private:
	SessionManager* _sm;
	SessionData* _sd;
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

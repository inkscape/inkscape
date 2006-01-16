/**
 * Undo / redo / undo log commit listener
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_UNDO_COMMIT_OBSERVER_H__
#define __WHITEBOARD_UNDO_COMMIT_OBSERVER_H__

#include <glibmm.h>
#include "../undo-stack-observer.h"
#include "jabber_whiteboard/typedefs.h"

namespace Inkscape {

namespace Whiteboard {

class SessionManager;

/**
 * Inkboard implementation of Inkscape::UndoStackObserver.
 */
class UndoStackObserver : public Inkscape::UndoStackObserver {
public:
	enum ObserverType {
		UNDO_EVENT,
		REDO_EVENT,
		UNDO_COMMIT_EVENT
	};

	UndoStackObserver(SessionManager* sm);
	~UndoStackObserver();
	void notifyUndoEvent(XML::Event* log);
	void notifyRedoEvent(XML::Event* log);
	void notifyUndoCommitEvent(XML::Event* log);

	void lockObserverFromSending(ObserverType type);
	void unlockObserverFromSending(ObserverType type);

private:
	SessionManager* _sm;

	// common action handler
	void _doAction(XML::Event* log);

	// noncopyable, nonassignable
	UndoStackObserver(UndoStackObserver const& other);
	UndoStackObserver& operator=(UndoStackObserver const& other);

	unsigned int _undoSendEventLocks;
	unsigned int _redoSendEventLocks;
	unsigned int _undoCommitSendEventLocks;
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

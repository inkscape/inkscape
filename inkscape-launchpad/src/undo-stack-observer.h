/*
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UNDO_COMMIT_OBSERVER_H
#define SEEN_UNDO_COMMIT_OBSERVER_H

#include "inkgc/gc-managed.h"

namespace Inkscape {

struct Event;

/**
 * Observes changes made to the undo and redo stacks.
 *
 * More specifically, an UndoStackObserver is a class that receives notifications when
 * any of the following events occur:
 * <ul>
 * 	<li>A change is committed to the undo stack.</li>
 * 	<li>An undo action is made.</li>
 * 	<li>A redo action is made.</li>
 * </ul>
 *
 * UndoStackObservers should not be used on their own.  Instead, they should be registered
 * with a CompositeUndoStackObserver.
 */
class UndoStackObserver : public GC::Managed<> {
public:
	UndoStackObserver() { }
	virtual ~UndoStackObserver() { }

	/**
	 * Triggered when the user issues an undo command.
	 *
	 * \param log Pointer to an Event describing the undone event.
	 */
	virtual void notifyUndoEvent(Event* log) = 0;

	/**
	 * Triggered when the user issues a redo command.
	 *
	 * \param log Pointer to an Event describing the redone event.
	 */
	virtual void notifyRedoEvent(Event* log) = 0;

	/**
	 * Triggered when a set of transactions is committed to the undo log.
	 *
	 * \param log Pointer to an Event describing the committed events.
	 */
	virtual void notifyUndoCommitEvent(Event* log) = 0;

	/**
	 * Triggered when the undo log is cleared.
	 */
	virtual void notifyClearUndoEvent() = 0;

	/**
	 * Triggered when the redo log is cleared.
	 */
	virtual void notifyClearRedoEvent() = 0;

};

}

#endif // SEEN_UNDO_COMMIT_OBSERVER_H

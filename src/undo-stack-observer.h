/**
 * Undo stack observer interface
 *
 * Observes undo, redo, and undo log commit events.
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __UNDO_COMMIT_OBSERVER_H__
#define __UNDO_COMMIT_OBSERVER_H__

namespace Inkscape {

namespace XML {

class Event;

}

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
class UndoStackObserver {
public:
	UndoStackObserver() { }
	virtual ~UndoStackObserver() { }

	/**
	 * Triggered when the user issues an undo command.
	 *
	 * \param log Pointer to an XML::Event describing the undone event.
	 */
	virtual void notifyUndoEvent(XML::Event* log) = 0;

	/**
	 * Triggered when the user issues a redo command.
	 *
	 * \param log Pointer to an XML::Event describing the redone event.
	 */
	virtual void notifyRedoEvent(XML::Event* log) = 0;

	/**
	 * Triggered when a set of transactions is committed to the undo log.
	 *
	 * \param log Pointer to an XML::Event describing the committed events.
	 */
	virtual void notifyUndoCommitEvent(XML::Event* log) = 0;
};

}

#endif

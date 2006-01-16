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

#include "document.h"
#include "document-private.h"

#include "xml/event.h"
#include "xml/event-fns.h"
#include "undo-stack-observer.h"

#include "util/list.h"
#include "util/reverse-list.h"

#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/node-tracker.h"
#include "jabber_whiteboard/serializer.h"
#include "jabber_whiteboard/message-utilities.h"
#include "jabber_whiteboard/message-aggregator.h"
#include "jabber_whiteboard/message-tags.h"


#include <glibmm.h>

namespace Inkscape {

namespace Whiteboard {

UndoStackObserver::UndoStackObserver(SessionManager* sm) : _sm(sm), _undoSendEventLocks(0), _redoSendEventLocks(0), _undoCommitSendEventLocks(0) { 
}

UndoStackObserver::~UndoStackObserver() { }

void
UndoStackObserver::notifyUndoEvent(XML::Event* log)
{
	if (this->_undoSendEventLocks == 0) {
		bool chatroom = this->_sm->session_data->status.test(IN_CHATROOM);
		Glib::ustring commit = MessageUtilities::makeTagWithContent(MESSAGE_UNDO, "");
		this->_sm->sendChange(commit, CHANGE_COMMIT, "", chatroom);
	}

	// Retrieve and process added/deleted nodes in the undo log
	// TODO: re-enable; right now it doesn't work because we can't recover the names
	// of deleted nodes (although perhaps creating a subclass of XML::Event that stored
	// names of serialized nodes along with all other Event information would fix this)
	/*
	KeyToNodeActionMap node_actions = this->_action_observer.getNodeActionMap();
	this->_sm->node_tracker()->process(node_actions);
	this->_action_observer.clearNodeBuffers();
	*/
	
}

void
UndoStackObserver::notifyRedoEvent(XML::Event* log)
{
	if (this->_redoSendEventLocks == 0) {
		bool chatroom = this->_sm->session_data->status.test(IN_CHATROOM);
		Glib::ustring commit = MessageUtilities::makeTagWithContent(MESSAGE_REDO, "");
		this->_sm->sendChange(commit, CHANGE_COMMIT, "", chatroom);
	}

	// Retrieve and process added/deleted nodes in the redo log
	/*
	KeyToNodeActionMap node_actions = this->_action_observer.getNodeActionMap();
	this->_sm->node_tracker()->process(node_actions);
	this->_action_observer.clearNodeBuffers();
	*/
}

void
UndoStackObserver::notifyUndoCommitEvent(XML::Event* log)
{
	if (this->_undoCommitSendEventLocks == 0) {
		this->_doAction(log);
	}
}

void
UndoStackObserver::lockObserverFromSending(ObserverType type)
{
	switch (type) {
		case UNDO_EVENT:
			++this->_undoSendEventLocks;
			break;
		case REDO_EVENT:
			++this->_redoSendEventLocks;
			break;
		case UNDO_COMMIT_EVENT:
			++this->_undoCommitSendEventLocks;
			break;
		default:
			break;
	}
}

void
UndoStackObserver::unlockObserverFromSending(ObserverType type)
{
	switch(type) {
		case UNDO_EVENT:
			if (this->_undoSendEventLocks) {
				--this->_undoSendEventLocks;
			}
			break;
		case REDO_EVENT:
			if (this->_redoSendEventLocks) {
				--this->_redoSendEventLocks;
			}
			break;
		case UNDO_COMMIT_EVENT:
			if (this->_undoCommitSendEventLocks) {
				--this->_undoCommitSendEventLocks;
			}
			break;
		default:
			break;
	}
}

void
UndoStackObserver::_doAction(XML::Event* log)
{
	if (this->_sm->serializer()) {
		bool chatroom = this->_sm->session_data->status.test(IN_CHATROOM);
		XML::replay_log_to_observer(log, *this->_sm->serializer());

		this->_sm->serializer()->synthesizeChildNodeAddEvents();

		SerializedEventList& events = this->_sm->serializer()->getEventList();

		SerializedEventList::iterator i = events.begin();
		MessageAggregator& agg = MessageAggregator::instance();
		Glib::ustring msgbuf;

		while(i != events.end()) {
			while(agg.addOne(*i++, msgbuf)) {
				if (i == events.end()) {
					break;
				}
			}

			if (i != events.end()) {
				i--;
			}

			this->_sm->sendChange(msgbuf, CHANGE_REPEATABLE, "", chatroom);
			msgbuf.clear();
		}

		KeyToNodeActionList& node_actions = this->_sm->serializer()->getNodeTrackerActions();
		this->_sm->node_tracker()->process(node_actions);
		this->_sm->serializer()->reset();
		Glib::ustring commit = MessageUtilities::makeTagWithContent(MESSAGE_COMMIT, "");
		this->_sm->sendChange(commit, CHANGE_COMMIT, "", chatroom);
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

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

extern "C" {
#include <loudmouth/loudmouth.h>
}

#include <glibmm/i18n.h>

#include "message-stack.h"
#include "document.h"
#include "desktop-handles.h"

#include "jabber_whiteboard/undo-stack-observer.h"
#include "jabber_whiteboard/jabber-handlers.h"
#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/message-queue.h"
#include "jabber_whiteboard/message-handler.h"
#include "jabber_whiteboard/message-node.h"
#include "jabber_whiteboard/callbacks.h"

namespace Inkscape {

namespace Whiteboard {

Callbacks::Callbacks(SessionManager* sm) : _sm(sm) 
{
	this->_sd = this->_sm->session_data;
}

Callbacks::~Callbacks() 
{
}

bool
Callbacks::dispatchSendQueue()
{
	// If we're not in a whiteboard session, don't dispatch anything
	if (!(this->_sd->status[IN_WHITEBOARD])) {
		return false;
	}

	// If the connection is not open, inform the user that an error has occurred
	// and stop the queue
	LmConnectionState state = lm_connection_get_state(this->_sd->connection);

	if (state != LM_CONNECTION_STATE_OPEN && state != LM_CONNECTION_STATE_AUTHENTICATED) {
		SP_DT_MSGSTACK(this->_sm->desktop())->flash(Inkscape::INFORMATION_MESSAGE, _("Jabber connection lost."));
		return false;
	}

	// If there's nothing to send, don't do anything
	if (this->_sd->send_queue->empty()) {
		return true;
	}

	// otherwise, send out the first change
	MessageNode* first = this->_sd->send_queue->first();

	SP_DT_MSGSTACK(this->_sm->desktop())->flashF(Inkscape::NORMAL_MESSAGE,
                                                     ngettext("Sending message; %u message remaining in send queue.",
                                                              "Sending message; %u messages remaining in send queue.",
                                                              this->_sd->send_queue->size()),
                                                     this->_sd->send_queue->size());

	if (this->_sd->send_queue->empty()) {
		SP_DT_MSGSTACK(this->_sm->desktop())->flash(Inkscape::NORMAL_MESSAGE, _("Receive queue empty."));
	}

	switch (first->type()) {
		case CHANGE_REPEATABLE:
		case CHANGE_NOT_REPEATABLE:
		case CHANGE_COMMIT:
		case DOCUMENT_BEGIN:
		case DOCUMENT_END:
			this->_sm->sendMessage(first->type(), first->sequence(), first->message(), first->recipient().c_str(), first->chatroom());
			break;
		default:
			g_warning("MessageNode with unknown change type found in send queue; discarding message.  This may lead to desynchronization!");
			break;
	}

	this->_sd->send_queue->popFront();

	return true;
}

bool
Callbacks::dispatchReceiveQueue()
{
	CommitsQueue& rcq = this->_sd->recipients_committed_queue;
	// See if we have any commits submitted. 
	if (!rcq.empty()) {
		// Pick the first one off the queue.
		ReceivedCommitEvent& committer = rcq.front();

		// Find the commit event sender's receive queue.
		ReceiveMessageQueue* rmq = this->_sd->receive_queues[committer];

		if (rmq != NULL) {
			if (!rmq->empty()) {
				// Get the first message off the sender's receive queue.
				MessageNode* msg = rmq->first();

				// There are a few message change types that demand special processing;
				// handle them here.
				//
				// TODO: clean this up.  This should be a simple dispatching routine,
				// and should not be performing operations like it's doing right now.
				// These really should go into connection-establishment.cpp
				// (although that should only happen after SessionManager itself
				// is cleaned up).
				switch(msg->type()) {
					case CHANGE_COMMIT:
						rcq.pop_front();
						break;
					case DOCUMENT_BEGIN:
						if (this->_sm->session_data->status[WAITING_TO_SYNC_TO_CHAT]) {
							this->_sm->session_data->status.set(WAITING_TO_SYNC_TO_CHAT, 0);
							this->_sm->session_data->status.set(SYNCHRONIZING_WITH_CHAT, 1);
						}
						break;
					case DOCUMENT_END:
						rcq.pop_front();
						if (this->_sm->session_data->status[SYNCHRONIZING_WITH_CHAT]) {
							this->_sm->sendMessage(CONNECTED_SIGNAL, 0, "", this->_sm->session_data->recipient, true);
							this->_sm->session_data->status.set(SYNCHRONIZING_WITH_CHAT, 0);
							this->_sm->session_data->status.set(IN_CHATROOM, 1);
						} else {
							this->_sm->sendMessage(CONNECTED_SIGNAL, 0, "", msg->sender().c_str(), false);
						}
						break;
					case CHANGE_REPEATABLE:
					case CHANGE_NOT_REPEATABLE:
					default:
						break;
				}
				

				// Pass the message to the received change handler.
				this->_sm->receiveChange(msg->message());
				SP_DT_MSGSTACK(this->_sm->desktop())->flashF(Inkscape::NORMAL_MESSAGE,
                                                                             ngettext("Receiving change; %u change left to process.",
                                                                                      "Receiving change; %u changes left to process.",
                                                                                      rmq->size()),
                                                                             rmq->size());

				
				// Register this message as the latest message received from this
				// sender.
				rmq->setLatestProcessedPacket(msg->sequence());

				// Pop this message off the receive queue.
				rmq->popFront();
				return true;
			} else {
				// This really shouldn't happen.
				// If we have a commit request from a valid sender, there should
				// be something in the receive queue to process.  However, 
				// if a client is buggy or has managed to trick us into accepting
				// a commit, we should handle the event gracefully.
				g_warning("Processing commit, but no changes to commit were found; ignoring commit event.");

				// Remove this sender from the commit list.  If they want to commit
				// later, they can.
				rcq.pop_front();
				return true;
			}
		} else {
			// If the receive queue returned is NULL, then we don't know about
			// this sender.  Remove the sender from the commit list.
			g_warning("Attempting to process commit from unknown sender; ignoring.");
			rcq.pop_front();
			return true;
		}
	} else {
		return true;
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

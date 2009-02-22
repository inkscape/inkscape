/**
 * Whiteboard message queue 
 * 
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_MESSAGE_QUEUE_H__
#define __WHITEBOARD_MESSAGE_QUEUE_H__

#include <list>
#include <map>

#include "gc-alloc.h"
#include "gc-managed.h"

#include "util/list-container.h"

namespace Inkscape {

namespace Whiteboard {

class MessageNode;

/// Definition of the basic message node queue
typedef std::list< MessageNode*, GC::Alloc< MessageNode*, GC::MANUAL > > MessageQueueBuffer;

/**
 * MessageQueue interface.
 *
 * A message queue is used to queue up document change messages for sending and receiving.
 *
 * Message queues exist to allow us to send/process messages at a given rate rather than
 * immediately: this allows us to avoid flooding Jabber servers and clients.
 *
 * Only one message queue should be created per sender.  Message queues store MessageNodes.
 *
 * \see Inkscape::Whiteboard::MessageNode
 */
class MessageQueue {
public:
	/**
	 * Constructor.
	 *
	 * \param sm The SessionManager to associate this MessageQueue with. 
	 */
	MessageQueue() { }
	virtual ~MessageQueue()
	{
		this->_queue.clear();
	}

	/**
	 * Retrieve the MessageNode at the front of the queue.
	 */
	virtual MessageNode* first();

	/**
	 * Remove the element at the front of the queue. 
	 */
	virtual void popFront();

	/**
	 * Get the size of the queue.
	 *
	 * \return The size of the queue.
	 */
	virtual unsigned int size();

	/**
	 * Returns whether or not the queue is empty.
	 *
	 * \return Whether or not the queue is empty.
	 */
	virtual bool empty();

	/**
	 * Clear the queue.
	 */
	virtual void clear();

	/**
	 * The insertion method.  The insertion procedure must be defined
	 * by a subclass.
	 *
	 * \param msg The MessageNode to insert.
	 */
	virtual void insert(MessageNode* msg) = 0;

protected:
	/**
	 * Implementation of the queue.
	 */
	MessageQueueBuffer _queue;
};


/**
 * MessageQueue subclass designed to queue up received messages.
 * Received messages are dispatched for processing on a periodic basis by a timeout.
 *
 * \see Inkscape::Whiteboard::Callbacks::dispatchReceiveQueue
 */
class ReceiveMessageQueue : public MessageQueue, public GC::Managed<> {
public:
	ReceiveMessageQueue() : _latest(0) { }

	/**
	 * Insert a message into the queue.  
	 * Late messages (out-of-sequence messages) will be discarded.
	 *
	 * \param msg The message node to insert.
	 */
	void insert(MessageNode* msg);

	/**
	 * Insert a message into the deferred queue.
	 * The deferred message queue is used for messages that are not discarded,
	 * but cannot yet be processed due to missing dependencies.
	 *
	 * \param msg The message node to insert.
	 */
	void insertDeferred(MessageNode* msg);

	/**
	 * Update the latest processed packet count for this message queue.
	 *
	 * \param seq The sequence number of the latest processed packet.
	 */
	void setLatestProcessedPacket(unsigned int seq);
private:
	MessageQueueBuffer _deferred;
	unsigned int _latest;
};

/**
 * MessageQueue subclass designed to queue up messages for sending.
 * Messages in this queue are dispatched on a periodic basis by a timeout.
 *
 * \see Inkscape::Whiteboard::Callbacks::dispatchSendQueue
 */
class SendMessageQueue : public MessageQueue {
public:
	SendMessageQueue() { }

	/**
	 * Insert a message into the queue.  
	 *
	 * \param msg The message node to insert.
	 */
	void insert(MessageNode* msg);
};

}

}

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

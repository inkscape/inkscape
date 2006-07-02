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

#include <glibmm/i18n.h>

#include "desktop-handles.h"
#include "message-stack.h"

#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/message-node.h"
#include "jabber_whiteboard/message-queue.h"

namespace Inkscape {

namespace Whiteboard {

//###################################
//# MESSAGE QUEUE
//###################################

MessageNode*
MessageQueue::first()
{
    return _queue.front();
}

void
MessageQueue::popFront()
{
     _queue.pop_front();
    //g_log(NULL, G_LOG_LEVEL_DEBUG,
    // "Removed element, queue size (for %s): %u", 
    //lm_connection_get_jid(this->_sm->session_data->connection), this->_queue.size());
}

unsigned int
MessageQueue::size()
{
    return _queue.size();
}

bool
MessageQueue::empty()
{
    return _queue.empty();
}

void
MessageQueue::clear()
{
    _queue.clear();
}



//###################################
//# RECEIVE MESSAGE QUEUE
//###################################
void
ReceiveMessageQueue::insert(MessageNode* msg)
{
    // Check to see if the incoming message has a sequence number
    // lower than the sequence number of the latest message processed
    // by this message's sender.  If it does, drop the message and produce
    // a warning.
    if (msg->sequence() < _latest) {
        g_warning("Received late message (message sequence number is %u, but latest processed message had sequence number %u).  Discarding message; session may be desynchronized.", msg->sequence(), this->_latest);
        return;
    }

    // Otherwise, it is safe to insert this message.
    //Inkscape::GC::anchor(msg);
    _queue.push_back(msg);
	/*
    SP_DT_MSGSTACK(_sm->getDesktop())->flashF(
              Inkscape::NORMAL_MESSAGE, 
              _("%u changes queued in receive queue."),
              _queue.size());
	*/
    //g_log(NULL, G_LOG_LEVEL_DEBUG, "Receive queue size (for %s): %u",
    // lm_connection_get_jid(this->_sm->session_data->connection), this->_queue.size());
}

void
ReceiveMessageQueue::insertDeferred(MessageNode* msg)
{
    _deferred.push_back(msg);
}

void
ReceiveMessageQueue::setLatestProcessedPacket(unsigned int seq)
{
     _latest = seq;
}


//###################################
//# SEND MESSAGE QUEUE
//###################################
void
SendMessageQueue::insert(MessageNode* msg)
{
    //Inkscape::GC::anchor(msg);
    _queue.push_back(msg);
	/*
    SP_DT_MSGSTACK(_sm->getDesktop())->flashF(
            Inkscape::NORMAL_MESSAGE, 
            _("%u changes queued in send queue."),
            _queue.size());
	*/
    //g_log(NULL, G_LOG_LEVEL_DEBUG, "Send queue size (for %s): %u",
    //  lm_connection_get_jid(this->_sm->session_data->connection),
    //this->_queue.size());
}

}  // namespace Whiteboard

}  // namespace Inkscape


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

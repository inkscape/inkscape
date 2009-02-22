/**
 * Whiteboard message queue and queue handler functions
 * Node for storing messages in message queues
 * 
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_MESSAGE_NODE_H__
#define __WHITEBOARD_MESSAGE_NODE_H__

#include <string>
#include <glibmm.h>

#include "gc-managed.h"
#include "gc-anchored.h"
#include "gc-finalized.h"
#include "message.h"

namespace Inkscape {

namespace Whiteboard {

/**
 * Encapsulates a document change message received by or sent to an Inkboard client.
 *
 * Received messages that end up in a MessageNode are of the following types:
 * <ol>
 * 	<li>CHANGE_REPEATABLE</li>
 * 	<li>CHANGE_NOT_REPEATABLE</li>
 * 	<li>CHANGE_COMMIT</li>
 * 	<li>DOCUMENT_BEGIN</li>
 * 	<li>DOCUMENT_END</li>
 * 	<li>DUMMY_CHANGE</li>
 * </ol>
 *
 * This class is intended for use in MessageQueues, although it could potentially
 * see use outside of that context.
 *
 * \see Inkscape::Whiteboard::MessageQueue
 */
class MessageNode : public GC::Managed<>, public GC::Anchored, public GC::Finalized {
public:
	/**
	 * Constructor.
	 *
	 * \param seq The sequence number of the message being encapsulated.
	 * \param sender The sender of the message.
	 * \param recip The intended recipient. 
	 * \param message_body The body of the message.
	 * \param type The type of the message.
	 * \param chatroom Whether or not this message is to be sent to / was received from a chatroom.
	 */
	MessageNode(unsigned int seq, std::string sender, std::string recip, Glib::ustring const& message_body, MessageType type, bool document, bool chatroom) :
		_seq(seq), _type(type), _message(message_body), _document(document), _chatroom(chatroom)
	{
		this->_sender = sender;
		this->_recipient = recip;
	}

    virtual ~MessageNode() 
	{
//		g_log(NULL, G_LOG_LEVEL_DEBUG, "MessageNode destructor");
		/*
		if (this->_message) {
			delete this->_message;
		}
		*/
	}

	unsigned int sequence()
	{
		return this->_seq;
	}

	MessageType type()
	{
		return this->_type;
	}

	bool chatroom()
	{
		return this->_chatroom;
	}

	bool document()
	{
		return this->_document;
	}

	std::string recipient()
	{
		return this->_recipient;
	}

	std::string sender()
	{
		return this->_sender;
	}

	Glib::ustring const& message()
	{
		return this->_message;
	}

private:
	unsigned int _seq;
	std::string _sender;
	std::string _recipient;
	MessageType _type;
	Glib::ustring _message;
	bool _document;
	bool _chatroom;
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

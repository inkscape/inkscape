/**
 * Whiteboard session manager
 * Jabber received message processors
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_MESSAGE_PROCESSORS_H__
#define __WHITEBOARD_MESSAGE_PROCESSORS_H__

#include "jabber_whiteboard/typedefs.h"

#include "gc-managed.h"
#include "gc-finalized.h"

namespace Inkscape {

namespace Whiteboard {

class SessionManager;

// Processor forward declarations
struct ChangeHandler;
struct DocumentSignalHandler;
struct ConnectRequestHandler;
struct ConnectErrorHandler;
struct ChatSynchronizeHandler;

/**
 * Encapsulates a pointer to an LmMessage along with additional information,
 * such as the message's sequence number, its sender, and its body.
 *
 * All of the above data members can be extracted directly from the LmMessage;
 * they are provided for convenience.
 */
struct JabberMessage {
public:
	/**
	 * Constructor.
	 *
	 * The constructor attaches a reference to the LmMessage to prevent Loudmouth from
	 * freeing the message.
	 */
	JabberMessage(LmMessage* m) : message(m), sequence(0)
	{	
		lm_message_ref(this->message);
	}

	/**
	 * Destructor.
	 *
	 * The destructor deletes a reference to the LmMessage, which, assuming all other
	 * references have been deleted, will allow Loudmouth to free the LmMessage object.
	 */
	~JabberMessage() 
	{
		lm_message_unref(this->message);
	}


	 // TODO: Hide, or, better, remove this.  There's no real reason why it should be here,
	 // and it allows for the possibility of reference count-induced memory leaks.
	/**
	 * Pointer to original Loudmouth message object.
	 */
	LmMessage* message;
	
	/**
	 * Sequence number of this message.
	 */
	unsigned int sequence;
	
	/**
	 * The JID of this message's sender.
	 */
	std::string sender;

	/**
	 * The body of this message.
	 */
	Glib::ustring body;

private:
	// noncopyable, nonassignable (for now, anyway...)
//	JabberMessage(JabberMessage const&);
//	JabberMessage& operator=(JabberMessage const&);
};

/**
 * A MessageProcessor is a functor that is associated with one or more Inkboard message types.
 * When an Inkboard client receives an Inkboard message, it passes it to the appropriate
 * MessageProcessor.
 */
struct MessageProcessor : public GC::Managed<>, public GC::Finalized {
public:
	virtual ~MessageProcessor() 
	{

	}

	/**
	 * Functor action operator.
	 *
	 * \param mode The type of the message being processed.
	 * \param m A reference to the JabberMessage encapsulating the received Jabber message.
	 */
	virtual LmHandlerResult operator()(MessageType mode, JabberMessage& m) = 0;

	/**
	 * Constructor.
	 *
	 * \param sm The SessionManager with which a MessageProcessor instance is associated.
	 */
	MessageProcessor(SessionManager* sm) : _sm(sm) { }
protected:
	/**
	 * Pointer to the associated SessionManager object.
	 */
	SessionManager *_sm;

private:
	// noncopyable, nonassignable
	MessageProcessor(MessageProcessor const&);
	MessageProcessor& operator=(MessageProcessor const&);
};

/*
struct ProcessorShell : public GC::Managed<>, public std::binary_function< MessageType, JabberMessage, LmHandlerResult > {
public:
	ProcessorShell(MessageProcessor* mpm) : _mpm(mpm) { }

	LmHandlerResult operator()(MessageType type, JabberMessage msg)
	{
		return (*this->_mpm)(type, msg);
	}
private:
	MessageProcessor* _mpm;
};
*/

/**
 * Initialize the message -> MessageProcessor map.
 *
 * \param sm The SessionManager with which all created MessageProcessors should be associated with.
 * \param mpm Reference to the MessageProcessorMap to initialize.
 */
void initialize_received_message_processors(SessionManager* sm, MessageProcessorMap& mpm);

/**
 * Clean up the message -> MessageProcessor map.
 *
 * \param mpm Reference to the MessageProcessorMap to clean up.
 */
void destroy_received_message_processors(MessageProcessorMap& mpm);

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

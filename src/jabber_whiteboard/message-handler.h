/**
 * Whiteboard session manager
 * Jabber message handling
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 * Steven Montgomery, Jonas Collaros (original C version)
 *
 * Copyright (c) 2004-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_MESSAGE_HANDLER_H__
#define __WHITEBOARD_MESSAGE_HANDLER_H__

extern "C" {
#include <loudmouth/loudmouth.h>
}

#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/message-contexts.h"

namespace Inkscape {

namespace Whiteboard {

struct JabberMessage;
class SessionManager;

/**
 * Handles received Jabber messages.
 */
class MessageHandler {
public:
	MessageHandler(SessionManager* sm);
	~MessageHandler();
	LmHandlerResult handle(LmMessage* message, HandlerMode mode);	
	
	bool _hasValidReceiveContext(LmMessage* message);

	static char const* ink_type_to_string(gint ink_type);

private:
	static void _initializeContexts();

	void _initializeProcessors();
	void _destructProcessors();

	// Utilities
	bool _isValidMessage(LmMessage* message);
	MessageType _getType(LmMessage* message);
	struct JabberMessage _extractData(LmMessage* message);


	// Individual message handlers
	LmHandlerResult _default(LmMessage* message);
	LmHandlerResult _error(LmMessage* message);
	LmHandlerResult _presence(LmMessage* message);
	
	// Message processors map
//	MessageContextMap _received_message_contexts;
	MessageProcessorMap _received_message_processors;

	SessionManager* _sm;

	// noncopyable, nonassignable
	MessageHandler(MessageHandler const&);
	MessageHandler& operator=(MessageHandler const&);
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

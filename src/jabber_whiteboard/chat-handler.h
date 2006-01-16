/**
 * Whiteboard session manager
 * Chatroom message handler
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_CHAT_HANDLER_H__
#define __WHITEBOARD_CHAT_HANDLER_H__

extern "C" {
#include <loudmouth/loudmouth.h>
}

namespace Inkscape {

namespace Whiteboard {

class SessionManager;


// TODO: find some way to better integrate this with the rest of the message
// handling framework (i.e. message-processors.cpp, message-handler.cpp,
// message-contexts.cpp)
class ChatMessageHandler {
public:
	ChatMessageHandler(SessionManager* sm);
	~ChatMessageHandler();

	LmHandlerResult parse(LmMessage* message);

private:
	LmHandlerResult _finishConnection();
	void _handleError(char const* errcode);

	SessionManager* _sm;	

	// noncopyable, nonassignable
	ChatMessageHandler(ChatMessageHandler&);
	void operator=(ChatMessageHandler&);
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

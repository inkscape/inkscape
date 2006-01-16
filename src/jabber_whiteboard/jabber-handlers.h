/**
 * Whiteboard session manager
 * C-style Loudmouth callbacks
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_LOUDMOUTH_CALLBACKS__
#define __WHITEBOARD_LOUDMOUTH_CALLBACKS__

extern "C" {
#include <loudmouth/loudmouth.h>
}

#include <glib.h>

namespace Inkscape {

namespace Whiteboard {

/**
 * C-style callback for Loudmouth to handle received presence messages.
 *
 * \param handler The LmMessageHandler handling this event.
 * \param connection The LmConnection with which the LmMessageHandler is associated.
 * \param message The Jabber message received that triggered this handler.
 * \param user_data A pointer to an instance of Inkscape::Whiteboard::MessageHandler, which performs
 * the real message processing work.
 */
LmHandlerResult presence_handler(LmMessageHandler* handler, LmConnection* connection, LmMessage* message, gpointer user_data);

/**
 * C-style callback for Loudmouth to handle received messages.
 *
 * This handler handles messages that are not presence or error messages.
 *
 * \param handler The LmMessageHandler handling this event.
 * \param connection The LmConnection with which the LmMessageHandler is associated.
 * \param message The Jabber message received that triggered this handler.
 * \param user_data A pointer to an instance of Inkscape::Whiteboard::MessageHandler, which performs
 * the real message processing work.
 */
LmHandlerResult default_handler(LmMessageHandler* handler, LmConnection* connection, LmMessage* message, gpointer user_data);

/**
 * C-style callback for Loudmouth to handle received error messages.
 *
 * \param handler The LmMessageHandler handling this event.
 * \param connection The LmConnection with which the LmMessageHandler is associated.
 * \param message The Jabber message received that triggered this handler.
 * \param user_data A pointer to an instance of Inkscape::Whiteboard::MessageHandler, which performs
 * the real message processing work.
 */
LmHandlerResult stream_error_handler(LmMessageHandler* handler, LmConnection* connection, LmMessage* message, gpointer user_data);

/**
 * C-style callback for Loudmouth to handle SSL errors.
 * 
 * \param ssl The SSL data structure used by Loudmouth.
 * \param status The error code representing the error that occurred.
 * \param user_data A pointer to the SessionManager instance handling associated with the connection attempt that 
 * threw the SSL error.
 */
LmSSLResponse ssl_error_handler(LmSSL* ssl, LmSSLStatus status, gpointer user_data);

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

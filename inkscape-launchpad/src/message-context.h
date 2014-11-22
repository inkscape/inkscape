/** \file
 * Interface for locally managing a current status message
 */

/*
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_MESSAGE_CONTEXT_H
#define SEEN_INKSCAPE_MESSAGE_CONTEXT_H

#include <cstdarg>
#include <glib.h>

#include "message.h"

namespace Inkscape {

class MessageStack;

/** A convenience class for working with MessageStacks.
  *
  * In general, a particular piece of code will only want to display
  * one status message at a time.  This class takes care of tracking
  * a "current" message id in a particular stack for us, and provides
  * a convenient means to remove or replace it.
  *
  * @see Inkscape::MessageStack
  */
class MessageContext {
public:
    /** Constructs an Inkscape::MessageContext referencing a particular
      * Inkscape::MessageStack, which will be used for our messages
      *
      * MessageContexts retain references to the MessageStacks they use.
      *
      * @param stack the Inkscape::MessageStack to use for our messages
      */
    MessageContext(MessageStack *stack);
    ~MessageContext();

    /** @brief pushes a message on the stack, replacing our old message
      *
      * @param type the message type
      * @param message the message text
      */
    void set(MessageType type, char const *message);

    /** @brief pushes a message on the stack using prinf-style formatting,
      *        and replacing our old message
      *
      * @param type the message type
      * @param format a printf-style formatting string
      */
    void setF(MessageType type, char const *format, ...) G_GNUC_PRINTF(3,4);

    /** @brief pushes a message on the stack using printf-style formatting,
      *        and a stdarg argument list
      *
      * @param type the message type
      * @param format a printf-style formatting string
      * @param args printf-style arguments
      */
    void setVF(MessageType type, char const *format, va_list args);

    /** @brief pushes a message onto the stack for a brief period of time
      *        without disturbing our "current" message
      *
      * @param type the message type
      * @param message the message text
      */
    void flash(MessageType type, char const *message);

    /** @brief pushes a message onto the stack for a brief period of time
      *        using printf-style formatting, without disturbing our current
      *        message
      *
      * @param type the message type
      * @param format a printf-style formatting string
      */
    void flashF(MessageType type, char const *format, ...) G_GNUC_PRINTF(3,4);

    /** @brief pushes a message onto the stack for a brief period of time
      *        using printf-style formatting and a stdarg argument list;
      *        it does not disturb our "current" message
      *
      * @param type the message type
      * @param format a printf-style formatting string
      * @param args printf-style arguments
      */
    void flashVF(MessageType type, char const *format, va_list args);

    /** @brief removes our current message from the stack */
    void clear();

private:
    MessageStack *_stack; ///< the message stack to use
    MessageId _message_id; ///< our current message id, or 0
    MessageId _flash_message_id; ///< current flashed message id, or 0
};

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

/** \file
 * Raw stack of active status messages
 */

/*
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2004 MenTaLguY
 * Copyright (C) 2011 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_MESSAGE_STACK_H
#define SEEN_INKSCAPE_MESSAGE_STACK_H

#include <cstdarg>
#include <cstddef>
#include <glib.h> // G_GNUC_PRINTF is the only thing worth having from here
#include <glibmm/ustring.h>
#include <sigc++/sigc++.h>

#include "inkgc/gc-managed.h"
#include "gc-finalized.h"
#include "gc-anchored.h"
#include "message.h"

namespace Inkscape {

/**
 * A class which holds a stack of displayed messages.  
 *
 * Messages can be pushed onto the top of the stack, and removed 
 * from any point in the stack by their id.
 *
 * Messages may also be "flashed", meaning that they will be
 * automatically removed from the stack a fixed period of time
 * after they are pushed.
 *
 * "Flashed" warnings and errors will persist longer than normal
 * messages.
 *
 * There is no simple "pop" operation provided, since these
 * stacks are intended to be shared by many different clients;
 * assuming that the message you pushed is still on top is an
 * invalid and unsafe assumption.
 */
class MessageStack : public GC::Managed<>,
                     public GC::Finalized,
                     public GC::Anchored
{
public:
    MessageStack();
    ~MessageStack();

    /** @brief returns the type of message currently at the top of the stack */
    MessageType currentMessageType() {
        return _messages ? _messages->type : NORMAL_MESSAGE;
    }
    /** @brief returns the text of the message currently at the top of
      *        the stack
      */
    char const *currentMessage() {
        return _messages ? _messages->message : NULL;
    }

    /** @brief connects to the "changed" signal which is emitted whenever
      *        the topmost message on the stack changes.
      */
    sigc::connection connectChanged(sigc::slot<void, MessageType, char const *> slot)
    {
        return _changed_signal.connect(slot);
    }

    /** @brief pushes a message onto the stack
      *
      * @param type the message type
      * @param message the message text
      *
      * @return the id of the pushed message
      */
    MessageId push(MessageType type, char const *message);

    /** @brief pushes a message onto the stack using printf-like formatting
      *
      * @param type the message type
      * @param format a printf-style format string
      *
      * @return the id of the pushed message
      */
    MessageId pushF(MessageType type, char const *format, ...) G_GNUC_PRINTF(3,4);

    /** @brief pushes a message onto the stack using printf-like formatting,
      *        using a stdarg argument list
      *
      * @param type the message type
      * @param format a printf-style format string
      * @param args the subsequent printf-style arguments
      *
      * @return the id of the pushed message
      */
    MessageId pushVF(MessageType type, char const *format, va_list args);

    /** @brief removes a message from the stack, given its id
      *
      * This method will remove a message from the stack if it has not
      * already been removed.  It may be removed from any part of the stack.
      * 
      * @param id the message id to remove
      */
    void cancel(MessageId id);

    /**
     * Temporarily pushes a message onto the stack.
     *
     * @param type the message type
     * @param message the message text
     *
     * @return the id of the pushed message
     */
    MessageId flash(MessageType type, char const *message);

    /**
     * Temporarily pushes a message onto the stack.
     *
     * @param type the message type
     * @param message the message text
     *
     * @return the id of the pushed message
     */
    MessageId flash(MessageType type, Glib::ustring const &message);


    /** @brief temporarily pushes a message onto the stack using
      *        printf-like formatting
      *
      * @param type the message type
      * @param format a printf-style format string
      *
      * @return the id of the pushed message
      */
    MessageId flashF(MessageType type, char const *format, ...) G_GNUC_PRINTF(3,4);

    /** @brief temporarily pushes a message onto the stack using
      *        printf-like formatting, using a stdarg argument list
      *
      * @param type the message type
      * @param format a printf-style format string
      * @param args the printf-style arguments
      *
      * @return the id of the pushed message
      */
    MessageId flashVF(MessageType type, char const *format, va_list args);

private:
    struct Message {
        Message *next;
        MessageStack *stack;
        MessageId id;
        MessageType type;
        gchar *message;
        guint timeout_id;
    };

    MessageStack(MessageStack const &); // no copy
    void operator=(MessageStack const &); // no assign

    /// pushes a message onto the stack with an optional timeout
    MessageId _push(MessageType type, unsigned int lifetime, char const *message);

    Message *_discard(Message *m); ///< frees a message struct and returns the next such struct in the list
    void _emitChanged(); ///< emits the "changed" signal
    static int _timeout(void* data); ///< callback to expire flashed messages

    sigc::signal<void, MessageType, char const *> _changed_signal;
    Message *_messages; ///< the stack of messages as a linked list
    MessageId _next_id; ///< the next message id to assign
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

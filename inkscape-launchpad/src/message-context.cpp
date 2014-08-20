/*
 * MessageContext - context for posting status messages
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include "message-context.h"
#include "message-stack.h"

namespace Inkscape {

MessageContext::MessageContext(MessageStack *stack)
: _stack(stack), _message_id(0), _flash_message_id(0)
{
    GC::anchor(_stack);
}

MessageContext::~MessageContext() {
    clear();
    GC::release(_stack);
    _stack = NULL;
}

void MessageContext::set(MessageType type, gchar const *message) {
    if (_message_id) {
        _stack->cancel(_message_id);
    }
    _message_id = _stack->push(type, message);
}

void MessageContext::setF(MessageType type, gchar const *format, ...)
{
    va_list args;
    va_start(args, format);
    setVF(type, format, args);
    va_end(args);
}

void MessageContext::setVF(MessageType type, gchar const *format, va_list args)
{
    gchar *message=g_strdup_vprintf(format, args);
    set(type, message);
    g_free(message);
}

void MessageContext::flash(MessageType type, gchar const *message) {
    if (_flash_message_id) {
        _stack->cancel(_flash_message_id);
    }
    _flash_message_id = _stack->flash(type, message);
}

void MessageContext::flashF(MessageType type, gchar const *format, ...) {
    va_list args;
    va_start(args, format);
    flashVF(type, format, args);
    va_end(args);
}

void MessageContext::flashVF(MessageType type, gchar const *format, va_list args) {
    gchar *message=g_strdup_vprintf(format, args);
    flash(type, message);
    g_free(message);
}

void MessageContext::clear() {
    if (_message_id) {
        _stack->cancel(_message_id);
        _message_id = 0;
    }
    if (_flash_message_id) {
        _stack->cancel(_flash_message_id);
        _flash_message_id = 0;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

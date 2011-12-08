/*
 * MessageStack - manages stack of active status messages
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>
#include <glib.h>
#include <cstring>
#include <string>
#include "message-stack.h"

namespace Inkscape {

MessageStack::MessageStack()
: _messages(NULL), _next_id(1)
{
}

MessageStack::~MessageStack()
{
    while (_messages) {
        _messages = _discard(_messages);
    }
}

MessageId MessageStack::push(MessageType type, gchar const *message) {
    return _push(type, 0, message);
}

MessageId MessageStack::pushF(MessageType type, gchar const *format, ...)
{
    va_list args;
    va_start(args, format);
    MessageId id=pushVF(type, format, args);
    va_end(args);
    return id;
}

MessageId MessageStack::pushVF(MessageType type, gchar const *format, va_list args)
{
    MessageId id;
    gchar *message=g_strdup_vprintf(format, args);
    id = push(type, message);
    g_free(message);
    return id;
}

void MessageStack::cancel(MessageId id) {
    Message **ref;
    for ( ref = &_messages ; *ref ; ref = &(*ref)->next ) {
        if ( (*ref)->id == id ) {
            *ref = _discard(*ref);
            _emitChanged();
            break;
        }
    }
}

MessageId MessageStack::flash(MessageType type, Glib::ustring const &message)
{
    MessageId id = flash( type, message.c_str() );
    return id;
}

MessageId MessageStack::flash(MessageType type, gchar const *message) {
    switch (type) {
    case INFORMATION_MESSAGE: // stay rather long so as to seem permanent, but eventually disappear
        return _push(type, 6000 + 80*strlen(message), message);
        break;
    case ERROR_MESSAGE: // pretty important stuff, but temporary
        return _push(type, 4000 + 60*strlen(message), message);
        break;
    case WARNING_MESSAGE: // a bit less important than error
        return _push(type, 2000 + 40*strlen(message), message);
        break;
    case IMMEDIATE_MESSAGE: // same length as normal, higher priority
        return _push(type, 1000 + 20*strlen(message), message);
        break;
    case NORMAL_MESSAGE: // something ephemeral
    default:
        return _push(type, 1000 + 20*strlen(message), message);
        break;
    }
}

MessageId MessageStack::flashF(MessageType type, gchar const *format, ...) {
    va_list args;
    va_start(args, format);
    MessageId id = flashVF(type, format, args);
    va_end(args);
    return id;
}

MessageId MessageStack::flashVF(MessageType type, gchar const *format, va_list args)
{
    gchar *message=g_strdup_vprintf(format, args);
    MessageId id = flash(type, message);
    g_free(message);
    return id;
}

MessageId MessageStack::_push(MessageType type, guint lifetime, gchar const *message)
{
    Message *m=new Message;
    MessageId id=_next_id++;

    m->stack = this;
    m->id = id;
    m->type = type;
    m->message = g_strdup(message);

    if (lifetime) {
        m->timeout_id = g_timeout_add(lifetime, &MessageStack::_timeout, m);
    } else {
        m->timeout_id = 0;
    }

    m->next = _messages;
    _messages = m;

    _emitChanged();

    return id;
}

MessageStack::Message *MessageStack::_discard(MessageStack::Message *m)
{
    Message *next=m->next;
    if (m->timeout_id) {
        g_source_remove(m->timeout_id);
        m->timeout_id = 0;
    }
    g_free(m->message);
    m->message = NULL;
    m->stack = NULL;
    delete m;
    return next;
}

void MessageStack::_emitChanged() {
    if (_messages) {
        _changed_signal.emit(_messages->type, _messages->message);
    } else {
        _changed_signal.emit(NORMAL_MESSAGE, NULL);
    }
}

gboolean MessageStack::_timeout(gpointer data) {
    Message *m=reinterpret_cast<Message *>(data);
    m->timeout_id = 0;
    m->stack->cancel(m->id);
    return FALSE;
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

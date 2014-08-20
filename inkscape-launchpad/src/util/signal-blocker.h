/*
 * Base RAII blocker for sgic++ signals.
 *
 * Authors:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2014 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_SIGNAL_BLOCKER_H
#define SEEN_INKSCAPE_UTIL_SIGNAL_BLOCKER_H

#include <string>
#include <sigc++/connection.h>

/**
 * Base RAII blocker for sgic++ signals.
 */
class SignalBlocker
{
public:
    /**
     * Creates a new instance that if the signal is currently unblocked will block
     * it until this instance is destructed and then will unblock it.
     */
    SignalBlocker( sigc::connection *connection ) :
        _connection(connection),
        _wasBlocked(_connection->blocked())
    {
        if (!_wasBlocked)
        {
            _connection->block();
        }
    }

    /**
     * Destructor that will unblock the signal if it was blocked initially by this
     * instance.
     */
    ~SignalBlocker()
    {
        if (!_wasBlocked)
        {
            _connection->block(false);
        }
    }

private:
    // noncopyable, nonassignable
    SignalBlocker(SignalBlocker const &other);
    SignalBlocker& operator=(SignalBlocker const &other);
    
    sigc::connection *_connection;
    bool _wasBlocked;
};

#endif // SEEN_INKSCAPE_UTIL_SIGNAL_BLOCKER_H

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

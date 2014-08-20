/*
 * Inkscape::Debug::EventTracker - semi-automatically track event lifetimes
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2005 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_DEBUG_EVENT_TRACKER_H
#define SEEN_INKSCAPE_DEBUG_EVENT_TRACKER_H

#include "debug/logger.h"

namespace Inkscape {

namespace Debug {

struct NoInitialEvent {};

template <typename Event=NoInitialEvent> class EventTracker;

class EventTrackerBase {
public:
    virtual ~EventTrackerBase() {
        if (_active) {
            Logger::finish();
        }
    }

    template <typename EventType>
    inline void set() {
        if (_active) {
            Logger::finish();
        }
        Logger::start<EventType>();
        _active = true;
    }

    template <typename EventType, typename A>
    inline void set(A const &a) {
        if (_active) {
            Logger::finish();
        }
        Logger::start<EventType>(a);
        _active = true;
    }

    template <typename EventType, typename A, typename B>
    inline void set(A const &a, B const &b) {
        if (_active) {
            Logger::finish();
        }
        Logger::start<EventType>(a, b);
        _active = true;
    }

    template <typename EventType, typename A, typename B, typename C>
    inline void set(A const &a, B const &b, C const &c) {
        if (_active) {
            Logger::finish();
        }
        Logger::start<EventType>(a, b, c);
        _active = true;
    }

    template <typename EventType, typename A, typename B,
                                  typename C, typename D>
    inline void set(A const &a, B const &b, C const &c, D const &d) {
        if (_active) {
            Logger::finish();
        }
        Logger::start<EventType>(a, b, c, d);
        _active = true;
    }

    template <typename EventType, typename A, typename B, typename C,
                                  typename D, typename E>
    inline void set(A const &a, B const &b, C const &c, D const &d, E const &e)
    {
        if (_active) {
            Logger::finish();
        }
        Logger::start<EventType>(a, b, c, d, e);
        _active = true;
    }

    template <typename EventType, typename A, typename B, typename C,
                                  typename D, typename E, typename F>
    inline void set(A const &a, B const &b, C const &c,
                    D const &d, E const &e, F const &f)
    {
        if (_active) {
            Logger::finish();
        }
        Logger::start<EventType>(a, b, c, d, e, f);
        _active = true;
    }

    template <typename EventType, typename A, typename B, typename C,
                                  typename D, typename E, typename F,
                                  typename G>
    inline void set(A const &a, B const &b, C const &c, D const &d,
                    E const &e, F const &f, G const &g)
    {
        if (_active) {
            Logger::finish();
        }
        Logger::start<EventType>(a, b, c, d, e, f, g);
        _active = true;
    }

    template <typename EventType, typename A, typename B, typename C,
                                  typename D, typename E, typename F,
                                  typename G, typename H>
    inline void set(A const &a, B const &b, C const &c, D const &d,
                    E const &e, F const &f, G const &g, H const &h)
    {
        if (_active) {
            Logger::finish();
        }
        Logger::start<EventType>(a, b, c, d, e, f, g, h);
        _active = true;
    }

    void clear() {
        if (_active) {
            Logger::finish();
            _active = false;
        }
    }

protected:
    EventTrackerBase(bool active) : _active(active) {}

private:
    EventTrackerBase(EventTrackerBase const &); // no copy
    void operator=(EventTrackerBase const &); // no assign
    bool _active;
};

template <typename EventType> class EventTracker : public EventTrackerBase {
public:
    EventTracker() : EventTrackerBase(true) { Logger::start<EventType>(); }

    template <typename A>
    EventTracker(A const &a) : EventTrackerBase(true) {
        Logger::start<EventType>(a);
    }

    template <typename A, typename B>
    EventTracker(A const &a, B const &b) : EventTrackerBase(true) {
        Logger::start<EventType>(a, b);
    }

    template <typename A, typename B, typename C>
    EventTracker(A const &a, B const &b, C const &c) : EventTrackerBase(true) {
        Logger::start<EventType>(a, b, c);
    }

    template <typename A, typename B, typename C, typename D>
    EventTracker(A const &a, B const &b, C const &c, D const &d)
    : EventTrackerBase(true)
    {
        Logger::start<EventType>(a, b, c, d);
    }

    template <typename A, typename B, typename C, typename D, typename E>
    EventTracker(A const &a, B const &b, C const &c, D const &d, E const &e)
    : EventTrackerBase(true)
    {
        Logger::start<EventType>(a, b, c, d, e);
    }

    template <typename A, typename B, typename C, typename D,
              typename E, typename F>
    EventTracker(A const &a, B const &b, C const &c, D const &d,
                 E const &e, F const &f)
    : EventTrackerBase(true)
    {
        Logger::start<EventType>(a, b, c, d, e, f);
    }

    template <typename A, typename B, typename C, typename D,
              typename E, typename F, typename G>
    EventTracker(A const &a, B const &b, C const &c, D const &d,
                 E const &e, F const &f, G const &g)
    : EventTrackerBase(true)
    {
        Logger::start<EventType>(a, b, c, d, e, f, g);
    }

    template <typename A, typename B, typename C, typename D,
              typename E, typename F, typename G, typename H>
    EventTracker(A const &a, B const &b, C const &c, D const &d,
                 E const &e, F const &f, G const &g, H const &h)
    : EventTrackerBase(true)
    {
        Logger::start<EventType>(a, b, c, d, e, f, g, h);
    }
};

template <> class EventTracker<NoInitialEvent> : public EventTrackerBase {
public:
    EventTracker() : EventTrackerBase(false) {}
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

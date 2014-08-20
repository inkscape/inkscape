/*
 * Inkscape::Debug::Logger - debug logging facility
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2005 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_DEBUG_LOGGER_H
#define SEEN_INKSCAPE_DEBUG_LOGGER_H

#include "debug/event.h"

namespace Inkscape {

namespace Debug {

class Logger {
public:
    static void init();

    template <typename EventType>
    inline static void start() {
        if (_enabled) {
            if (_category_mask[EventType::category()]) {
                _start(EventType());
            } else {
                _skip();
            }
        }
    }

    template <typename EventType, typename A>
    inline static void start(A const &a) {
        if (_enabled) {
            if (_category_mask[EventType::category()]) {
                _start(EventType(a));
            } else {
                _skip();
            }
        }
    }

    template <typename EventType, typename A, typename B>
    inline static void start(A const &a, B const &b) {
        if (_enabled) {
            if (_category_mask[EventType::category()]) {
                _start(EventType(a, b));
            } else {
                _skip();
            }
        }
    }

    template <typename EventType, typename A, typename B, typename C>
    inline static void start(A const &a, B const &b, C const &c) {
        if (_enabled) {
            if (_category_mask[EventType::category()]) {
                _start(EventType(a, b, c));
            } else {
                _skip();
            }
        }
    }

    template <typename EventType, typename A, typename B,
                                  typename C, typename D>
    inline static void start(A const &a, B const &b, C const &c, D const &d) {
        if (_enabled) {
            if (_category_mask[EventType::category()]) {
                _start(EventType(a, b, c, d));
            } else {
                _skip();
            }
        }
    }

    template <typename EventType, typename A, typename B, typename C,
                                  typename D, typename E>
    inline static void start(A const &a, B const &b, C const &c,
                             D const &d, E const &e)
    {
        if (_enabled) {
            if (_category_mask[EventType::category()]) {
                _start(EventType(a, b, c, d, e));
            } else {
                _skip();
            }
        }
    }

    template <typename EventType, typename A, typename B, typename C,
                                  typename D, typename E, typename F>
    inline static void start(A const &a, B const &b, C const &c,
                             D const &d, E const &e, F const &f)
    {
        if (_enabled) {
            if (_category_mask[EventType::category()]) {
                _start(EventType(a, b, c, d, e, f));
            } else {
                _skip();
            }
        }
    }

    template <typename EventType, typename A, typename B, typename C,
                                  typename D, typename E, typename F,
                                  typename G>
    inline static void start(A const &a, B const &b, C const &c, D const &d,
                             E const &e, F const &f, G const &g)
    {
        if (_enabled) {
            if (_category_mask[EventType::category()]) {
                _start(EventType(a, b, c, d, e, f, g));
            } else {
                _skip();
            }
        }
    }

    template <typename EventType, typename A, typename B, typename C,
                                  typename D, typename E, typename F,
                                  typename G, typename H>
    inline static void start(A const &a, B const &b, C const &c, D const &d,
                             E const &e, F const &f, G const &g, H const &h)
    {
        if (_enabled) {
            if (_category_mask[EventType::category()]) {
                _start(EventType(a, b, c, d, e, f, g, h));
            } else {
                _skip();
            }
        }
    }

    inline static void finish() {
        if (_enabled) {
            _finish();
        }
    }

    template <typename EventType>
    inline static void write() {
        start<EventType>();
        finish();
    }

    template <typename EventType, typename A>
    inline static void write(A const &a) {
        start<EventType, A>(a);
        finish();
    }

    template <typename EventType, typename A, typename B>
    inline static void write(A const &a, B const &b) {
        start<EventType, A, B>(a, b);
        finish();
    }

    template <typename EventType, typename A, typename B, typename C>
    inline static void write(A const &a, B const &b, C const &c) {
        start<EventType, A, B, C>(a, b, c);
        finish();
    }

    template <typename EventType, typename A, typename B,
                                  typename C, typename D>
    inline static void write(A const &a, B const &b, C const &c, D const &d) {
        start<EventType, A, B, C, D>(a, b, c, d);
        finish();
    }

    template <typename EventType, typename A, typename B,
                                  typename C, typename D,
                                  typename E>
    inline static void write(A const &a, B const &b, C const &c,
                             D const &d, E const &e)
    {
        start<EventType, A, B, C, D, E>(a, b, c, d, e);
        finish();
    }

    template <typename EventType, typename A, typename B,
                                  typename C, typename D,
                                  typename E, typename F>
    inline static void write(A const &a, B const &b, C const &c,
                             D const &d, E const &e, F const &f)
    {
        start<EventType, A, B, C, D, E, F>(a, b, c, d, e, f);
        finish();
    }

    template <typename EventType, typename A, typename B,
                                  typename C, typename D,
                                  typename E, typename F,
                                  typename G>
    inline static void write(A const &a, B const &b, C const &c,
                             D const &d, E const &e, F const &f,
                             G const &g)
    {
        start<EventType, A, B, C, D, E, F, G>(a, b, c, d, e, f, g);
        finish();
    }

    template <typename EventType, typename A, typename B,
                                  typename C, typename D,
                                  typename E, typename F,
                                  typename G, typename H>
    inline static void write(A const &a, B const &b, C const &c,
                             D const &d, E const &e, F const &f,
                             G const &g, H const &h)
    {
        start<EventType, A, B, C, D, E, F, G, H>(a, b, c, d, e, f, g, h);
        finish();
    }

    static void shutdown();

private:
    static bool _enabled;

    static void _start(Event const &event);
    static void _skip();
    static void _finish();

    static bool _category_mask[Event::N_CATEGORIES];
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

#ifndef __NR_MAYBE_H__
#define __NR_MAYBE_H__

/*
 * Nullable values for C++
 *
 * Copyright 2004, 2007  MenTaLguY <mental@rydia.net>
 *
 * This code is licensed under the GNU GPL; see COPYING for more information.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdexcept>
#include <string>

namespace NR {

class IsNothing : public std::domain_error {
public:
    IsNothing() : domain_error(std::string("Is nothing")) {}
};

struct Nothing {};

template <typename T>
class Maybe {
public:
    Maybe(Nothing) : _is_nothing(true) {}
    Maybe(T const &t) : _t(t), _is_nothing(false) {}
    Maybe(Maybe const &m) : _t(m._t), _is_nothing(m._is_nothing) {}

    template <typename T2> Maybe(Maybe<T2> const &m)
    : _is_nothing(!m)
    {
        if (m) {
            _t = *m;
        }
    }

    template <typename T2> Maybe(T2 const &t)
    : _t(t), _is_nothing(false) {}

    operator bool() const { return !_is_nothing; }

    T const &operator*() const throw(IsNothing) {
        if (_is_nothing) {
            throw IsNothing();
        } else {
            return _t;
        }
    }
    T &operator*() throw(IsNothing) {
        if (_is_nothing) {
            throw IsNothing();
        } else {
            return _t;
        }
    }

    T const *operator->() const throw(IsNothing) {
        if (_is_nothing) {
            throw IsNothing();
        } else {
            return &_t;
        }
    }
    T *operator->() throw(IsNothing) {
        if (_is_nothing) {
            throw IsNothing();
        } else {
            return &_t;
        }
    }

    template <typename T2>
    bool operator==(NR::Maybe<T2> const &other) const {
        if ( _is_nothing || !other ) {
            return _is_nothing && !other;
        } else {
            return _t == *other;
        }
    }
    template <typename T2>
    bool operator!=(NR::Maybe<T2> const &other) const {
        if ( _is_nothing || !other ) {
            return !_is_nothing || other;
        } else {
            return _t != *other;
        }
    }

private:
    T _t;
    bool _is_nothing;
};

template <typename T>
class Maybe<T const> {
public:
    Maybe(Nothing) : _is_nothing(true) {}
    Maybe(T const &t) : _t(t), _is_nothing(false) {}
    Maybe(Maybe const &m) : _t(m._t), _is_nothing(m._is_nothing) {}

    template <typename T2> Maybe(Maybe<T2> const &m)
    : _is_nothing(!m)
    {
        if (m) {
            _t = *m;
        }
    }

    template <typename T2> Maybe(T2 const &t)
    : _t(t), _is_nothing(false) {}

    operator bool() const { return !_is_nothing; }

    T const &operator*() const throw(IsNothing) {
        if (_is_nothing) {
            throw IsNothing();
        } else {
            return _t;
        }
    }

    T const *operator->() const throw(IsNothing) {
        if (_is_nothing) {
            throw IsNothing();
        } else {
            return &_t;
        }
    }

    template <typename T2>
    bool operator==(NR::Maybe<T2> const &other) const {
        if ( _is_nothing || !other ) {
            return _is_nothing && !other;
        } else {
            return _t == *other;
        }
    }
    template <typename T2>
    bool operator!=(NR::Maybe<T2> const &other) const {
        if ( _is_nothing || !other ) {
            return !_is_nothing || other;
        } else {
            return _t != *other;
        }
    }

private:
    T const _t;
    bool _is_nothing;
};

template <typename T>
class Maybe<T &> {
public:
    Maybe(Nothing) : _ref(NULL) {}
    Maybe(T &t) : _ref(&t) {}
    Maybe(Maybe const &m) : _ref(m._ref) {}

    template <typename T2> Maybe(Maybe<T2> const &m)
    : _ref( m ? &*m : NULL ) {}
    template <typename T2> Maybe(T2 &t) : _ref(&t) {}

    operator bool() const { return _ref; }

    T &operator*() const throw(IsNothing) {
        if (_ref) {
            throw IsNothing();
        } else {
            return *_ref;
        }
    }

    T *operator->() const throw(IsNothing) {
        if (_ref) {
            throw IsNothing();
        } else {
            return _ref;
        }
    }

    template <typename T2>
    bool operator==(NR::Maybe<T2> const &other) const {
        if ( !_ref || !other ) {
            return !_ref && !other;
        } else {
            return *_ref == *other;
        }
    }
    template <typename T2>
    bool operator!=(NR::Maybe<T2> const &other) const {
        if ( !_ref || !other ) {
            return _ref || other;
        } else {
            return *_ref != *other;
        }
    }

private:
    void operator=(Maybe const &); // no assign

    T *_ref;
};

} /* namespace NR */

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

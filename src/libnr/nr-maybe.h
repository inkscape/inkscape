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
#include <algorithm>

namespace NR {

class IsNothing : public std::domain_error {
public:
    IsNothing() : domain_error(std::string("Is nothing")) {}
};

struct Nothing {};

template <typename T>
class MaybeStorage {
public:
    MaybeStorage() : _is_nothing(true) {}
    MaybeStorage(T const &value)
    : _value(value), _is_nothing(false) {}

    bool is_nothing() const { return _is_nothing; }
    T &value() { return _value; }
    T const &value() const { return _value; }

private:
    T _value;
    bool _is_nothing;
};

template <typename T>
class Maybe {
public:
    Maybe() {}
    Maybe(Nothing) {}
    Maybe(T const &t) : _storage(t) {}
    Maybe(Maybe const &m) : _storage(m._storage) {}

    template <typename T2>
    Maybe(Maybe<T2> const &m) {
        if (m) {
            _storage = *m;
        }
    }

    template <typename T2>
    Maybe(Maybe<T2 const &> m) {
        if (m) {
            _storage = *m;
        }
    }

    operator bool() const { return !_storage.is_nothing(); }

    T const &operator*() const throw(IsNothing) {
        if (_storage.is_nothing()) {
            throw IsNothing();
        } else {
            return _storage.value();
        }
    }
    T &operator*() throw(IsNothing) {
        if (_storage.is_nothing()) {
            throw IsNothing();
        } else {
            return _storage.value();
        }
    }

    T const *operator->() const throw(IsNothing) {
        if (_storage.is_nothing()) {
            throw IsNothing();
        } else {
            return &_storage.value();
        }
    }
    T *operator->() throw(IsNothing) {
        if (_storage.is_nothing()) {
            throw IsNothing();
        } else {
            return &_storage.value();
        }
    }

    template <typename T2>
    bool operator==(Maybe<T2> const &other) const {
        bool is_nothing = _storage.is_nothing();
        if ( is_nothing || !other ) {
            return is_nothing && !other;
        } else {
            return _storage.value() == *other;
        }
    }
    template <typename T2>
    bool operator!=(Maybe<T2> const &other) const {
        bool is_nothing = _storage.is_nothing();
        if ( is_nothing || !other ) {
            return !is_nothing || other;
        } else {
            return _storage.value() != *other;
        }
    }

private:
    MaybeStorage<T> _storage;
};

template <typename T>
class Maybe<T &> {
public:
    Maybe() : _ref(NULL) {}
    Maybe(Nothing) : _ref(NULL) {}
    Maybe(T &t) : _ref(&t) {}

    template <typename T2>
    Maybe(Maybe<T2> const &m) {
        if (m) {
            _ref = &*m;
        } 
    }

    template <typename T2>
    Maybe(Maybe<T2 &> m) {
        if (m) {
            _ref = *m;
        }
    }

    template <typename T2>
    Maybe(Maybe<T2 const &> m) {
        if (m) {
            _ref = *m;
        }
    }

    operator bool() const { return _ref; }

    T &operator*() const throw(IsNothing) {
        if (!_ref) {
            throw IsNothing();
        } else {
            return *_ref;
        }
    }
    T *operator->() const throw(IsNothing) {
        if (!_ref) {
            throw IsNothing();
        } else {
            return _ref;
        }
    }

    template <typename T2>
    bool operator==(Maybe<T2> const &other) const {
        if ( !_ref || !other ) {
            return !_ref && !other;
        } else {
            return *_ref == *other;
        }
    }
    template <typename T2>
    bool operator!=(Maybe <T2> const &other) const {
        if ( !_ref || !other ) {
            return _ref || other;
        } else {
            return *_ref != *other;
        }
    }

private:
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

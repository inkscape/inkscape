#ifndef __NR_MAYBE_H__
#define __NR_MAYBE_H__

/*
 * Functionalesque "Maybe" class
 *
 * Copyright 2004  MenTaLguY
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * This code is licensed under the GNU GPL; see COPYING for more information.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdexcept>
#include <typeinfo>
#include <string>

namespace NR {

/** An exception class for run-time type errors */
template <typename T>
class IsNot : public std::domain_error {
public:
    IsNot() : domain_error(std::string("Is not ") + typeid(T).name()) {}
};

/** A type with only one value, which (in principle) is only equal to itself.
 *
 *  Types that may (at runtime) pretend to be Nothing need only provide an
 *  operator bool operator==(Type, Nothing); the rest of the operator
 *  definitions will be taken care of automatically.
 *
 *  Such types should also provide a casting operator to Nothing, obviously.
 */
struct Nothing {
    bool operator==(Nothing n) { return true; }
    bool operator!=(Nothing n) { return false; }
    template <typename T>
    bool operator==(T t) { return t == *this; }
    template <typename T>
    bool operator!=(T t) { return t != *this; }
};

template <typename T>
bool operator==(T t, Nothing n) { return false; }
template <typename T>
bool operator!=(T t, Nothing n) { return !( t == n ); }

template <typename T>
struct MaybeTraits;

template <typename T>
class Maybe {
public:
    typedef MaybeTraits<T> traits;
    typedef typename traits::storage storage;
    typedef typename traits::reference reference;

    Maybe(Nothing n) : _is_nothing(true), _t() {}

    Maybe(const Maybe<T> &m) : _is_nothing(m._is_nothing), _t(m._t) {}

    template <typename T2>
    Maybe(const Maybe<T2> &m)
    : _is_nothing(m._is_nothing),
      _t(traits::to_storage(MaybeTraits<T2>::from_storage(m._t))) {}

    template <typename T2>
    Maybe(T2 t) : _is_nothing(false), _t(traits::to_storage(t)) {}

    reference assume() const throw(IsNot<T>) {
        if (_is_nothing) {
            throw IsNot<T>();
        } else {
            return traits::from_storage(_t);
        }
    }

    operator reference() const throw(IsNot<T>) {
        if (_is_nothing) {
            throw IsNot<T>();
        } else {
            return traits::from_storage(_t);
        }
    }
    operator Nothing() const throw(IsNot<Nothing>) {
        if (!_is_nothing) {
            throw IsNot<Nothing>();
        } else {
            return Nothing();
        }
    }

    bool operator==(Nothing n) { return _is_nothing; }
    bool operator==(reference r) {
        return traits::from_storage(_t) == r;
    }

private:
    bool _is_nothing;
    storage _t;
};

/* traits classes used by Maybe */

template <typename T>
struct MaybeTraits {
    typedef T const storage;
    typedef T const &reference;
    static reference to_storage(reference t) { return t; }
    static reference from_storage(reference t) { return t; }
};

template <typename T>
struct MaybeTraits<T&> {
    typedef T *storage;
    typedef T &reference;
    static storage to_storage(reference t) { return &t; }
    static reference from_storage(storage t) { return *t; }
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

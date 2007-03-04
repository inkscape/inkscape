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

struct Nothing {};

template <typename T>
struct MaybeTraits;

template <typename T>
class Maybe {
public:
    typedef MaybeTraits<T> traits;
    typedef typename traits::storage storage;
    typedef typename traits::pointer pointer;
    typedef typename traits::reference reference;

    Maybe(Nothing n) : _is_nothing(true), _t() {}

    Maybe(const Maybe<T> &m) : _is_nothing(m._is_nothing), _t(m._t) {}

    template <typename T2>
    Maybe(const Maybe<T2> &m)
    : _is_nothing(m._is_nothing),
      _t(traits::to_storage(MaybeTraits<T2>::from_storage(m._t))) {}

    template <typename T2>
    Maybe(T2 t) : _is_nothing(false), _t(traits::to_storage(t)) {}

    bool isNothing() const { return _is_nothing; }
    operator bool() const { return !_is_nothing; }

    reference assume() const throw(IsNot<T>) {
        if (_is_nothing) {
            throw IsNot<T>();
        } else {
            return traits::from_storage(_t);
        }
    }
    reference operator*() const throw(IsNot<T>) {
        return assume();
    }
    pointer operator->() const throw(IsNot<T>) {
        return &assume();
    }

private:
    bool _is_nothing;
    storage _t;
};

/* traits classes used by Maybe */

template <typename T>
struct MaybeTraits {
    typedef T const storage;
    typedef T const *pointer;
    typedef T const &reference;
    static reference to_storage(reference t) { return t; }
    static reference from_storage(reference t) { return t; }
};

template <typename T>
struct MaybeTraits<T&> {
    typedef T *storage;
    typedef T *pointer;
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

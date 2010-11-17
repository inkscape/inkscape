/*
 * Inkscape::Util::Tuple - generic tuple type
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_TUPLE_H
#define SEEN_INKSCAPE_UTIL_TUPLE_H

#include "util/reference.h"

namespace Inkscape {

namespace Util {

template <typename A=void, typename B=void, typename C=void,
          typename D=void, typename E=void, typename F=void>
struct Tuple {
    Tuple() {}
    Tuple(typename Traits::Reference<A>::RValue a_,
          typename Traits::Reference<B>::RValue b_,
          typename Traits::Reference<C>::RValue c_,
          typename Traits::Reference<D>::RValue d_,
          typename Traits::Reference<E>::RValue e_,
          typename Traits::Reference<F>::RValue f_)
    : a(a_), b(b_), c(c_), d(d_), e(e_), f(f_) {}

    A a;
    B b;
    C c;
    D d;
    E e;
    F f;
};

template <typename A, typename B, typename C,
          typename D, typename E>
struct Tuple<A, B, C, D, E, void> {
    Tuple() {}
    Tuple(typename Traits::Reference<A>::RValue a_,
          typename Traits::Reference<B>::RValue b_,
          typename Traits::Reference<C>::RValue c_,
          typename Traits::Reference<D>::RValue d_,
          typename Traits::Reference<E>::RValue e_)
    : a(a_), b(b_), c(c_), d(d_), e(e_) {}

    A a;
    B b;
    C c;
    D d;
    E e;
};

template <typename A, typename B, typename C, typename D>
struct Tuple<A, B, C, D, void, void> {
    Tuple() {}
    Tuple(typename Traits::Reference<A>::RValue a_,
          typename Traits::Reference<B>::RValue b_,
          typename Traits::Reference<C>::RValue c_,
          typename Traits::Reference<D>::RValue d_)
    : a(a_), b(b_), c(c_), d(d_) {}

    A a;
    B b;
    C c;
    D d;
};

template <typename A, typename B, typename C>
struct Tuple<A, B, C, void, void, void> {
    Tuple() {}
    Tuple(typename Traits::Reference<A>::RValue a_,
          typename Traits::Reference<B>::RValue b_,
          typename Traits::Reference<C>::RValue c_)
    : a(a_), b(b_), c(c_) {}

    A a;
    B b;
    C c;
};

template <typename A, typename B>
struct Tuple<A, B, void, void, void, void> {
    Tuple() {}
    Tuple(typename Traits::Reference<A>::RValue a_,
          typename Traits::Reference<B>::RValue b_)
    : a(a_), b(b_) {}

    A a;
    B b;
};

template <typename A>
struct Tuple<A, void, void, void, void, void> {
    Tuple() {}
    Tuple(typename Traits::Reference<A>::RValue a_)
    : a(a_) {}

    A a;
};

template <> struct Tuple<void, void, void, void, void, void> {};

template <typename A, typename B, typename C,
          typename D, typename E, typename F>
inline Tuple<A, B, C, D, E, F>
tuple(typename Traits::Reference<A>::RValue a,
      typename Traits::Reference<B>::RValue b,
      typename Traits::Reference<C>::RValue c,
      typename Traits::Reference<D>::RValue d,
      typename Traits::Reference<E>::RValue e,
      typename Traits::Reference<F>::RValue f)
{
    return Tuple<A, B, C, D, E, F>(a, b, c, d, e, f);
}

template <typename A, typename B, typename C, typename D, typename E>
inline Tuple<A, B, C, D, E>
tuple(typename Traits::Reference<A>::RValue a,
      typename Traits::Reference<B>::RValue b,
      typename Traits::Reference<C>::RValue c,
      typename Traits::Reference<D>::RValue d,
      typename Traits::Reference<E>::RValue e)
{
    return Tuple<A, B, C, D, E>(a, b, c, d, e);
}

template <typename A, typename B, typename C, typename D>
inline Tuple<A, B, C, D>
tuple(typename Traits::Reference<A>::RValue a,
      typename Traits::Reference<B>::RValue b,
      typename Traits::Reference<C>::RValue c,
      typename Traits::Reference<D>::RValue d)
{
    return Tuple<A, B, C, D>(a, b, c, d);
}

template <typename A, typename B, typename C>
inline Tuple<A, B, C>
tuple(typename Traits::Reference<A>::RValue a,
      typename Traits::Reference<B>::RValue b,
      typename Traits::Reference<C>::RValue c)
{
    return Tuple<A, B, C>(a, b, c);
}

template <typename A, typename B>
inline Tuple<A, B>
tuple(typename Traits::Reference<A>::RValue a,
      typename Traits::Reference<B>::RValue b)
{
    return Tuple<A, B>(a, b);
}

template <typename A>
inline Tuple<A>
tuple(typename Traits::Reference<A>::RValue a) {
    return Tuple<A>(a);
}

inline Tuple<> tuple() { return Tuple<>(); }

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

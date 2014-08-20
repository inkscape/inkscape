/*
 * Inkscape::Traits::Function - traits class for C++ "functors"
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_TRAITS_FUNCTION_H
#define SEEN_INKSCAPE_TRAITS_FUNCTION_H

namespace Inkscape {

namespace Traits {

template <typename F> struct Function;

template <typename R>
struct Function<R (*)()> {
    typedef R Result;

    typedef void Arg0;
    typedef void Arg1;
    typedef void Arg2;
    typedef void Arg3;
    typedef void Arg4;
    typedef void Arg5;
};

template <typename R, typename A0>
struct Function<R (*)(A0)> {
    typedef R Result;

    typedef A0 Arg0;
    typedef void Arg1;
    typedef void Arg2;
    typedef void Arg3;
    typedef void Arg4;
    typedef void Arg5;
};

template <typename R, typename A0, typename A1>
struct Function<R (*)(A0, A1)> {
    typedef R Result;

    typedef A0 Arg0;
    typedef A1 Arg1;
    typedef void Arg2;
    typedef void Arg3;
    typedef void Arg4;
    typedef void Arg5;
};

template <typename R, typename A0, typename A1, typename A2>
struct Function<R (*)(A0, A1, A2)> {
    typedef R Result;

    typedef A0 Arg0;
    typedef A1 Arg1;
    typedef A2 Arg2;
    typedef void Arg3;
    typedef void Arg4;
    typedef void Arg5;
};

template <typename R, typename A0, typename A1, typename A2,
                      typename A3>
struct Function<R (*)(A0, A1, A2, A3)> {
    typedef R Result;

    typedef A0 Arg0;
    typedef A1 Arg1;
    typedef A2 Arg2;
    typedef A3 Arg3;
    typedef void Arg4;
    typedef void Arg5;
};

template <typename R, typename A0, typename A1, typename A2,
                      typename A3, typename A4>
struct Function<R (*)(A0, A1, A2, A3, A4)> {
    typedef R Result;

    typedef A0 Arg0;
    typedef A1 Arg1;
    typedef A2 Arg2;
    typedef A3 Arg3;
    typedef A4 Arg4;
    typedef void Arg5;
};

template <typename R, typename A0, typename A1, typename A2,
                      typename A3, typename A4, typename A5>
struct Function<R (*)(A0, A1, A2, A3, A4, A5)> {
    typedef R Result;

    typedef A0 Arg0;
    typedef A1 Arg1;
    typedef A2 Arg2;
    typedef A3 Arg3;
    typedef A4 Arg4;
    typedef A5 Arg5;
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

/*
 * Inkscape::Traits::Copy - traits class to determine types to use when copying
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_TRAITS_COPY_H
#define SEEN_INKSCAPE_TRAITS_COPY_H

namespace Inkscape {

namespace Traits {

template <typename T>
struct Copy {
    typedef T Type;
};

template <typename T>
struct Copy<T const> {
    typedef T Type;
};

template <typename T>
struct Copy<T &> {
    typedef T &Type;
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

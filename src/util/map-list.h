/*
 * Inkscape::Util::map_list - apply a function over a list
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_MAP_LIST_H
#define SEEN_INKSCAPE_UTIL_MAP_LIST_H

#include <algorithm>
#include "util/list.h"

namespace Inkscape {

namespace Util {

template <typename T, typename InputIterator, typename UnaryFunction>
inline MutableList<T>
map_list(UnaryFunction f, InputIterator start, InputIterator end)
{
    if ( start != end ) {
        MutableList<T> head(f(*start));
        MutableList<T> tail(head);
        while ( ++start != end ) {
            MutableList<T> cell(f(*start));
            set_rest(tail, cell);
            tail = cell;
        }
        return head;
    } else {
        return MutableList<T>();
    }
}

template <typename T1, typename T2, typename UnaryFunction>
inline MutableList<T1> map_list(UnaryFunction f, List<T2> list) {
    return map_list(f, list, List<T2>());
}

template <typename T, typename UnaryFunction>
inline List<T>
map_list_in_place(UnaryFunction f, List<T> start,
                                   List<T> end=List<T>())
{
    std::transform(start, end, start, f);
    return start;
}

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

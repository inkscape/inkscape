/*
 * Inkscape::Util::reverse_list - generate a reversed list from iterator range
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_REVERSE_LIST_H
#define SEEN_INKSCAPE_UTIL_REVERSE_LIST_H

#include "util/list.h"
#include "util/list-copy.h"

namespace Inkscape {

namespace Util {

template <typename InputIterator>
inline typename Traits::ListCopy<InputIterator>::ResultList
reverse_list(InputIterator start, InputIterator end) {
    typename Traits::ListCopy<InputIterator>::ResultList head;
    while ( start != end ) {
        head = cons(*start, head);
        ++start;
    }
    return head;
}

template <typename T>
inline typename Traits::ListCopy<List<T> >::ResultList
reverse_list(List<T> const &list) {
    return reverse_list(list, List<T>());
}

template <typename T>
inline MutableList<T>
reverse_list_in_place(MutableList<T> start,
                      MutableList<T> end=MutableList<T>())
{
    MutableList<T> reversed(end); 
    while ( start != end ) {
        MutableList<T> temp(start);
        ++start;
        set_rest(temp, reversed);
        reversed = temp;
    }
    return reversed;
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

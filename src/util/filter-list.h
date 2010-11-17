/*
 * Inkscape::Util::filter_list - select a subset of the items in a list
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_FILTER_LIST_H
#define SEEN_INKSCAPE_UTIL_FILTER_LIST_H

#include "util/list.h"
#include "util/list-copy.h"

namespace Inkscape {

namespace Util {

template <typename InputIterator, typename UnaryPredicate>
inline typename Traits::ListCopy<InputIterator>::ResultList
filter_list(UnaryPredicate p, InputIterator start, InputIterator end) {
    typedef typename Traits::ListCopy<InputIterator>::ResultList ResultList;
    ResultList head;
    ResultList tail;
    while ( start != end && !p(*start) ) {
        ++start;
    }
    if ( start != end ) {
        head = tail = ResultList(*start);
        ++start;
    }
    while ( start != end ) {
        if (p(*start)) {
            set_rest(tail, ResultList(*start));
            ++tail;
        }
        ++start;
    }
    return head;
}

template <typename T, typename UnaryPredicate>
inline typename Traits::ListCopy<List<T> >::ResultList
filter_list(UnaryPredicate p, List<T> const &list) {
    return filter_list(p, list, List<T>());
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

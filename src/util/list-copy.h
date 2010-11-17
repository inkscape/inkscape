/*
 * Inkscape::Traits::ListCopy - helper traits class for copying lists
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_TRAITS_LIST_COPY_H
#define SEEN_INKSCAPE_TRAITS_LIST_COPY_H

#include <iterator>
#include "util/copy.h"
#include "util/list.h"

namespace Inkscape {

namespace Traits {

template <typename InputIterator>
struct ListCopy {
    typedef typename Copy<
        typename std::iterator_traits<InputIterator>::value_type
    >::Type ResultValue;
    typedef typename Util::MutableList<ResultValue> ResultList;
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

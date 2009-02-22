/*
 * Inkscape::Algorithms::find_if_before - finds the position before
 *                                        the first value that satisifes
 *                                        the predicate
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2005 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_ALGORITHMS_FIND_IF_BEFORE_H
#define SEEN_INKSCAPE_ALGORITHMS_FIND_IF_BEFORE_H

#include <algorithm>

namespace Inkscape {

namespace Algorithms {

template <typename ForwardIterator, typename UnaryPredicate>
inline ForwardIterator find_if_before(ForwardIterator start,
                                      ForwardIterator end,
                                      UnaryPredicate pred)
{
    ForwardIterator before=end;
    while ( start != end && !pred(*start) ) {
        before = start;
        ++start;
    }
    return ( start != end ) ? before : end;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

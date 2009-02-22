/*
 * Inkscape::Algorithms::find_last_if
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_ALGORITHMS_FIND_LAST_IF_H
#define SEEN_INKSCAPE_ALGORITHMS_FIND_LAST_IF_H

#include <algorithm>

namespace Inkscape {

namespace Algorithms {

template <typename ForwardIterator, typename UnaryPredicate>
inline ForwardIterator find_last_if(ForwardIterator start, ForwardIterator end,
                                    UnaryPredicate pred)
{
    ForwardIterator last_found(end);
    while ( start != end ) {
        start = std::find_if(start, end, pred);
        if ( start != end ) {
            last_found = start;
            ++start;
        }
    }
    return last_found;
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

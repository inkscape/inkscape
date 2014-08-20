/*
 * Inkscape::Algorithms::longest_common_suffix 
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_ALGORITHMS_LONGEST_COMMON_SUFFIX_H
#define SEEN_INKSCAPE_ALGORITHMS_LONGEST_COMMON_SUFFIX_H

#include <iterator>
#include <functional>
#include "util/list.h"

namespace Inkscape {

namespace Algorithms {

/**
 * Time costs:
 *
 * The case of sharing a common successor is handled in O(1) time.
 *
 * If \a a is the longest common suffix, then runs in O(len(rest of b)) time.
 *
 * Otherwise, runs in O(len(a) + len(b)) time.
 */

template <typename ForwardIterator>
ForwardIterator longest_common_suffix(ForwardIterator a, ForwardIterator b,
                                      ForwardIterator end)
{
    typedef typename std::iterator_traits<ForwardIterator>::value_type value_type;
    return longest_common_suffix(a, b, end, std::equal_to<value_type>());
}

template <typename ForwardIterator, typename BinaryPredicate>
ForwardIterator longest_common_suffix(ForwardIterator a, ForwardIterator b,
                                      ForwardIterator end, BinaryPredicate pred)
{
    if ( a == end || b == end ) {
        return end;
    }

    /* Handle in O(1) time the common cases of identical lists or tails. */
    {
        /* identical lists? */
        if ( a == b ) {
            return a;
        }

        /* identical tails? */
        ForwardIterator tail_a(a);
        ForwardIterator tail_b(b);
        if ( ++tail_a == ++tail_b ) {
            return tail_a;
        }
    }

    /* Build parallel lists of suffixes, ordered by increasing length. */

    using Inkscape::Util::List;
    using Inkscape::Util::cons;
    ForwardIterator lists[2] = { a, b };
    List<ForwardIterator> suffixes[2];

    for ( int i=0 ; i < 2 ; i++ ) {
        for ( ForwardIterator iter(lists[i]) ; iter != end ; ++iter ) {
            if ( iter == lists[1-i] ) {
                // the other list is a suffix of this one
                return lists[1-i];
            }

            suffixes[i] = cons(iter, suffixes[i]);
        }
    }

    /* Iterate in parallel through the lists of suffix lists from shortest to
     * longest, stopping before the first pair of suffixes that differs
     */

    ForwardIterator longest_common(end);

    while ( suffixes[0] && suffixes[1] &&
            pred(**suffixes[0], **suffixes[1]) )
    {
        longest_common = *suffixes[0];
        ++suffixes[0];
        ++suffixes[1];
    }

    return longest_common;
}

}

}

#endif /* !SEEN_INKSCAPE_ALGORITHMS_LONGEST_COMMON_SUFFIX_H */

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

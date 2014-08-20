/** @file
 * Frequently used accumulators for use with libsigc++
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UTIL_ACCUMULATORS_H
#define SEEN_UTIL_ACCUMULATORS_H

#include <iterator>

namespace Inkscape {
namespace Util {

/**
 * Accumulator which evaluates slots in reverse connection order.
 * The slot that was connected last is evaluated first.
 */
struct Reverse {
    typedef void result_type;
    template <typename T_iterator>
    result_type operator()(T_iterator first, T_iterator last) const {
        while (first != last) *(--last);
    }
};

/**
 * Accumulator type for interruptible signals. Slots return a boolean value; emission
 * is stopped when true is returned from a slot.
 */
struct Interruptible {
    typedef bool result_type;
    template <typename T_iterator>
    result_type operator()(T_iterator first, T_iterator last) const {
        for (; first != last; ++first)
            if (*first) return true;
        return false;
    }
};

/**
 * Same as Interruptible, but the slots are called in reverse order of connection,
 * e.g. the slot that was connected last is evaluated first.
 */
struct ReverseInterruptible {
    typedef bool result_type;
    template <typename T_iterator>
    result_type operator()(T_iterator first, T_iterator last) const {
        while (first != last) {
            if (*(--last)) return true;
        }
        return false;
    }
};

/**
 * The template parameter specifies how many slots from the beginning of the list
 * should be evaluated after other slots. Useful for signals which invoke other signals
 * once complete. Undefined results if the signal does not have at least @c num_chained
 * slots before first emission.
 *
 * For example, if template param = 3, the execution order is as follows:
 * @verbatim
   8. 1. 2. 3. 4. 5. 6. 7.
   S1 S2 S3 S4 S5 S6 S7 S8 @endverbatim
 */
template <unsigned num_chained = 1>
struct Chained {
    typedef void result_type;
    template <typename T_iterator>
    result_type operator()(T_iterator first, T_iterator last) const {
        T_iterator save_first = first;
        // ARGH, iterator_traits is not defined for slot iterators!
        //std::advance(first, num_chained);
        for (unsigned i = 0; i < num_chained && first != last; ++i) ++first;
        for (; first != last; ++first) *first;
        for (unsigned i = 0; i < num_chained && save_first != last; ++i, ++save_first)
            *save_first;
    }
};

/**
 * Executes a logical OR on the results from slots.
 */
struct LogicalOr {
    typedef bool result_type;
    template <typename T_iterator>
    result_type operator()(T_iterator first, T_iterator last) const {
        bool ret = false;
        for (; first != last; ++first) ret |= *first;
        return ret;
    }
};

} // namespace Util
} // namespace Inkscape

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

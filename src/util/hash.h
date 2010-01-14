/** @file
 * Hash function for various things
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UTIL_HASH_H
#define SEEN_UTIL_HASH_H

#include <boost/shared_ptr.hpp>

namespace __gnu_cxx {

/** Hash function for Boost shared pointers */
template <typename T>
struct hash< boost::shared_ptr<T> > : public std::unary_function<boost::shared_ptr<T>, size_t> {
    size_t operator()(boost::shared_ptr<T> p) const {
        return reinterpret_cast<size_t>(p.get());
    }
};

} // namespace __gnu_cxx

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

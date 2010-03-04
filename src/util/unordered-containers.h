/** @file
 * @brief Compatibility wrapper for unordered containers.
 */
/* Authors:
 *   Jon A. Cruz <jon@joncruz.org>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2010 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INK_UTIL_UNORDERED_CONTAINERS_H
#define SEEN_INK_UTIL_UNORDERED_CONTAINERS_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#if defined(HAVE_TR1_UNORDERED_SET)

# include <tr1/unordered_set>
# include <tr1/unordered_map>
# define INK_UNORDERED_SET std::tr1::unordered_set
# define INK_UNORDERED_MAP std::tr1::unordered_map
# define INK_HASH std::tr1::hash

#elif defined(HAVE_BOOST_UNORDERED_SET)
# include <boost/unordered_set.hpp>
# include <boost/unordered_map.hpp>
# define INK_UNORDERED_SET boost::unordered_set
# define INK_UNORDERED_MAP boost::unordered_map
# define INK_HASH boost::hash

#elif defined(HAVE_EXT_HASH_SET)

# include <functional>
# include <ext/hash_set>
# include <ext/hash_map>
# define INK_UNORDERED_SET __gnu_cxx::hash_set
# define INK_UNORDERED_MAP __gnu_cxx::hash_map
# define INK_HASH __gnu_cxx::hash

namespace __gnu_cxx {
// hash function for pointers
// TR1 and Boost have this defined by default, __gnu_cxx doesn't
template<typename T>
struct hash<T *> : public std::unary_function<T *, std::size_t> {
    std::size_t operator()(T *p) const {
        // Taken from Boost
        std::size_t x = static_cast<std::size_t>(reinterpret_cast<std::ptrdiff_t>(p));
        return x + (x >> 3);
    }
};
} // namespace __gnu_cxx
#endif

#else
/// Name (with namespace) of the unordered set template.
#define INK_UNORDERED_SET
/// Name (with namespace) of the unordered map template.
#define INK_UNORDERED_MAP
/// Name (with namespace) of the hash template.
#define INK_HASH

#endif

#endif // SEEN_SET_TYPES_H
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

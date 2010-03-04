#ifndef SEEN_SET_TYPES_H
#define SEEN_SET_TYPES_H


/** @file
 * Simple wrapper to disambiguate hash/unordered lists & sets.
 */
/* Authors:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2010 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if defined(HAVE_TR1_UNORDERED_SET)

# include <tr1/unordered_set>
# include <tr1/unordered_map>
# define optim_set std::tr1::unordered_set
# define optim_map std::tr1::unordered_map

// TODO test on platform with this detected:
//#elif defined(HAVE_UNORDERED_SET)
//
//# include <unordered_set>
//# include <unordered_map>
//#define optim_set std::unordered_set
//#define optim_map std::unordered_map

#elif defined(HAVE_EXT_HASH_SET)

# include <ext/hash_set>
# include <ext/hash_map>
# define optim_set __gnu_cxx::hash_set
# define optim_map __gnu_cxx::hash_map
# define USE_GNU_HASHES 1

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

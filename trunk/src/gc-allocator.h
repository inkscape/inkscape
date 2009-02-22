/** @file
 * @brief Garbage-collected STL allocator for standard containers
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright 2008 Authors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 */

#ifndef SEEN_INKSCAPE_GC_ALLOCATOR_H
#define SEEN_INKSCAPE_GC_ALLOCATOR_H

#include <cstddef>
#include <limits>
#include "gc-core.h"

namespace Inkscape {
namespace GC {

/**
 * @brief Garbage-collected allocator for the standard containers
 *
 * STL containers with default parameters cannot be used as members in garbage-collected
 * objects, because by default the destructors are not called, causing a memory leak
 * (the memory allocated by the container is not freed). To address this, STL containers
 * can be told to use this garbage-collected allocator. It usually is the last template
 * parameter. For example, to define a GC-managed map of ints to Unicode strings:
 *
 * @code typedef std::map<int, Glib::ustring, less<int>, Inkscape::GC::Allocator> gcmap; @endcode
 *
 * Afterwards, you can place gcmap as a member in a non-finalized GC-managed object, because
 * all memory used by gcmap will also be reclaimable by the garbage collector, therefore
 * avoiding memory leaks.
 */
template <typename T>
class Allocator {
    // required typedefs
    typedef T           value_type;
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef T *         pointer;
    typedef T const *   const_pointer;
    typedef T &         reference;
    typedef T const &   const_reference;
    
    // required structure that allows accessing the same allocator for a different type
    template <typename U>
    struct rebind {
        typedef Allocator<U> other;
    };
    
    // constructors - no-ops since the allocator doesn't have any state
    Allocator() throw() {}
    Allocator(Allocator const &) throw() {}
    template <typename U> Allocator(Allocator<U> const &) throw() {}
    ~Allocator() throw() {}
    
    // trivial required methods
    pointer address(reference ref) { return &ref; }
    const_pointer address(const_reference ref) { return &ref; }
    void construct(pointer p, T const &value) { new (static_cast<void*>(p)) T(value); }
    void destroy(pointer p) { p->~T(); }
    
    // maximum meaningful memory amount that can be requested from the allocator
    size_type max_size() {
        return numeric_limits<size_type>::max() / sizeof(T);
    }
    
    // allocate memory for num elements without initializing them
    pointer allocate(size_type num, Allocator<void>::const_pointer) {
        return static_cast<pointer>( Inkscape::GC::Core::malloc(num * sizeof(T)) );
    }
    
    // deallocate memory at p
    void deallocate(pointer p, size_type) {
        Inkscape::GC::Core::free(p);
    }
};

// required comparison operators
template <typename T1, typename T2>
bool operator==(Allocator<T1> const &, Allocator<T2> const &) { return true; }
template <typename T1, typename T2>
bool operator!=(Allocator<T1> const &, Allocator<T2> const &) { return false; }

} // namespace GC
} // namespace Inkscape

#endif // !SEEN_INKSCAPE_GC_ALLOCATOR_H
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

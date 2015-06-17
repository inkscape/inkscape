/*
 * Inkscape::GC::Alloc - GC-aware STL allocator
 *
 * Copyright 2004 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_GC_ALLOC_H
#define SEEN_INKSCAPE_GC_ALLOC_H

#include <limits>
#include <cstddef>
#include "inkgc/gc-core.h"

namespace Inkscape {

namespace GC {

template <typename T, CollectionPolicy collect>
class Alloc {
public:
    typedef T value_type;
    typedef T *pointer;
    typedef T const *const_pointer;
    typedef T &reference;
    typedef T const &const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    template <typename U>
    struct rebind { typedef Alloc<U, collect> other; };

    Alloc() {}
    template <typename U> Alloc(Alloc<U, collect> const &) {}

    pointer address(reference r) { return &r; }
    const_pointer address(const_reference r) { return &r; }

    size_type max_size() const {
        return std::numeric_limits<std::size_t>::max() / sizeof(T);
    }

    pointer allocate(size_type count, void const * =NULL) {
        return static_cast<pointer>(::operator new(count * sizeof(T), SCANNED, collect));
    }

    void construct(pointer p, const_reference value) {
        new (static_cast<void *>(p)) T(value);
    }
    void destroy(pointer p) { p->~T(); }

    void deallocate(pointer p, size_type) { ::operator delete(p, GC); }
};

// allocators with the same collection policy are interchangable

template <typename T1, typename T2,
          CollectionPolicy collect1, CollectionPolicy collect2>
bool operator==(Alloc<T1, collect1> const &, Alloc<T2, collect2> const &) {
    return collect1 == collect2;
}

template <typename T1, typename T2,
          CollectionPolicy collect1, CollectionPolicy collect2>
bool operator!=(Alloc<T1, collect1> const &, Alloc<T2, collect2> const &) {
    return collect1 != collect2;
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

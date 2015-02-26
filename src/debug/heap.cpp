/*
 * Inkscape::Debug::Heap - interface for gathering heap statistics
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2005 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "inkgc/gc-alloc.h"
#include "debug/gc-heap.h"
#include "debug/sysv-heap.h"
#include <vector>

namespace Inkscape {
namespace Debug {

namespace {

typedef std::vector<Heap *, GC::Alloc<Heap *, GC::MANUAL> > HeapCollection;

HeapCollection &heaps() {
    static bool is_initialized=false;
    static HeapCollection heaps;
    if (!is_initialized) {
        heaps.push_back(new SysVHeap());
        heaps.push_back(new GCHeap());
        is_initialized = true;
    }
    return heaps;
}

}

unsigned heap_count() {
    return heaps().size();
}

Heap *get_heap(unsigned i) {
    return heaps()[i];
}

void register_extra_heap(Heap &heap) {
    heaps().push_back(&heap);
}

}
}

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

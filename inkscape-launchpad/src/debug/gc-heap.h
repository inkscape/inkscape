/*
 * Inkscape::Debug::GCHeap - heap statistics for libgc heap
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_DEBUG_GC_HEAP_H
#define SEEN_INKSCAPE_DEBUG_GC_HEAP_H

#include "inkgc/gc-core.h"
#include "debug/heap.h"

namespace Inkscape {
namespace Debug {

class GCHeap : public Debug::Heap {
public:
    int features() const {
        return SIZE_AVAILABLE | USED_AVAILABLE | GARBAGE_COLLECTED;
    }
    Util::ptr_shared<char> name() const {
        return Util::share_static_string("libgc");
    }
    Heap::Stats stats() const {
        Stats stats;
        stats.size = GC::Core::get_heap_size();
        stats.bytes_used = stats.size - GC::Core::get_free_bytes();
        return stats;
    }
    void force_collect() { GC::Core::gcollect(); }
};

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

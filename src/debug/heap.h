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

#ifndef SEEN_INKSCAPE_DEBUG_HEAP_H
#define SEEN_INKSCAPE_DEBUG_HEAP_H

#include <cstddef>
#include "util/share.h"

namespace Inkscape {

namespace Debug {

class Heap {
public:
    virtual ~Heap() {}

    struct Stats {
        std::size_t size;
        std::size_t bytes_used;
    };

    enum {
        SIZE_AVAILABLE    = ( 1 << 0 ),
        USED_AVAILABLE    = ( 1 << 1 ),
        GARBAGE_COLLECTED = ( 1 << 2 )
    };

    virtual int features() const=0;

    virtual Util::ptr_shared<char> name() const=0;
    virtual Stats stats() const=0;
    virtual void force_collect()=0;
};

unsigned heap_count();
Heap *get_heap(unsigned i);

void register_extra_heap(Heap &heap);

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

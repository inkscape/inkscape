/*
 * Inkscape::Debug::SysVHeap - malloc() statistics via System V interface
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

#ifdef HAVE_MALLOC_H
# include <malloc.h>
#endif

#include "debug/sysv-heap.h"

namespace Inkscape {
namespace Debug {

int SysVHeap::features() const {
#ifdef HAVE_MALLINFO
    return SIZE_AVAILABLE | USED_AVAILABLE;
#else
    return 0;
#endif
}

Heap::Stats SysVHeap::stats() const {
    Stats stats = { 0, 0 };

#ifdef HAVE_MALLINFO
    struct mallinfo info=mallinfo();

#ifdef HAVE_STRUCT_MALLINFO_USMBLKS
    stats.size += info.usmblks;
    stats.bytes_used += info.usmblks;
#endif

#ifdef HAVE_STRUCT_MALLINFO_FSMBLKS
    stats.size += info.fsmblks;
#endif

#ifdef HAVE_STRUCT_MALLINFO_UORDBLKS
    stats.size += info.uordblks;
    stats.bytes_used += info.uordblks;
#endif

#ifdef HAVE_STRUCT_MALLINFO_FORDBLKS
    stats.size += info.fordblks;
#endif

#ifdef HAVE_STRUCT_MALLINFO_HBLKHD
    stats.size += info.hblkhd;
    stats.bytes_used += info.hblkhd;
#endif

#endif

    return stats;
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

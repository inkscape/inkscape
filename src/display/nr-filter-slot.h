#ifndef __NR_FILTER_SLOT_H__
#define __NR_FILTER_SLOT_H__

/*
 * A container class for filter slots. Allows for simple getting and
 * setting images in filter slots without having to bother with
 * table indexes and such.
 *
 * Author:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006 Niko Kiirala
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "libnr/nr-pixblock.h"

namespace NR {

class FilterSlot {
public:
    FilterSlot();
    FilterSlot(int slots);
    ~FilterSlot();

    NRPixBlock *get(int slot);
    void set(int slot, NRPixBlock *pb);

    int get_slot_count();

private:
    NRPixBlock **_slot;
    int *_slot_number;
    int _slot_count;

    int _last_out;

    int _get_index(int slot);
};

}

#endif // __NR_FILTER_SLOT_H__
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

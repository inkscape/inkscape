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
    /** Creates a new FilterSlot object, with two slots. */
    FilterSlot();
    /** Creates a new FilterSlot object, with specified amount of slots */
    FilterSlot(int slots);
    /** Destroys the FilterSlot object and all its contents */
    ~FilterSlot();

    /** Returns the pixblock in specified slot.
     * Parameter 'slot' may be either an positive integer or one of
     * pre-defined filter slot types: NR_FILTER_SLOT_NOT_SET,
     * NR_FILTER_SOURCEGRAPHIC, NR_FILTER_SOURCEALPHA,
     * NR_FILTER_BACKGROUNDIMAGE, NR_FILTER_BACKGROUNDALPHA,
     * NR_FILTER_FILLPAINT, NR_FILTER_SOURCEPAINT.
     * If the defined filter slot is not set before, this function
     * returns NULL. Also, that filter slot is created in process.
     */
    NRPixBlock *get(int slot);

    /** Sets or re-sets the pixblock associated with given slot.
     * If there was a pixblock already assigned with this slot,
     * that pixblock is destroyed.
     * Pixblocks passed to this function should be considered
     * managed by this FilterSlot object.
     * Pixblocks passed to this function should be reserved with
     * c++ -style new-operator.
     */
    void set(int slot, NRPixBlock *pb);

    /** Returns the number of slots in use. */
    int get_slot_count();

private:
    NRPixBlock **_slot;
    int *_slot_number;
    int _slot_count;

    int _last_out;

    /** Returns the table index of given slot. If that slot dows not exist,
     * it is created. Table index can be used to read the correct
     * pixblock from _slot */
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

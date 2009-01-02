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
 * Copyright (C) 2006,2007 Niko Kiirala
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "libnr/nr-pixblock.h"
#include "display/nr-filter-types.h"
#include "display/nr-filter-units.h"

struct NRArenaItem;

namespace Inkscape {
namespace Filters {

class FilterSlot {
public:
    /** Creates a new FilterSlot object.
     * First parameter specifies the amount of slots this SilterSlot
     * should reserve beforehand. If a negative number is given,
     * two slots will be reserved.
     * Second parameter specifies the arena item, which should be used
     * for background accesses from filters.
     */
    FilterSlot(int slots, NRArenaItem const *item);
    /** Destroys the FilterSlot object and all its contents */
    virtual ~FilterSlot();

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

    /** Gets the final result from this filter.
     * The result is fetched from the specified slot, see description of
     * method get for valid values. The pixblock 'result' will be modified
     * to contain the result image, ready to be used in the rest of rendering
     * pipeline
     */
    void get_final(int slot, NRPixBlock *result);

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

    /** arenaitem getter method*/
    NRArenaItem const* get_arenaitem();

    /** Sets the unit system to be used for the internal images. */
    void set_units(FilterUnits const &units);

    /** Sets the filtering quality. Affects used interpolation methods */
    void set_quality(FilterQuality const q);

private:
    NRPixBlock **_slot;
    int *_slot_number;
    int _slot_count;

    int _last_out;

    FilterQuality filterquality;

    NRArenaItem const *_arena_item;

    FilterUnits units;

    /** Returns the table index of given slot. If that slot does not exist,
     * it is created. Table index can be used to read the correct
     * pixblock from _slot */
    int _get_index(int slot);
};

} /* namespace Filters */
} /* namespace Inkscape */

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

#define __NR_FILTER_SLOT_CPP__

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

#include <assert.h>

#include "display/nr-arena-item.h"
#include "display/nr-filter-types.h"
#include "display/nr-filter-slot.h"
#include "libnr/nr-pixblock.h"
#include "libnr/nr-blit.h"

namespace NR {

FilterSlot::FilterSlot(int slots, NRArenaItem const *item)
{
    _slot_count = ((slots > 0) ? slots : 2);
    _slot = new NRPixBlock*[_slot_count];
    _slot_number = new int[_slot_count];

    for (int i = 0 ; i < _slot_count ; i++) {
        _slot[i] = NULL;
        _slot_number[i] = NR_FILTER_SLOT_NOT_SET;
    }

    _last_out = -1;

    _arena_item = item;
}

FilterSlot::~FilterSlot()
{
    for (int i = 0 ; i < _slot_count ; i++) {
        if (_slot[i]) {
            nr_pixblock_release(_slot[i]);
            delete _slot[i];
        }
    }
    delete[] _slot;
    delete[] _slot_number;
}

NRPixBlock *FilterSlot::get(int slot_nr)
{
    int index = _get_index(slot_nr);
    assert(index >= 0);

    /* If we didn't have the specified image, but we could create it
     * from the other information we have, let's do that */
    if (_slot[index] == NULL
        && (slot_nr == NR_FILTER_SOURCEALPHA
            || slot_nr == NR_FILTER_BACKGROUNDIMAGE
            || slot_nr == NR_FILTER_BACKGROUNDALPHA
            || slot_nr == NR_FILTER_FILLPAINT
            || slot_nr == NR_FILTER_STROKEPAINT))
    {
        /* If needed, fetch background */
        if (slot_nr == NR_FILTER_BACKGROUNDIMAGE
            || slot_nr == NR_FILTER_BACKGROUNDALPHA)
        {
            NRPixBlock *pb;
            pb = nr_arena_item_get_background(_arena_item);
            if (pb) {
                NRPixBlock *bg = new NRPixBlock;
                nr_pixblock_setup_fast(bg, pb->mode,
                                       pb->area.x0, pb->area.y0,
                                       pb->area.x1, pb->area.y1, true);
                bool empty = pb->empty;
                pb->empty = false;
                nr_blit_pixblock_pixblock(bg, pb);
                pb->empty = empty;
                bg->empty = false;
                this->set(NR_FILTER_BACKGROUNDIMAGE, bg);
            } else {
                NRPixBlock *source = this->get(NR_FILTER_SOURCEGRAPHIC);
                pb = new NRPixBlock();
                if (!pb) return NULL; // Allocation failed
                nr_pixblock_setup_fast(pb, source->mode,
                                       source->area.x0, source->area.y0,
                                       source->area.x1, source->area.y1, true);
                if (pb->size != NR_PIXBLOCK_SIZE_TINY && pb->data.px == NULL) {
                    // allocation failed
                    delete pb;
                    return NULL;
                }
                pb->empty = FALSE;
                this->set(NR_FILTER_BACKGROUNDIMAGE, pb);
            }
        }
        /* If only a alpha channel is needed, strip it from full image */
        if (slot_nr == NR_FILTER_SOURCEALPHA) {
            // TODO
        }
        if (slot_nr == NR_FILTER_BACKGROUNDALPHA) {
            // TODO
        }
        /* When a paint is needed, fetch it from arena item */
        if (slot_nr == NR_FILTER_FILLPAINT) {
            // TODO
        }
        if (slot_nr == NR_FILTER_STROKEPAINT) {
            // TODO
        }
    }

    assert(slot_nr == NR_FILTER_SLOT_NOT_SET ||_slot_number[index] == slot_nr);
    return _slot[index];
}

void FilterSlot::set(int slot_nr, NRPixBlock *pb)
{
    int index = _get_index(slot_nr);
    assert(index >= 0);
    assert(slot_nr == NR_FILTER_SLOT_NOT_SET ||_slot_number[index] == slot_nr);

    if(_slot[index]) {
        nr_pixblock_release(_slot[index]);
        delete _slot[index];
    }
    _slot[index] = pb;
    _last_out = index;
}

int FilterSlot::get_slot_count()
{
    int seek = _slot_count;
    do {
        seek--;
    } while (_slot[seek] == NULL);
    
    return seek + 1;
}

int FilterSlot::_get_index(int slot_nr)
{
    assert(slot_nr >= 0 ||
           slot_nr == NR_FILTER_SLOT_NOT_SET ||
           slot_nr == NR_FILTER_SOURCEGRAPHIC ||
           slot_nr == NR_FILTER_SOURCEALPHA ||
           slot_nr == NR_FILTER_BACKGROUNDIMAGE ||
           slot_nr == NR_FILTER_BACKGROUNDALPHA ||
           slot_nr == NR_FILTER_FILLPAINT ||
           slot_nr == NR_FILTER_STROKEPAINT);

    int index = -1;
    if (slot_nr == NR_FILTER_SLOT_NOT_SET) {
        return _last_out;
    }
    /* Search, if the slot already exists */
    for (int i = 0 ; i < _slot_count ; i++) {
        if (_slot_number[i] == slot_nr) {
            index = i;
            break;
        }
    }

    /* If the slot doesn't already exist, create it */
    if (index == -1) {
        int seek = _slot_count;
        do {
            seek--;
        } while (_slot[seek] == NULL && seek > 0);
        /* If there is no space for more slots, create more space */
        if (seek == _slot_count - 1) {
            NRPixBlock **new_slot = new NRPixBlock*[_slot_count * 2];
            int *new_number = new int[_slot_count * 2];
            for (int i = 0 ; i < _slot_count ; i++) {
                new_slot[i] = _slot[i];
                new_number[i] = _slot_number[i];
            }
            for (int i = _slot_count ; i < _slot_count * 2 ; i++) {
                new_slot[i] = NULL;
                new_number[i] = NR_FILTER_SLOT_NOT_SET;
            }
            delete[] _slot;
            delete[] _slot_number;
            _slot = new_slot;
            _slot_number = new_number;
            _slot_count *= 2;
        }
        /* Now that there is space, create the slot */
        _slot_number[seek + 1] = slot_nr;
        index = seek + 1;
    }
    return index;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

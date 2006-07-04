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
#include "libnr/nr-pixblock.h"
#include "display/nr-filter-types.h"
#include "display/nr-filter-slot.h"

namespace NR {

FilterSlot::FilterSlot()
{
    _slot_count = 2;
    _slot = new NRPixBlock*[_slot_count];
    _slot_number = new int[_slot_count];

    for (int i = 0 ; i < _slot_count ; i++) {
        _slot[i] = NULL;
        _slot_number[i] = NR_FILTER_SLOT_NOT_SET;
    }

    _last_out = -1;
}

FilterSlot::FilterSlot(int slots)
{
    _slot_count = slots;
    _slot = new NRPixBlock*[_slot_count];
    _slot_number = new int[_slot_count];

    for (int i = 0 ; i < _slot_count ; i++) {
        _slot[i] = NULL;
        _slot_number[i] = NR_FILTER_SLOT_NOT_SET;
    }

    _last_out = -1;
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
           slot_nr == NR_FILTER_SOURCEPAINT);

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
        } while (_slot[seek] == NULL);
        /* If there is no space for more slots, create more space */
        if (seek == _slot_count - 1) {
            NRPixBlock **new_slot = new NRPixBlock*[_slot_count * 2];
            int *new_number = new int[_slot_count * 2];
            for (int i = 0 ; i < _slot_count ; i++) {
                new_slot[i] = _slot[i];
                new_number[i] = _slot_number[i];
            }
            for (int i = _slot_count ; i < _slot_count * 2 ; i++) {
                _slot[i] = NULL;
                _slot_number[i] = NR_FILTER_SLOT_NOT_SET;
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

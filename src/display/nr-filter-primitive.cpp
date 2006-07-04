#define __NR_FILTER_PRIMITIVE_CPP__

/*
 * SVG filters rendering
 *
 * Author:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006 Niko Kiirala
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-primitive.h"
#include "display/nr-filter-types.h"
#include "libnr/nr-pixblock.h"
#include "svg/svg-length.h"

namespace NR {

FilterPrimitive::FilterPrimitive()
{
    _input = NR_FILTER_SLOT_NOT_SET;
    _output = NR_FILTER_SLOT_NOT_SET;

    _region_x.set(SVGLength::PERCENT, 0, 0);
    _region_y.set(SVGLength::PERCENT, 0, 0);
    _region_width.set(SVGLength::PERCENT, 100, 0);
    _region_height.set(SVGLength::PERCENT, 100, 0);
}

FilterPrimitive::~FilterPrimitive()
{
    // Nothing to do here
}

int FilterPrimitive::render(FilterSlot &slot, NRMatrix const *trans) {
    if(trans) {
        return this->render(slot, *trans);
    } else {
        Matrix tmp;
        tmp.set_identity();
        return this->render(slot, tmp);
    }
}

int FilterPrimitive::get_enlarge(Matrix const &m)
{
    return 0;
}

void FilterPrimitive::set_input(int slot) {
    set_input(0, slot);
}

void FilterPrimitive::set_input(int input, int slot) {
    if (slot == 0) _input = slot;
}

void FilterPrimitive::set_output(int slot) {
    if (slot >= 0) _output = slot;
}

} /* namespace NR */

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

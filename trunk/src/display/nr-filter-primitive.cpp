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

namespace Inkscape {
namespace Filters {

FilterPrimitive::FilterPrimitive()
{
    _input = NR_FILTER_SLOT_NOT_SET;
    _output = NR_FILTER_SLOT_NOT_SET;

    // These defaults are according to SVG standard.
    // NB: SVGLength.set takes prescaled percent values: 1 means 100%
    _region_x.set(SVGLength::PERCENT, 0, 0);
    _region_y.set(SVGLength::PERCENT, 0, 0);
    _region_width.set(SVGLength::PERCENT, 1, 0);
    _region_height.set(SVGLength::PERCENT, 1, 0);
}

FilterPrimitive::~FilterPrimitive()
{
    // Nothing to do here
}

void FilterPrimitive::area_enlarge(NRRectL &/*area*/, Geom::Matrix const &/*m*/)
{
    // This doesn't need to do anything by default
}

void FilterPrimitive::set_input(int slot) {
    set_input(0, slot);
}

void FilterPrimitive::set_input(int input, int slot) {
    if (input == 0) _input = slot;
}

void FilterPrimitive::set_output(int slot) {
    if (slot >= 0) _output = slot;
}

FilterTraits FilterPrimitive::get_input_traits() {
    return TRAIT_ANYTHING;
}

} /* namespace Filters */
} /* namespace Inkscape */

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

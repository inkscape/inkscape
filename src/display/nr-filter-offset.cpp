/*
 * feOffset filter primitive renderer
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-offset.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
#include "libnr/nr-blit.h"
#include "libnr/nr-pixblock.h"
#include "libnr/nr-rect-l.h"

namespace Inkscape {
namespace Filters {

using Geom::X;
using Geom::Y;

FilterOffset::FilterOffset() :
    dx(0), dy(0)
{}

FilterPrimitive * FilterOffset::create() {
    return new FilterOffset();
}

FilterOffset::~FilterOffset()
{}

int FilterOffset::render(FilterSlot &slot, FilterUnits const &units) {
    NRPixBlock *in = slot.get(_input);
    // Bail out if source image is missing
    if (!in) {
        g_warning("Missing source image for feOffset (in=%d)", _input);
        return 1;
    }

    NRPixBlock *out = new NRPixBlock;

    Geom::Matrix trans = units.get_matrix_primitiveunits2pb();
    Geom::Point offset(dx, dy);
    offset *= trans;
    offset[X] -= trans[4];
    offset[Y] -= trans[5];

    nr_pixblock_setup_fast(out, in->mode,
                           in->area.x0, in->area.y0, in->area.x1, in->area.y1,
                           true);
    nr_blit_pixblock_pixblock(out, in);

    out->area.x0 += static_cast<NR::ICoord>(offset[X]);
    out->area.y0 += static_cast<NR::ICoord>(offset[Y]);
    out->area.x1 += static_cast<NR::ICoord>(offset[X]);
    out->area.y1 += static_cast<NR::ICoord>(offset[Y]);
    out->visible_area = out->area;

    out->empty = FALSE;
    slot.set(_output, out);

    return 0;
}

void FilterOffset::set_dx(double amount) {
    dx = amount;
}

void FilterOffset::set_dy(double amount) {
    dy = amount;
}

void FilterOffset::area_enlarge(NRRectL &area, Geom::Matrix const &trans)
{
    Geom::Point offset(dx, dy);
    offset *= trans;
    offset[X] -= trans[4];
    offset[Y] -= trans[5];

    if (offset[X] > 0) {
        area.x0 -= static_cast<NR::ICoord>(offset[X]);
    } else {
        area.x1 -= static_cast<NR::ICoord>(offset[X]);
    }

    if (offset[Y] > 0) {
        area.y0 -= static_cast<NR::ICoord>(offset[Y]);
    } else {
        area.y1 -= static_cast<NR::ICoord>(offset[Y]);
    }
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

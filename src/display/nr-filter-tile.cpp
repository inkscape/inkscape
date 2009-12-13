/*
 * feTile filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-tile.h"
#include "display/nr-filter-units.h"

namespace Inkscape {
namespace Filters {

FilterTile::FilterTile()
{
    g_warning("FilterTile::render not implemented.");
}

FilterPrimitive * FilterTile::create() {
    return new FilterTile();
}

FilterTile::~FilterTile()
{}

int FilterTile::render(FilterSlot &slot, FilterUnits const &/*units*/) {
    NRPixBlock *in = slot.get(_input);
    if (!in) {
        g_warning("Missing source image for feTile (in=%d)", _input);
        return 1;
    }

    NRPixBlock *out = new NRPixBlock;

    nr_pixblock_setup_fast(out, in->mode,
                           in->area.x0, in->area.y0, in->area.x1, in->area.y1,
                           true);

    unsigned char *in_data = NR_PIXBLOCK_PX(in);
    unsigned char *out_data = NR_PIXBLOCK_PX(out);

//IMPLEMENT ME!
    g_warning("Renderer for feTile is not implemented.");
    (void)in_data;
    (void)out_data;

    out->empty = FALSE;
    slot.set(_output, out);
    return 0;
}

void FilterTile::area_enlarge(NRRectL &/*area*/, Geom::Matrix const &/*trans*/)
{
}

FilterTraits FilterTile::get_input_traits() {
    return TRAIT_PARALLER;
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

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

#include <glib.h>
#include "display/nr-filter-tile.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"

namespace Inkscape {
namespace Filters {

FilterTile::FilterTile()
{
}

FilterPrimitive * FilterTile::create() {
    return new FilterTile();
}

FilterTile::~FilterTile()
{}

void FilterTile::render_cairo(FilterSlot &slot)
{
    static bool tile_warning = false;

//IMPLEMENT ME!
    if (!tile_warning) {
        g_warning("Renderer for feTile is not implemented.");
        tile_warning = true;
    }

    cairo_surface_t *in = slot.getcairo(_input);
    slot.set(_output, in);
}

double FilterTile::complexity(Geom::Affine const &)
{
    return 1.0;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

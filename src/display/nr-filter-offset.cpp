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

#include "display/cairo-utils.h"
#include "display/nr-filter-offset.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
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

void FilterOffset::render_cairo(FilterSlot &slot)
{
    cairo_surface_t *in = slot.getcairo(_input);
    cairo_surface_t *out = ink_cairo_surface_create_identical(in);
    cairo_t *ct = cairo_create(out);

    Geom::Affine trans = slot.get_units().get_matrix_primitiveunits2pb();
    Geom::Point offset(dx, dy);
    offset *= trans;
    offset[X] -= trans[4];
    offset[Y] -= trans[5];

    cairo_set_source_surface(ct, in, offset[X], offset[Y]);
    cairo_paint(ct);
    cairo_destroy(ct);

    slot.set(_output, out);
    cairo_surface_destroy(out);
}

bool FilterOffset::can_handle_affine(Geom::Affine const &)
{
    return true;
}

void FilterOffset::set_dx(double amount) {
    dx = amount;
}

void FilterOffset::set_dy(double amount) {
    dy = amount;
}

void FilterOffset::area_enlarge(NRRectL &area, Geom::Affine const &trans)
{
    Geom::Point offset(dx, dy);
    offset *= trans;
    offset[X] -= trans[4];
    offset[Y] -= trans[5];

    if (offset[X] > 0) {
        area.x0 -= ceil(offset[X]);
    } else {
        area.x1 -= floor(offset[X]);
    }

    if (offset[Y] > 0) {
        area.y0 -= ceil(offset[Y]);
    } else {
        area.y1 -= floor(offset[Y]);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

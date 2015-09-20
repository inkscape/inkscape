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
    // color_interpolation_filters for out same as in. See spec (DisplacementMap).
    copy_cairo_surface_ci(in, out);
    cairo_t *ct = cairo_create(out);

    Geom::Rect vp = filter_primitive_area( slot.get_units() );
    slot.set_primitive_area(_output, vp); // Needed for tiling

    // Handle bounding box case
    double x = dx;
    double y = dy;
    if( slot.get_units().get_primitive_units() == SP_FILTER_UNITS_OBJECTBOUNDINGBOX ) {
        Geom::OptRect bbox = slot.get_units().get_item_bbox();
        if( bbox ) {
            x *= (*bbox).width();
            y *= (*bbox).height();
        }
    }

    Geom::Affine trans = slot.get_units().get_matrix_user2pb();

    Geom::Point offset(x, y);
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

void FilterOffset::area_enlarge(Geom::IntRect &area, Geom::Affine const &trans)
{
    Geom::Point offset(dx, dy);
    offset *= trans;
    offset[X] -= trans[4];
    offset[Y] -= trans[5];
    double x0, y0, x1, y1;
    x0 = area.left();
    y0 = area.top();
    x1 = area.right();
    y1 = area.bottom();

    if (offset[X] > 0) {
        x0 -= ceil(offset[X]);
    } else {
        x1 -= floor(offset[X]);
    }

    if (offset[Y] > 0) {
        y0 -= ceil(offset[Y]);
    } else {
        y1 -= floor(offset[Y]);
    }
    area = Geom::IntRect(x0, y0, x1, y1);
}

double FilterOffset::complexity(Geom::Affine const &)
{
    return 1.02;
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

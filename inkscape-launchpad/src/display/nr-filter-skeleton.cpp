/*
 * Filter primitive renderer skeleton class
 * You can create your new filter primitive renderer by replacing
 * all occurences of Skeleton, skeleton and SKELETON with your filter
 * type, like gaussian or blend in respective case.
 *
 * This can be accomplished with the following sed command:
 * sed -e "s/Skeleton/Name/g" -e "s/skeleton/name/" -e "s/SKELETON/NAME/"
 * nr-filter-skeleton.cpp >nr-filter-name.cpp
 *
 * (on one line, replace occurences of 'name' with your filter name)
 *
 * Remember to convert the .h file too. The sed command is same for both
 * files.
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-skeleton.h"
#include "display/cairo-utils.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"

namespace Inkscape {
namespace Filters {

FilterSkeleton::FilterSkeleton() 
{}

FilterPrimitive * FilterSkeleton::create() {
    return new FilterSkeleton();
}

FilterSkeleton::~FilterSkeleton()
{}

void FilterSkeleton::render_cairo(FilterSlot &slot) {
    cairo_surface_t *in = slot.getcairo(_input);
    cairo_surface_t *out = ink_cairo_surface_create_identical(in);

//    cairo_t *ct = cairo_create(out);

//    cairo_set_source_surface(ct, in, offset[X], offset[Y]);
//    cairo_paint(ct);
//    cairo_destroy(ct);

    slot.set(_output, out);
    cairo_surface_destroy(out);
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

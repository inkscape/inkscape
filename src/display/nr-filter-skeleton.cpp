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
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
#include "libnr/nr-pixblock.h"

namespace Inkscape {
namespace Filters {

FilterSkeleton::FilterSkeleton() 
{}

FilterPrimitive * FilterSkeleton::create() {
    return new FilterSkeleton();
}

FilterSkeleton::~FilterSkeleton()
{}

int FilterSkeleton::render(FilterSlot &slot,
                           FilterUnits const &/*units*/) {
    //NRPixBlock *in = slot.get(_input);
    NRPixBlock *out = new NRPixBlock();

    /* Insert rendering code here */

    out->empty = FALSE;
    slot.set(_output, out);

    return 0;
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

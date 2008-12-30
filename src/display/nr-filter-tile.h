#ifndef __NR_FILTER_TILE_H__
#define __NR_FILTER_TILE_H__

/*
 * feTile filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-primitive.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
#include "libnr/nr-rect-l.h"

namespace NR {

class FilterTile : public FilterPrimitive {
public:
    FilterTile();
    static FilterPrimitive *create();
    virtual ~FilterTile();

    virtual int render(FilterSlot &slot, FilterUnits const &units);
    virtual void area_enlarge(NRRectL &area, Geom::Matrix const &trans);
    virtual FilterTraits get_input_traits();
};

} /* namespace NR */

#endif /* __NR_FILTER_TILE_H__ */
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

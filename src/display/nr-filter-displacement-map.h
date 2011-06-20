#ifndef __NR_FILTER_DISPLACEMENT_MAP_H__
#define __NR_FILTER_DISPLACEMENT_MAP_H__

/*
 * feDisplacementMap filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "filters/displacementmap.h"
#include "display/nr-filter-primitive.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
#include "libnr/nr-rect-l.h"

namespace Inkscape {
namespace Filters {

class FilterDisplacementMap : public FilterPrimitive {
public:
    FilterDisplacementMap();
    static FilterPrimitive *create();
    virtual ~FilterDisplacementMap();

    virtual void set_input(int slot);
    virtual void set_input(int input, int slot);
    virtual void set_scale(double s);
    virtual void set_channel_selector(int s, FilterDisplacementMapChannelSelector channel);
    virtual void render_cairo(FilterSlot &slot);
    virtual void area_enlarge(NRRectL &area, Geom::Affine const &trans);

private:
    double scale;
    int _input2;
    unsigned Xchannel, Ychannel;
};

} /* namespace Filters */
} /* namespace Inkscape */

#endif /* __NR_FILTER_DISPLACEMENT_MAP_H__ */
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

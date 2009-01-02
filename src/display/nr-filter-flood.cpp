/*
 * feFlood filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-flood.h"
#include "display/nr-filter-utils.h"

namespace Inkscape {
namespace Filters {

FilterFlood::FilterFlood()
{}

FilterPrimitive * FilterFlood::create() {
    return new FilterFlood();
}

FilterFlood::~FilterFlood()
{}

int FilterFlood::render(FilterSlot &slot, FilterUnits const &/*units*/) {
    NRPixBlock *in = slot.get(_input);
    if (!in) {
        g_warning("Missing source image for feFlood (in=%d)", _input);
        return 1;
    }

    int i;
    int in_w = in->area.x1 - in->area.x0;
    int in_h = in->area.y1 - in->area.y0;
 
    NRPixBlock *out = new NRPixBlock;

    nr_pixblock_setup_fast(out, NR_PIXBLOCK_MODE_R8G8B8A8N,
                           in->area.x0, in->area.y0, in->area.x1, in->area.y1,
                           true);

    unsigned char *out_data = NR_PIXBLOCK_PX(out);

    unsigned char r,g,b,a;
    r = CLAMP_D_TO_U8((color >> 24) % 256);
    g = CLAMP_D_TO_U8((color >> 16) % 256);
    b = CLAMP_D_TO_U8((color >>  8) % 256);
    a = CLAMP_D_TO_U8(opacity*255);

    for(i=0; i < 4*in_h*in_w; i+=4){
            out_data[i]=r;
            out_data[i+1]=g;
            out_data[i+2]=b;
            out_data[i+3]=a;
    }

    out->empty = FALSE;
    slot.set(_output, out);
    return 0;
}

void FilterFlood::set_color(guint32 c) {
    color = c;
}

void FilterFlood::set_opacity(double o) {
    opacity = o;
}

void FilterFlood::area_enlarge(NRRectL &/*area*/, Geom::Matrix const &/*trans*/)
{
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

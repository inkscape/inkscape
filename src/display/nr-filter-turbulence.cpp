/*
 * feTurbulence filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com> 
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-turbulence.h"

namespace NR {

FilterTurbulence::FilterTurbulence()
: XbaseFrequency(0),
  YbaseFrequency(0),
  numOctaves(1),
  seed(0)
{
    g_warning("FilterTurbulence::render not implemented.");
}

FilterPrimitive * FilterTurbulence::create() {
    return new FilterTurbulence();
}

FilterTurbulence::~FilterTurbulence()
{}

void FilterTurbulence::set_baseFrequency(int axis, double freq){
    if (axis==0) XbaseFrequency=freq;
    if (axis==1) YbaseFrequency=freq;
}

void FilterTurbulence::set_numOctaves(int num){
    numOctaves=num;
}

void FilterTurbulence::set_seed(double s){
    seed=s;
}

void FilterTurbulence::set_stitchTiles(bool st){
    stitchTiles=st;
}

void FilterTurbulence::set_type(int t){
    type=t;
}


int FilterTurbulence::render(FilterSlot &slot, Matrix const &trans) {
/* TODO: Implement this renderer method.
        Specification: http://www.w3.org/TR/SVG11/filters.html#feTurbulence

*/

/*debug: these are the available parameters
    printf("XbaseFrequency = %f; ", XbaseFrequency);
    printf("YbaseFrequency = %f; ", YbaseFrequency);
    printf("numOctaves = %d;\n", numOctaves);
    printf("seed = %f; ", seed);
    printf("stitchTiles = %s; ", stitchTiles ? "stitch" : "noStitch");
    printf("type = %s;\n\n", type==0 ? "FractalNoise" : "turbulence");
*/

//sample code: the following fills the whole area in semi-transparent red.    
    NRPixBlock *in = slot.get(_input);
    NRPixBlock *out = new NRPixBlock;
    int x,y;
    int x0 = in->area.x0, y0 = in->area.y0;
    int x1 = in->area.x1, y1 = in->area.y1;
    int w = x1 - x0;
    nr_pixblock_setup_fast(out, in->mode, x0, y0, x1, y1, true);

    unsigned char *out_data = NR_PIXBLOCK_PX(out);
    for (x=x0; x < x1; x++){
        for (y=y0; y < y1; y++){
            out_data[4*((x - x0)+w*(y - y0)) + 0] = 255;
            out_data[4*((x - x0)+w*(y - y0)) + 1] = 0;
            out_data[4*((x - x0)+w*(y - y0)) + 2] = 0;
            out_data[4*((x - x0)+w*(y - y0)) + 3] = 128;
        }
    }

    out->empty = FALSE;
    slot.set(_output, out);
    return 0;
}

} /* namespace NR */

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

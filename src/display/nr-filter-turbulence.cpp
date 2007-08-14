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
 
#include "display/nr-arena-item.h"
#include "display/nr-filter.h"
#include "display/nr-filter-turbulence.h"

namespace NR {

FilterTurbulence::FilterTurbulence()
: XbaseFrequency(0),
  YbaseFrequency(0),
  numOctaves(1),
  seed(0),
  updated(false),
  pix(NULL)
{
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

void FilterTurbulence::set_type(FilterTurbulenceType t){
    type=t;
}

void FilterTurbulence::set_updated(bool u){
    updated=u;
}

void FilterTurbulence::update_pixbuffer(FilterSlot &slot) {
    int bbox_x0 = (int) slot.get_arenaitem()->bbox.x0;
    int bbox_y0 = (int) slot.get_arenaitem()->bbox.y0;
    int bbox_x1 = (int) slot.get_arenaitem()->bbox.x1;
    int bbox_y1 = (int) slot.get_arenaitem()->bbox.y1;

    int w = bbox_x1 - bbox_x0;
    int h = bbox_y1 - bbox_y0;    
    int x,y;

    if (!pix){
        pix = new NRPixBlock;
        nr_pixblock_setup_fast(pix, NR_PIXBLOCK_MODE_R8G8B8A8P, bbox_x0, bbox_y0, bbox_x1, bbox_y1, true);
        pix_data = NR_PIXBLOCK_PX(pix);
    }
    
//  TODO: implement here the turbulence rendering.

/*debug: these are the available parameters
    printf("XbaseFrequency = %f; ", XbaseFrequency);
    printf("YbaseFrequency = %f; ", YbaseFrequency);
    printf("numOctaves = %d;\n", numOctaves);
    printf("seed = %f; ", seed);
    printf("stitchTiles = %s; ", stitchTiles ? "stitch" : "noStitch");
    printf("type = %s;\n\n", type==0 ? "FractalNoise" : "turbulence");
*/

    for (x=0; x < w; x++){
        for (y=0; y < h; y++){
            pix_data[4*(x + w*y)] = (unsigned char)(int(XbaseFrequency)%256);
            pix_data[4*(x + w*y) + 1] = (unsigned char)(int(YbaseFrequency)%256);
            pix_data[4*(x + w*y) + 2] = (unsigned char)(int(numOctaves)%256);
            pix_data[4*(x + w*y) + 3] = (unsigned char)(int(seed)%256);
        }
    }
    updated=true;
}

int FilterTurbulence::render(FilterSlot &slot, Matrix const &trans) {
    if (!updated) update_pixbuffer(slot);
    
    NRPixBlock *in = slot.get(_input);
    NRPixBlock *out = new NRPixBlock;
    int x,y;
    int x0 = in->area.x0, y0 = in->area.y0;
    int x1 = in->area.x1, y1 = in->area.y1;
    int w = x1 - x0;
    nr_pixblock_setup_fast(out, in->mode, x0, y0, x1, y1, true);

    int bbox_x0 = (int) slot.get_arenaitem()->bbox.x0;
    int bbox_y0 = (int) slot.get_arenaitem()->bbox.y0;

    unsigned char *out_data = NR_PIXBLOCK_PX(out);
    for (x=x0; x < x1; x++){
        for (y=y0; y < y1; y++){
            out_data[4*((x - x0)+w*(y - y0))] = pix_data[x - bbox_x0 + w*(y - bbox_y0)];
            out_data[4*((x - x0)+w*(y - y0)) + 1] = pix_data[x - bbox_x0 + w*(y - bbox_y0)];
            out_data[4*((x - x0)+w*(y - y0)) + 2] = pix_data[x - bbox_x0 + w*(y - bbox_y0)];
            out_data[4*((x - x0)+w*(y - y0)) + 3] = pix_data[x - bbox_x0 + w*(y - bbox_y0)];
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

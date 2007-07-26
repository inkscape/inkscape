/*
 * feDisplacementMap filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-displacement-map.h"
#include "display/nr-filter-types.h"
#include "libnr/nr-pixops.h"

namespace NR {

FilterDisplacementMap::FilterDisplacementMap()
: scale(0),
  _input2(NR_FILTER_SLOT_NOT_SET),
   Xchannel(3),
  Ychannel(3)
{}

FilterPrimitive * FilterDisplacementMap::create() {
    return new FilterDisplacementMap();
}

FilterDisplacementMap::~FilterDisplacementMap()
{}

int FilterDisplacementMap::render(FilterSlot &slot, Matrix const &trans) {
    g_warning("FIX-ME: FilterDisplacementMap::render method is still a bit buggy. Needs Love.");
    
    NRPixBlock *texture = slot.get(_input);
    NRPixBlock *map = slot.get(_input2);
    NRPixBlock *out = new NRPixBlock;
    
    // Bail out if either one of source images is missing
    if (!map || !texture) {
        g_warning("Missing source image for feDisplacementMap (map=%d texture=%d)", _input, _input2);
        return 1;
    }
    
    nr_pixblock_setup_fast(out, map->mode,
                           map->area.x0, map->area.y0, map->area.x1, map->area.y1,
                           true);

    unsigned char *map_data = NR_PIXBLOCK_PX(map);
    unsigned char *texture_data = NR_PIXBLOCK_PX(texture);
    unsigned char *out_data = NR_PIXBLOCK_PX(out);
    int x, y, x0, y0, x1, y1, width;
    double coordx, coordy;
//    unsigned int alpha; //used for demultiplication
    
    x0 = out->area.x0;
    y0 = out->area.y0;
    x1 = out->area.x1;
    y1 = out->area.y1;
    width = x1 - x0;
   
    for (x=x0 + scale/2; x < x1 - scale/2; x++){
        for (y=y0 + scale/2; y < y1 - scale/2; y++){
/* SVG spec states that pixel values must be alpha-demultiplied before processing this filter operation.
The following code does it, but when we DON'T do it, output is more similar to output from Batik.

Batik output:
 http://bighead.poli.usp.br/~juca/code/inkscape/batik-fed02.png
Inkscape output without demultiplication:
 http://bighead.poli.usp.br/~juca/code/inkscape/displacement-map-test.png
 
 There is also this other bug that can be seen in the above screenshot: the lower and the right portions are not rendered, I dont know why.

 --JucaBlues
  
            if (map->mode == NR_PIXBLOCK_MODE_R8G8B8A8P){
                alpha = (unsigned int) map_data[4*((x-x0) + width*(y-y0)) + 3];
                if (alpha==0){
                    coordx = x-x0;
                    coordy = y-y0;
                } else {
                    coordx = x-x0 + scale * ( ((double)NR_DEMUL_111( (unsigned int)map_data[4*((x-x0) + width*(y-y0)) + Xchannel], alpha))/255 - 0.5 );
                    coordy = y-y0 + scale * ( ((double)NR_DEMUL_111( (unsigned int)map_data[4*((x-x0) + width*(y-y0)) + Ychannel], alpha))/255 - 0.5 );
                }
            } else {*/
            coordx = x-x0 + scale * ( ((double) map_data[4*((x-x0) + width*(y-y0)) + Xchannel])/255 - 0.5 );
            coordy = y-y0 + scale * ( ((double) map_data[4*((x-x0) + width*(y-y0)) + Ychannel])/255 - 0.5 );

            out_data[4*((x-x0) + width*(y-y0))] = texture_data[4*((int)coordx + ((int)coordy)*width)];
            out_data[4*((x-x0) + width*(y-y0)) + 1] = texture_data[4*((int)coordx + ((int)coordy)*width) + 1];
            out_data[4*((x-x0) + width*(y-y0)) + 2] = texture_data[4*((int)coordx + ((int)coordy)*width) + 2];
            out_data[4*((x-x0) + width*(y-y0)) + 3] = texture_data[4*((int)coordx + ((int)coordy)*width) + 3];
        }
    }

    out->empty = FALSE;
    slot.set(_output, out);
            return 0;
}

void FilterDisplacementMap::set_input(int slot) {
    _input = slot;
}

void FilterDisplacementMap::set_scale(int s) {
    scale = s;
}

void FilterDisplacementMap::set_input(int input, int slot) {
    if (input == 0) _input = slot;
    if (input == 1) _input2 = slot;
}

void FilterDisplacementMap::set_channel_selector(int s, int channel) {
    if (s == 0) Xchannel = channel;
    if (s == 1) Ychannel = channel;
}

void FilterDisplacementMap::area_enlarge(NRRectL &area, Matrix const &trans)
{
    //I'm in doubt whether this affects all input buffers or only 'in'
    area.x0 -= scale/2;
    area.y0 -= scale/2;
    area.x1 += scale/2;
    area.y1 += scale/2;
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

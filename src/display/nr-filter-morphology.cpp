/*
 * feMorphology filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-morphology.h"
#include "display/nr-filter-units.h"
#include "libnr/nr-blit.h"

namespace NR {

FilterMorphology::FilterMorphology()
{
}

FilterPrimitive * FilterMorphology::create() {
    return new FilterMorphology();
}

FilterMorphology::~FilterMorphology()
{}

int FilterMorphology::render(FilterSlot &slot, FilterUnits const &units) {
    NRPixBlock *in = slot.get(_input);
    if (!in) {
        g_warning("Missing source image for feMorphology (in=%d)", _input);
        return 1;
    }

    NRPixBlock *out = new NRPixBlock;

    // this primitive is defined for premultiplied RGBA values,
    // thus convert them to that format
    bool free_in_on_exit = false;
    if (in->mode != NR_PIXBLOCK_MODE_R8G8B8A8P) {
        NRPixBlock *original_in = in;
        in = new NRPixBlock;
        nr_pixblock_setup_fast(in, NR_PIXBLOCK_MODE_R8G8B8A8P,
                               original_in->area.x0, original_in->area.y0,
                               original_in->area.x1, original_in->area.y1,
                               true);
        nr_blit_pixblock_pixblock(in, original_in);
        free_in_on_exit = true;
    }

    Geom::Matrix p2pb = units.get_matrix_primitiveunits2pb();
    int const xradius = (int)round(this->xradius * p2pb.expansionX());
    int const yradius = (int)round(this->yradius * p2pb.expansionY());

    int x0=in->area.x0;
    int y0=in->area.y0;
    int x1=in->area.x1;
    int y1=in->area.y1;
    int w=x1-x0, h=y1-y0;
    int x, y, i, j;
    int rmax,gmax,bmax,amax;
    int rmin,gmin,bmin,amin;

    nr_pixblock_setup_fast(out, in->mode, x0, y0, x1, y1, true);

    unsigned char *in_data = NR_PIXBLOCK_PX(in);
    unsigned char *out_data = NR_PIXBLOCK_PX(out);

    for(x = 0 ; x < w ; x++){
        for(y = 0 ; y < h ; y++){
            rmin = gmin = bmin = amin = 255;
            rmax = gmax = bmax = amax = 0;
            for(i = x - xradius ; i < x + xradius ; i++){
                if (i < 0 || i >= w) continue;
                for(j = y - yradius ; j < y + yradius ; j++){
                    if (j < 0 || j >= h) continue;
                    if(in_data[4*(i + w*j)]>rmax) rmax = in_data[4*(i + w*j)];
                    if(in_data[4*(i + w*j)+1]>gmax) gmax = in_data[4*(i + w*j)+1];
                    if(in_data[4*(i + w*j)+2]>bmax) bmax = in_data[4*(i + w*j)+2];
                    if(in_data[4*(i + w*j)+3]>amax) amax = in_data[4*(i + w*j)+3];

                    if(in_data[4*(i + w*j)]<rmin) rmin = in_data[4*(i + w*j)];
                    if(in_data[4*(i + w*j)+1]<gmin) gmin = in_data[4*(i + w*j)+1];
                    if(in_data[4*(i + w*j)+2]<bmin) bmin = in_data[4*(i + w*j)+2];
                    if(in_data[4*(i + w*j)+3]<amin) amin = in_data[4*(i + w*j)+3];
                }
            }
            if (Operator==MORPHOLOGY_OPERATOR_DILATE){
                out_data[4*(x + w*y)]=rmax;
                out_data[4*(x + w*y)+1]=gmax;
                out_data[4*(x + w*y)+2]=bmax;
                out_data[4*(x + w*y)+3]=amax;
            } else {
                out_data[4*(x + w*y)]=rmin;
                out_data[4*(x + w*y)+1]=gmin;
                out_data[4*(x + w*y)+2]=bmin;
                out_data[4*(x + w*y)+3]=amin;
            }
        }
    }

    if (free_in_on_exit) {
        nr_pixblock_release(in);
        delete in;
    }

    out->empty = FALSE;
    slot.set(_output, out);
    return 0;
}

void FilterMorphology::area_enlarge(NRRectL &area, Geom::Matrix const &trans)
{
    int const enlarge_x = (int)round(this->xradius * trans.expansionX());
    int const enlarge_y = (int)round(this->yradius * trans.expansionY());

    area.x0 -= enlarge_x;
    area.x1 += enlarge_x;
    area.y0 -= enlarge_y;
    area.y1 += enlarge_y;
}

void FilterMorphology::set_operator(FilterMorphologyOperator &o){
    Operator = o;
}

void FilterMorphology::set_xradius(double x){
    xradius = x;
}

void FilterMorphology::set_yradius(double y){
    yradius = y;
}

FilterTraits FilterMorphology::get_input_traits() {
    return TRAIT_PARALLER;
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

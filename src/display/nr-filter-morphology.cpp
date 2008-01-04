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

namespace NR {

FilterMorphology::FilterMorphology()
{
}

FilterPrimitive * FilterMorphology::create() {
    return new FilterMorphology();
}

FilterMorphology::~FilterMorphology()
{}

int FilterMorphology::render(FilterSlot &slot, FilterUnits const &/*units*/) {
    NRPixBlock *in = slot.get(_input);
    if (!in) {
        g_warning("Missing source image for feMorphology (in=%d)", _input);
        return 1;
    }

    NRPixBlock *out = new NRPixBlock;

    int x0=in->area.x0;
    int y0=in->area.y0;
    int x1=in->area.x1;
    int y1=in->area.y1;
    int w=x1-x0, h=y1-y0;
    int x,y,i,j;
    int rmax,gmax,bmax,amax;
    int rmin,gmin,bmin,amin;

    nr_pixblock_setup_fast(out, in->mode, x0, y0, x1, y1, true);

    unsigned char *in_data = NR_PIXBLOCK_PX(in);
    unsigned char *out_data = NR_PIXBLOCK_PX(out);

    for(x=xradius; x<w-xradius; x++){
        for(y=yradius; y<h-yradius; y++){
            rmin=rmax=in_data[4*(x + w*y)];
            gmin=gmax=in_data[4*(x + w*y)+1];
            bmin=bmax=in_data[4*(x + w*y)+2];
            amin=amax=in_data[4*(x + w*y)+3];
            for(i=x-xradius;i<x+xradius;i++){
                for(j=y-yradius;j<y+yradius;j++){
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

    out->empty = FALSE;
    slot.set(_output, out);
    return 0;
}

void FilterMorphology::area_enlarge(NRRectL &area, Matrix const &/*trans*/)
{
    area.x0-=xradius;
    area.x1+=xradius;
    area.y0-=yradius;
    area.y1+=yradius;
}

void FilterMorphology::set_operator(FilterMorphologyOperator &o){
    Operator = o;
}

void FilterMorphology::set_xradius(int x){
    xradius = x;
}

void FilterMorphology::set_yradius(int y){
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

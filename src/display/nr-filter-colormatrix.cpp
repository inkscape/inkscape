/*
 * feColorMatrix filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-colormatrix.h"
namespace NR {

FilterColorMatrix::FilterColorMatrix()
{
    g_warning("FilterColorMatrix::render not implemented.");
}

FilterPrimitive * FilterColorMatrix::create() {
    return new FilterColorMatrix();
}

FilterColorMatrix::~FilterColorMatrix()
{}

int FilterColorMatrix::render(FilterSlot &slot, Matrix const &trans) {
    NRPixBlock *in = slot.get(_input);
    NRPixBlock *out = new NRPixBlock;

    nr_pixblock_setup_fast(out, in->mode,
                           in->area.x0, in->area.y0, in->area.x1, in->area.y1,
                           true);

    unsigned char *in_data = NR_PIXBLOCK_PX(in);
    unsigned char *out_data = NR_PIXBLOCK_PX(out);

//IMPLEMENT ME!
        printf("type = %d\n", type);
        if (type==0){
            for (int i=0;i<20;i++){
                printf("values[%d]=%f\n", i, values[i]);
            }
        } else {
                printf("value = %f\n", value);
        }
        
    out->empty = FALSE;
    slot.set(_output, out);
    return 0;
}

void FilterColorMatrix::area_enlarge(NRRectL &area, Matrix const &trans)
{
}

void FilterColorMatrix::set_type(int t){
        type = t;
}

void FilterColorMatrix::set_value(gdouble v){
        value = v;
}

void FilterColorMatrix::set_values(std::vector<gdouble> v){
        values = v;
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

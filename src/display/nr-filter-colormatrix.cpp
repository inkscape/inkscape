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
#include "display/nr-filter-units.h"
#include "display/nr-filter-utils.h"
#include "libnr/nr-blit.h"
#include <math.h>

namespace NR {

FilterColorMatrix::FilterColorMatrix()
{
}

FilterPrimitive * FilterColorMatrix::create() {
    return new FilterColorMatrix();
}

FilterColorMatrix::~FilterColorMatrix()
{}

int FilterColorMatrix::render(FilterSlot &slot, FilterUnits const &/*units*/) {
    NRPixBlock *in = slot.get(_input);
    if (!in) {
        g_warning("Missing source image for feColorMatrix (in=%d)", _input);
        return 1;
    }

    NRPixBlock *out = new NRPixBlock;

    nr_pixblock_setup_fast(out, NR_PIXBLOCK_MODE_R8G8B8A8N,
                           in->area.x0, in->area.y0, in->area.x1, in->area.y1,
                           true);

    // this primitive is defined for non-premultiplied RGBA values,
    // thus convert them to that format
    bool free_in_on_exit = false;
    if (in->mode != NR_PIXBLOCK_MODE_R8G8B8A8N) {
        NRPixBlock *original_in = in;
        in = new NRPixBlock;
        nr_pixblock_setup_fast(in, NR_PIXBLOCK_MODE_R8G8B8A8N,
                               original_in->area.x0, original_in->area.y0,
                               original_in->area.x1, original_in->area.y1,
                               true);
        nr_blit_pixblock_pixblock(in, original_in);
        free_in_on_exit = true;
    }

    unsigned char *in_data = NR_PIXBLOCK_PX(in);
    unsigned char *out_data = NR_PIXBLOCK_PX(out);
    unsigned char r,g,b,a;
    int x,y,x0,y0,x1,y1,i;
    double a00,a01,a02,a10,a11,a12,a20,a21,a22, coshue, sinhue;
    x0=in->area.x0;
    y0=in->area.y0;
    x1=in->area.x1;
    y1=in->area.y1;

    switch(type){
        case COLORMATRIX_MATRIX:
            if (values.size()!=20) {
                g_warning("ColorMatrix: values parameter error. Wrong size: %i.", static_cast<int>(values.size()));
                return -1;
            }
            for (x=x0;x<x1;x++){
                for (y=y0;y<y1;y++){
                    i = ((x-x0) + (x1-x0)*(y-y0))*4;
                    r = in_data[i];
                    g = in_data[i+1];
                    b = in_data[i+2];
                    a = in_data[i+3];
                    out_data[i] = CLAMP_D_TO_U8( r*values[0] + g*values[1] + b*values[2] + a*values[3] + 255*values[4] );
                    out_data[i+1] = CLAMP_D_TO_U8( r*values[5] + g*values[6] + b*values[7] + a*values[8] + 255*values[9] );
                    out_data[i+2] = CLAMP_D_TO_U8( r*values[10] + g*values[11] + b*values[12] + a*values[13] + 255*values[14] );
                    out_data[i+3] = CLAMP_D_TO_U8( r*values[15] + g*values[16] + b*values[17] + a*values[18] + 255*values[19] );
                }
            }
            break;
        case COLORMATRIX_SATURATE:
            for (x=x0;x<x1;x++){
                for (y=y0;y<y1;y++){
                    i = ((x-x0) + (x1-x0)*(y-y0))*4;
                    r = in_data[i];
                    g = in_data[i+1];
                    b = in_data[i+2];
                    a = in_data[i+3];
                    out_data[i] = CLAMP_D_TO_U8( r*(0.213+0.787*value) + g*(0.715-0.715*value) + b*(0.072-0.072*value) );
                    out_data[i+1] = CLAMP_D_TO_U8( r*(0.213-0.213*value) + g*(0.715+0.285*value) + b*(0.072-0.072*value) );
                    out_data[i+2] = CLAMP_D_TO_U8( r*(0.213-0.213*value) + g*(0.715-0.715*value) + b*(0.072+0.928*value) );
                    out_data[i+3] = a;
                }
            }
            break;
        case COLORMATRIX_HUEROTATE:
            coshue = cos(value * M_PI/180.0);
            sinhue = sin(value * M_PI/180.0);
            a00 = 0.213 + coshue*( 0.787) + sinhue*(-0.213);
            a01 = 0.715 + coshue*(-0.715) + sinhue*(-0.715);
            a02 = 0.072 + coshue*(-0.072) + sinhue*( 0.928);
            a10 = 0.213 + coshue*(-0.213) + sinhue*( 0.143);
            a11 = 0.715 + coshue*( 0.285) + sinhue*( 0.140);
            a12 = 0.072 + coshue*(-0.072) + sinhue*(-0.283);
            a20 = 0.213 + coshue*(-0.213) + sinhue*(-0.787);
            a21 = 0.715 + coshue*(-0.715) + sinhue*( 0.715);
            a22 = 0.072 + coshue*( 0.928) + sinhue*( 0.072);

            for (x=x0;x<x1;x++){
                for (y=y0;y<y1;y++){
                    i = ((x-x0) + (x1-x0)*(y-y0))*4;
                    r = in_data[i];
                    g = in_data[i+1];
                    b = in_data[i+2];
                    a = in_data[i+3];

                    out_data[i] = CLAMP_D_TO_U8( r*a00 + g*a01 + b*a02 );
                    out_data[i+1] = CLAMP_D_TO_U8( r*a10 + g*a11 + b*a12 );
                    out_data[i+2] = CLAMP_D_TO_U8( r*a20 + g*a21 + b*a22 );
                    out_data[i+3] = a;
                }
            }
            break;
        case COLORMATRIX_LUMINANCETOALPHA:
            for (x=x0;x<x1;x++){
                for (y=y0;y<y1;y++){
                    i = ((x-x0) + (x1-x0)*(y-y0))*4;
                    r = in_data[i];
                    g = in_data[i+1];
                    b = in_data[i+2];
                    out_data[i] = 0;
                    out_data[i+1] = 0;
                    out_data[i+2] = 0;
                    out_data[i+3] = CLAMP_D_TO_U8( r*0.2125 + g*0.7154 + b*0.0721);
                }
            }
            break;
        case COLORMATRIX_ENDTYPE:
            break;
    }

    if (free_in_on_exit) {
        nr_pixblock_release(in);
        delete in;
    }

    out->empty = FALSE;
    slot.set(_output, out);
    return 0;
}

void FilterColorMatrix::area_enlarge(NRRectL &/*area*/, Matrix const &/*trans*/)
{
}

void FilterColorMatrix::set_type(FilterColorMatrixType t){
        type = t;
}

void FilterColorMatrix::set_value(gdouble v){
        value = v;
}

void FilterColorMatrix::set_values(std::vector<gdouble> &v){
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

/*
 * feConvolveMatrix filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-convolve-matrix.h"
#include "display/nr-filter-units.h"
#include "display/nr-filter-utils.h"
#include <vector>

namespace Inkscape {
namespace Filters {

FilterConvolveMatrix::FilterConvolveMatrix()
{}

FilterPrimitive * FilterConvolveMatrix::create() {
    return new FilterConvolveMatrix();
}

FilterConvolveMatrix::~FilterConvolveMatrix()
{}

int FilterConvolveMatrix::render(FilterSlot &slot, FilterUnits const &/*units*/) {
    NRPixBlock *in = slot.get(_input);
    if (!in) {
        g_warning("Missing source image for feConvolveMatrix (in=%d)", _input);
        return 1;
    }

    if (bias!=0) {
        g_warning("It is unknown whether Inkscape's implementation of bias in feConvolveMatrix is correct!");
        // The SVG specification implies that feConvolveMatrix is defined for premultiplied colors (which makes sense).
        // It also says that bias should simply be added to the result for each color (without taking the alpha into account)
        // However, it also says that one purpose of bias is "to have .5 gray value be the zero response of the filter".
        // It seems sensible to indeed support the latter behaviour instead of the former, but this does appear to go against the standard.
        // Note that Batik simply does not support bias!=0
    }

    NRPixBlock *out = new NRPixBlock;

    nr_pixblock_setup_fast(out, in->mode,
                           in->area.x0, in->area.y0, in->area.x1, in->area.y1,
                           true);

    unsigned char *in_data = NR_PIXBLOCK_PX(in);
    unsigned char *out_data = NR_PIXBLOCK_PX(out);

    unsigned int const width = in->area.x1 - in->area.x0;
    unsigned int const height = in->area.y1 - in->area.y0;

    if (in->mode==NR_PIXBLOCK_MODE_R8G8B8A8P) {
        for (unsigned int y=targetY; y < height - (orderY - targetY); y++){
            for (unsigned int x=targetX; x < width - (orderX - targetX); x++){
                double result_R = 0;
                double result_G = 0;
                double result_B = 0;
                double result_A = 0;
                for (unsigned int i=0; i < orderY; i++){
                    for (int j=0; j < orderX; j++){
                        unsigned int index = 4*( x - targetX + j + width*(y - targetY + i) );
                        unsigned int kernel_index = orderX-j-1 + orderX*(orderY-i-1);
                        result_R += ( (double) in_data[index+0] * kernelMatrix[kernel_index] );
                        result_G += ( (double) in_data[index+1] * kernelMatrix[kernel_index] );
                        result_B += ( (double) in_data[index+2] * kernelMatrix[kernel_index] );
                        result_A += ( (double) in_data[index+3] * kernelMatrix[kernel_index] );
                    }
                }
                unsigned int out_index = 4*( x + width*y );
                if( preserveAlpha ) {
                    out_data[out_index+3] = in_data[out_index+3];
                } else {
                    out_data[out_index+3] = CLAMP_D_TO_U8(result_A / divisor + 255*bias);
                }
                out_data[out_index+0] = CLAMP_D_TO_U8_ALPHA(result_R / divisor + out_data[out_index+3]*bias, out_data[out_index+3]); // CLAMP includes rounding!
                out_data[out_index+1] = CLAMP_D_TO_U8_ALPHA(result_G / divisor + out_data[out_index+3]*bias, out_data[out_index+3]);
                out_data[out_index+2] = CLAMP_D_TO_U8_ALPHA(result_B / divisor + out_data[out_index+3]*bias, out_data[out_index+3]);
            }
        }
    } else {
        for (unsigned int y=targetY; y < height - (orderY - targetY); y++){
            for (unsigned int x=targetX; x < width - (orderX - targetX); x++){
                double result_R = 0;
                double result_G = 0;
                double result_B = 0;
                double result_A = 0;
                for (unsigned int i=0; i < orderY; i++){
                    for (unsigned int j=0; j < orderX; j++){
                        unsigned int index = 4*( x - targetX + j + width*(y - targetY + i) );
                        unsigned int kernel_index = orderX-j-1 + orderX*(orderY-i-1);
                        result_R += ( (double) in_data[index+0] * in_data[index+3] * kernelMatrix[kernel_index] );
                        result_G += ( (double) in_data[index+1] * in_data[index+3] * kernelMatrix[kernel_index] );
                        result_B += ( (double) in_data[index+2] * in_data[index+3] * kernelMatrix[kernel_index] );
                        result_A += ( (double) in_data[index+3] * kernelMatrix[kernel_index] );
                    }
                }
                unsigned int out_index = 4*( x + width*y );
                if( preserveAlpha ) {
                    out_data[out_index+3] = in_data[out_index+3];
                } else {
                    out_data[out_index+3] = CLAMP_D_TO_U8(result_A / divisor + 255*bias);
                }
                if (out_data[out_index+3]==0) {
                    out_data[out_index+0] = 0;
                    out_data[out_index+1] = 0;
                    out_data[out_index+2] = 0;
                } else {
                    out_data[out_index+0] = CLAMP_D_TO_U8(result_R / (divisor*out_data[out_index+3]) + 255*bias); // CLAMP includes rounding!
                    out_data[out_index+1] = CLAMP_D_TO_U8(result_G / (divisor*out_data[out_index+3]) + 255*bias);
                    out_data[out_index+2] = CLAMP_D_TO_U8(result_B / (divisor*out_data[out_index+3]) + 255*bias);
                }
            }
        }
    }

    out->empty = FALSE;
    slot.set(_output, out);
    return 0;
}

void FilterConvolveMatrix::set_targetX(int coord) {
    targetX = coord;
}

void FilterConvolveMatrix::set_targetY(int coord) {
    targetY = coord;
}

void FilterConvolveMatrix::set_orderX(int coord) {
    orderX = coord;
}

void FilterConvolveMatrix::set_orderY(int coord) {
    orderY = coord;
}

void FilterConvolveMatrix::set_divisor(double d) {
    divisor = d;
}

void FilterConvolveMatrix::set_bias(double b) {
    bias = b;
}

void FilterConvolveMatrix::set_kernelMatrix(std::vector<gdouble> &km) {
    kernelMatrix = km;
}

void FilterConvolveMatrix::set_edgeMode(FilterConvolveMatrixEdgeMode mode){
    edgeMode = mode;
}

void FilterConvolveMatrix::set_preserveAlpha(bool pa){
    preserveAlpha = pa;
}

void FilterConvolveMatrix::area_enlarge(NRRectL &area, Geom::Matrix const &/*trans*/)
{
    //Seems to me that since this filter's operation is resolution dependent,
    // some spurious pixels may still appear at the borders when low zooming or rotating. Needs a better fix.
    area.x0 -= targetX;
    area.y0 -= targetY;
    area.x1 += orderX - targetX;
    area.y1 += orderY - targetY;
}

FilterTraits FilterConvolveMatrix::get_input_traits() {
    return TRAIT_PARALLER;
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

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
#include <vector>
namespace NR {

FilterConvolveMatrix::FilterConvolveMatrix()
{}

FilterPrimitive * FilterConvolveMatrix::create() {
    return new FilterConvolveMatrix();
}

FilterConvolveMatrix::~FilterConvolveMatrix()
{}

static bool inside_area(int px, int py, int w, int h){
        if (px<0) return false;
        if (py<0) return false;
        if (px>w) return false;
        if (py>h) return false;
        return true;
}

int FilterConvolveMatrix::render(FilterSlot &slot, Matrix const &trans) {
    NRPixBlock *in = slot.get(_input);
    NRPixBlock *out = new NRPixBlock;

    nr_pixblock_setup_fast(out, in->mode,
                           in->area.x0, in->area.y0, in->area.x1, in->area.y1,
                           true);

    unsigned char *in_data = NR_PIXBLOCK_PX(in);
    unsigned char *out_data = NR_PIXBLOCK_PX(out);

    double result_R, result_G, result_B, result_A;
    int i, j, x, y;
    int width = in->area.x1 - in->area.x0;
    int height = in->area.y1 - in->area.y0;

    double div=0;

    if (divisor != 0){
        div = divisor;
    } else {
        for (i=0;i<orderX*orderY;i++){
            div += kernelMatrix[i];
        }
    }

    for (x=0; x < width; x++){
        for (y=0; y < height; y++){
            result_R = 0;
            result_G = 0;
            result_B = 0;
            result_A = 0;
            for (i=0; i < orderY; i++){
                for (j=0; j < orderX; j++){
                    if (inside_area(x - targetX + j, y - targetY + i, width, height)){
                        result_R += ( (double) in_data[4*( x - targetX + j + width*(y - targetY + i) )] * kernelMatrix[orderX-j-1 + orderX*(orderY-i-1)] );
                        result_G += ( (double) in_data[4*( x - targetX + j + width*(y - targetY + i) )+1] * kernelMatrix[orderX-j-1 + orderX*(orderY-i-1)] );
                        result_B += ( (double) in_data[4*( x - targetX + j + width*(y - targetY + i) )+2] * kernelMatrix[orderX-j-1 + orderX*(orderY-i-1)] );
                        result_A += ( (double) in_data[4*( x - targetX + j + width*(y - targetY + i) )+3] * kernelMatrix[orderX-j-1 + orderX*(orderY-i-1)] );
                    }
                }
            }
            result_R = result_R / div + bias;
            result_G = result_G / div + bias;
            result_B = result_B / div + bias;
            result_A = result_A / div + bias;

            result_R = (result_R > 0 ? result_R : 0);
            result_G = (result_G > 0 ? result_G : 0);
            result_B = (result_B > 0 ? result_B : 0);
            result_A = (result_A > 0 ? result_A : 0);

            out_data[4*( x + width*y )] = (result_R < 255 ? (unsigned char)result_R : 255);
            out_data[4*( x + width*y )+1] = (result_G < 255 ? (unsigned char)result_G : 255);
            out_data[4*( x + width*y )+2] = (result_B < 255 ? (unsigned char)result_B : 255);
            out_data[4*( x + width*y )+3] = (result_A < 255 ? (unsigned char)result_A : 255);
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

void FilterConvolveMatrix::area_enlarge(NRRectL &area, Matrix const &trans)
{
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

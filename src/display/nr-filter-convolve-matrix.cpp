/*
 * feConvolveMatrix filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *   Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 *
 * Copyright (C) 2007,2009 authors
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

template<bool PREMULTIPLIED, bool PRESERVE_ALPHA, bool X_LOWER, bool X_UPPER, bool Y_LOWER, bool Y_UPPER>
static inline void convolve2D_XY(unsigned int const x, unsigned int const y, unsigned char *const out_data, unsigned char const *const in_data, unsigned int const width, unsigned int const height, double const *const kernel, unsigned int const orderX, unsigned int const orderY, unsigned int const targetX, unsigned int const targetY, double const bias) {
    double result_R = 0;
    double result_G = 0;
    double result_B = 0;
    double result_A = 0;

    unsigned int iBegin = Y_LOWER ? targetY-y : 0; // Note that to prevent signed/unsigned problems this requires that y<=targetY (which is true)
    unsigned int iEnd   = Y_UPPER ? height+targetY-y : orderY; // And this requires that y<=height+targetY (which is trivially true), in addition it should be true that height+targetY-y<=orderY (or equivalently y>=height+targetY-orderY, which is true)
    unsigned int jBegin = X_LOWER ? targetX-x : 0;
    unsigned int jEnd   = X_UPPER ? width+targetX-x : orderX;

    for (unsigned int i=iBegin; i<iEnd; i++){
        for (unsigned int j=jBegin; j<jEnd; j++){
            unsigned int index = 4*( x - targetX + j + width*(y - targetY + i) );
            unsigned int kernel_index = orderX-j-1 + orderX*(orderY-i-1);
            double k = PREMULTIPLIED ? kernel[kernel_index] : in_data[index+3] * kernel[kernel_index];
            result_R += in_data[index+0] * k;
            result_G += in_data[index+1] * k;
            result_B += in_data[index+2] * k;
            result_A += in_data[index+3] * kernel[kernel_index];
        }
    }

    unsigned int const out_index = 4*( x + width*y );
    if (PRESERVE_ALPHA) {
        out_data[out_index+3] = in_data[out_index+3];
    } else if (PREMULTIPLIED) {
        out_data[out_index+3] = CLAMP_D_TO_U8(result_A + 255*bias);
    } else {
        out_data[out_index+3] = CLAMP_D_TO_U8(result_A + bias);
    }
    if (PREMULTIPLIED) {
        out_data[out_index+0] = CLAMP_D_TO_U8_ALPHA(result_R + out_data[out_index+3]*bias, out_data[out_index+3]); // CLAMP includes rounding!
        out_data[out_index+1] = CLAMP_D_TO_U8_ALPHA(result_G + out_data[out_index+3]*bias, out_data[out_index+3]);
        out_data[out_index+2] = CLAMP_D_TO_U8_ALPHA(result_B + out_data[out_index+3]*bias, out_data[out_index+3]);
    } else if (out_data[out_index+3]==0) {
        out_data[out_index+0] = 0; // TODO: Is there a more sensible value that can be used here?
        out_data[out_index+1] = 0;
        out_data[out_index+2] = 0;
    } else {
        out_data[out_index+0] = CLAMP_D_TO_U8(result_R / out_data[out_index+3] + bias); // CLAMP includes rounding!
        out_data[out_index+1] = CLAMP_D_TO_U8(result_G / out_data[out_index+3] + bias);
        out_data[out_index+2] = CLAMP_D_TO_U8(result_B / out_data[out_index+3] + bias);
    }
}

template<bool PREMULTIPLIED, bool PRESERVE_ALPHA, bool Y_LOWER, bool Y_UPPER>
static inline void convolve2D_Y(unsigned int const y, unsigned char *const out_data, unsigned char const *const in_data, unsigned int const width, unsigned int const height, double const *const kernel, unsigned int const orderX, unsigned int const orderY, unsigned int const targetX, unsigned int const targetY, double const bias) {
    // See convolve2D below for rationale.

    unsigned int const lowerEnd = std::min(targetX,width);
    unsigned int const upperBegin = width - std::min<unsigned int>(width,orderX - 1u - targetX);
    unsigned int const midXBegin = std::min(lowerEnd,upperBegin);
    unsigned int const midXEnd = std::max(lowerEnd,upperBegin);

    for (unsigned int x=0; x<midXBegin; x++) {
        convolve2D_XY<PREMULTIPLIED,PRESERVE_ALPHA,true,false,Y_LOWER,Y_UPPER>(x, y, out_data, in_data, width, height, kernel, orderX, orderY, targetX, targetY, bias);
    }
    if (lowerEnd==upperBegin) {
        // Do nothing, empty mid section
    } else if (lowerEnd<upperBegin) {
        // In the middle no bounds have to be adjusted
        for (unsigned int x=midXBegin; x<midXEnd; x++) {
            convolve2D_XY<PREMULTIPLIED,PRESERVE_ALPHA,false,false,Y_LOWER,Y_UPPER>(x, y, out_data, in_data, width, height, kernel, orderX, orderY, targetX, targetY, bias);
        }
    } else {
        // In the middle both bounds have to be adjusted
        for (unsigned int x=midXBegin; x<midXEnd; x++) {
            convolve2D_XY<PREMULTIPLIED,PRESERVE_ALPHA,true,true,Y_LOWER,Y_UPPER>(x, y, out_data, in_data, width, height, kernel, orderX, orderY, targetX, targetY, bias);
        }
    }
    for (unsigned int x=midXEnd; x<width; x++) {
        convolve2D_XY<PREMULTIPLIED,PRESERVE_ALPHA,false,true,Y_LOWER,Y_UPPER>(x, y, out_data, in_data, width, height, kernel, orderX, orderY, targetX, targetY, bias);
    }
}

template<bool PREMULTIPLIED, bool PRESERVE_ALPHA>
static void convolve2D(unsigned char *const out_data, unsigned char const *const in_data, unsigned int const width, unsigned int const height, double const *const kernel, unsigned int const orderX, unsigned int const orderY, unsigned int const targetX, unsigned int const targetY, double const _bias) {
    double const bias = PREMULTIPLIED ? _bias : 255*_bias; // If we're using non-premultiplied values the bias is always multiplied by 255.

    // For the middle section it should hold that (for all i such that 0<=i<orderY):
    //   0 <= y - targetY + i < height
    //   targetY <= y && y < height + targetY - orderY + 1
    // In other words, for y<targetY i's lower bound needs to be adjusted and for y>=height+targetY-orderY+1 i's upper bound needs to be adjusted.

    unsigned int const lowerEnd = std::min(targetY,height);
    unsigned int const upperBegin = height - std::min<unsigned int>(height,orderY - 1u - targetY);
    unsigned int const midYBegin = std::min(lowerEnd,upperBegin);
    unsigned int const midYEnd = std::max(lowerEnd,upperBegin);

    for (unsigned int y=0; y<midYBegin; y++) {
        convolve2D_Y<PREMULTIPLIED,PRESERVE_ALPHA,true,false>(y, out_data, in_data, width, height, kernel, orderX, orderY, targetX, targetY, bias);
    }
    if (lowerEnd==upperBegin) {
        // Do nothing, empty mid section
    } else if (lowerEnd<upperBegin) {
        // In the middle no bounds have to be adjusted
        for (unsigned int y=midYBegin; y<midYEnd; y++) {
            convolve2D_Y<PREMULTIPLIED,PRESERVE_ALPHA,false,false>(y, out_data, in_data, width, height, kernel, orderX, orderY, targetX, targetY, bias);
        }
    } else {
        // In the middle both bounds have to be adjusted
        for (unsigned int y=midYBegin; y<midYEnd; y++) {
            convolve2D_Y<PREMULTIPLIED,PRESERVE_ALPHA,true,true>(y, out_data, in_data, width, height, kernel, orderX, orderY, targetX, targetY, bias);
        }
    }
    for (unsigned int y=midYEnd; y<height; y++) {
        convolve2D_Y<PREMULTIPLIED,PRESERVE_ALPHA,false,true>(y, out_data, in_data, width, height, kernel, orderX, orderY, targetX, targetY, bias);
    }
}

int FilterConvolveMatrix::render(FilterSlot &slot, FilterUnits const &/*units*/) {
    NRPixBlock *in = slot.get(_input);
    if (!in) {
        g_warning("Missing source image for feConvolveMatrix (in=%d)", _input);
        return 1;
    }
    if (orderX<=0 || orderY<=0) {
        g_warning("Empty kernel!");
        return 1;
    }
    if (targetX<0 || targetX>=orderX || targetY<0 || targetY>=orderY) {
        g_warning("Invalid target!");
        return 1;
    }
    if (kernelMatrix.size()!=(unsigned int)(orderX*orderY)) {
        g_warning("kernelMatrix does not have orderX*orderY elements!");
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
    if (edgeMode!=CONVOLVEMATRIX_EDGEMODE_NONE) {
        g_warning("Inkscape only supports edgeMode=\"none\" (and a filter uses a different one)!");
        // Note that to properly support edgeMode the interaction with area_enlarge should be well understood (and probably something needs to change)
        // area_enlarge should NOT let Inkscape enlarge the area beyond the filter area, it should only enlarge the rendered area if a part of the object is rendered to make it overlapping (enough) with adjacent parts.
    }

    NRPixBlock *out = new NRPixBlock;

    nr_pixblock_setup_fast(out, in->mode,
                           in->area.x0, in->area.y0, in->area.x1, in->area.y1,
                           true);

    unsigned char *in_data = NR_PIXBLOCK_PX(in);
    unsigned char *out_data = NR_PIXBLOCK_PX(out);

    unsigned int const width = in->area.x1 - in->area.x0;
    unsigned int const height = in->area.y1 - in->area.y0;

    // Set up predivided kernel matrix
    std::vector<double> kernel(kernelMatrix);
    for(size_t i=0; i<kernel.size(); i++) {
        kernel[i] /= divisor; // The code that creates this object makes sure that divisor != 0
    }

    if (in->mode==NR_PIXBLOCK_MODE_R8G8B8A8P) {
        if (preserveAlpha) {
            convolve2D<true,true>(out_data, in_data, width, height, &kernel.front(), orderX, orderY, targetX, targetY, bias);
        } else {
            convolve2D<true,false>(out_data, in_data, width, height, &kernel.front(), orderX, orderY, targetX, targetY, bias);
        }
    } else {
        if (preserveAlpha) {
            convolve2D<false,true>(out_data, in_data, width, height, &kernel.front(), orderX, orderY, targetX, targetY, bias);
        } else {
            convolve2D<false,false>(out_data, in_data, width, height, &kernel.front(), orderX, orderY, targetX, targetY, bias);
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
    area.x1 += orderX - targetX - 1; // This makes sure the last row/column in the original image corresponds to the last row/column in the new image that can be convolved without adjusting the boundary conditions).
    area.y1 += orderY - targetY - 1;
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

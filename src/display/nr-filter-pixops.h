#ifndef __NR_FILTER_PIXOPS_H__
#define __NR_FILTER_PIXOPS_H__

#include "libnr/nr-pixblock.h"
#include <algorithm>

/*
 * Per-pixel image manipulation functions.
 * These can be used by all filter primitives, which combine two images on
 * per-pixel basis. These are at least feBlend, feComposite and feMerge.
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

namespace Inkscape {
namespace Filters {

/**
 * Mixes the two input images using the function given as template.
 * The result is placed in out.
 * The mixing function should have the following type:
 * void mix(unsigned char *result, unsigned char const *in1,
 *          unsigned char const *in2);
 * Each of the parameters for mix-function is a pointer to four bytes of data,
 * giving the RGBA values for that pixel. The mix function must only access
 * the four bytes beginning at a pointer given as parameter.
 */
/*
 * The implementation is in a header file because of the template. It has to
 * be in the same compilation unit as the code using it. Otherwise, linking
 * the program will not succeed.
 */
template <void(*blend)(unsigned char *cr, unsigned char const *ca, unsigned char const *cb)>
void pixops_mix(NRPixBlock &out, NRPixBlock &in1, NRPixBlock &in2) {
    unsigned char *in1_data = NR_PIXBLOCK_PX(&in1);
    unsigned char *in2_data = NR_PIXBLOCK_PX(&in2);
    unsigned char *out_data = NR_PIXBLOCK_PX(&out);
    unsigned char zero_rgba[4] = {0, 0, 0, 0};

    // Possible scenarios (omitting cases where an interval is empty and those which are the same by interchanging 1 and 2):
    // 01020, 01320, 01310 (no overlap, partial overlap, full overlap)
    int out_y0 = out.area.y0;
    int out_y1 = std::max(out.area.y1,out_y0); // Enforce sanity
    int in1_y0 = std::min(std::max(in1.area.y0,out_y0),out_y1);
    int in2_y0 = std::min(std::max(in2.area.y0,out_y0),out_y1);
    int in1_y1 = std::min(std::max(in1.area.y1,in1_y0),out_y1);
    int in2_y1 = std::min(std::max(in2.area.y1,in2_y0),out_y1);
    int min_in_y0 = std::min(in1_y0,in2_y0);
    int max_in_y0 = std::max(in1_y0,in2_y0);
    int min_in_y1 = std::min(in1_y1,in2_y1);
    int max_in_y1 = std::max(in1_y1,in2_y1);
    int const yBound[6] = {out_y0, min_in_y0, std::min(max_in_y0,min_in_y1), std::max(max_in_y0,min_in_y1), max_in_y1, out_y1};
    bool const in1_zero_y[5] = {true, in1_y0>yBound[1], in1_y0>yBound[2] || in1_y1<=yBound[2], in1_y1<=yBound[3], true};
    bool const in2_zero_y[5] = {true, in2_y0>yBound[1], in2_y0>yBound[2] || in2_y1<=yBound[2], in2_y1<=yBound[3], true};

    int out_x0 = out.area.x0;
    int out_x1 = std::max(out.area.x1,out_x0);
    int in1_x0 = std::min(std::max(in1.area.x0,out_x0),out_x1);
    int in2_x0 = std::min(std::max(in2.area.x0,out_x0),out_x1);
    int in1_x1 = std::min(std::max(in1.area.x1,in1_x0),out_x1);
    int in2_x1 = std::min(std::max(in2.area.x1,in2_x0),out_x1);
    int min_in_x0 = std::min(in1_x0,in2_x0);
    int max_in_x0 = std::max(in1_x0,in2_x0);
    int min_in_x1 = std::min(in1_x1,in2_x1);
    int max_in_x1 = std::max(in1_x1,in2_x1);
    int const xBound[6] = {out_x0, min_in_x0, std::min(max_in_x0,min_in_x1), std::max(max_in_x0,min_in_x1), max_in_x1, out_x1};
    bool const in1_zero_x[5] = {true, in1_x0>xBound[1], in1_x0>xBound[2] || in1_x1<=xBound[2], in1_x1<=xBound[3], true};
    bool const in2_zero_x[5] = {true, in2_x0>xBound[1], in2_x0>xBound[2] || in2_x1<=xBound[2], in2_x1<=xBound[3], true};

    for (int yr = 0 ; yr < 5 ; yr++) {
        for(int y = yBound[yr] ; y < yBound[yr+1] ; y++) {
            int out_line = (y - out.area.y0) * out.rs;
            int in1_line = (y - in1.area.y0) * in1.rs;
            int in2_line = (y - in2.area.y0) * in2.rs;
            for (int xr = 0 ; xr < 5 ; xr++) {
                for(int x = xBound[xr] ; x < xBound[xr+1] ; x++) {
                    blend(out_data + out_line + 4 * (x - out.area.x0),
                        (in1_zero_x[xr]||in1_zero_y[yr]) ? zero_rgba : (in1_data + in1_line + 4 * (x - in1.area.x0)),
                        (in2_zero_x[xr]||in2_zero_y[yr]) ? zero_rgba : (in2_data + in2_line + 4 * (x - in2.area.x0)));
                }
            }
        }
    }
}

} /* namespace Filters */
} /* namespace Inkscape */

#endif // __NR_FILTER_PIXOPS_H_
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

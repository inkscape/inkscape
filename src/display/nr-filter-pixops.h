#ifndef __NR_FILTER_PIXOPS_H__
#define __NR_FILTER_PIXOPS_H__

#include "libnr/nr-pixblock.h"

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

namespace NR {

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

    if (in1.area.y0 < in2.area.y0) {
        // in1 begins before in2 on y-axis
        for (int y = in1.area.y0 ; y < in2.area.y0 ; y++) {
            int out_line = (y - out.area.y0) * out.rs;
            int in_line = (y - in1.area.y0) * in1.rs;
            for (int x = in1.area.x0 ; x < in1.area.x1 ; x++) {
                blend(out_data + out_line + 4 * (x - out.area.x0),
                      in1_data + in_line + 4 * (x - in1.area.x0),
                      zero_rgba);
            }
        }
    } else if (in1.area.y0 > in2.area.y0) {
        // in2 begins before in1 on y-axis
        for (int y = in2.area.y0 ; y < in1.area.y0 ; y++) {
            int out_line = (y - out.area.y0) * out.rs;
            int in_line = (y - in2.area.y0) * in2.rs;
            for (int x = in2.area.x0 ; x < in2.area.x1 ; x++) {
                blend(out_data + out_line + 4 * (x - out.area.x0),
                      zero_rgba,
                      in2_data + in_line + 4 * (x - in2.area.x0));
            }
        }
    }

    for (int y = std::max(in1.area.y0, in2.area.y0) ;
         y < std::min(in1.area.y1, in2.area.y1) ; ++y) {
        int out_line = (y - out.area.y0) * out.rs;
        int in1_line = (y - in1.area.y0) * in1.rs;
        int in2_line = (y - in2.area.y0) * in2.rs;

        if (in1.area.x0 < in2.area.x0) {
            // in1 begins before in2 on x-axis
            for (int x = in1.area.x0 ; x < in2.area.x0 ; ++x) {
                blend(out_data + out_line + 4 * (x - out.area.x0),
                      in1_data + in1_line + 4 * (x - in1.area.x0),
                      zero_rgba);
            }
        } else if (in1.area.x0 > in2.area.x0) {
            // in2 begins before in1 on x-axis
            for (int x = in2.area.x0 ; x < in1.area.x0 ; ++x) {
                blend(out_data + out_line + 4 * (x - out.area.x0),
                      zero_rgba,
                      in2_data + in2_line + 4 * (x - in2.area.x0));
            }
        }

        for (int x = std::max(in1.area.x0, in2.area.x0) ;
             x < std::min(in1.area.x1, in2.area.x1) ; ++x) {
            blend(out_data + out_line + 4 * (x - out.area.x0),
                  in1_data + in1_line + 4 * (x - in1.area.x0),
                  in2_data + in2_line + 4 * (x - in2.area.x0));
        }

        if (in1.area.x1 > in2.area.x1) {
            // in1 ends after in2 on x-axis
            for (int x = in2.area.x1 ; x < in1.area.x1 ; ++x) {
                blend(out_data + out_line + 4 * (x - out.area.x0),
                      in1_data + in1_line + 4 * (x - in1.area.x0),
                      zero_rgba);
            }
        } else if (in1.area.x1 < in2.area.x1) {
            // in2 ends after in1 on x-axis
            for (int x = in1.area.x1 ; x < in2.area.x1 ; ++x) {
                blend(out_data + out_line + 4 * (x - out.area.x0),
                      zero_rgba,
                      in2_data + in2_line + 4 * (x - in2.area.x0));
            }
        }
    }

    if (in1.area.y1 > in2.area.y1) {
        // in1 ends after in2 on y-axis
        for (int y = in2.area.y1 ; y < in1.area.y1 ; y++) {
            int out_line = (y - out.area.y0) * out.rs;
            int in_line = (y - in1.area.y0) * in1.rs;
            for (int x = in1.area.x0 ; x < in1.area.x1 ; x++) {
                blend(out_data + out_line + 4 * (x - out.area.x0),
                      in1_data + in_line + 4 * (x - in1.area.x0),
                      zero_rgba);
            }
        }
    } else if (in1.area.y1 < in2.area.y1) {
        // in2 ends after in1 on y-axis
        for (int y = in1.area.y1 ; y < in2.area.y1 ; y++) {
            int out_line = (y - out.area.y0) * out.rs;
            int in_line = (y - in2.area.y0) * in2.rs;
            for (int x = in2.area.x0 ; x < in2.area.x1 ; x++) {
                blend(out_data + out_line + 4 * (x - out.area.x0),
                      zero_rgba,
                      in2_data + in_line + 4 * (x - in2.area.x0));
            }
        }
    }
}

} // namespace NR

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

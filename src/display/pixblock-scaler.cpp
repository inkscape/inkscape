#define __NR_PIXBLOCK_SCALER_CPP__

/*
 * Functions for blitting pixblocks using scaling
 *
 * Author:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006 Niko Kiirala
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <cmath>
using std::floor;

#include "libnr/nr-pixblock.h"

namespace NR {

struct RGBA {
    int r, g, b, a;
};

/** Calculates cubically interpolated value of the four given pixel values.
 * The pixel values should be from four vertically adjacent pixels.
 * If we are calculating a pixel, whose y-coordinate in source image is
 * i, these pixel values a, b, c and d should come from lines
 * floor(i) - 1, floor(i), floor(i) + 1, floor(i) + 2, respectively.
 * Parameter len should be set to i.
 * Returns the interpolated value in fixed point format with 8 bit
 * decimal part. (24.8 assuming 32-bit int)
 */
__attribute__ ((const))
inline int sampley(unsigned const char a, unsigned const char b,
                   unsigned const char c, unsigned const char d,
                   const double len)
{
    double lenf = len - floor(len);
    int sum = 0;
    sum += (int)((((-1.0 / 3.0) * lenf + 4.0 / 5.0) * lenf - 7.0 / 15.0)
                 * lenf * 256 * a);
    sum += (int)((((lenf - 9.0 / 5.0) * lenf - 1.0 / 5.0) * lenf + 1.0)
                 * 256 * b);
    sum += (int)(((((1 - lenf) - 9.0 / 5.0) * (1 - lenf) - 1.0 / 5.0)
                  * (1 - lenf) + 1.0) * 256 * c);
    sum += (int)((((-1.0 / 3.0) * (1 - lenf) + 4.0 / 5.0) * (1 - lenf)
                  - 7.0 / 15.0) * (1 - lenf) * 256 * d);
    return sum;
}

/** Calculates cubically interpolated value of the four given pixel values.
 * The pixel values should be interpolated values from sampley, from four
 * horizontally adjacent vertical lines. The parameters a, b, c and d
 * should be in fixed point format with 8-bit decimal part.
 * If we are calculating a pixel, whose x-coordinate in source image is
 * i, these vertical  lines from where a, b, c and d are calculated, should be
 * floor(i) - 1, floor(i), floor(i) + 1, floor(i) + 2, respectively.
 * Parameter len should be set to i.
 * Returns the interpolated value in 8-bit format, ready to be written
 * to output buffer.
 */
inline unsigned char samplex(const int a, const int b, const int c, const int d, const double len) {
    double lenf = len - floor(len);
    int sum = 0;
    sum += (int)(a * (((-1.0 / 3.0) * lenf + 4.0 / 5.0) * lenf - 7.0 / 15.0) * lenf);
    sum += (int)(b * (((lenf - 9.0 / 5.0) * lenf - 1.0 / 5.0) * lenf + 1.0));
    sum += (int)(c * ((((1 - lenf) - 9.0 / 5.0) * (1 - lenf) - 1.0 / 5.0) * (1 - lenf) + 1.0));
    sum += (int)(d * (((-1.0 / 3.0) * (1 - lenf) + 4.0 / 5.0) * (1 - lenf) - 7.0 / 15.0) * (1 - lenf));
    if (sum < 0) sum = 0;
    if (sum >= 256*256) sum = 255 * 256;
    return (unsigned char)(sum / 256);
}

/**
 * Sanity check function for indexing pixblocks.
 * Catches reading and writing outside the pixblock area.
 * When enabled, decreases filter rendering speed massively.
 */
inline void _check_index(NRPixBlock const * const pb, int const location, int const line)
{
    if(false) {
        int max_loc = pb->rs * (pb->area.y1 - pb->area.y0);
        if (location < 0 || (location + 4) > max_loc)
            g_warning("Location %d out of bounds (0 ... %d) at line %d", location, max_loc, line);
    }
}

void scale_bicubic(NRPixBlock *to, NRPixBlock *from)
{
    if (NR_PIXBLOCK_BPP(from) != 4 || NR_PIXBLOCK_BPP(to) != 4) {
        g_warning("A non-32-bpp image passed to scale_bicubic: scaling aborted.");
        return;
    }

    // Precalculate sizes of source and destination pixblocks
    int from_width = from->area.x1 - from->area.x0;
    int from_height = from->area.y1 - from->area.y0;
    int to_width = to->area.x1 - to->area.x0;
    int to_height = to->area.y1 - to->area.y0;

    // from_step: when advancing one pixel in destination image,
    // how much we should advance in source image
    double from_stepx = (double)from_width / (double)to_width;
    double from_stepy = (double)from_height / (double)to_height;

    // Loop through every pixel of destination image, a line at a time
    for (int to_y = 0 ; to_y < to_height ; to_y++) {
        double from_y = to_y * from_stepy + from_stepy / 2;
        // Pre-calculate beginning of the four horizontal lines, from
        // which we should read
        int from_line[4];
        for (int i = 0 ; i < 4 ; i++) {
            if ((int)floor(from_y) + i - 1 >= 0) {
                if ((int)floor(from_y) + i - 1 < from_height) {
                    from_line[i] = ((int)floor(from_y) + i - 1) * from->rs;
                } else {
                    from_line[i] = (from_height - 1) * from->rs;
                }
            } else {
                from_line[i] = 0;
            }
        }
        // Loop through this horizontal line in destination image
        // For every pixel, calculate the color of pixel with
        // bicubic interpolation and set the pixel value in destination image
        for (int to_x = 0 ; to_x < to_width ; to_x++) {
            double from_x = to_x * from_stepx + from_stepx / 2;
            RGBA line[4];
            for (int i = 0 ; i < 4 ; i++) {
                int k = (int)floor(from_x) + i - 1;
                if (k < 0) k = 0;
                if (k >= from_width) k = from_width - 1;
                k *= 4;
                _check_index(from, from_line[0] + k, __LINE__);
                _check_index(from, from_line[1] + k, __LINE__);
                _check_index(from, from_line[2] + k, __LINE__);
                _check_index(from, from_line[3] + k, __LINE__);
                line[i].r = sampley(NR_PIXBLOCK_PX(from)[from_line[0] + k],
                                    NR_PIXBLOCK_PX(from)[from_line[1] + k],
                                    NR_PIXBLOCK_PX(from)[from_line[2] + k],
                                    NR_PIXBLOCK_PX(from)[from_line[3] + k],
                                    from_y);
                line[i].g = sampley(NR_PIXBLOCK_PX(from)[from_line[0] + k + 1],
                                    NR_PIXBLOCK_PX(from)[from_line[1] + k + 1],
                                    NR_PIXBLOCK_PX(from)[from_line[2] + k + 1],
                                    NR_PIXBLOCK_PX(from)[from_line[3] + k + 1],
                                    from_y);
                line[i].b = sampley(NR_PIXBLOCK_PX(from)[from_line[0] + k + 2],
                                    NR_PIXBLOCK_PX(from)[from_line[1] + k + 2],
                                    NR_PIXBLOCK_PX(from)[from_line[2] + k + 2],
                                    NR_PIXBLOCK_PX(from)[from_line[3] + k + 2],
                                    from_y);
                line[i].a = sampley(NR_PIXBLOCK_PX(from)[from_line[0] + k + 3],
                                    NR_PIXBLOCK_PX(from)[from_line[1] + k + 3],
                                    NR_PIXBLOCK_PX(from)[from_line[2] + k + 3],
                                    NR_PIXBLOCK_PX(from)[from_line[3] + k + 3],
                                    from_y);
            }
            RGBA result;
            result.r = samplex(line[0].r, line[1].r, line[2].r, line[3].r,
                               from_x);
            result.g = samplex(line[0].g, line[1].g, line[2].g, line[3].g,
                               from_x);
            result.b = samplex(line[0].b, line[1].b, line[2].b, line[3].b,
                               from_x);
            result.a = samplex(line[0].a, line[1].a, line[2].a, line[3].a,
                               from_x);

            _check_index(to, to_y * to->rs + to_x * 4, __LINE__);
            NR_PIXBLOCK_PX(to)[to_y * to->rs + to_x * 4] = result.r;
            NR_PIXBLOCK_PX(to)[to_y * to->rs + to_x * 4 + 1] = result.g;
            NR_PIXBLOCK_PX(to)[to_y * to->rs + to_x * 4 + 2] = result.b;
            NR_PIXBLOCK_PX(to)[to_y * to->rs + to_x * 4 + 3] = result.a;
        }
    }
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

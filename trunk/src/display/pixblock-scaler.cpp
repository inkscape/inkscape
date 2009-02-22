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

#include "display/nr-filter-utils.h"
#include "libnr/nr-pixblock.h"
#include "libnr/nr-blit.h"

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
inline static int sampley(unsigned const char a, unsigned const char b,
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
inline static int samplex(const int a, const int b, const int c, const int d, const double len) {
    double lenf = len - floor(len);
    int sum = 0;
    sum += (int)(a * (((-1.0 / 3.0) * lenf + 4.0 / 5.0) * lenf - 7.0 / 15.0) * lenf);
    sum += (int)(b * (((lenf - 9.0 / 5.0) * lenf - 1.0 / 5.0) * lenf + 1.0));
    sum += (int)(c * ((((1 - lenf) - 9.0 / 5.0) * (1 - lenf) - 1.0 / 5.0) * (1 - lenf) + 1.0));
    sum += (int)(d * (((-1.0 / 3.0) * (1 - lenf) + 4.0 / 5.0) * (1 - lenf) - 7.0 / 15.0) * (1 - lenf));
    //if (sum < 0) sum = 0;
    //if (sum > 255 * 256) sum = 255 * 256;
    return sum / 256;
}

/**
 * Sanity check function for indexing pixblocks.
 * Catches reading and writing outside the pixblock area.
 * When enabled, decreases filter rendering speed massively.
 */
inline static void _check_index(NRPixBlock const * const pb, int const location, int const line)
{
    if(false) {
        int max_loc = pb->rs * (pb->area.y1 - pb->area.y0);
        if (location < 0 || (location + 4) > max_loc)
            g_warning("Location %d out of bounds (0 ... %d) at line %d", location, max_loc, line);
    }
}

static void scale_bicubic_rgba(NRPixBlock *to, NRPixBlock *from)
{
    if (NR_PIXBLOCK_BPP(from) != 4 || NR_PIXBLOCK_BPP(to) != 4) {
        g_warning("A non-32-bpp image passed to scale_bicubic_rgba: scaling aborted.");
        return;
    }

    bool free_from_on_exit = false;
    if (from->mode != to->mode){
        NRPixBlock *o_from = from;
        from = new NRPixBlock;
        nr_pixblock_setup_fast(from, to->mode, o_from->area.x0, o_from->area.y0, o_from->area.x1, o_from->area.y1, false);
        nr_blit_pixblock_pixblock(from, o_from);
        free_from_on_exit = true;
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

            using Inkscape::Filters::clamp;
            using Inkscape::Filters::clamp_alpha;
            if (to->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
                /* Clamp the colour channels to range from 0 to result.a to
                 * make sure, we don't exceed 100% per colour channel with
                 * images that have premultiplied alpha */

                result.a = clamp(result.a);

                NR_PIXBLOCK_PX(to)[to_y * to->rs + to_x * 4] = clamp_alpha(result.r, result.a);
                NR_PIXBLOCK_PX(to)[to_y * to->rs + to_x * 4 + 1] = clamp_alpha(result.g, result.a);
                NR_PIXBLOCK_PX(to)[to_y * to->rs + to_x * 4 + 2] = clamp_alpha(result.b, result.a);
                NR_PIXBLOCK_PX(to)[to_y * to->rs + to_x * 4 + 3] = result.a;
            } else {
                NR_PIXBLOCK_PX(to)[to_y * to->rs + to_x * 4] = clamp(result.r);
                NR_PIXBLOCK_PX(to)[to_y * to->rs + to_x * 4 + 1] = clamp(result.g);
                NR_PIXBLOCK_PX(to)[to_y * to->rs + to_x * 4 + 2] = clamp(result.b);
                NR_PIXBLOCK_PX(to)[to_y * to->rs + to_x * 4 + 3] = clamp(result.a);
            }
        }
    }
    if (free_from_on_exit) {
        nr_pixblock_release(from);
        delete from;
    }

}

void scale_bicubic_alpha(NRPixBlock *to, NRPixBlock *from)
{
    if (NR_PIXBLOCK_BPP(from) != 1 || NR_PIXBLOCK_BPP(to) != 1) {
        g_warning("A non-8-bpp image passed to scale_bicubic_alpha: scaling aborted.");
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
            int line[4];
            for (int i = 0 ; i < 4 ; i++) {
                int k = (int)floor(from_x) + i - 1;
                if (k < 0) k = 0;
                if (k >= from_width) k = from_width - 1;
                _check_index(from, from_line[0] + k, __LINE__);
                _check_index(from, from_line[1] + k, __LINE__);
                _check_index(from, from_line[2] + k, __LINE__);
                _check_index(from, from_line[3] + k, __LINE__);
                line[i] = sampley(NR_PIXBLOCK_PX(from)[from_line[0] + k],
                                  NR_PIXBLOCK_PX(from)[from_line[1] + k],
                                  NR_PIXBLOCK_PX(from)[from_line[2] + k],
                                  NR_PIXBLOCK_PX(from)[from_line[3] + k],
                                  from_y);
            }
            int result;
            result = samplex(line[0], line[1], line[2], line[3],
                             from_x);

            _check_index(to, to_y * to->rs + to_x, __LINE__);

            NR_PIXBLOCK_PX(to)[to_y * to->rs + to_x] = Inkscape::Filters::clamp(result);
        }
    }
}

void scale_bicubic(NRPixBlock *to, NRPixBlock *from)
{
    if (NR_PIXBLOCK_BPP(to) == 4 && NR_PIXBLOCK_BPP(from) == 4) {
        scale_bicubic_rgba(to, from);
    } else if (NR_PIXBLOCK_BPP(to) == 1 && NR_PIXBLOCK_BPP(from) == 1) {
        scale_bicubic_alpha(to, from);
    } else {
        g_warning("NR::scale_bicubic: unsupported bitdepths for scaling: to %d, from %d", NR_PIXBLOCK_BPP(to), NR_PIXBLOCK_BPP(from));
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

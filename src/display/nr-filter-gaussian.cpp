#define __NR_FILTER_GAUSSIAN_CPP__

/*
 * Gaussian blur renderer
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *   bulia byak
 *   Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 *
 * Copyright (C) 2006 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cmath>
#include <glib.h>

using std::isnormal;

#include "display/nr-filter-primitive.h"
#include "display/nr-filter-gaussian.h"
#include "display/nr-filter-types.h"
#include "libnr/nr-pixblock.h"
#include "libnr/nr-matrix.h"
#include "prefs-utils.h"

template<typename T> static inline T sqr(T const v) { return v*v; }

namespace NR {

FilterGaussian::FilterGaussian()
{
    _deviation_x = _deviation_y = prefs_get_double_attribute("options.filtertest", "value", 1.0);
}

FilterPrimitive *FilterGaussian::create()
{
    return new FilterGaussian();
}

FilterGaussian::~FilterGaussian()
{
    // Nothing to do here
}

int FilterGaussian::_kernel_size(double expansionX, double expansionY)
{
    int length_x = _effect_area_scr(_deviation_x, expansionX);
    int length_y = _effect_area_scr(_deviation_y, expansionY);
    return _max(length_x, length_y) + 1;
}

void FilterGaussian::_make_kernel(double *kernel, double deviation, double expansion)
{
    int const scr_len = _effect_area_scr(deviation, expansion);
    double const d_sq = sqr(deviation * expansion) * 2;

    // Compute kernel and sum of coefficients
    // Note that actually only half the kernel is computed, as it is symmetric
    double sum = 0;
    for ( int i = 0; i <= scr_len ; i++ ) {
        kernel[i] = std::exp(-sqr(i) / d_sq);
        sum += kernel[i];
    }
    sum = 2*sum - kernel[0]; // the sum of the complete kernel is twice as large (minus the center element to avoid counting it twice)

    // Normalize kernel
    for ( int i = 0; i <= scr_len ; i++ ) {
        kernel[i] /= sum;
    }
}

int FilterGaussian::_effect_area_scr(double deviation, double expansion)
{
    int ret = (int)std::ceil(deviation * 3.0 * expansion);
    return ret;
}

int FilterGaussian::_effect_subsample_step(int scr_len_x, int quality)
{
    switch (quality) {
        case BLUR_QUALITY_WORST:
            if (scr_len_x < 8) {
                return 1;
            } else if (scr_len_x < 32) {
                return 4;
            } else if (scr_len_x < 64) {
                return 8;
            } else if (scr_len_x < 128) {
                return 32;
            } else if (scr_len_x < 256) {
                return 128;
            } else if (scr_len_x < 512) {
                return 512;
            } else if (scr_len_x < 1024) {
                return 4096; 
            } else {
                return 65536;
            }
            break;
        case BLUR_QUALITY_WORSE:
            if (scr_len_x < 16) {
                return 1;
            } else if (scr_len_x < 64) {
                return 4;
            } else if (scr_len_x < 120) {
                return 8;
            } else if (scr_len_x < 200) {
                return 32;
            } else if (scr_len_x < 400) {
                return 64;
            } else if (scr_len_x < 800) {
                return 256;
            } else if (scr_len_x < 1200) {
                return 1024; 
            } else {
                return 65536;
            }
            break;
        case BLUR_QUALITY_BETTER:
            if (scr_len_x < 32) {
                return 1;
            } else if (scr_len_x < 160) {
                return 4;
            } else if (scr_len_x < 320) {
                return 8;
            } else if (scr_len_x < 640) {
                return 32;
            } else if (scr_len_x < 1280) {
                return 64;
            } else if (scr_len_x < 2560) {
                return 256;
            } else {
                return 1024;
            }
            break;
        case BLUR_QUALITY_BEST:
            return 1; // no subsampling at all
            break;
        case BLUR_QUALITY_NORMAL:
        default: 
            if (scr_len_x < 16) {
                return 1;
            } else if (scr_len_x < 80) {
                return 4;
            } else if (scr_len_x < 160) {
                return 8;
            } else if (scr_len_x < 320) {
                return 32;
            } else if (scr_len_x < 640) {
                return 64;
            } else if (scr_len_x < 1280) {
                return 256;
            } else if (scr_len_x < 2560) {
                return 1024; 
            } else {
                return 65536;
            }
            break;
    }
}

int FilterGaussian::_effect_subsample_step_log2(int scr_len_x, int quality)
{
    switch (quality) {
        case BLUR_QUALITY_WORST:
            if (scr_len_x < 8) {
                return 0;
            } else if (scr_len_x < 32) {
                return 2;
            } else if (scr_len_x < 64) {
                return 3;
            } else if (scr_len_x < 128) {
                return 5;
            } else if (scr_len_x < 256) {
                return 7;
            } else if (scr_len_x < 512) {
                return 9;
            } else if (scr_len_x < 1024) {
                return 12; 
            } else {
                return 16;
            }
            break;
        case BLUR_QUALITY_WORSE:
            if (scr_len_x < 16) {
                return 0;
            } else if (scr_len_x < 64) {
                return 2;
            } else if (scr_len_x < 120) {
                return 3;
            } else if (scr_len_x < 200) {
                return 5;
            } else if (scr_len_x < 400) {
                return 6;
            } else if (scr_len_x < 800) {
                return 8;
            } else if (scr_len_x < 1200) {
                return 10; 
            } else {
                return 16;
            }
            break;
        case BLUR_QUALITY_BETTER:
            if (scr_len_x < 32) {
                return 0;
            } else if (scr_len_x < 160) {
                return 2;
            } else if (scr_len_x < 320) {
                return 3;
            } else if (scr_len_x < 640) {
                return 5;
            } else if (scr_len_x < 1280) {
                return 6;
            } else if (scr_len_x < 2560) {
                return 8;
            } else {
                return 10;
            }
            break;
        case BLUR_QUALITY_BEST:
            return 0; // no subsampling at all
            break;
        case BLUR_QUALITY_NORMAL:
        default:
            if (scr_len_x < 16) {
                return 0;
            } else if (scr_len_x < 80) {
                return 2;
            } else if (scr_len_x < 160) {
                return 3;
            } else if (scr_len_x < 320) {
                return 5;
            } else if (scr_len_x < 640) {
                return 6;
            } else if (scr_len_x < 1280) {
                return 8;
            } else if (scr_len_x < 2560) {
                return 10; 
            } else {
                return 16;
            }
            break;
    }
}

/**
 * Sanity check function for indexing pixblocks.
 * Catches reading and writing outside the pixblock area.
 * When enabled, decreases filter rendering speed massively.
 */
inline void _check_index(NRPixBlock const * const pb, int const location, int const line)
{
    if (false) {
        int max_loc = pb->rs * (pb->area.y1 - pb->area.y0);
        if (location < 0 || location >= max_loc)
            g_warning("Location %d out of bounds (0 ... %d) at line %d", location, max_loc, line);
    }
}

int FilterGaussian::render(FilterSlot &slot, Matrix const &trans_)
{
    /* in holds the input pixblock */
    NRPixBlock *in = slot.get(_input);

    /* If to either direction, the standard deviation is zero or
     * input image is not defined,
     * a transparent black image should be returned. */
    if (_deviation_x <= 0 || _deviation_y <= 0 || in == NULL) {
        NRPixBlock *out = new NRPixBlock;
        if (in == NULL) {
            // A bit guessing here, but source graphic is likely to be of
            // right size
            in = slot.get(NR_FILTER_SOURCEGRAPHIC);
        }
        nr_pixblock_setup_fast(out, in->mode, in->area.x0, in->area.y0,
                               in->area.x1, in->area.y1, true);
        if (out->data.px != NULL) {
            out->empty = false;
            slot.set(_output, out);
        }
        return 0;
    }

    /* Blur radius in screen units (pixels) */
    double expansion_x = trans_.expansionX();
    double expansion_y = trans_.expansionY();
    int scr_len_x = _effect_area_scr(_deviation_x, expansion_x);
    int scr_len_y = _effect_area_scr(_deviation_y, expansion_y);

    // subsampling step; it depends on the radius, but somewhat nonlinearly, to make high zooms
    // workable; is adjustable by quality in -2..2; 0 is the default; 2 is the best quality with no
    // subsampling
    int quality = prefs_get_int_attribute ("options.blurquality", "value", 0);
    int stepx = _effect_subsample_step(scr_len_x, quality);
    int stepx_l2 = _effect_subsample_step_log2(scr_len_x, quality);
    int stepy = _effect_subsample_step(scr_len_y, quality);
    int stepy_l2 = _effect_subsample_step_log2(scr_len_y, quality);

    // Take subsampling into account
    expansion_x /= stepx;
    expansion_y /= stepy;
    scr_len_x = _effect_area_scr(_deviation_x, expansion_x);
    scr_len_y = _effect_area_scr(_deviation_y, expansion_y);

    /* buffer for x-axis blur */
    NRPixBlock *bufx = new NRPixBlock;
    /* buffer for y-axis blur */
    NRPixBlock *bufy = new NRPixBlock;

    // boundaries of the subsampled (smaller, unless step==1) buffers
    int xd0 = (in->area.x0 >> stepx_l2);
    int xd1 = (in->area.x1 >> stepx_l2) + 1;
    int yd0 = (in->area.y0 >> stepy_l2);
    int yd1 = (in->area.y1 >> stepy_l2) + 1;

    // set up subsampled buffers
    nr_pixblock_setup_fast(bufx, in->mode, xd0, yd0, xd1, yd1, true);
    nr_pixblock_setup_fast(bufy, in->mode, xd0, yd0, xd1, yd1, true);
    if (bufx->data.px == NULL || bufy->data.px == NULL) { // no memory
        return 0;
    }

    /* Array for filter kernel, big enough to fit kernels for both X and Y
     * direction kernel, one at time */
    double kernel[_kernel_size(expansion_x, expansion_y)];
    
    /* 1. Blur in direction of X-axis, from in to bufx (they have different resolution)*/
    _make_kernel(kernel, _deviation_x, expansion_x);

    for ( int y = bufx->area.y0 ; y < bufx->area.y1; y++ ) {

        // corresponding line in the source buffer
        int in_line;
        if ((y << stepy_l2) >= in->area.y1) {
            in_line = (in->area.y1 - in->area.y0 - 1) * in->rs;
        } else {
            in_line = ((y << stepy_l2) - (in->area.y0))  * in->rs;
            if (in_line < 0)
                in_line = 0;
        }

        // current line in bufx
        int bufx_line = (y - yd0) * bufx->rs;

        int skipbuf[4] = {INT_MIN, INT_MIN, INT_MIN, INT_MIN};

        for ( int x = bufx->area.x0 ; x < bufx->area.x1 ; x++ ) {

            // for all bytes of the pixel
            for ( int byte = 0 ; byte < NR_PIXBLOCK_BPP(in) ; byte++) {

                if(skipbuf[byte] > x) continue;

                double sum = 0;
                int last_in = -1;
                int different_count = 0;

                // go over our point's neighborhood on x axis in the in buffer, with stepx increment
                for ( int i = -scr_len_x ; i <= scr_len_x ; i++ ) {

                    // the pixel we're looking at
                    int x_in = (x+i)<<stepx_l2;

                    if (x_in >= in->area.x1) {
                        x_in = (in->area.x1 - in->area.x0 - 1);
                    } else {
                        x_in = (x_in - in->area.x0);
                        if (x_in < 0)
                            x_in = 0;
                    }

                    // value at the pixel
                    _check_index(in, in_line + NR_PIXBLOCK_BPP(in) * x_in + byte, __LINE__);
                    unsigned char in_byte = NR_PIXBLOCK_PX(in)[in_line + NR_PIXBLOCK_BPP(in) * x_in + byte];

                    // is it the same as last one we saw?
                    if(in_byte != last_in) different_count++;
                    last_in = in_byte;

                    // sum pixels weighted by the kernel
                    sum += in_byte * kernel[std::abs(i)];
                }

                // store the result in bufx
                _check_index(bufx, bufx_line + NR_PIXBLOCK_BPP(bufx) * (x - xd0) + byte, __LINE__);
                NR_PIXBLOCK_PX(bufx)[bufx_line + NR_PIXBLOCK_BPP(bufx) * (x - xd0) + byte] = (unsigned char)sum;

                // optimization: if there was no variation within this point's neighborhood, 
                // skip ahead while we keep seeing the same last_in byte: 
                // blurring flat color would not change it anyway
                if (different_count <= 1) {
                    int pos = x + 1;
                    while(((pos + scr_len_x) << stepx_l2) < in->area.x1 &&
                          NR_PIXBLOCK_PX(in)[in_line + NR_PIXBLOCK_BPP(in) * (((pos + scr_len_x) << stepx_l2) - in->area.x0) + byte] == last_in)
                    {
                        _check_index(in, in_line + NR_PIXBLOCK_BPP(in) * (((pos + scr_len_x) << stepx_l2) - in->area.x0) + byte, __LINE__);
                        _check_index(bufx, bufx_line + NR_PIXBLOCK_BPP(bufx) * (pos - xd0) + byte, __LINE__);
                        NR_PIXBLOCK_PX(bufx)[bufx_line + NR_PIXBLOCK_BPP(bufx) * (pos - xd0) + byte] = last_in;
                        pos++;
                    }
                    skipbuf[byte] = pos;
                }
            }
        }
    }


    /* 2. Blur in direction of Y-axis, from bufx to bufy (they have the same resolution) */
    _make_kernel(kernel, _deviation_y, expansion_y);

    for ( int x = bufy->area.x0 ; x < bufy->area.x1; x++ ) {

        int bufy_disp = NR_PIXBLOCK_BPP(bufy) * (x - xd0);
        int bufx_disp = NR_PIXBLOCK_BPP(bufx) * (x - xd0);

        int skipbuf[4] = {INT_MIN, INT_MIN, INT_MIN, INT_MIN};

        for ( int y = bufy->area.y0; y < bufy->area.y1; y++ ) {

            int bufy_line = (y - yd0) * bufy->rs;

            for ( int byte = 0 ; byte < NR_PIXBLOCK_BPP(bufx) ; byte++) {

                if (skipbuf[byte] > y) continue;

                double sum = 0;
                int last_in = -1;
                int different_count = 0;

                for ( int i = -scr_len_y ; i <= scr_len_y ; i ++ ) {

                    int y_in = y + i - yd0;

                    if (y_in >= (yd1 - yd0)) y_in = (yd1 - yd0) - 1;
                    if (y_in < 0) y_in = 0;

                    _check_index(bufx, y_in * bufx->rs + NR_PIXBLOCK_BPP(bufx) * (x - xd0) + byte, __LINE__);
                    unsigned char in_byte = NR_PIXBLOCK_PX(bufx)[y_in * bufx->rs + NR_PIXBLOCK_BPP(bufx) * (x - xd0) + byte];
                    if(in_byte != last_in) different_count++;
                    last_in = in_byte;
                    sum += in_byte * kernel[std::abs(i)];
                }

                _check_index(bufy, bufy_line + bufy_disp + byte, __LINE__);
                NR_PIXBLOCK_PX(bufy)[bufy_line + bufy_disp + byte] = (unsigned char)sum;

                if (different_count <= 1) {
                    int pos = y + 1;
                    while((pos + scr_len_y + 1) < yd1 &&
                          NR_PIXBLOCK_PX(bufx)[(pos + scr_len_y + 1 - yd0) * bufx->rs + bufx_disp + byte] == last_in)
                    {
                        _check_index(bufx, (pos + scr_len_y + 1 - yd0) * bufx->rs + bufx_disp + byte, __LINE__);
                        _check_index(bufy, (pos - yd0) * bufy->rs + bufy_disp + byte, __LINE__);
                        NR_PIXBLOCK_PX(bufy)[(pos - yd0) * bufy->rs + bufy_disp + byte] = last_in;
                        pos++;
                    }
                    skipbuf[byte] = pos;
                }

            }
        }
    }

    // we don't need bufx anymore
    nr_pixblock_release(bufx);
    delete bufx;

    // interpolation will need to divide by stepx * stepy
    int divisor = stepx_l2 + stepy_l2;

    // new buffer for the final output, same resolution as the in buffer
    NRPixBlock *out = new NRPixBlock;
    nr_pixblock_setup_fast(out, in->mode, in->area.x0, in->area.y0,
                           in->area.x1, in->area.y1, true);
    if (out->data.px == NULL) {
        // alas, we've accomplished a lot, but ran out of memory - so abort
        return 0;
    }

    for ( int y = yd0 ; y < yd1 - 1; y++ ) {
        for ( int x = xd0 ; x < xd1 - 1; x++ ) {
            for ( int byte = 0 ; byte < NR_PIXBLOCK_BPP(bufy) ; byte++) {

                // get 4 values at the corners of the pixel from bufy
                _check_index(bufy, ((y - yd0) * bufy->rs) + NR_PIXBLOCK_BPP(bufy) + (x - xd0) + byte, __LINE__);
                unsigned char a00 = NR_PIXBLOCK_PX(bufy)[((y - yd0) * bufy->rs) + NR_PIXBLOCK_BPP(bufy) * (x - xd0) + byte];
                if (stepx == 1 && stepy == 1) { // if there was no subsampling, just use a00
                    _check_index(out, ((y - yd0) * out->rs) + NR_PIXBLOCK_BPP(out) * (x - xd0) + byte, __LINE__);
                    NR_PIXBLOCK_PX(out)[((y - yd0) * out->rs) + NR_PIXBLOCK_BPP(out) * (x - xd0) + byte] = a00;
                    continue;
                }
                _check_index(bufy, ((y - yd0) * bufy->rs) + NR_PIXBLOCK_BPP(bufy) * (x + 1 - xd0) + byte, __LINE__);
                unsigned char a10 = NR_PIXBLOCK_PX(bufy)[((y - yd0) * bufy->rs) + NR_PIXBLOCK_BPP(bufy) * (x + 1 - xd0) + byte];
                _check_index(bufy, ((y + 1 - yd0) * bufy->rs) + NR_PIXBLOCK_BPP(bufy) * (x - xd0) + byte, __LINE__);
                unsigned char a01 = NR_PIXBLOCK_PX(bufy)[((y + 1 - yd0) * bufy->rs) + NR_PIXBLOCK_BPP(bufy) * (x - xd0) + byte];
                _check_index(bufy, ((y + 1 - yd0) * bufy->rs) + NR_PIXBLOCK_BPP(bufy) * (x + 1 - xd0) + byte, __LINE__);
                unsigned char a11 = NR_PIXBLOCK_PX(bufy)[((y + 1 - yd0) * bufy->rs) + NR_PIXBLOCK_BPP(bufy) * (x + 1 - xd0) + byte];

                // iterate over the rectangle to be interpolated
                for ( int yi = 0 ; yi < stepy; yi++ ) {
                    int iy = stepy - yi;
                    int y_out = (y << stepy_l2) + yi;
                    if ((y_out < out->area.y0) || (y_out >= out->area.y1))
                        continue;
                    int out_line  = (y_out - out->area.y0) * out->rs;

                    for ( int xi = 0 ; xi < stepx; xi++ ) {
                        int ix = stepx - xi;
                        int x_out = (x << stepx_l2) + xi;
                        if ((x_out < out->area.x0) || (x_out >= out->area.x1))
                            continue;

                        // simple linear interpolation
                        int a = (a00*ix*iy + a10*xi*iy + a01*ix*yi + a11*xi*yi) >> divisor;

                        _check_index(out, out_line + NR_PIXBLOCK_BPP(out) * (x_out - out->area.x0) + byte, __LINE__);
                        NR_PIXBLOCK_PX(out)[out_line + NR_PIXBLOCK_BPP(out) * (x_out - out->area.x0) + byte] = (unsigned char) a;
                    }
                }
            }
        }
    }

    nr_pixblock_release(bufy);
    delete bufy;

    out->empty = FALSE;
    slot.set(_output, out);

    return 0;
}

int FilterGaussian::get_enlarge(Matrix const &trans)
{
    int area_x = _effect_area_scr(_deviation_x, trans.expansionX());
    int area_y = _effect_area_scr(_deviation_y, trans.expansionY());
    return _max(area_x, area_y);
}

void FilterGaussian::set_deviation(double deviation)
{
    if(isnormal(deviation) && deviation >= 0) {
        _deviation_x = _deviation_y = deviation;
    }
}

void FilterGaussian::set_deviation(double x, double y)
{
    if(isnormal(x) && x >= 0 && isnormal(y) && y >= 0) {
        _deviation_x = x;
        _deviation_y = y;
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

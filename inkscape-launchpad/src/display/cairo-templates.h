/**
 * @file
 * Cairo software blending templates.
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2010 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_DISPLAY_CAIRO_TEMPLATES_H
#define SEEN_INKSCAPE_DISPLAY_CAIRO_TEMPLATES_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>

#ifdef HAVE_OPENMP
#include <omp.h>
#include "preferences.h"
// single-threaded operation if the number of pixels is below this threshold
static const int OPENMP_THRESHOLD = 2048;
#endif

#include <algorithm>
#include <cairo.h>
#include <math.h>
#include "display/nr-3dutils.h"
#include "display/cairo-utils.h"

/**
 * Blend two surfaces using the supplied functor.
 * This template blends two Cairo image surfaces using a blending functor that takes
 * two 32-bit ARGB pixel values and returns a modified 32-bit pixel value.
 * Differences in input surface formats are handled transparently. In future, this template
 * will also handle software fallback for GL surfaces.
 */
template <typename Blend>
void ink_cairo_surface_blend(cairo_surface_t *in1, cairo_surface_t *in2, cairo_surface_t *out, Blend blend)
{
    cairo_surface_flush(in1);
    cairo_surface_flush(in2);

    // ASSUMPTIONS
    // 1. Cairo ARGB32 surface strides are always divisible by 4
    // 2. We can only receive CAIRO_FORMAT_ARGB32 or CAIRO_FORMAT_A8 surfaces
    // 3. Both surfaces are of the same size
    // 4. Output surface is ARGB32 if at least one input is ARGB32

    int w = cairo_image_surface_get_width(in2);
    int h = cairo_image_surface_get_height(in2);
    int stride1   = cairo_image_surface_get_stride(in1);
    int stride2   = cairo_image_surface_get_stride(in2);
    int strideout = cairo_image_surface_get_stride(out);
    int bpp1   = cairo_image_surface_get_format(in1) == CAIRO_FORMAT_A8 ? 1 : 4;
    int bpp2   = cairo_image_surface_get_format(in2) == CAIRO_FORMAT_A8 ? 1 : 4;
    int bppout = std::max(bpp1, bpp2);

    // Check whether we can loop over pixels without taking stride into account.
    bool fast_path = true;
    fast_path &= (stride1 == w * bpp1);
    fast_path &= (stride2 == w * bpp2);
    fast_path &= (strideout == w * bppout);

    int limit = w * h;

    guint32 *const in1_data = reinterpret_cast<guint32*>(cairo_image_surface_get_data(in1));
    guint32 *const in2_data = reinterpret_cast<guint32*>(cairo_image_surface_get_data(in2));
    guint32 *const out_data = reinterpret_cast<guint32*>(cairo_image_surface_get_data(out));

    // NOTE
    // OpenMP probably doesn't help much here.
    // It would be better to render more than 1 tile at a time.
    #if HAVE_OPENMP
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int numOfThreads = prefs->getIntLimited("/options/threading/numthreads", omp_get_num_procs(), 1, 256);
    if (numOfThreads){} // inform compiler we are using it.
    #endif

    // The number of code paths here is evil.
    if (bpp1 == 4) {
        if (bpp2 == 4) {
            if (fast_path) {
                #if HAVE_OPENMP
                #pragma omp parallel for if(limit > OPENMP_THRESHOLD) num_threads(numOfThreads)
                #endif
                for (int i = 0; i < limit; ++i) {
                    *(out_data + i) = blend(*(in1_data + i), *(in2_data + i));
                }
            } else {
                #if HAVE_OPENMP
                #pragma omp parallel for if(limit > OPENMP_THRESHOLD) num_threads(numOfThreads)
                #endif
                for (int i = 0; i < h; ++i) {
                    guint32 *in1_p = in1_data + i * stride1/4;
                    guint32 *in2_p = in2_data + i * stride2/4;
                    guint32 *out_p = out_data + i * strideout/4;
                    for (int j = 0; j < w; ++j) {
                        *out_p = blend(*in1_p, *in2_p);
                        ++in1_p; ++in2_p; ++out_p;
                    }
                }
            }
        } else {
            // bpp2 == 1
            #if HAVE_OPENMP
            #pragma omp parallel for if(limit > OPENMP_THRESHOLD) num_threads(numOfThreads)
            #endif
            for (int i = 0; i < h; ++i) {
                guint32 *in1_p = in1_data + i * stride1/4;
                guint8  *in2_p = reinterpret_cast<guint8*>(in2_data) + i * stride2;
                guint32 *out_p = out_data + i * strideout/4;
                for (int j = 0; j < w; ++j) {
                    guint32 in2_px = *in2_p;
                    in2_px <<= 24;
                    *out_p = blend(*in1_p, in2_px);
                    ++in1_p; ++in2_p; ++out_p;
                }
            }
        }
    } else {
        if (bpp2 == 4) {
            // bpp1 == 1
            #if HAVE_OPENMP
            #pragma omp parallel for if(limit > OPENMP_THRESHOLD) num_threads(numOfThreads)
            #endif
            for (int i = 0; i < h; ++i) {
                guint8  *in1_p = reinterpret_cast<guint8*>(in1_data) + i * stride1;
                guint32 *in2_p = in2_data + i * stride2/4;
                guint32 *out_p = out_data + i * strideout/4;
                for (int j = 0; j < w; ++j) {
                    guint32 in1_px = *in1_p;
                    in1_px <<= 24;
                    *out_p = blend(in1_px, *in2_p);
                    ++in1_p; ++in2_p; ++out_p;
                }
            }
        } else {
            // bpp1 == 1 && bpp2 == 1
            if (fast_path) {
                #if HAVE_OPENMP
                #pragma omp parallel for if(limit > OPENMP_THRESHOLD) num_threads(numOfThreads)
                #endif
                for (int i = 0; i < limit; ++i) {
                    guint8 *in1_p = reinterpret_cast<guint8*>(in1_data) + i;
                    guint8 *in2_p = reinterpret_cast<guint8*>(in2_data) + i;
                    guint8 *out_p = reinterpret_cast<guint8*>(out_data) + i;
                    guint32 in1_px = *in1_p; in1_px <<= 24;
                    guint32 in2_px = *in2_p; in2_px <<= 24;
                    guint32 out_px = blend(in1_px, in2_px);
                    *out_p = out_px >> 24;
                }
            } else {
                #if HAVE_OPENMP
                #pragma omp parallel for if(limit > OPENMP_THRESHOLD) num_threads(numOfThreads)
                #endif
                for (int i = 0; i < h; ++i) {
                    guint8 *in1_p = reinterpret_cast<guint8*>(in1_data) + i * stride1;
                    guint8 *in2_p = reinterpret_cast<guint8*>(in2_data) + i * stride2;
                    guint8 *out_p = reinterpret_cast<guint8*>(out_data) + i * strideout;
                    for (int j = 0; j < w; ++j) {
                        guint32 in1_px = *in1_p; in1_px <<= 24;
                        guint32 in2_px = *in2_p; in2_px <<= 24;
                        guint32 out_px = blend(in1_px, in2_px);
                        *out_p = out_px >> 24;
                        ++in1_p; ++in2_p; ++out_p;
                    }
                }
            }
        }
    }

    cairo_surface_mark_dirty(out);
}

template <typename Filter>
void ink_cairo_surface_filter(cairo_surface_t *in, cairo_surface_t *out, Filter filter)
{
    cairo_surface_flush(in);

    // ASSUMPTIONS
    // 1. Cairo ARGB32 surface strides are always divisible by 4
    // 2. We can only receive CAIRO_FORMAT_ARGB32 or CAIRO_FORMAT_A8 surfaces
    // 3. Surfaces have the same dimensions
    // 4. Output surface is A8 if input is A8

    int w = cairo_image_surface_get_width(in);
    int h = cairo_image_surface_get_height(in);
    int stridein   = cairo_image_surface_get_stride(in);
    int strideout = cairo_image_surface_get_stride(out);
    int bppin = cairo_image_surface_get_format(in) == CAIRO_FORMAT_A8 ? 1 : 4;
    int bppout = cairo_image_surface_get_format(out) == CAIRO_FORMAT_A8 ? 1 : 4;
    int limit = w * h;

    // Check whether we can loop over pixels without taking stride into account.
    bool fast_path = true;
    fast_path &= (stridein == w * bppin);
    fast_path &= (strideout == w * bppout);

    guint32 *const in_data  = reinterpret_cast<guint32*>(cairo_image_surface_get_data(in));
    guint32 *const out_data = reinterpret_cast<guint32*>(cairo_image_surface_get_data(out));

    #if HAVE_OPENMP
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int numOfThreads = prefs->getIntLimited("/options/threading/numthreads", omp_get_num_procs(), 1, 256);
    if (numOfThreads){} // inform compiler we are using it.
    #endif

    // this is provided just in case, to avoid problems with strict aliasing rules
    if (in == out) {
        if (bppin == 4) {
            #if HAVE_OPENMP
            #pragma omp parallel for if(limit > OPENMP_THRESHOLD) num_threads(numOfThreads)
            #endif
            for (int i = 0; i < limit; ++i) {
                *(in_data + i) = filter(*(in_data + i));
            }
        } else {
            #if HAVE_OPENMP
            #pragma omp parallel for if(limit > OPENMP_THRESHOLD) num_threads(numOfThreads)
            #endif
            for (int i = 0; i < limit; ++i) {
                guint8 *in_p = reinterpret_cast<guint8*>(in_data) + i;
                guint32 in_px = *in_p; in_px <<= 24;
                guint32 out_px = filter(in_px);
                *in_p = out_px >> 24;
            }
        }
        cairo_surface_mark_dirty(out);
        return;
    }

    if (bppin == 4) {
        if (bppout == 4) {
            // bppin == 4, bppout == 4
            if (fast_path) {
                #if HAVE_OPENMP
                #pragma omp parallel for if(limit > OPENMP_THRESHOLD) num_threads(numOfThreads)
                #endif
                for (int i = 0; i < limit; ++i) {
                    *(out_data + i) = filter(*(in_data + i));
                }
            } else {
                #if HAVE_OPENMP
                #pragma omp parallel for if(limit > OPENMP_THRESHOLD) num_threads(numOfThreads)
                #endif
                for (int i = 0; i < h; ++i) {
                    guint32 *in_p = in_data + i * stridein/4;
                    guint32 *out_p = out_data + i * strideout/4;
                    for (int j = 0; j < w; ++j) {
                        *out_p = filter(*in_p);
                        ++in_p; ++out_p;
                    }
                }
            }
        } else {
            // bppin == 4, bppout == 1
            // we use this path with COLORMATRIX_LUMINANCETOALPHA
            #if HAVE_OPENMP
            #pragma omp parallel for if(limit > OPENMP_THRESHOLD) num_threads(numOfThreads)
            #endif
            for (int i = 0; i < h; ++i) {
                guint32 *in_p = in_data + i * stridein/4;
                guint8 *out_p = reinterpret_cast<guint8*>(out_data) + i * strideout;
                for (int j = 0; j < w; ++j) {
                    guint32 out_px = filter(*in_p);
                    *out_p = out_px >> 24;
                    ++in_p; ++out_p;
                }
            }
        }
    } else {
        // bppin == 1, bppout == 1
        // Note: there is no path for bppin == 1, bppout == 4 because it is useless
        if (fast_path) {
            #if HAVE_OPENMP
            #pragma omp parallel for if(limit > OPENMP_THRESHOLD) num_threads(numOfThreads)
            #endif
            for (int i = 0; i < limit; ++i) {
                guint8 *in_p = reinterpret_cast<guint8*>(in_data) + i;
                guint8 *out_p = reinterpret_cast<guint8*>(out_data) + i;
                guint32 in_px = *in_p; in_px <<= 24;
                guint32 out_px = filter(in_px);
                *out_p = out_px >> 24;
            }
        } else {
            #if HAVE_OPENMP
            #pragma omp parallel for if(limit > OPENMP_THRESHOLD) num_threads(numOfThreads)
            #endif
            for (int i = 0; i < h; ++i) {
                guint8 *in_p = reinterpret_cast<guint8*>(in_data) + i * stridein;
                guint8 *out_p = reinterpret_cast<guint8*>(out_data) + i * strideout;
                for (int j = 0; j < w; ++j) {
                    guint32 in_px = *in_p; in_px <<= 24;
                    guint32 out_px = filter(in_px);
                    *out_p = out_px >> 24;
                    ++in_p; ++out_p;
                }
            }
        }
    }
    cairo_surface_mark_dirty(out);
}


/**
 * Synthesize surface pixels based on their position.
 * This template accepts a functor that gets called with the x and y coordinates of the pixels,
 * given as integers.
 * @param out       Output surface
 * @param out_area  The region of the output surface that should be synthesized
 * @param synth     Synthesis functor
 */
template <typename Synth>
void ink_cairo_surface_synthesize(cairo_surface_t *out, cairo_rectangle_t const &out_area, Synth synth)
{
    // ASSUMPTIONS
    // 1. Cairo ARGB32 surface strides are always divisible by 4
    // 2. We can only receive CAIRO_FORMAT_ARGB32 or CAIRO_FORMAT_A8 surfaces

    int w = out_area.width;
    int h = out_area.height;
    int strideout = cairo_image_surface_get_stride(out);
    int bppout = cairo_image_surface_get_format(out) == CAIRO_FORMAT_A8 ? 1 : 4;
    // NOTE: fast path is not used, because we would need 2 divisions to get pixel indices

    unsigned char *out_data = cairo_image_surface_get_data(out);

    #if HAVE_OPENMP
    int limit = w * h;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int numOfThreads = prefs->getIntLimited("/options/threading/numthreads", omp_get_num_procs(), 1, 256);
    if (numOfThreads){} // inform compiler we are using it.
    #endif

    if (bppout == 4) {
        #if HAVE_OPENMP
        #pragma omp parallel for if(limit > OPENMP_THRESHOLD) num_threads(numOfThreads)
        #endif
        for (int i = out_area.y; i < h; ++i) {
            guint32 *out_p = reinterpret_cast<guint32*>(out_data + i * strideout);
            for (int j = out_area.x; j < w; ++j) {
                *out_p = synth(j, i);
                ++out_p;
            }
        }
    } else {
        // bppout == 1
        #if HAVE_OPENMP
        #pragma omp parallel for if(limit > OPENMP_THRESHOLD) num_threads(numOfThreads)
        #endif
        for (int i = out_area.y; i < h; ++i) {
            guint8 *out_p = out_data + i * strideout;
            for (int j = out_area.x; j < w; ++j) {
                guint32 out_px = synth(j, i);
                *out_p = out_px >> 24;
                ++out_p;
            }
        }
    }
    cairo_surface_mark_dirty(out);
}

template <typename Synth>
void ink_cairo_surface_synthesize(cairo_surface_t *out, Synth synth)
{
    int w = cairo_image_surface_get_width(out);
    int h = cairo_image_surface_get_height(out);

    cairo_rectangle_t area;
    area.x = 0;
    area.y = 0;
    area.width = w;
    area.height = h;

    ink_cairo_surface_synthesize(out, area, synth);
}

struct SurfaceSynth {
    SurfaceSynth(cairo_surface_t *surface)
        : _px(cairo_image_surface_get_data(surface))
        , _w(cairo_image_surface_get_width(surface))
        , _h(cairo_image_surface_get_height(surface))
        , _stride(cairo_image_surface_get_stride(surface))
        , _alpha(cairo_surface_get_content(surface) == CAIRO_CONTENT_ALPHA)
    {
        cairo_surface_flush(surface);
    }

    guint32 pixelAt(int x, int y) const {
        if (_alpha) {
            unsigned char *px = _px + y*_stride + x;
            return *px << 24;
        } else {
            unsigned char *px = _px + y*_stride + x*4;
            return *reinterpret_cast<guint32*>(px);
        }
    }
    guint32 alphaAt(int x, int y) const {
        if (_alpha) {
            unsigned char *px = _px + y*_stride + x;
            return *px;
        } else {
            unsigned char *px = _px + y*_stride + x*4;
            guint32 p = *reinterpret_cast<guint32*>(px);
            return (p & 0xff000000) >> 24;
        }
    }

    // retrieve a pixel value with bilinear interpolation
    guint32 pixelAt(double x, double y) const {
        if (_alpha) {
            return alphaAt(x, y) << 24;
        }

        double xf = floor(x), yf = floor(y);
        int xi = xf, yi = yf;
        guint32 xif = round((x - xf) * 255), yif = round((y - yf) * 255);
        guint32 p00, p01, p10, p11;

        unsigned char *pxi = _px + yi*_stride + xi*4;
        guint32 *pxu = reinterpret_cast<guint32*>(pxi);
        guint32 *pxl = reinterpret_cast<guint32*>(pxi + _stride);
        p00 = *pxu;  p10 = *(pxu + 1);
        p01 = *pxl;  p11 = *(pxl + 1);

        guint32 comp[4];

        for (unsigned i = 0; i < 4; ++i) {
            guint32 shift = i*8;
            guint32 mask = 0xff << shift;
            guint32 c00 = (p00 & mask) >> shift;
            guint32 c10 = (p10 & mask) >> shift;
            guint32 c01 = (p01 & mask) >> shift;
            guint32 c11 = (p11 & mask) >> shift;

            guint32 iu = (255-xif) * c00 + xif * c10;
            guint32 il = (255-xif) * c01 + xif * c11;
            comp[i] = (255-yif) * iu + yif * il;
            comp[i] = (comp[i] + (255*255/2)) / (255*255);
        }

        guint32 result = comp[0] | (comp[1] << 8) | (comp[2] << 16) | (comp[3] << 24);
        return result;
    }

    // retrieve an alpha value with bilinear interpolation
    guint32 alphaAt(double x, double y) const {
        double xf = floor(x), yf = floor(y);
        int xi = xf, yi = yf;
        guint32 xif = round((x - xf) * 255), yif = round((y - yf) * 255);
        guint32 p00, p01, p10, p11;
        if (_alpha) {
            unsigned char *pxu = _px + yi*_stride + xi;
            unsigned char *pxl = pxu + _stride;
            p00 = *pxu;  p10 = *(pxu + 1);
            p01 = *pxl;  p11 = *(pxl + 1);
        } else {
            unsigned char *pxi = _px + yi*_stride + xi*4;
            guint32 *pxu = reinterpret_cast<guint32*>(pxi);
            guint32 *pxl = reinterpret_cast<guint32*>(pxi + _stride);
            p00 = (*pxu & 0xff000000) >> 24;  p10 = (*(pxu + 1) & 0xff000000) >> 24;
            p01 = (*pxl & 0xff000000) >> 24;  p11 = (*(pxl + 1) & 0xff000000) >> 24;
        }
        guint32 iu = (255-xif) * p00 + xif * p10;
        guint32 il = (255-xif) * p01 + xif * p11;
        guint32 result = (255-yif) * iu + yif * il;
        result = (result + (255*255/2)) / (255*255);
        return result;
    }

    // compute surface normal at given coordinates using 3x3 Sobel gradient filter
    NR::Fvector surfaceNormalAt(int x, int y, double scale) const {
        // Below there are some multiplies by zero. They will be optimized out.
        // Do not remove them, because they improve readability.
        // NOTE: fetching using alphaAt is slightly lazy.
        NR::Fvector normal;
        double fx = -scale/255.0, fy = -scale/255.0;
        normal[Z_3D] = 1.0;
        if (G_UNLIKELY(x == 0)) {
            // leftmost column
            if (G_UNLIKELY(y == 0)) {
                // upper left corner
                fx *= (2.0/3.0);
                fy *= (2.0/3.0);
                double p00 = alphaAt(x,y),   p10 = alphaAt(x+1, y),
                       p01 = alphaAt(x,y+1), p11 = alphaAt(x+1, y+1);
                normal[X_3D] =
                    -2.0 * p00 +2.0 * p10
                    -1.0 * p01 +1.0 * p11;
                normal[Y_3D] = 
                    -2.0 * p00 -1.0 * p10
                    +2.0 * p01 +1.0 * p11;
            } else if (G_UNLIKELY(y == (_h - 1))) {
                // lower left corner
                fx *= (2.0/3.0);
                fy *= (2.0/3.0);
                double p00 = alphaAt(x,y-1), p10 = alphaAt(x+1, y-1),
                       p01 = alphaAt(x,y  ), p11 = alphaAt(x+1, y);
                normal[X_3D] =
                    -1.0 * p00 +1.0 * p10
                    -2.0 * p01 +2.0 * p11;
                normal[Y_3D] = 
                    -2.0 * p00 -1.0 * p10
                    +2.0 * p01 +1.0 * p11;
            } else {
                // leftmost column
                fx *= (1.0/2.0);
                fy *= (1.0/3.0);
                double p00 = alphaAt(x, y-1), p10 = alphaAt(x+1, y-1),
                       p01 = alphaAt(x, y  ), p11 = alphaAt(x+1, y  ),
                       p02 = alphaAt(x, y+1), p12 = alphaAt(x+1, y+1);
                normal[X_3D] =
                    -1.0 * p00 +1.0 * p10
                    -2.0 * p01 +2.0 * p11
                    -1.0 * p02 +1.0 * p12;
                normal[Y_3D] =
                    -2.0 * p00 -1.0 * p10
                    +0.0 * p01 +0.0 * p11 // this will be optimized out
                    +2.0 * p02 +1.0 * p12;
            }
        } else if (G_UNLIKELY(x == (_w - 1))) {
            // rightmost column
            if (G_UNLIKELY(y == 0)) {
                // top right corner
                fx *= (2.0/3.0);
                fy *= (2.0/3.0);
                double p00 = alphaAt(x-1,y),   p10 = alphaAt(x, y),
                       p01 = alphaAt(x-1,y+1), p11 = alphaAt(x, y+1);
                normal[X_3D] =
                    -2.0 * p00 +2.0 * p10
                    -1.0 * p01 +1.0 * p11;
                normal[Y_3D] = 
                    -1.0 * p00 -2.0 * p10
                    +1.0 * p01 +2.0 * p11;
            } else if (G_UNLIKELY(y == (_h - 1))) {
                // bottom right corner
                fx *= (2.0/3.0);
                fy *= (2.0/3.0);
                double p00 = alphaAt(x-1,y-1), p10 = alphaAt(x, y-1),
                       p01 = alphaAt(x-1,y  ), p11 = alphaAt(x, y);
                normal[X_3D] =
                    -1.0 * p00 +1.0 * p10
                    -2.0 * p01 +2.0 * p11;
                normal[Y_3D] = 
                    -1.0 * p00 -2.0 * p10
                    +1.0 * p01 +2.0 * p11;
            } else {
                // rightmost column
                fx *= (1.0/2.0);
                fy *= (1.0/3.0);
                double p00 = alphaAt(x-1, y-1), p10 = alphaAt(x, y-1),
                       p01 = alphaAt(x-1, y  ), p11 = alphaAt(x, y  ),
                       p02 = alphaAt(x-1, y+1), p12 = alphaAt(x, y+1);
                normal[X_3D] =
                    -1.0 * p00 +1.0 * p10
                    -2.0 * p01 +2.0 * p11
                    -1.0 * p02 +1.0 * p12;
                normal[Y_3D] =
                    -1.0 * p00 -2.0 * p10
                    +0.0 * p01 +0.0 * p11
                    +1.0 * p02 +2.0 * p12;
            }
        } else {
            // interior
            if (G_UNLIKELY(y == 0)) {
                // top row
                fx *= (1.0/3.0);
                fy *= (1.0/2.0);
                double p00 = alphaAt(x-1, y  ), p10 = alphaAt(x, y  ), p20 = alphaAt(x+1, y  ),
                       p01 = alphaAt(x-1, y+1), p11 = alphaAt(x, y+1), p21 = alphaAt(x+1, y+1);
                normal[X_3D] =
                    -2.0 * p00 +0.0 * p10 +2.0 * p20
                    -1.0 * p01 +0.0 * p11 +1.0 * p21;
                normal[Y_3D] =
                    -1.0 * p00 -2.0 * p10 -1.0 * p20
                    +1.0 * p01 +2.0 * p11 +1.0 * p21;
            } else if (G_UNLIKELY(y == (_h - 1))) {
                // bottom row
                fx *= (1.0/3.0);
                fy *= (1.0/2.0);
                double p00 = alphaAt(x-1, y-1), p10 = alphaAt(x, y-1), p20 = alphaAt(x+1, y-1),
                       p01 = alphaAt(x-1, y  ), p11 = alphaAt(x, y  ), p21 = alphaAt(x+1, y  );
                normal[X_3D] =
                    -1.0 * p00 +0.0 * p10 +1.0 * p20
                    -2.0 * p01 +0.0 * p11 +2.0 * p21;
                normal[Y_3D] =
                    -1.0 * p00 -2.0 * p10 -1.0 * p20
                    +1.0 * p01 +2.0 * p11 +1.0 * p21;
            } else {
                // interior pixels
                // note: p11 is actually unused, so we don't fetch its value
                fx *= (1.0/4.0);
                fy *= (1.0/4.0);
                double p00 = alphaAt(x-1, y-1), p10 = alphaAt(x, y-1), p20 = alphaAt(x+1, y-1),
                       p01 = alphaAt(x-1, y  ), p11 = 0.0,             p21 = alphaAt(x+1, y  ),
                       p02 = alphaAt(x-1, y+1), p12 = alphaAt(x, y+1), p22 = alphaAt(x+1, y+1);
                normal[X_3D] =
                    -1.0 * p00 +0.0 * p10 +1.0 * p20
                    -2.0 * p01 +0.0 * p11 +2.0 * p21
                    -1.0 * p02 +0.0 * p12 +1.0 * p22;
                normal[Y_3D] =
                    -1.0 * p00 -2.0 * p10 -1.0 * p20
                    +0.0 * p01 +0.0 * p11 +0.0 * p21
                    +1.0 * p02 +2.0 * p12 +1.0 * p22;
            }
        }
        normal[X_3D] *= fx;
        normal[Y_3D] *= fy;
        NR::normalize_vector(normal);
        return normal;
    }

    unsigned char *_px;
    int _w, _h, _stride;
    bool _alpha;
};

/*
// simple pixel accessor for image surface that handles different edge wrapping modes
class PixelAccessor {
public:
    typedef PixelAccessor self;
    enum EdgeMode {
        EDGE_PAD,
        EDGE_WRAP,
        EDGE_ZERO
    };

    PixelAccessor(cairo_surface_t *s, EdgeMode e)
        : _surface(s)
        , _px(cairo_image_surface_get_data(s))
        , _x(0), _y(0)
        , _w(cairo_image_surface_get_width(s))
        , _h(cairo_image_surface_get_height(s))
        , _stride(cairo_image_surface_get_stride(s))
        , _edge_mode(e)
        , _alpha(cairo_image_surface_get_format(s) == CAIRO_FORMAT_A8)
    {}

    guint32 pixelAt(int x, int y) {
        // This is a lot of ifs for a single pixel access. However, branch prediction
        // should help us a lot, as the result of ifs is always the same for a single image.
        int real_x = x, real_y = y;
        switch (_edge_mode) {
        case EDGE_PAD:
            real_x = CLAMP(x, 0, _w-1);
            real_y = CLAMP(y, 0, _h-1);
            break;
        case EDGE_WRAP:
            real_x %= _w;
            real_y %= _h;
            break;
        case EDGE_ZERO:
        default:
            if (x < 0 || x >= _w || y < 0 || y >= _h)
                return 0;
            break;
        }
        if (_alpha) {
            return *(_px + real_y*_stride + real_x) << 24;
        } else {
            guint32 *px = reinterpret_cast<guint32*>(_px +real_y*_stride + real_x*4);
            return *px;
        }
    }
private:
    cairo_surface_t *_surface;
    guint8 *_px;
    int _x, _y, _w, _h, _stride;
    EdgeMode _edge_mode;
    bool _alpha;
};*/

// Some helpers for pixel manipulation
G_GNUC_CONST inline gint32
pxclamp(gint32 v, gint32 low, gint32 high) {
    // NOTE: it is possible to write a "branchless" clamping operation.
    // However, it will be slower than this function, because the code below
    // is compiled to conditional moves.
    if (v < low) return low;
    if (v > high) return high;
    return v;
}

#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

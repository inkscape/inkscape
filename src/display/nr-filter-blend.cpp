/*
 * SVG feBlend renderer
 *
 * "This filter composites two objects together using commonly used
 * imaging software blending modes. It performs a pixel-wise combination
 * of two input images." 
 * http://www.w3.org/TR/SVG11/filters.html#feBlend
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-blend.h"
#include "display/nr-filter-primitive.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-types.h"
#include "libnr/nr-pixblock.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-blit.h"
#include "libnr/nr-pixops.h"

template <void(*blend)(unsigned char *cr, unsigned char const *ca, unsigned char const *cb)>
static void _render(NRPixBlock &out, NRPixBlock &in1, NRPixBlock &in2) {
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

/*
 * From http://www.w3.org/TR/SVG11/filters.html#feBlend
 *
 * For all feBlend modes, the result opacity is computed as follows:
 * qr = 1 - (1-qa)*(1-qb)
 *
 * For the compositing formulas below, the following definitions apply:
 * cr = Result color (RGB) - premultiplied
 * qa = Opacity value at a given pixel for image A
 * qb = Opacity value at a given pixel for image B
 * ca = Color (RGB) at a given pixel for image A - premultiplied
 * cb = Color (RGB) at a given pixel for image B - premultiplied
 */

/*
 * These blending equations given in SVG standard are for color values
 * in the range 0..1. As these values are stored as unsigned char values,
 * they need some reworking. An unsigned char value can be thought as
 * 0.8 fixed point representation of color value. This is how I've
 * ended up with these equations here.
 */

// Set alpha / opacity. This line is same for all the blending modes,
// so let's save some copy-pasting.
#define SET_ALPHA r[3] = NR_NORMALIZE_21((255 * 255) - (255 - a[3]) * (255 - b[3]))

// cr = (1 - qa) * cb + ca
inline void blend_normal(unsigned char *r, unsigned char const *a, unsigned char const *b) {
    r[0] = NR_NORMALIZE_21((255 - a[3]) * b[0]) + a[0];
    r[1] = NR_NORMALIZE_21((255 - a[3]) * b[1]) + a[1];
    r[2] = NR_NORMALIZE_21((255 - a[3]) * b[2]) + a[2];
    SET_ALPHA;
}

// cr = (1-qa)*cb + (1-qb)*ca + ca*cb
inline void blend_multiply(unsigned char *r, unsigned char const *a, unsigned char const *b) {
    r[0] = NR_NORMALIZE_21((255 - a[3]) * b[0] + (255 - b[3]) * a[0]
                           + a[0] * b[0]);
    r[1] = NR_NORMALIZE_21((255 - a[3]) * b[1] + (255 - b[3]) * a[1]
                           + a[1] * b[1]);
    r[2] = NR_NORMALIZE_21((255 - a[3]) * b[2] + (255 - b[3]) * a[2]
                           + a[2] * b[2]);
    SET_ALPHA;
}

// cr = cb + ca - ca * cb
inline void blend_screen(unsigned char *r, unsigned char const *a, unsigned char const *b) {
    r[0] = NR_NORMALIZE_21(b[0] * 255 + a[0] * 255 - a[0] * b[0]);
    r[1] = NR_NORMALIZE_21(b[1] * 255 + a[1] * 255 - a[1] * b[1]);
    r[2] = NR_NORMALIZE_21(b[2] * 255 + a[2] * 255 - a[2] * b[2]);
    SET_ALPHA;
}

// cr = Min ((1 - qa) * cb + ca, (1 - qb) * ca + cb)
inline void blend_darken(unsigned char *r, unsigned char const *a, unsigned char const *b) {
    r[0] = std::min(NR_NORMALIZE_21((255 - a[3]) * b[0]) + a[0],
                    NR_NORMALIZE_21((255 - b[3]) * a[0]) + b[0]);
    r[1] = std::min(NR_NORMALIZE_21((255 - a[3]) * b[1]) + a[1],
                    NR_NORMALIZE_21((255 - b[3]) * a[1]) + b[1]);
    r[2] = std::min(NR_NORMALIZE_21((255 - a[3]) * b[2]) + a[2],
                    NR_NORMALIZE_21((255 - b[3]) * a[2]) + b[2]);
    SET_ALPHA;
}

// cr = Max ((1 - qa) * cb + ca, (1 - qb) * ca + cb)
inline void blend_lighten(unsigned char *r, unsigned char const *a, unsigned char const *b) {
    r[0] = std::max(NR_NORMALIZE_21((255 - a[3]) * b[0]) + a[0],
                    NR_NORMALIZE_21((255 - b[3]) * a[0]) + b[0]);
    r[1] = std::max(NR_NORMALIZE_21((255 - a[3]) * b[1]) + a[1],
                    NR_NORMALIZE_21((255 - b[3]) * a[1]) + b[1]);
    r[2] = std::max(NR_NORMALIZE_21((255 - a[3]) * b[2]) + a[2],
                    NR_NORMALIZE_21((255 - b[3]) * a[2]) + b[2]);
    SET_ALPHA;
}

namespace NR {

FilterBlend::FilterBlend() 
    : _blend_mode(BLEND_NORMAL),
      _input2(NR_FILTER_SLOT_NOT_SET)
{}

FilterPrimitive * FilterBlend::create() {
    return new FilterBlend();
}

FilterBlend::~FilterBlend()
{}

int FilterBlend::render(FilterSlot &slot, Matrix const &trans) {
    NRPixBlock *in1 = slot.get(_input);
    NRPixBlock *in2 = slot.get(_input2);
    NRPixBlock *original_in1 = in1;
    NRPixBlock *original_in2 = in2;
    NRPixBlock *out;

    // Bail out if either one of source images is missing
    if (!in1 || !in2) {
        g_warning("Missing source image for feBlend (in=%d in2=%d)", _input, _input2);
        return 1;
    }

    out = new NRPixBlock;
    NRRectL out_area;
    nr_rect_l_union(&out_area, &in1->area, &in2->area);
    nr_pixblock_setup_fast(out, NR_PIXBLOCK_MODE_R8G8B8A8P,
                           out_area.x0, out_area.y0, out_area.x1, out_area.y1,
                           true);

    // Blending modes are defined for premultiplied RGBA values,
    // thus convert them to that format before blending
    if (in1->mode != NR_PIXBLOCK_MODE_R8G8B8A8P) {
        in1 = new NRPixBlock;
        nr_pixblock_setup_fast(in1, NR_PIXBLOCK_MODE_R8G8B8A8P,
                               original_in1->area.x0, original_in1->area.y0,
                               original_in1->area.x1, original_in1->area.y1,
                               false);
        nr_blit_pixblock_pixblock(in1, original_in1);
    }
    if (in2->mode != NR_PIXBLOCK_MODE_R8G8B8A8P) {
        in2 = new NRPixBlock;
        nr_pixblock_setup_fast(in2, NR_PIXBLOCK_MODE_R8G8B8A8P,
                               original_in2->area.x0, original_in2->area.y0,
                               original_in2->area.x1, original_in2->area.y1,
                               false);
        nr_blit_pixblock_pixblock(in2, original_in2);
    }

    switch (_blend_mode) {
        case BLEND_MULTIPLY:
            _render<blend_multiply>(*out, *in1, *in2);
            break;
        case BLEND_SCREEN:
            _render<blend_screen>(*out, *in1, *in2);
            break;
        case BLEND_DARKEN:
            _render<blend_darken>(*out, *in1, *in2);
            break;
        case BLEND_LIGHTEN:
            _render<blend_lighten>(*out, *in1, *in2);
            break;
        case BLEND_NORMAL:
        default:
            _render<blend_normal>(*out, *in1, *in2);
            break;
    }

    if (in1 != original_in1) {
        nr_pixblock_release(in1);
        delete in1;
    }
    if (in2 != original_in2) {
        nr_pixblock_release(in2);
        delete in2;
    }

    out->empty = FALSE;
    slot.set(_output, out);

    return 0;
}

void FilterBlend::set_input(int slot) {
    _input = slot;
}

void FilterBlend::set_input(int input, int slot) {
    if (input == 0) _input = slot;
    if (input == 1) _input2 = slot;
}

void FilterBlend::set_mode(FilterBlendMode mode) {
    if (mode == BLEND_NORMAL || mode == BLEND_MULTIPLY ||
        mode == BLEND_SCREEN || mode == BLEND_DARKEN ||
        mode == BLEND_LIGHTEN)
    {
        _blend_mode = mode;
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

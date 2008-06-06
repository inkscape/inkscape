/*
 * feComposite filter effect renderer
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cmath>

#include "isnan.h"
#include "sp-fecomposite.h"
#include "display/nr-filter-composite.h"
#include "display/nr-filter-pixops.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
#include "display/nr-filter-utils.h"
#include "libnr/nr-blit.h"
#include "libnr/nr-pixblock.h"
#include "libnr/nr-pixops.h"
#include "libnr/nr-matrix.h"

inline void
composite_over(unsigned char *r, unsigned char const *a, unsigned char const *b)
{
    r[0] = a[0] + NR_NORMALIZE_21(b[0] * (255 - a[3]));
    r[1] = a[1] + NR_NORMALIZE_21(b[1] * (255 - a[3]));
    r[2] = a[2] + NR_NORMALIZE_21(b[2] * (255 - a[3]));
    r[3] = a[3] + NR_NORMALIZE_21(b[3] * (255 - a[3]));
}

inline void
composite_in(unsigned char *r, unsigned char const *a, unsigned char const *b)
{
    r[0] = NR_NORMALIZE_21(a[0] * b[3]);
    r[1] = NR_NORMALIZE_21(a[1] * b[3]);
    r[2] = NR_NORMALIZE_21(a[2] * b[3]);
    r[3] = NR_NORMALIZE_21(a[3] * b[3]);
}

inline void
composite_out(unsigned char *r, unsigned char const *a, unsigned char const *b)
{
    r[0] = NR_NORMALIZE_21(a[0] * (255 - b[3]));
    r[1] = NR_NORMALIZE_21(a[1] * (255 - b[3]));
    r[2] = NR_NORMALIZE_21(a[2] * (255 - b[3]));
    r[3] = NR_NORMALIZE_21(a[3] * (255 - b[3]));
}

inline void
composite_atop(unsigned char *r, unsigned char const *a, unsigned char const *b)
{
    r[0] = NR_NORMALIZE_21(a[0] * b[3] + b[0] * (255 - a[3]));
    r[1] = NR_NORMALIZE_21(a[1] * b[3] + b[1] * (255 - a[3]));
    r[2] = NR_NORMALIZE_21(a[2] * b[3] + b[2] * (255 - a[3]));
    r[3] = b[3];
}

inline void
composite_xor(unsigned char *r, unsigned char const *a, unsigned char const *b)
{
    r[0] = NR_NORMALIZE_21(a[0] * (255 - b[3]) + b[0] * (255 - a[3]));
    r[1] = NR_NORMALIZE_21(a[1] * (255 - b[3]) + b[1] * (255 - a[3]));
    r[2] = NR_NORMALIZE_21(a[2] * (255 - b[3]) + b[2] * (255 - a[3]));
    r[3] = NR_NORMALIZE_21(a[3] * (255 - b[3]) + b[3] * (255 - a[3]));
}

// BUGBUG / TODO
// This makes arithmetic compositing non re-entrant and non thread safe.
static int arith_k1, arith_k2, arith_k3, arith_k4;
inline void
composite_arithmetic(unsigned char *r, unsigned char const *a, unsigned char const *b)
{
    r[0] = NR_NORMALIZE_31(NR::clamp3(arith_k1 * a[0] * b[0]
                 + arith_k2 * a[0] + arith_k3 * b[0] + arith_k4));
    r[1] = NR_NORMALIZE_31(NR::clamp3(arith_k1 * a[1] * b[1]
                 + arith_k2 * a[1] + arith_k3 * b[1] + arith_k4));
    r[2] = NR_NORMALIZE_31(NR::clamp3(arith_k1 * a[2] * b[2]
                 + arith_k2 * a[2] + arith_k3 * b[2] + arith_k4));
    r[3] = NR_NORMALIZE_31(NR::clamp3(arith_k1 * a[3] * b[3]
                 + arith_k2 * a[3] + arith_k3 * b[3] + arith_k4));
}

namespace NR {

FilterComposite::FilterComposite() :
    op(COMPOSITE_DEFAULT), k1(0), k2(0), k3(0), k4(0),
    _input2(NR::NR_FILTER_SLOT_NOT_SET)
{}

FilterPrimitive * FilterComposite::create() {
    return new FilterComposite();
}

FilterComposite::~FilterComposite()
{}

int FilterComposite::render(FilterSlot &slot, FilterUnits const &/*units*/) {
    NRPixBlock *in1 = slot.get(_input);
    NRPixBlock *in2 = slot.get(_input2);
    NRPixBlock *original_in1 = in1;
    NRPixBlock *original_in2 = in2;
    NRPixBlock *out;

    // Bail out if either one of source images is missing
    if (!in1 || !in2) {
        g_warning("Missing source image for feComposite (in=%d in2=%d)", _input, _input2);
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

    /* pixops_mix is defined in display/nr-filter-pixops.h
     * It mixes the two input images with the function given as template
     * and places the result in output image.
     */
    switch (op) {
        case COMPOSITE_IN:
            pixops_mix<composite_in>(*out, *in1, *in2);
            break;
        case COMPOSITE_OUT:
            pixops_mix<composite_out>(*out, *in1, *in2);
            break;
        case COMPOSITE_ATOP:
            pixops_mix<composite_atop>(*out, *in1, *in2);
            break;
        case COMPOSITE_XOR:
            pixops_mix<composite_xor>(*out, *in1, *in2);
            break;
        case COMPOSITE_ARITHMETIC:
            arith_k1 = (int)(k1 * 255);
            arith_k2 = (int)(k2 * 255 * 255);
            arith_k3 = (int)(k3 * 255 * 255);
            arith_k4 = (int)(k4 * 255 * 255 * 255);
            pixops_mix<composite_arithmetic>(*out, *in1, *in2);
            break;
        case COMPOSITE_DEFAULT:
        case COMPOSITE_OVER:
        default:
            pixops_mix<composite_over>(*out, *in1, *in2);
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

void FilterComposite::set_input(int input) {
    _input = input;
}

void FilterComposite::set_input(int input, int slot) {
    if (input == 0) _input = slot;
    if (input == 1) _input2 = slot;
}

void FilterComposite::set_operator(FeCompositeOperator op) {
    if (op == COMPOSITE_DEFAULT) {
        this->op = COMPOSITE_OVER;
    } else if (op == COMPOSITE_OVER ||
               op == COMPOSITE_IN ||
               op == COMPOSITE_OUT ||
               op == COMPOSITE_ATOP ||
               op == COMPOSITE_XOR ||
               op == COMPOSITE_ARITHMETIC)
    {
        this->op = op;
    }
}

void FilterComposite::set_arithmetic(double k1, double k2, double k3, double k4) {
    if (!IS_FINITE(k1) || !IS_FINITE(k2) || !IS_FINITE(k3) || !IS_FINITE(k4)) {
        g_warning("Non-finite parameter for feComposite arithmetic operator");
        return;
    }
    this->k1 = k1;
    this->k2 = k2;
    this->k3 = k3;
    this->k4 = k4;
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

/*
 * feMerge filter effect renderer
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cmath>
#include <vector>

#include "2geom/isnan.h"
#include "filters/merge.h"
#include "display/nr-filter-merge.h"
#include "display/nr-filter-pixops.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
#include "display/nr-filter-utils.h"
#include "libnr/nr-blit.h"
#include "libnr/nr-pixblock.h"
#include "libnr/nr-pixops.h"

inline void
composite_over(unsigned char *r, unsigned char const *a, unsigned char const *b)
{
    r[0] = a[0] + NR_NORMALIZE_21(b[0] * (255 - a[3]));
    r[1] = a[1] + NR_NORMALIZE_21(b[1] * (255 - a[3]));
    r[2] = a[2] + NR_NORMALIZE_21(b[2] * (255 - a[3]));
    r[3] = a[3] + NR_NORMALIZE_21(b[3] * (255 - a[3]));
}

namespace Inkscape {
namespace Filters {

FilterMerge::FilterMerge() :
    _input_image(1, NR_FILTER_SLOT_NOT_SET)
{}

FilterPrimitive * FilterMerge::create() {
    return new FilterMerge();
}

FilterMerge::~FilterMerge()
{}

int FilterMerge::render(FilterSlot &slot, FilterUnits const &/*units*/) {
    NRPixBlock *in[_input_image.size()];
    NRPixBlock *original_in[_input_image.size()];

    for (unsigned int i = 0 ; i < _input_image.size() ; i++) {
        in[i] = slot.get(_input_image[i]);
        original_in[i] = in[i];
    }

    NRPixBlock *out;

    // Bail out if one of source images is missing
    for (unsigned int i = 0 ; i < _input_image.size() ; i++) {
        bool missing = false;
        if (!in[i]) {
            g_warning("Missing source image for feMerge (number=%d slot=%d)", i, _input_image[i]);
            missing = true;
        }
        if (missing) return 1;
    }

    out = new NRPixBlock;
    NRRectL out_area = in[0]->area;
    for (unsigned int i = 1 ; i < _input_image.size() ; i++) {
        nr_rect_l_union(&out_area, &out_area, &in[i]->area);
    }
    nr_pixblock_setup_fast(out, NR_PIXBLOCK_MODE_R8G8B8A8P,
                           out_area.x0, out_area.y0, out_area.x1, out_area.y1,
                           true);

    // Merge is defined for premultiplied RGBA values, thus convert them to
    // that format before blending
    for (unsigned int i = 0 ; i < _input_image.size() ; i++) {
        if (in[i]->mode != NR_PIXBLOCK_MODE_R8G8B8A8P) {
            in[i] = new NRPixBlock;
            nr_pixblock_setup_fast(in[i], NR_PIXBLOCK_MODE_R8G8B8A8P,
                                   original_in[i]->area.x0,
                                   original_in[i]->area.y0,
                                   original_in[i]->area.x1,
                                   original_in[i]->area.y1,
                                   false);
            nr_blit_pixblock_pixblock(in[i], original_in[i]);
        }
    }

    /* pixops_mix is defined in display/nr-filter-pixops.h
     * It mixes the two input images with the function given as template
     * and places the result in output image.
     */
    for (unsigned int i = 0 ; i < _input_image.size() ; i++) {
        pixops_mix<composite_over>(*out, *in[i], *out);
    }

    for (unsigned int i = 0 ; i < _input_image.size() ; i++) {
        if (in[i] != original_in[i]) {
            nr_pixblock_release(in[i]);
            delete in[i];
        }
    }

    out->empty = FALSE;
    slot.set(_output, out);

    return 0;
}

void FilterMerge::set_input(int slot) {
    _input_image[0] = slot;
}

void FilterMerge::set_input(int input, int slot) {
    if (input < 0) return;

    if (static_cast<int>(_input_image.size()) > input) {
        _input_image[input] = slot;
    } else {
        for (int i = static_cast<int>(_input_image.size()) ; i < input ; i++) {
            _input_image.push_back(NR_FILTER_SLOT_NOT_SET);
        }
        _input_image.push_back(slot);
    }
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

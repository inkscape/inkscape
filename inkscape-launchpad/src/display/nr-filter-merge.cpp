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

#include <vector>
#include "display/cairo-utils.h"
#include "display/nr-filter-merge.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-utils.h"

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

void FilterMerge::render_cairo(FilterSlot &slot)
{
    if (_input_image.empty()) return;

    SPColorInterpolation ci_fp  = SP_CSS_COLOR_INTERPOLATION_AUTO;
    if( _style ) {
        ci_fp = (SPColorInterpolation)_style->color_interpolation_filters.computed;
    }

    // output is RGBA if at least one input is RGBA
    bool rgba32 = false;
    cairo_surface_t *out = NULL;
    for (std::vector<int>::iterator i = _input_image.begin(); i != _input_image.end(); ++i) {
        cairo_surface_t *in = slot.getcairo(*i);
        if (cairo_surface_get_content(in) == CAIRO_CONTENT_COLOR_ALPHA) {
            out = ink_cairo_surface_create_identical(in);
            set_cairo_surface_ci( out, ci_fp );
            rgba32 = true;
            break;
        }
    }

    if (!rgba32) {
        out = ink_cairo_surface_create_identical(slot.getcairo(_input_image[0]));
    }
    cairo_t *out_ct = cairo_create(out);

    for (std::vector<int>::iterator i = _input_image.begin(); i != _input_image.end(); ++i) {
        cairo_surface_t *in = slot.getcairo(*i);

        set_cairo_surface_ci( in, ci_fp );
        cairo_set_source_surface(out_ct, in, 0, 0);
        cairo_paint(out_ct);
    }

    cairo_destroy(out_ct);
    slot.set(_output, out);
    cairo_surface_destroy(out);
}

bool FilterMerge::can_handle_affine(Geom::Affine const &)
{
    // Merge is a per-pixel primitive and is immutable under transformations
    return true;
}

double FilterMerge::complexity(Geom::Affine const &)
{
    return 1.02;
}

bool FilterMerge::uses_background()
{
    for (unsigned int i = 0; i < _input_image.size(); ++i) {
        int input = _input_image[i];
        if (input == NR_FILTER_BACKGROUNDIMAGE || input == NR_FILTER_BACKGROUNDALPHA) {
            return true;
        }
    }
    return false;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

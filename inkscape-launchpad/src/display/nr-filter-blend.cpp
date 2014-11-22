/** @file
 * SVG feBlend renderer
 *//*
 * "This filter composites two objects together using commonly used
 * imaging software blending modes. It performs a pixel-wise combination
 * of two input images." 
 * http://www.w3.org/TR/SVG11/filters.html#feBlend
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *   Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2007-2008 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glibmm.h>
#include "display/cairo-templates.h"
#include "display/cairo-utils.h"
#include "display/nr-filter-blend.h"
#include "display/nr-filter-primitive.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-types.h"
#include "preferences.h"

namespace Inkscape {
namespace Filters {

FilterBlend::FilterBlend() 
    : _blend_mode(BLEND_NORMAL),
      _input2(NR_FILTER_SLOT_NOT_SET)
{}

FilterPrimitive * FilterBlend::create() {
    return new FilterBlend();
}

FilterBlend::~FilterBlend()
{}

void FilterBlend::render_cairo(FilterSlot &slot)
{
    cairo_surface_t *input1 = slot.getcairo(_input);
    cairo_surface_t *input2 = slot.getcairo(_input2);

    // We may need to transform input surface to correct color interpolation space. The input surface
    // might be used as input to another primitive but it is likely that all the primitives in a given
    // filter use the same color interpolation space so we don't copy the input before converting.
    SPColorInterpolation ci_fp  = SP_CSS_COLOR_INTERPOLATION_AUTO;
    if( _style ) {
        ci_fp = (SPColorInterpolation)_style->color_interpolation_filters.computed;
    }
    set_cairo_surface_ci( input1, ci_fp );
    set_cairo_surface_ci( input2, ci_fp );

    // input2 is the "background" image
    // out should be ARGB32 if any of the inputs is ARGB32
    cairo_surface_t *out = ink_cairo_surface_create_output(input1, input2);
    set_cairo_surface_ci( out, ci_fp );

    ink_cairo_surface_blit(input2, out);
    cairo_t *out_ct = cairo_create(out);
    cairo_set_source_surface(out_ct, input1, 0, 0);

    // All of the blend modes are implemented in Cairo as of 1.10.
    // For a detailed description, see:
    // http://cairographics.org/operators/
    switch (_blend_mode) {
    case BLEND_MULTIPLY:
        cairo_set_operator(out_ct, CAIRO_OPERATOR_MULTIPLY);
        break;
    case BLEND_SCREEN:
        cairo_set_operator(out_ct, CAIRO_OPERATOR_SCREEN);
        break;
    case BLEND_DARKEN:
        cairo_set_operator(out_ct, CAIRO_OPERATOR_DARKEN);
        break;
    case BLEND_LIGHTEN:
        cairo_set_operator(out_ct, CAIRO_OPERATOR_LIGHTEN);
        break;
    // New in CSS Compositing and Blending Level 1
    case BLEND_OVERLAY:   
        cairo_set_operator(out_ct, CAIRO_OPERATOR_OVERLAY);
        break;
    case BLEND_COLORDODGE:
        cairo_set_operator(out_ct, CAIRO_OPERATOR_COLOR_DODGE);
        break;
    case BLEND_COLORBURN:
        cairo_set_operator(out_ct, CAIRO_OPERATOR_COLOR_BURN);
        break;
    case BLEND_HARDLIGHT:
        cairo_set_operator(out_ct, CAIRO_OPERATOR_HARD_LIGHT);
        break;
    case BLEND_SOFTLIGHT:
        cairo_set_operator(out_ct, CAIRO_OPERATOR_SOFT_LIGHT);
        break;
    case BLEND_DIFFERENCE:
        cairo_set_operator(out_ct, CAIRO_OPERATOR_DIFFERENCE);
        break;
    case BLEND_EXCLUSION:
        cairo_set_operator(out_ct, CAIRO_OPERATOR_EXCLUSION);
        break;
    case BLEND_HUE:       
        cairo_set_operator(out_ct, CAIRO_OPERATOR_HSL_HUE);
        break;
    case BLEND_SATURATION:
        cairo_set_operator(out_ct, CAIRO_OPERATOR_HSL_SATURATION);
        break;
    case BLEND_COLOR:
        cairo_set_operator(out_ct, CAIRO_OPERATOR_HSL_COLOR);
        break;
    case BLEND_LUMINOSITY:
        cairo_set_operator(out_ct, CAIRO_OPERATOR_HSL_LUMINOSITY);
        break;

    case BLEND_NORMAL:
    default:
        cairo_set_operator(out_ct, CAIRO_OPERATOR_OVER);
        break;
    }

    cairo_paint(out_ct);
    cairo_destroy(out_ct);

    slot.set(_output, out);
    cairo_surface_destroy(out);
}

bool FilterBlend::can_handle_affine(Geom::Affine const &)
{
    // blend is a per-pixel primitive and is immutable under transformations
    return true;
}

double FilterBlend::complexity(Geom::Affine const &)
{
    return 1.1;
}

bool FilterBlend::uses_background()
{
    if (_input == NR_FILTER_BACKGROUNDIMAGE || _input == NR_FILTER_BACKGROUNDALPHA ||
        _input2 == NR_FILTER_BACKGROUNDIMAGE || _input2 == NR_FILTER_BACKGROUNDALPHA)
    {
        return true;
    } else {
        return false;
    }
}

void FilterBlend::set_input(int slot) {
    _input = slot;
}

void FilterBlend::set_input(int input, int slot) {
    if (input == 0) _input = slot;
    if (input == 1) _input2 = slot;
}

void FilterBlend::set_mode(FilterBlendMode mode) {
    if (mode == BLEND_NORMAL     || mode == BLEND_MULTIPLY   ||
        mode == BLEND_SCREEN     || mode == BLEND_DARKEN     ||
        mode == BLEND_LIGHTEN    || mode == BLEND_OVERLAY    ||
        mode == BLEND_COLORDODGE || mode == BLEND_COLORBURN  ||
        mode == BLEND_HARDLIGHT  || mode == BLEND_SOFTLIGHT  ||
        mode == BLEND_DIFFERENCE || mode == BLEND_EXCLUSION  ||
        mode == BLEND_HUE        || mode == BLEND_SATURATION ||
        mode == BLEND_COLOR      || mode == BLEND_LUMINOSITY
        )
    {
        _blend_mode = mode;
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

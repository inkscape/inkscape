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
 *   Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 *
 * Copyright (C) 2007-2008 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "display/cairo-templates.h"
#include "display/cairo-utils.h"
#include "display/nr-filter-blend.h"
#include "display/nr-filter-primitive.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-types.h"
#include "preferences.h"

namespace Inkscape {
namespace Filters {

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

FilterBlend::FilterBlend() 
    : _blend_mode(BLEND_NORMAL),
      _input2(NR_FILTER_SLOT_NOT_SET)
{}

FilterPrimitive * FilterBlend::create() {
    return new FilterBlend();
}

FilterBlend::~FilterBlend()
{}

// cr = (1-qa)*cb + (1-qb)*ca + ca*cb
struct BlendMultiply {
    guint32 operator()(guint32 in1, guint32 in2)
    {
        EXTRACT_ARGB32(in1, aa, ra, ga, ba)
        EXTRACT_ARGB32(in2, ab, rb, gb, bb)

        guint32 ao = 255*255 - (255-aa)*(255-ab);        ao = (ao + 127) / 255;
        guint32 ro = (255-aa)*rb + (255-ab)*ra + ra*rb;  ro = (ro + 127) / 255;
        guint32 go = (255-aa)*gb + (255-ab)*ga + ga*gb;  go = (go + 127) / 255;
        guint32 bo = (255-aa)*bb + (255-ab)*ba + ba*bb;  bo = (bo + 127) / 255;

        ASSEMBLE_ARGB32(pxout, ao, ro, go, bo)
        return pxout;
    }
};

// cr = cb + ca - ca * cb
struct BlendScreen {
    guint32 operator()(guint32 in1, guint32 in2)
    {
        EXTRACT_ARGB32(in1, aa, ra, ga, ba)
        EXTRACT_ARGB32(in2, ab, rb, gb, bb)

        guint32 ao = 255*255 - (255-aa)*(255-ab);    ao = (ao + 127) / 255;
        guint32 ro = 255*(rb + ra) - ra * rb;        ro = (ro + 127) / 255;
        guint32 go = 255*(gb + ga) - ga * gb;        go = (go + 127) / 255;
        guint32 bo = 255*(bb + ba) - ba * bb;        bo = (bo + 127) / 255;

        ASSEMBLE_ARGB32(pxout, ao, ro, go, bo)
        return pxout;
    }
};

// cr = Min ((1 - qa) * cb + ca, (1 - qb) * ca + cb)
struct BlendDarken {
    guint32 operator()(guint32 in1, guint32 in2)
    {
        EXTRACT_ARGB32(in1, aa, ra, ga, ba)
        EXTRACT_ARGB32(in2, ab, rb, gb, bb)

        guint32 ao = 255*255 - (255-aa)*(255-ab);                           ao = (ao + 127) / 255;
        guint32 ro = std::min((255-aa)*rb + 255*ra, (255-ab)*ra + 255*rb);  ro = (ro + 127) / 255;
        guint32 go = std::min((255-aa)*gb + 255*ga, (255-ab)*ga + 255*gb);  go = (go + 127) / 255;
        guint32 bo = std::min((255-aa)*bb + 255*ba, (255-ab)*ba + 255*bb);  bo = (bo + 127) / 255;

        ASSEMBLE_ARGB32(pxout, ao, ro, go, bo)
        return pxout;
    }
};

// cr = Max ((1 - qa) * cb + ca, (1 - qb) * ca + cb)
struct BlendLighten {
    guint32 operator()(guint32 in1, guint32 in2)
    {
        EXTRACT_ARGB32(in1, aa, ra, ga, ba)
        EXTRACT_ARGB32(in2, ab, rb, gb, bb)

        guint32 ao = 255*255 - (255-aa)*(255-ab);                           ao = (ao + 127) / 255;
        guint32 ro = std::max((255-aa)*rb + 255*ra, (255-ab)*ra + 255*rb);  ro = (ro + 127) / 255;
        guint32 go = std::max((255-aa)*gb + 255*ga, (255-ab)*ga + 255*gb);  go = (go + 127) / 255;
        guint32 bo = std::max((255-aa)*bb + 255*ba, (255-ab)*ba + 255*bb);  bo = (bo + 127) / 255;

        ASSEMBLE_ARGB32(pxout, ao, ro, go, bo)
        return pxout;
    }
};

/*
struct BlendAlpha
static inline void blend_alpha(guint32 in1, guint32 in2, guint32 *out)
{
    EXTRACT_ARGB32(in1, a1, a2, a3, a4);
    EXTRACT_ARGB32(in2, b1, b2, b3, b4);

    guint32 o1 = 255*255 - (255-a1)*(255-b1);  o1 = (o1+127) / 255;
    guint32 o2 = 255*255 - (255-a2)*(255-b2);  o2 = (o2+127) / 255;
    guint32 o3 = 255*255 - (255-a3)*(255-b3);  o3 = (o3+127) / 255;
    guint32 o4 = 255*255 - (255-a4)*(255-b4);  o4 = (o4+127) / 255;

    ASSEMBLE_ARGB32(pxout, o1, o2, o3, o4);
    *out = pxout;
}
*/

void FilterBlend::render_cairo(FilterSlot &slot)
{
    cairo_surface_t *input1 = slot.getcairo(_input);
    cairo_surface_t *input2 = slot.getcairo(_input2);

    // We may need to transform input surface to correct color interpolation space. The input surface
    // might be used as input to another primitive but it is likely that all the primitives in a given
    // filter use the same color interpolation space so we don't copy the input before converting.
    // The converting function tags surface with the proper ci value.
    // Note: an alpha only surface should not have ci set.
    SPColorInterpolation ci_in1 = get_cairo_surface_ci(input1);
    SPColorInterpolation ci_in2 = get_cairo_surface_ci(input2);
    SPColorInterpolation ci_fp  = SP_CSS_COLOR_INTERPOLATION_AUTO;
    if( _style ) {
        ci_fp = (SPColorInterpolation)_style->color_interpolation_filters.computed;
    }
    if( ci_in1 == SP_CSS_COLOR_INTERPOLATION_SRGB &&
        ci_fp  == SP_CSS_COLOR_INTERPOLATION_LINEARRGB ) {
        //std::cout << "FilterColorBlend: srgb -> linear" << std::endl;
        ink_cairo_surface_srgb_to_linear( input1 );
    }
    if( ci_in1 == SP_CSS_COLOR_INTERPOLATION_LINEARRGB &&
        ci_fp  == SP_CSS_COLOR_INTERPOLATION_SRGB ) {
        //std::cout << "FilterColorBlend: linear -> srgb" << std::endl;
        ink_cairo_surface_linear_to_srgb( input1 );
    }
    if( ci_in2 == SP_CSS_COLOR_INTERPOLATION_SRGB &&
        ci_fp  == SP_CSS_COLOR_INTERPOLATION_LINEARRGB ) {
        //std::cout << "FilterColorBlend: srgb -> linear" << std::endl;
        ink_cairo_surface_srgb_to_linear( input2 );
    }
    if( ci_in2 == SP_CSS_COLOR_INTERPOLATION_LINEARRGB &&
        ci_fp  == SP_CSS_COLOR_INTERPOLATION_SRGB ) {
        //std::cout << "FilterColorBlend: linear -> srgb" << std::endl;
        ink_cairo_surface_linear_to_srgb( input2 );
    }

    // input2 is the "background" image
    // out should be ARGB32 if any of the inputs is ARGB32
    cairo_surface_t *out = ink_cairo_surface_create_output(input1, input2);
    if( cairo_surface_get_content( out ) == CAIRO_CONTENT_COLOR_ALPHA ) {
        set_cairo_surface_ci(out, ci_fp );
    }

    // std::cout << "FilterBlend: ci data: "
    //           << " in1: " << get_cairo_surface_ci(input1)
    //           << " in2: " << get_cairo_surface_ci(input2)
    //           << " out: " << get_cairo_surface_ci(out)
    //           << std::endl;

    cairo_content_t ct1 = cairo_surface_get_content(input1);
    cairo_content_t ct2 = cairo_surface_get_content(input2);
    if ((ct1 == CAIRO_CONTENT_ALPHA && ct2 == CAIRO_CONTENT_ALPHA)
        || _blend_mode == BLEND_NORMAL)
    {
        ink_cairo_surface_blit(input2, out);
        cairo_t *out_ct = cairo_create(out);
        cairo_set_source_surface(out_ct, input1, 0, 0);
        cairo_paint(out_ct);
        cairo_destroy(out_ct);
    } else {
        // blend mode != normal and at least 1 surface is not pure alpha

        // TODO: convert to Cairo blending operators once we start using the 1.10 series
        switch (_blend_mode) {
            case BLEND_MULTIPLY:
                ink_cairo_surface_blend(input1, input2, out, BlendMultiply());
                break;
            case BLEND_SCREEN:
                ink_cairo_surface_blend(input1, input2, out, BlendScreen());
                break;
            case BLEND_DARKEN:
                ink_cairo_surface_blend(input1, input2, out, BlendDarken());
                break;
            case BLEND_LIGHTEN:
                ink_cairo_surface_blend(input1, input2, out, BlendLighten());
                break;
            case BLEND_NORMAL:
            default:
                // this was handled before
                g_assert_not_reached();
                break;
        }
    }

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
    if (mode == BLEND_NORMAL || mode == BLEND_MULTIPLY ||
        mode == BLEND_SCREEN || mode == BLEND_DARKEN ||
        mode == BLEND_LIGHTEN)
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

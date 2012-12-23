/*
 * feColorMatrix filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *   Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <math.h>
#include <algorithm>
#include "display/cairo-templates.h"
#include "display/cairo-utils.h"
#include "display/nr-filter-colormatrix.h"
#include "display/nr-filter-slot.h"
#include <2geom/math-utils.h>

namespace Inkscape {
namespace Filters {

FilterColorMatrix::FilterColorMatrix()
{
}

FilterPrimitive * FilterColorMatrix::create() {
    return new FilterColorMatrix();
}

FilterColorMatrix::~FilterColorMatrix()
{}

FilterColorMatrix::ColorMatrixMatrix::ColorMatrixMatrix(std::vector<double> const &values) {
    unsigned limit = std::min(static_cast<size_t>(20), values.size());
    for (unsigned i = 0; i < limit; ++i) {
        if (i % 5 == 4) {
            _v[i] = round(values[i]*255*255);
        } else {
            _v[i] = round(values[i]*255);
        }
    }
    for (unsigned i = limit; i < 20; ++i) {
        _v[i] = 0;
    }
}

guint32 FilterColorMatrix::ColorMatrixMatrix::operator()(guint32 in) {
    EXTRACT_ARGB32(in, a, r, g, b)
    // we need to un-premultiply alpha values for this type of matrix
    // TODO: unpremul can be ignored if there is an identity mapping on the alpha channel
    if (a != 0) {
        r = unpremul_alpha(r, a);
        g = unpremul_alpha(g, a);
        b = unpremul_alpha(b, a);
    }

    gint32 ro = r*_v[0]  + g*_v[1]  + b*_v[2]  + a*_v[3]  + _v[4];
    gint32 go = r*_v[5]  + g*_v[6]  + b*_v[7]  + a*_v[8]  + _v[9];
    gint32 bo = r*_v[10] + g*_v[11] + b*_v[12] + a*_v[13] + _v[14];
    gint32 ao = r*_v[15] + g*_v[16] + b*_v[17] + a*_v[18] + _v[19];
    ro = (pxclamp(ro, 0, 255*255) + 127) / 255;
    go = (pxclamp(go, 0, 255*255) + 127) / 255;
    bo = (pxclamp(bo, 0, 255*255) + 127) / 255;
    ao = (pxclamp(ao, 0, 255*255) + 127) / 255;

    ro = premul_alpha(ro, ao);
    go = premul_alpha(go, ao);
    bo = premul_alpha(bo, ao);

    ASSEMBLE_ARGB32(pxout, ao, ro, go, bo)
    return pxout;
}


struct ColorMatrixSaturate {
    ColorMatrixSaturate(double v_in) {
        // clamp parameter instead of clamping color values
        double v = CLAMP(v_in, 0.0, 1.0);
        _v[0] = 0.213+0.787*v; _v[1] = 0.715-0.715*v; _v[2] = 0.072-0.072*v;
        _v[3] = 0.213-0.213*v; _v[4] = 0.715+0.285*v; _v[5] = 0.072-0.072*v;
        _v[6] = 0.213-0.213*v; _v[7] = 0.715-0.715*v; _v[8] = 0.072+0.928*v;
    }

    guint32 operator()(guint32 in) {
        EXTRACT_ARGB32(in, a, r, g, b)

        // Note: this cannot be done in fixed point, because the loss of precision
        //       causes overflow for some values of v
        guint32 ro = r*_v[0] + g*_v[1] + b*_v[2] + 0.5;
        guint32 go = r*_v[3] + g*_v[4] + b*_v[5] + 0.5;
        guint32 bo = r*_v[6] + g*_v[7] + b*_v[8] + 0.5;

        ASSEMBLE_ARGB32(pxout, a, ro, go, bo)
        return pxout;
    }
private:
    double _v[9];
};

struct ColorMatrixHueRotate {
    ColorMatrixHueRotate(double v) {
        double sinhue, coshue;
        Geom::sincos(v * M_PI/180.0, sinhue, coshue);

        _v[0] = round((0.213 +0.787*coshue -0.213*sinhue)*255);
        _v[1] = round((0.715 -0.715*coshue -0.715*sinhue)*255);
        _v[2] = round((0.072 -0.072*coshue +0.928*sinhue)*255);

        _v[3] = round((0.213 -0.213*coshue +0.143*sinhue)*255);
        _v[4] = round((0.715 +0.285*coshue +0.140*sinhue)*255);
        _v[5] = round((0.072 -0.072*coshue -0.283*sinhue)*255);

        _v[6] = round((0.213 -0.213*coshue -0.787*sinhue)*255);
        _v[7] = round((0.715 -0.715*coshue +0.715*sinhue)*255);
        _v[8] = round((0.072 +0.928*coshue +0.072*sinhue)*255);
    }
    guint32 operator()(guint32 in) {
        EXTRACT_ARGB32(in, a, r, g, b)
        gint32 maxpx = a*255;
        gint32 ro = r*_v[0] + g*_v[1] + b*_v[2];
        gint32 go = r*_v[3] + g*_v[4] + b*_v[5];
        gint32 bo = r*_v[6] + g*_v[7] + b*_v[8];
        ro = (pxclamp(ro, 0, maxpx) + 127) / 255;
        go = (pxclamp(go, 0, maxpx) + 127) / 255;
        bo = (pxclamp(bo, 0, maxpx) + 127) / 255;

        ASSEMBLE_ARGB32(pxout, a, ro, go, bo)
        return pxout;
    }
private:
    gint32 _v[9];
};

struct ColorMatrixLuminanceToAlpha {
    guint32 operator()(guint32 in) {
        // original computation in double: r*0.2125 + g*0.7154 + b*0.0721
        EXTRACT_ARGB32(in, a, r, g, b)
        // unpremultiply color values
        if (a != 0) {
            r = unpremul_alpha(r, a);
            g = unpremul_alpha(g, a);
            b = unpremul_alpha(b, a);
        }
        guint32 ao = r*54 + g*182 + b*18;
        return ((ao + 127) / 255) << 24;
    }
};

void FilterColorMatrix::render_cairo(FilterSlot &slot)
{
    cairo_surface_t *input = slot.getcairo(_input);
    cairo_surface_t *out = NULL;

    // We may need to transform input surface to correct color interpolation space. The input surface
    // might be used as input to another primitive but it is likely that all the primitives in a given
    // filter use the same color interpolation space so we don't copy the input before converting.
    SPColorInterpolation ci_fp = SP_CSS_COLOR_INTERPOLATION_AUTO;
    if( _style ) {
        ci_fp = (SPColorInterpolation)_style->color_interpolation_filters.computed;
    }
    set_cairo_surface_ci( input, ci_fp );

    if (type == COLORMATRIX_LUMINANCETOALPHA) {
        out = ink_cairo_surface_create_same_size(input, CAIRO_CONTENT_ALPHA);
    } else {
        out = ink_cairo_surface_create_identical(input);
        // Set ci to that used for computation
        set_cairo_surface_ci(out, ci_fp);
    }

    switch (type) {
    case COLORMATRIX_MATRIX:
        ink_cairo_surface_filter(input, out, FilterColorMatrix::ColorMatrixMatrix(values));
        break;
    case COLORMATRIX_SATURATE:
        ink_cairo_surface_filter(input, out, ColorMatrixSaturate(value));
        break;
    case COLORMATRIX_HUEROTATE:
        ink_cairo_surface_filter(input, out, ColorMatrixHueRotate(value));
        break;
    case COLORMATRIX_LUMINANCETOALPHA:
        ink_cairo_surface_filter(input, out, ColorMatrixLuminanceToAlpha());
        break;
    case COLORMATRIX_ENDTYPE:
    default:
        break;
    }

    slot.set(_output, out);
    cairo_surface_destroy(out);
}

bool FilterColorMatrix::can_handle_affine(Geom::Affine const &)
{
    return true;
}

double FilterColorMatrix::complexity(Geom::Affine const &)
{
    return 2.0;
}

void FilterColorMatrix::set_type(FilterColorMatrixType t){
        type = t;
}

void FilterColorMatrix::set_value(gdouble v){
        value = v;
}

void FilterColorMatrix::set_values(std::vector<gdouble> const &v){
        values = v;
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

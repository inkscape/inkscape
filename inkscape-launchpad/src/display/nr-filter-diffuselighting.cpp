/*
 * feDiffuseLighting renderer
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *   Jean-Rene Reinhard <jr@komite.net>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2007-2010 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

#include "display/cairo-templates.h"
#include "display/cairo-utils.h"
#include "display/nr-3dutils.h"
#include "display/nr-filter-diffuselighting.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
#include "display/nr-filter-utils.h"
#include "display/nr-light.h"
#include "svg/svg-icc-color.h"
#include "svg/svg-color.h"

namespace Inkscape {
namespace Filters {

FilterDiffuseLighting::FilterDiffuseLighting() 
{
    light_type = NO_LIGHT;
    diffuseConstant = 1;
    surfaceScale = 1;
    lighting_color = 0xffffffff;
}

FilterPrimitive * FilterDiffuseLighting::create() {
    return new FilterDiffuseLighting();
}

FilterDiffuseLighting::~FilterDiffuseLighting()
{}

struct DiffuseLight : public SurfaceSynth {
    DiffuseLight(cairo_surface_t *bumpmap, double scale, double kd)
        : SurfaceSynth(bumpmap)
        , _scale(scale)
        , _kd(kd)
    {}

protected:
    guint32 diffuseLighting(int x, int y, NR::Fvector const &light, NR::Fvector const &light_components) {
        NR::Fvector normal = surfaceNormalAt(x, y, _scale);
        double k = _kd * NR::scalar_product(normal, light);

        guint32 r = CLAMP_D_TO_U8(k * light_components[LIGHT_RED]);
        guint32 g = CLAMP_D_TO_U8(k * light_components[LIGHT_GREEN]);
        guint32 b = CLAMP_D_TO_U8(k * light_components[LIGHT_BLUE]);

        ASSEMBLE_ARGB32(pxout, 255,r,g,b)
        return pxout;
    }
    double _scale, _kd;
};

struct DiffuseDistantLight : public DiffuseLight {
    DiffuseDistantLight(cairo_surface_t *bumpmap, SPFeDistantLight *light, guint32 color,
            double scale, double diffuse_constant)
        : DiffuseLight(bumpmap, scale, diffuse_constant)
    {
        DistantLight dl(light, color);
        dl.light_vector(_lightv);
        dl.light_components(_light_components);
    }

    guint32 operator()(int x, int y) {
        return diffuseLighting(x, y, _lightv, _light_components);
    }
private:
    NR::Fvector _lightv, _light_components;
};

struct DiffusePointLight : public DiffuseLight {
    DiffusePointLight(cairo_surface_t *bumpmap, SPFePointLight *light, guint32 color,
            Geom::Affine const &trans, double scale, double diffuse_constant, double x0, double y0)
        : DiffuseLight(bumpmap, scale, diffuse_constant)
        , _light(light, color, trans)
        , _x0(x0)
        , _y0(y0)
    {
        _light.light_components(_light_components);
    }

    guint32 operator()(int x, int y) {
        NR::Fvector light;
        _light.light_vector(light, _x0 + x, _y0 + y, _scale * alphaAt(x, y)/255.0);
        return diffuseLighting(x, y, light, _light_components);
    }
private:
    PointLight _light;
    NR::Fvector _light_components;
    double _x0, _y0;
};

struct DiffuseSpotLight : public DiffuseLight {
    DiffuseSpotLight(cairo_surface_t *bumpmap, SPFeSpotLight *light, guint32 color,
            Geom::Affine const &trans, double scale, double diffuse_constant, double x0, double y0)
        : DiffuseLight(bumpmap, scale, diffuse_constant)
        , _light(light, color, trans)
        , _x0(x0)
        , _y0(y0)
    {}

    guint32 operator()(int x, int y) {
        NR::Fvector light, light_components;
        _light.light_vector(light, _x0 + x, _y0 + y, _scale * alphaAt(x, y)/255.0);
        _light.light_components(light_components, light);
        return diffuseLighting(x, y, light, light_components);
    }
private:
    SpotLight _light;
    double _x0, _y0;
};

void FilterDiffuseLighting::render_cairo(FilterSlot &slot)
{
    cairo_surface_t *input = slot.getcairo(_input);
    cairo_surface_t *out = ink_cairo_surface_create_same_size(input, CAIRO_CONTENT_COLOR_ALPHA);

    double r = SP_RGBA32_R_F(lighting_color);
    double g = SP_RGBA32_G_F(lighting_color);
    double b = SP_RGBA32_B_F(lighting_color);

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

    if (icc) {
        guchar ru, gu, bu;
        icc_color_to_sRGB(icc, &ru, &gu, &bu);
        r = SP_COLOR_U_TO_F(ru);
        g = SP_COLOR_U_TO_F(gu);
        b = SP_COLOR_U_TO_F(bu);
    }
#endif

    // Only alpha channel of input is used, no need to check input color_interpolation_filter value.
    SPColorInterpolation ci_fp  = SP_CSS_COLOR_INTERPOLATION_AUTO;
    if( _style ) {
        ci_fp = (SPColorInterpolation)_style->color_interpolation_filters.computed;

        // Lighting color is always defined in terms of sRGB, preconvert to linearRGB
        // if color_interpolation_filters set to linearRGB (for efficiency assuming
        // next filter primitive has same value of cif).
        if( ci_fp == SP_CSS_COLOR_INTERPOLATION_LINEARRGB ) {
            r = srgb_to_linear( r );
            g = srgb_to_linear( g );
            b = srgb_to_linear( b );
        }
    }
    set_cairo_surface_ci(out, ci_fp );
    guint32 color = SP_RGBA32_F_COMPOSE( r, g, b, 1.0 ); 

    Geom::Rect slot_area = slot.get_slot_area();
    Geom::Point p = slot_area.min();
    Geom::Affine trans = slot.get_units().get_matrix_primitiveunits2pb();
    double x0 = p[Geom::X], y0 = p[Geom::Y];
    double scale = surfaceScale * trans.descrim();

    switch (light_type) {
    case DISTANT_LIGHT:
        ink_cairo_surface_synthesize(out,
            DiffuseDistantLight(input, light.distant, color, scale, diffuseConstant));
        break;
    case POINT_LIGHT:
        ink_cairo_surface_synthesize(out,
            DiffusePointLight(input, light.point, color, trans, scale, diffuseConstant, x0, y0));
        break;
    case SPOT_LIGHT:
        ink_cairo_surface_synthesize(out,
            DiffuseSpotLight(input, light.spot, color, trans, scale, diffuseConstant, x0, y0));
        break;
    default: {
        cairo_t *ct = cairo_create(out);
        cairo_set_source_rgba(ct, 0,0,0,1);
        cairo_set_operator(ct, CAIRO_OPERATOR_SOURCE);
        cairo_paint(ct);
        cairo_destroy(ct);
        } break;
    }

    slot.set(_output, out);
    cairo_surface_destroy(out);
}

void FilterDiffuseLighting::set_icc(SVGICCColor *icc_color) {
    icc = icc_color;
}

void FilterDiffuseLighting::area_enlarge(Geom::IntRect &area, Geom::Affine const & /*trans*/)
{
    // TODO: support kernelUnitLength

    // We expand the area by 1 in every direction to avoid artifacts on tile edges.
    // However, it means that edge pixels will be incorrect.
    area.expandBy(1);
}

double FilterDiffuseLighting::complexity(Geom::Affine const &)
{
    return 9.0;
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

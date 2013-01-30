/*
 * feSpecularLighting renderer
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *   Jean-Rene Reinhard <jr@komite.net>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <cmath>

#include "display/cairo-templates.h"
#include "display/cairo-utils.h"
#include "display/nr-3dutils.h"
#include "display/nr-filter-specularlighting.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
#include "display/nr-filter-utils.h"
#include "display/nr-light.h"
#include "svg/svg-icc-color.h"
#include "svg/svg-color.h"

namespace Inkscape {
namespace Filters {

FilterSpecularLighting::FilterSpecularLighting()
{
    light_type = NO_LIGHT;
    specularConstant = 1;
    specularExponent = 1;
    surfaceScale = 1;
    lighting_color = 0xffffffff;
}

FilterPrimitive * FilterSpecularLighting::create() {
    return new FilterSpecularLighting();
}

FilterSpecularLighting::~FilterSpecularLighting()
{}

struct SpecularLight : public SurfaceSynth {
    SpecularLight(cairo_surface_t *bumpmap, double scale, double specular_constant,
            double specular_exponent)
        : SurfaceSynth(bumpmap)
        , _scale(scale)
        , _ks(specular_constant)
        , _exp(specular_exponent)
    {}
protected:
    guint32 specularLighting(int x, int y, NR::Fvector const &halfway, NR::Fvector const &light_components) {
        NR::Fvector normal = surfaceNormalAt(x, y, _scale);
        double sp = NR::scalar_product(normal, halfway);
        double k = sp <= 0.0 ? 0.0 : _ks * pow(sp, _exp);

        guint32 r = CLAMP_D_TO_U8(k * light_components[LIGHT_RED]);
        guint32 g = CLAMP_D_TO_U8(k * light_components[LIGHT_GREEN]);
        guint32 b = CLAMP_D_TO_U8(k * light_components[LIGHT_BLUE]);
        guint32 a = std::max(std::max(r, g), b);

        r = premul_alpha(r, a);
        g = premul_alpha(g, a);
        b = premul_alpha(b, a);

        ASSEMBLE_ARGB32(pxout, a,r,g,b)
        return pxout;
    }
    double _scale, _ks, _exp;
};

struct SpecularDistantLight : public SpecularLight {
    SpecularDistantLight(cairo_surface_t *bumpmap, SPFeDistantLight *light, guint32 color,
            double scale, double specular_constant, double specular_exponent)
        : SpecularLight(bumpmap, scale, specular_constant, specular_exponent)
    {
        DistantLight dl(light, color);
        NR::Fvector lv;
        dl.light_vector(lv);
        dl.light_components(_light_components);
        NR::normalized_sum(_halfway, lv, NR::EYE_VECTOR);
    }
    guint32 operator()(int x, int y) {
        return specularLighting(x, y, _halfway, _light_components);
    }
private:
    NR::Fvector _halfway, _light_components;
};

struct SpecularPointLight : public SpecularLight {
    SpecularPointLight(cairo_surface_t *bumpmap, SPFePointLight *light, guint32 color,
            Geom::Affine const &trans, double scale, double specular_constant,
            double specular_exponent, double x0, double y0)
        : SpecularLight(bumpmap, scale, specular_constant, specular_exponent)
        , _light(light, color, trans)
        , _x0(x0)
        , _y0(y0)
    {
        _light.light_components(_light_components);
    }

    guint32 operator()(int x, int y) {
        NR::Fvector light, halfway;
        _light.light_vector(light, _x0 + x, _y0 + y, _scale * alphaAt(x, y)/255.0);
        NR::normalized_sum(halfway, light, NR::EYE_VECTOR);
        return specularLighting(x, y, halfway, _light_components);
    }
private:
    PointLight _light;
    NR::Fvector _light_components;
    double _x0, _y0;
};

struct SpecularSpotLight : public SpecularLight {
    SpecularSpotLight(cairo_surface_t *bumpmap, SPFeSpotLight *light, guint32 color,
            Geom::Affine const &trans, double scale, double specular_constant,
            double specular_exponent, double x0, double y0)
        : SpecularLight(bumpmap, scale, specular_constant, specular_exponent)
        , _light(light, color, trans)
        , _x0(x0)
        , _y0(y0)
    {}

    guint32 operator()(int x, int y) {
        NR::Fvector light, halfway, light_components;
        _light.light_vector(light, _x0 + x, _y0 + y, _scale * alphaAt(x, y)/255.0);
        _light.light_components(light_components, light);
        NR::normalized_sum(halfway, light, NR::EYE_VECTOR);
        return specularLighting(x, y, halfway, light_components);
    }
private:
    SpotLight _light;
    double _x0, _y0;
};

void FilterSpecularLighting::render_cairo(FilterSlot &slot)
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

    Geom::Affine trans = slot.get_units().get_matrix_primitiveunits2pb();
    Geom::Point p = slot.get_slot_area().min();
    double x0 = p[Geom::X];
    double y0 = p[Geom::Y];
    double scale = surfaceScale * trans.descrim();
    double ks = specularConstant;
    double se = specularExponent;

    switch (light_type) {
    case DISTANT_LIGHT:
        ink_cairo_surface_synthesize(out,
            SpecularDistantLight(input, light.distant, color, scale, ks, se));
        break;
    case POINT_LIGHT:
        ink_cairo_surface_synthesize(out,
            SpecularPointLight(input, light.point, color, trans, scale, ks, se, x0, y0));
        break;
    case SPOT_LIGHT:
        ink_cairo_surface_synthesize(out,
            SpecularSpotLight(input, light.spot, color, trans, scale, ks, se, x0, y0));
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

void FilterSpecularLighting::set_icc(SVGICCColor *icc_color) {
    icc = icc_color;
}

void FilterSpecularLighting::area_enlarge(Geom::IntRect &area, Geom::Affine const & /*trans*/)
{
    // TODO: support kernelUnitLength
    area.expandBy(1);
}

double FilterSpecularLighting::complexity(Geom::Affine const &)
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

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

#include <glib/gmessages.h>
#include <cmath>

#include "display/cairo-templates.h"
#include "display/cairo-utils.h"
#include "display/nr-3dutils.h"
#include "display/nr-filter-specularlighting.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
#include "display/nr-filter-utils.h"
#include "display/nr-light.h"
#include "libnr/nr-rect-l.h"

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
            SpecularDistantLight(input, light.distant, lighting_color, scale, ks, se));
        break;
    case POINT_LIGHT:
        ink_cairo_surface_synthesize(out,
            SpecularPointLight(input, light.point, lighting_color, trans, scale, ks, se, x0, y0));
        break;
    case SPOT_LIGHT:
        ink_cairo_surface_synthesize(out,
            SpecularSpotLight(input, light.spot, lighting_color, trans, scale, ks, se, x0, y0));
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

/*
int FilterSpecularLighting::render(FilterSlot &slot, FilterUnits const &units) {
    NRPixBlock *in = slot.get(_input);
    if (!in) {
        g_warning("Missing source image for feSpecularLighting (in=%d)", _input);
        return 1;
    }

    NRPixBlock *out = new NRPixBlock;

    //Fvector *L = NULL; //vector to the light

    int w = in->area.x1 - in->area.x0;
    int h = in->area.y1 - in->area.y0;
    int x0 = in->area.x0;
    int y0 = in->area.y0;
    int i, j;
    //As long as FilterRes and kernel unit is not supported we hardcode the
    //default value
    int dx = 1; //TODO setup
    int dy = 1; //TODO setup
    //surface scale
    Geom::Affine trans = units.get_matrix_primitiveunits2pb();
    gdouble ss = surfaceScale * trans[0];
    gdouble ks = specularConstant; //diffuse lighting constant
    NR::Fvector L, N, LC, H;
    gdouble inter;

    nr_pixblock_setup_fast(out, NR_PIXBLOCK_MODE_R8G8B8A8N,
            in->area.x0, in->area.y0, in->area.x1, in->area.y1,
            true);
    unsigned char *data_i = NR_PIXBLOCK_PX (in);
    unsigned char *data_o = NR_PIXBLOCK_PX (out);
    //No light, nothing to do
    switch (light_type) {
        case DISTANT_LIGHT:
            //the light vector is constant
            {
            DistantLight *dl = new DistantLight(light.distant, lighting_color);
            dl->light_vector(L);
            dl->light_components(LC);
            NR::normalized_sum(H, L, NR::EYE_VECTOR);
            //finish the work
            for (i = 0, j = 0; i < w*h; i++) {
                NR::compute_surface_normal(N, ss, in, i / w, i % w, dx, dy);
                COMPUTE_INTER(inter, N, H, ks, specularExponent);

                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_RED]); // CLAMP includes rounding!
                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_GREEN]);
                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_BLUE]);
                data_o[j] = MAX(MAX(data_o[j-3], data_o[j-2]), data_o[j-1]);
                ++j;
            }
            out->empty = FALSE;
            delete dl;
            }
            break;
        case POINT_LIGHT:
            {
            PointLight *pl = new PointLight(light.point, lighting_color, trans);
            pl->light_components(LC);
        //TODO we need a reference to the filter to determine primitiveUnits
        //if objectBoundingBox is used, use a different matrix for light_vector
        // UPDATE: trans is now correct matrix from primitiveUnits to
        // pixblock coordinates
            //finish the work
            for (i = 0, j = 0; i < w*h; i++) {
                NR::compute_surface_normal(N, ss, in, i / w, i % w, dx, dy);
                pl->light_vector(L,
                        i % w + x0,
                        i / w + y0,
                        ss * (double) data_i[4*i+3]/ 255);
                NR::normalized_sum(H, L, NR::EYE_VECTOR);
                COMPUTE_INTER(inter, N, H, ks, specularExponent);

                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_RED]);
                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_GREEN]);
                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_BLUE]);
                data_o[j] = MAX(MAX(data_o[j-3], data_o[j-2]), data_o[j-1]);
                ++j;
            }
            out->empty = FALSE;
            delete pl;
            }
            break;
        case SPOT_LIGHT:
            {
            SpotLight *sl = new SpotLight(light.spot, lighting_color, trans);
        //TODO we need a reference to the filter to determine primitiveUnits
        //if objectBoundingBox is used, use a different matrix for light_vector
        // UPDATE: trans is now correct matrix from primitiveUnits to
        // pixblock coordinates
            //finish the work
            for (i = 0, j = 0; i < w*h; i++) {
                NR::compute_surface_normal(N, ss, in, i / w, i % w, dx, dy);
                sl->light_vector(L,
                    i % w + x0,
                    i / w + y0,
                    ss * (double) data_i[4*i+3]/ 255);
                sl->light_components(LC, L);
                NR::normalized_sum(H, L, NR::EYE_VECTOR);
                COMPUTE_INTER(inter, N, H, ks, specularExponent);

                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_RED]);
                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_GREEN]);
                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_BLUE]);
                data_o[j] = MAX(MAX(data_o[j-3], data_o[j-2]), data_o[j-1]);
                ++j;
            }
            out->empty = FALSE;
            delete sl;
            }
            break;
        //else unknown light source, doing nothing
        case NO_LIGHT:
        default:
            {
            if (light_type != NO_LIGHT)
                g_warning("unknown light source %d", light_type);
            out->empty = false;
            }
    }

    //finishing
    slot.set(_output, out);
    //nr_pixblock_release(in);
    //delete in;
    return 0;
}*/

void FilterSpecularLighting::area_enlarge(NRRectL &area, Geom::Affine const &trans)
{
    // TODO: support kernelUnitLength

    area.x0 -= 1;
    area.x1 += 1;
    area.y0 -= 1;
    area.y1 += 1;
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

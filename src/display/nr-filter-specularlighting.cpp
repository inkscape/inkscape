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

#include "display/nr-3dutils.h"
#include "display/nr-arena-item.h"
#include "display/nr-filter-specularlighting.h"
#include "display/nr-filter-getalpha.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
#include "display/nr-filter-utils.h"
#include "display/nr-light.h"
#include "libnr/nr-blit.h"
#include "libnr/nr-pixblock.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-rect-l.h"
#include "color.h"

namespace NR {

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

//Investigating Phong Lighting model we should not take N.H but
//R.E which equals to 2*N.H^2 - 1
//replace the second line by
//gdouble scal = scalar_product((N), (H)); scal = 2 * scal * scal - 1;
//to get the expected formula
#define COMPUTE_INTER(inter, H, N, ks, speculaExponent) \
do {\
    gdouble scal = scalar_product((N), (H)); \
    if (scal <= 0)\
        (inter) = 0;\
    else\
        (inter) = (ks) * std::pow(scal, (specularExponent));\
}while(0)

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
    Matrix trans = units.get_matrix_primitiveunits2pb();
    gdouble ss = surfaceScale * trans[0];
    gdouble ks = specularConstant; //diffuse lighting constant
    Fvector L, N, LC, H;
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
            normalized_sum(H, L, EYE_VECTOR);
            //finish the work
            for (i = 0, j = 0; i < w*h; i++) {
                compute_surface_normal(N, ss, in, i / w, i % w, dx, dy);
                COMPUTE_INTER(inter, N, H, ks, specularExponent);

                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_RED]);
                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_GREEN]);
                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_BLUE]);
                data_o[j++] = MAX(MAX(data_o[j-3], data_o[j-2]), data_o[j-1]);
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
                compute_surface_normal(N, ss, in, i / w, i % w, dx, dy);
                pl->light_vector(L,
                        i % w + x0,
                        i / w + y0,
                        ss * (double) data_i[4*i+3]/ 255);
                normalized_sum(H, L, EYE_VECTOR);
                COMPUTE_INTER(inter, N, H, ks, specularExponent);

                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_RED]);
                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_GREEN]);
                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_BLUE]);
                data_o[j++] = MAX(MAX(data_o[j-3], data_o[j-2]), data_o[j-1]);
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
                compute_surface_normal(N, ss, in, i / w, i % w, dx, dy);
                sl->light_vector(L,
                    i % w + x0,
                    i / w + y0,
                    ss * (double) data_i[4*i+3]/ 255);
                sl->light_components(LC, L);
                normalized_sum(H, L, EYE_VECTOR);
                COMPUTE_INTER(inter, N, H, ks, specularExponent);

                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_RED]);
                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_GREEN]);
                data_o[j++] = CLAMP_D_TO_U8(inter * LC[LIGHT_BLUE]);
                data_o[j++] = MAX(MAX(data_o[j-3], data_o[j-2]), data_o[j-1]);
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
}

FilterTraits FilterSpecularLighting::get_input_traits() {
    return TRAIT_PARALLER;
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

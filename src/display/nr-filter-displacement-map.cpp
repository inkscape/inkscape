/*
 * feDisplacementMap filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/cairo-templates.h"
#include "display/cairo-utils.h"
#include "display/nr-filter-displacement-map.h"
#include "display/nr-filter-types.h"
#include "display/nr-filter-units.h"

namespace Inkscape {
namespace Filters {

FilterDisplacementMap::FilterDisplacementMap()
{}

FilterPrimitive * FilterDisplacementMap::create() {
    return new FilterDisplacementMap();
}

FilterDisplacementMap::~FilterDisplacementMap()
{}

#if 0
struct pixel_t {
    unsigned char channels[4];
    inline unsigned char operator[](int c) const { return channels[c]; }
    inline unsigned char& operator[](int c) { return channels[c]; }
    static inline pixel_t blank() {
        pixel_t p;
        for(unsigned int i=0; i<4; i++) {
            p[i] = 0;
        }
        return p;
    }
};

static inline pixel_t pixelValue(NRPixBlock const* pb, int x, int y) {
    if ( x < pb->area.x0 || x >= pb->area.x1 || y < pb->area.y0 || y >= pb->area.y1 ) return pixel_t::blank(); // This assumes anything outside the defined range is (0,0,0,0)
    pixel_t const* rowData = reinterpret_cast<pixel_t const*>(NR_PIXBLOCK_PX(pb) + (y-pb->area.y0)*pb->rs);
    return rowData[x-pb->area.x0];
}

template<bool PREMULTIPLIED>
static pixel_t interpolatePixels(NRPixBlock const* pb, double x, double y) {
    // NOTE: The values of x and y are shifted by -0.5 (the "true" values would be x+0.5 and y+0.5).
    //       This is done because otherwise the pixel values first have to be shifted by +0.5 and then by -0.5 again...
    unsigned int const sfl = 8u;
    unsigned int const sf = 1u<<sfl;
    unsigned int const sf2h = 1u<<(2u*sfl-1);
    int xi = (int)floor(x), yi = (int)floor(y);
    unsigned int xf = static_cast<unsigned int>(round(sf * (x - xi))),
        yf = static_cast<unsigned int>(round(sf * (y - yi)));
    pixel_t p00 = pixelValue(pb, xi+0, yi+0);
    pixel_t p01 = pixelValue(pb, xi+1, yi+0);
    pixel_t p10 = pixelValue(pb, xi+0, yi+1);
    pixel_t p11 = pixelValue(pb, xi+1, yi+1);

    /* It's a good idea to interpolate premultiplied colors:
     *
     *   Consider two pixels, one being rgba(255,0,0,0), which is fully transparent,
     *   and the other being rgba(0,0,255,255), or blue (fully opaque).
     *   If these two colors are interpolated the expected result would be bluish pixels
     *   containing no red.
     *
     * However, if our final alpha value is zero, then the RGB values aren't really determinate.
     * We might as well avoid premultiplication in this case, which still gives us a fully
     * transparent result, but with interpolated RGB parts. */

    pixel_t r;
    if (PREMULTIPLIED) {
        /* Premultiplied, so do simple interpolation. */
        for (unsigned i = 0; i != 4; ++i) {
            // y0,y1 have range [0,a*sf]
            unsigned const y0 = sf*p00[i] + xf*((unsigned int)p01[i]-(unsigned int)p00[i]);
            unsigned const y1 = sf*p10[i] + xf*((unsigned int)p11[i]-(unsigned int)p10[i]);

            unsigned const ri = sf*y0 + yf*(y1-y0); // range [0,a*sf*sf]
            r[i] = (ri + sf2h)>>(2*sfl); // range [0,a]
        }
    } else {
        /* First calculate interpolated alpha value. */
        unsigned const y0 = sf*p00[3] + xf*((unsigned int)p01[3]-(unsigned int)p00[3]); // range [0,a*sf]
        unsigned const y1 = sf*p10[3] + xf*((unsigned int)p11[3]-(unsigned int)p10[3]);
        unsigned const ra = sf*y0 + yf*(y1-y0); // range [0,a*sf*sf]

        if (ra==0) {
            /* Fully transparent, so do simple interpolation. */
            for (unsigned i = 0; i != 3; ++i) {
                // y0,y1 have range [0,255*sf]
                unsigned const y0 = sf*p00[i] + xf*((unsigned int)p01[i]-(unsigned int)p00[i]);
                unsigned const y1 = sf*p10[i] + xf*((unsigned int)p11[i]-(unsigned int)p10[i]);

                unsigned const ri = sf*y0 + yf*(y1-y0); // range [0,255*sf*sf]
                r[i] = (ri + sf2h)>>(2*sfl); // range [0,255]
            }
            r[3] = 0;
        } else {
            /* Do premultiplication ourselves. */
            for (unsigned i = 0; i != 3; ++i) {
                // Premultiplied versions.  Range [0,255*a].
                unsigned const c00 = p00[i]*p00[3];
                unsigned const c01 = p01[i]*p01[3];
                unsigned const c10 = p10[i]*p10[3];
                unsigned const c11 = p11[i]*p11[3];

                // Interpolation.
                unsigned const y0 = sf*c00 + xf*(c01-c00); // range [0,255*a*sf]
                unsigned const y1 = sf*c10 + xf*(c11-c10); // range [0,255*a*sf]
                unsigned const ri = sf*y0 + yf*(y1-y0); // range [0,255*a*sf*sf]
                r[i] = (ri + ra/2) / ra;  // range [0,255]
            }
            r[3] = (ra + sf2h)>>(2*sfl); // range [0,a]
        }
    }

    return r;
}

template<bool MAP_PREMULTIPLIED, bool DATA_PREMULTIPLIED>
static void performDisplacement(NRPixBlock const* texture, NRPixBlock const* map, int Xchannel, int Ychannel, NRPixBlock* out, double scalex, double scaley) {
    bool Xneedsdemul = MAP_PREMULTIPLIED && Xchannel<3;
    bool Yneedsdemul = MAP_PREMULTIPLIED && Ychannel<3;
    if (!Xneedsdemul) scalex /= 255.0;
    if (!Yneedsdemul) scaley /= 255.0;

    for (int yout=out->area.y0; yout < out->area.y1; yout++){
        pixel_t const* mapRowData = reinterpret_cast<pixel_t const*>(NR_PIXBLOCK_PX(map) + (yout-map->area.y0)*map->rs);
        pixel_t* outRowData = reinterpret_cast<pixel_t*>(NR_PIXBLOCK_PX(out) + (yout-out->area.y0)*out->rs);
        for (int xout=out->area.x0; xout < out->area.x1; xout++){
            pixel_t const mapValue = mapRowData[xout-map->area.x0];

            double xtex = xout + (Xneedsdemul ? // Although the value of the pixel corresponds to the MIDDLE of the pixel, no +0.5 is needed because we're interpolating pixels anyway (so to get the actual pixel locations 0.5 would have to be subtracted again).
                (mapValue[3]==0?0:(scalex * (mapValue[Xchannel] - mapValue[3]*0.5) / mapValue[3])) :
                (scalex * (mapValue[Xchannel] - 127.5)));
            double ytex = yout + (Yneedsdemul ?
                (mapValue[3]==0?0:(scaley * (mapValue[Ychannel] - mapValue[3]*0.5) / mapValue[3])) :
                (scaley * (mapValue[Ychannel] - 127.5)));

            outRowData[xout-out->area.x0] = interpolatePixels<DATA_PREMULTIPLIED>(texture, xtex, ytex);
        }
    }
}
#endif

struct Displace {
    Displace(cairo_surface_t *texture, cairo_surface_t *map,
            unsigned xch, unsigned ych, double scalex, double scaley)
        : _texture(texture)
        , _map(map)
        , _xch(xch)
        , _ych(ych)
        , _scalex(scalex/255.0)
        , _scaley(scaley/255.0)
    {}
    guint32 operator()(int x, int y) {
        guint32 mappx = _map.pixelAt(x, y);
        guint32 a = (mappx & 0xff000000) >> 24;
        guint32 xpx = 0, ypx = 0;
        double xtex = x, ytex = y;
        if (a) {
            guint32 xshift = _xch * 8, yshift = _ych * 8;
            xpx = (mappx & (0xff << xshift)) >> xshift;
            ypx = (mappx & (0xff << yshift)) >> yshift;
            if (_xch != 3) xpx = unpremul_alpha(xpx, a);
            if (_ych != 3) ypx = unpremul_alpha(ypx, a);
            xtex += _scalex * (xpx - 127.5);
            ytex += _scaley * (ypx - 127.5);
        }

        if (xtex >= 0 && xtex < (_texture._w - 1) &&
            ytex >= 0 && ytex < (_texture._h - 1))
        {
            return _texture.pixelAt(xtex, ytex);
        } else {
            return 0;
        }
    }
private:
    SurfaceSynth _texture;
    SurfaceSynth _map;
    unsigned _xch, _ych;
    double _scalex, _scaley;
};

void FilterDisplacementMap::render_cairo(FilterSlot &slot)
{
    cairo_surface_t *texture = slot.getcairo(_input);
    cairo_surface_t *map = slot.getcairo(_input2);
    cairo_surface_t *out = ink_cairo_surface_create_identical(texture);

    Geom::Affine trans = slot.get_units().get_matrix_primitiveunits2pb();
    double scalex = scale * trans.expansionX();
    double scaley = scale * trans.expansionY();

    ink_cairo_surface_synthesize(out, Displace(texture, map, Xchannel, Ychannel, scalex, scaley));

    slot.set(_output, out);
    cairo_surface_destroy(out);
}

/*
int FilterDisplacementMap::render(FilterSlot &slot, FilterUnits const &units) {
    NRPixBlock *texture = slot.get(_input);
    NRPixBlock *map = slot.get(_input2);

    // Bail out if either one of source images is missing
    if (!map || !texture) {
        g_warning("Missing source image for feDisplacementMap (map=%d texture=%d)", _input, _input2);
        return 1;
    }

    NR::IRect area = units.get_pixblock_filterarea_paraller();
    int x0 = std::max(map->area.x0,area.min()[NR::X]);
    int y0 = std::max(map->area.y0,area.min()[NR::Y]);
    int x1 = std::min(map->area.x1,area.max()[NR::X]);
    int y1 = std::min(map->area.y1,area.max()[NR::Y]);

    //TODO: check whether we really need this check:
    if (x1 <= x0 || y1 <= y0) return 0; //nothing to do!

    if (texture->mode != NR_PIXBLOCK_MODE_R8G8B8A8N && texture->mode != NR_PIXBLOCK_MODE_R8G8B8A8P) {
        g_warning("Source images without an alpha channel are not supported by feDisplacementMap at the moment.");
        return 1;
    }

    NRPixBlock *out = new NRPixBlock;
    nr_pixblock_setup_fast(out, texture->mode, x0, y0, x1, y1, true);

    // convert to a suitable format
    bool free_map_on_exit = false;
    if (map->mode != NR_PIXBLOCK_MODE_R8G8B8A8N && map->mode != NR_PIXBLOCK_MODE_R8G8B8A8P) {
        NRPixBlock *original_map = map;
        map = new NRPixBlock;
        nr_pixblock_setup_fast(map, NR_PIXBLOCK_MODE_R8G8B8A8N,
                               original_map->area.x0, original_map->area.y0,
                               original_map->area.x1, original_map->area.y1,
                               false);
        nr_blit_pixblock_pixblock(map, original_map);
        free_map_on_exit = true;
    }
    bool map_premultiplied = (map->mode == NR_PIXBLOCK_MODE_R8G8B8A8P);
    bool data_premultiplied = (out->mode == NR_PIXBLOCK_MODE_R8G8B8A8P);

    Geom::Affine trans = units.get_matrix_primitiveunits2pb();
    double scalex = scale * trans.expansionX();
    double scaley = scale * trans.expansionY();

    if (map_premultiplied && data_premultiplied) {
        performDisplacement<true,true>(texture, map, Xchannel, Ychannel, out, scalex, scaley);
    } else if (map_premultiplied && !data_premultiplied) {
        performDisplacement<true,false>(texture, map, Xchannel, Ychannel, out, scalex, scaley);
    } else if (data_premultiplied) {
        performDisplacement<false,true>(texture, map, Xchannel, Ychannel, out, scalex, scaley);
    } else {
        performDisplacement<false,false>(texture, map, Xchannel, Ychannel, out, scalex, scaley);
    }

    if (free_map_on_exit) {
        nr_pixblock_release(map);
        delete map;
    }

    out->empty = FALSE;
    slot.set(_output, out);
            return 0;
}*/

void FilterDisplacementMap::set_input(int slot) {
    _input = slot;
}

void FilterDisplacementMap::set_scale(double s) {
    scale = s;
}

void FilterDisplacementMap::set_input(int input, int slot) {
    if (input == 0) _input = slot;
    if (input == 1) _input2 = slot;
}

void FilterDisplacementMap::set_channel_selector(int s, FilterDisplacementMapChannelSelector channel) {
    if (channel > DISPLACEMENTMAP_CHANNEL_ALPHA || channel < DISPLACEMENTMAP_CHANNEL_RED) {
        g_warning("Selected an invalid channel value. (%d)", channel);
        return;
    }

    // channel numbering:
    // a = 3, r = 2, g = 1, b = 0
    // this way we can get the component value using:
    // component = (color & (ch*8)) >> (ch*8)
    unsigned ch = 4;
    switch (channel) {
    case DISPLACEMENTMAP_CHANNEL_ALPHA:
        ch = 3; break;
    case DISPLACEMENTMAP_CHANNEL_RED:
        ch = 2; break;
    case DISPLACEMENTMAP_CHANNEL_GREEN:
        ch = 1; break;
    case DISPLACEMENTMAP_CHANNEL_BLUE:
        ch = 0; break;
    default: break;
    }
    if (ch == 4) return;

    if (s == 0) Xchannel = ch;
    if (s == 1) Ychannel = ch;
}

void FilterDisplacementMap::area_enlarge(NRRectL &area, Geom::Affine const &trans)
{
    //I assume scale is in user coordinates (?!?)
    //FIXME: trans should be multiplied by some primitiveunits2user, shouldn't it?
    
    double scalex = scale/2.*(std::fabs(trans[0])+std::fabs(trans[1]));
    double scaley = scale/2.*(std::fabs(trans[2])+std::fabs(trans[3]));

    //FIXME: no +2 should be there!... (noticable only for big scales at big zoom factor)
    area.x0 -= (int)(scalex)+2;
    area.x1 += (int)(scalex)+2;
    area.y0 -= (int)(scaley)+2;
    area.y1 += (int)(scaley)+2;
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

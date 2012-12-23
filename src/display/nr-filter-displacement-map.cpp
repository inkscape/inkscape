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
        
        guint32 xshift = _xch * 8, yshift = _ych * 8;
        xpx = (mappx & (0xff << xshift)) >> xshift;
        ypx = (mappx & (0xff << yshift)) >> yshift;
        if (a) {
            if (_xch != 3) xpx = unpremul_alpha(xpx, a);
            if (_ych != 3) ypx = unpremul_alpha(ypx, a);
        }
        xtex += _scalex * (xpx - 127.5);
        ytex += _scaley * (ypx - 127.5);

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
    // color_interpolation_filters for out same as texture. See spec.
    copy_cairo_surface_ci( texture, out );

    // We may need to transform map surface to correct color interpolation space. The map surface
    // might be used as input to another primitive but it is likely that all the primitives in a given
    // filter use the same color interpolation space so we don't copy the map before converting.
    SPColorInterpolation ci_fp = SP_CSS_COLOR_INTERPOLATION_AUTO;
    if( _style ) {
        ci_fp = (SPColorInterpolation)_style->color_interpolation_filters.computed;
    }
    set_cairo_surface_ci( map, ci_fp );

    Geom::Affine trans = slot.get_units().get_matrix_primitiveunits2pb();
    double scalex = scale * trans.expansionX();
    double scaley = scale * trans.expansionY();

    ink_cairo_surface_synthesize(out, Displace(texture, map, Xchannel, Ychannel, scalex, scaley));

    slot.set(_output, out);
    cairo_surface_destroy(out);
}

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

void FilterDisplacementMap::area_enlarge(Geom::IntRect &area, Geom::Affine const &trans)
{
    //I assume scale is in user coordinates (?!?)
    //FIXME: trans should be multiplied by some primitiveunits2user, shouldn't it?
    
    double scalex = scale/2.*(std::fabs(trans[0])+std::fabs(trans[1]));
    double scaley = scale/2.*(std::fabs(trans[2])+std::fabs(trans[3]));

    //FIXME: no +2 should be there!... (noticable only for big scales at big zoom factor)
    area.expandBy(scalex+2, scaley+2);
}

double FilterDisplacementMap::complexity(Geom::Affine const &)
{
    return 3.0;
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

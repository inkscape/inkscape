/*
 * feFlood filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *   Tavmjong Bah <tavmjong@free.fr> (use primitive filter region)
 *
 * Copyright (C) 2007, 2011 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "display/nr-filter-flood.h"
#include "display/nr-filter-utils.h"
#include "svg/svg-icc-color.h"
#include "svg/svg-color.h"

namespace Inkscape {
namespace Filters {

FilterFlood::FilterFlood()
{}

FilterPrimitive * FilterFlood::create() {
    return new FilterFlood();
}

FilterFlood::~FilterFlood()
{}

int FilterFlood::render(FilterSlot &slot, FilterUnits const &units) {
//g_message("rendering feflood");
    NRPixBlock *in = slot.get(_input);
    if (!in) {
        g_warning("Missing source image for feFlood (in=%d)", _input);
        return 1;
    }

    // Region being drawn on screen in screen coordinates.
    int x0 = in->area.x0, y0 = in->area.y0;
    int x1 = in->area.x1, y1 = in->area.y1;
    int w = x1 - x0;

    // Set up pix block
    NRPixBlock *out = new NRPixBlock;
    nr_pixblock_setup_fast(out, NR_PIXBLOCK_MODE_R8G8B8A8N, x0, y0, x1, y1,  true);
    unsigned char *out_data = NR_PIXBLOCK_PX(out);

    // Get RGBA values.
    unsigned char r,g,b,a;
    r = CLAMP_D_TO_U8((color >> 24) % 256);
    g = CLAMP_D_TO_U8((color >> 16) % 256);
    b = CLAMP_D_TO_U8((color >>  8) % 256);
    a = CLAMP_D_TO_U8(opacity*255);

#if ENABLE_LCMS
    icc_color_to_sRGB(icc, &r, &g, &b);
    //g_message("result: r:%d g:%d b:%d", r, g, b);
#endif //ENABLE_LCMS

    // Only fill primitive subregion

    // Get subregion in user units
    Geom::Rect fp = filter_primitive_area( units );

    // Need to convert to pixbuff units
    Geom::Rect fp_pb = fp * units.get_matrix_user2pb();

    // Make sure we are in pixbuff area
    int fp_x0 = fp_pb.min()[Geom::X];
    int fp_x1 = fp_pb.max()[Geom::X];
    int fp_y0 = fp_pb.min()[Geom::Y];
    int fp_y1 = fp_pb.max()[Geom::Y];
    if( fp_x0 < x0 ) fp_x0 = x0;
    if( fp_x1 > x1 ) fp_x1 = x1;
    if( fp_y0 < y0 ) fp_y0 = y0;
    if( fp_y1 > y1 ) fp_y1 = y1;

    // Do fill
    for (int x=fp_x0; x < fp_x1; x++){
        for (int y=fp_y0; y < fp_y1; y++){
            out_data[ 4*((x - x0) + w*(y - y0))     ] = r;
            out_data[ 4*((x - x0) + w*(y - y0)) + 1 ] = g;
            out_data[ 4*((x - x0) + w*(y - y0)) + 2 ] = b;
            out_data[ 4*((x - x0) + w*(y - y0)) + 3 ] = a;
        }
    }

    out->empty = FALSE;
    slot.set(_output, out);
    return 0;
}

void FilterFlood::set_color(guint32 c) {
    color = c;
}

void FilterFlood::set_opacity(double o) {
    opacity = o;
}

void FilterFlood::set_icc(SVGICCColor *icc_color) {
    icc = icc_color;
}

void FilterFlood::area_enlarge(NRRectL &/*area*/, Geom::Affine const &/*trans*/)
{
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

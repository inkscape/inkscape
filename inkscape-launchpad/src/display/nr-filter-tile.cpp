/*
 * feTile filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

#include "display/cairo-utils.h"
#include "display/nr-filter-tile.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"

namespace Inkscape {
namespace Filters {

FilterTile::FilterTile()
{
}

FilterPrimitive * FilterTile::create() {
    return new FilterTile();
}

FilterTile::~FilterTile()
{}

void FilterTile::render_cairo(FilterSlot &slot)
{
    // FIX ME!
    static bool tile_warning = false;
    if (!tile_warning) {
        g_warning("Renderer for feTile has non-optimal implementation, expect slowness and bugs.");
        tile_warning = true;
    }

    // Fixing isn't so easy as the Inkscape renderer breaks the canvas into "rendering" tiles for
    // faster rendering. (The "rendering" tiles are not the same as the tiles in this primitive.)
    // Only if the the feTile tile source falls inside the current "rendering" tile will the tile
    // image be available.

    // This input source contains only the "rendering" tile.
    cairo_surface_t *in = slot.getcairo(_input);

    // For debugging
    // static int i = 0;
    // ++i;
    // std::stringstream filename;
    // filename << "dump." << i << ".png";
    // cairo_surface_write_to_png( in, filename.str().c_str() );

    // This is the feTile source area as determined by the input primitive area (see SVG spec).
    Geom::Rect tile_area = slot.get_primitive_area(_input);

    if( tile_area.width() == 0.0 || tile_area.height() == 0.0 ) {

        slot.set(_output, in);
        std::cerr << "FileTile::render_cairo: tile has zero width or height" << std::endl;

    } else {

        cairo_surface_t *out = ink_cairo_surface_create_identical(in);
        // color_interpolation_filters for out same as in.
        copy_cairo_surface_ci(in, out);
        cairo_t *ct = cairo_create(out);

        // The rectangle of the "rendering" tile.
        Geom::Rect sa = slot.get_slot_area();

        Geom::Affine trans = slot.get_units().get_matrix_user2pb();

        // Create feTile tile ----------------

        // Get tile area in pixbuf units (tile transformed).
        Geom::Rect tt = tile_area * trans;
        
        // Shift between "rendering" tile and feTile tile
        Geom::Point shift = sa.min() - tt.min(); 

        // Create feTile tile surface
        cairo_surface_t *tile = cairo_surface_create_similar(in, cairo_surface_get_content(in),
                                                             tt.width(), tt.height());
        cairo_t *ct_tile = cairo_create(tile);
        cairo_set_source_surface(ct_tile, in, shift[Geom::X], shift[Geom::Y]);
        cairo_paint(ct_tile);

        // Paint tiles ------------------
        
        // For debugging
        // std::stringstream filename;
        // filename << "tile." << i << ".png";
        // cairo_surface_write_to_png( tile, filename.str().c_str() );
        
        // Determine number of feTile rows and columns
        Geom::Rect pr = filter_primitive_area( slot.get_units() );
        int tile_cols = ceil( pr.width()  / tile_area.width() );
        int tile_rows = ceil( pr.height() / tile_area.height() );

        // Do tiling (TO DO: restrict to slot area.)
        for( int col=0; col < tile_cols; ++col ) {
            for( int row=0; row < tile_rows; ++row ) {

                Geom::Point offset( col*tile_area.width(), row*tile_area.height() );
                offset *= trans;
                offset[Geom::X] -= trans[4];
                offset[Geom::Y] -= trans[5];
        
                cairo_set_source_surface(ct, tile, offset[Geom::X], offset[Geom::Y]);
                cairo_paint(ct);
            }
        }
        slot.set(_output, out);

        // Clean up
        cairo_destroy(ct);
        cairo_surface_destroy(out);
        cairo_destroy(ct_tile);
        cairo_surface_destroy(tile);
    }
}

void FilterTile::area_enlarge(Geom::IntRect &area, Geom::Affine const &trans)
{
    // We need to enlarge enough to get tile source... we don't the area of the source tile in this
    // function so we guess. This is VERY inefficient.
    Geom::Point enlarge(200, 200);
    enlarge *= trans;
    area.expandBy( enlarge[Geom::X] < 100 ? 100: enlarge[Geom::X] );
}

double FilterTile::complexity(Geom::Affine const &)
{
    return 1.0;
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

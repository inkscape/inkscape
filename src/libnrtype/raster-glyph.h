#ifndef SEEN_LIBNRTYPE_RASTER_GLYPH_H
#define SEEN_LIBNRTYPE_RASTER_GLYPH_H

#include <libnr/nr-forward.h>
#include <libnrtype/nrtype-forward.h>
#include <livarot/livarot-forward.h>

// a little utility class that holds data to render a styled glyph
// ie. it's like a polygon. its function is to wrap the subpixel positionning
class raster_glyph {
public:
	// raster_font that created me
    raster_font*      daddy;
	// the glyph i am (the style is in daddy)
    int               glyph_id;
		// internal structure: the styled path, and the associated uncrossed polygon
		// they could be removed after the raster_position have been computed
    Path*             outline;  // transformed by the matrix in style (may be factorized, but is small)
    Shape*            polygon;
		// subpixel positions
		// nb_sub_pixel is set to 4 when the glyph is created (it's hardcoded)
    int               nb_sub_pixel;
    raster_position*  sub_pixel;

    raster_glyph(void);
    virtual ~raster_glyph(void);

		// utility
    void      SetSubPixelPositionning(int nb_pos);
    void      LoadSubPixelPosition(int no);

		// the interesting function: blits the glyph onto over
		// over should be a mask, ie a NRPixBlock with one 8bit plane
    void      Blit(Geom::Point const &at, NRPixBlock &over); // alpha only
};


#endif /* !SEEN_LIBNRTYPE_RASTER_GLYPH_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

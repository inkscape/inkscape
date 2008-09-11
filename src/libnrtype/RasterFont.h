/*
 *  RasterFont.h
 *  testICU
 *
 */

#ifndef my_raster_font
#define my_raster_font

#include <map>

#include <libnr/nr-forward.h>
#include <libnrtype/nrtype-forward.h>
#include <libnrtype/font-style.h>

// one rasterfont is one way to draw a font on the screen
// the way it's drawn is stored in style
class raster_font {
public:
    font_instance*                daddy;
    int                           refCount;

    font_style                    style;  

    std::map<int,int>             glyph_id_to_raster_glyph_no;
		// an array of glyphs in this rasterfont.
		// it's a bit redundant with the one in the daddy font_instance, but these glyphs
		// contains the real rasterization data
    int                           nbBase,maxBase;
    raster_glyph**                bases;

    explicit raster_font(font_style const &fstyle);
    virtual ~raster_font(void);
   
    void                          Unref(void);
    void                          Ref(void);

		// utility functions
    Geom::Point      Advance(int glyph_id);
    void           BBox(int glyph_id,NRRect *area);         

		// attempts to load a glyph and return a raster_glyph on which you can call Blit
    raster_glyph*  GetGlyph(int glyph_id);
		// utility
    void           LoadRasterGlyph(int glyph_id); // refreshes outline/polygon if needed
    void           RemoveRasterGlyph(raster_glyph* who);

private:
    /* Disable the default copy constructor and operator=: they do the wrong thing for refCount. */
    raster_font(raster_font const &);
    raster_font &operator=(raster_font const &);
};

#endif


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

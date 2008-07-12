#ifndef SEEN_LIBNRTYPE_FONT_INSTANCE_H
#define SEEN_LIBNRTYPE_FONT_INSTANCE_H

#include <ext/hash_map>
#include <map>
#include <pango/pango-types.h>
#include <pango/pango-font.h>
#include <require-config.h>
#include "FontFactory.h"

#include <libnr/nr-forward.h>
#include <libnrtype/nrtype-forward.h>
#include <libnrtype/font-style.h>
#include <livarot/livarot-forward.h>
#include "libnr/nr-rect.h"

// the font_instance are the template of several raster_font; they provide metrics and outlines
// that are drawn by the raster_font, so the raster_font needs info relative to the way the 
// font need to be drawn. note that fontsize is a scale factor in the transform matrix
// of the style
// the various raster_font in use at a given time are held in a hash_map whose indices are the
// styles, hence the 2 following 'classes'
struct font_style_hash : public std::unary_function<font_style, size_t> {
    size_t operator()(font_style const &x) const;
};

struct font_style_equal : public std::binary_function<font_style, font_style, bool> {
    bool operator()(font_style const &a, font_style const &b);
};

class font_instance {
public:
	// hashmap to get the raster_font for a given style
    __gnu_cxx::hash_map<font_style, raster_font*, font_style_hash, font_style_equal>     loadedStyles;
	// the real source of the font
    PangoFont*            pFont;
		// depending on the rendering backend, different temporary data

		// that's the font's fingerprint; this particular PangoFontDescription gives the entry at which this font_instance
		// resides in the font_factory loadedFaces hash_map
    PangoFontDescription* descr;
		// refcount
    int                   refCount;
		// font_factory owning this font_instance
    font_factory*         daddy;

    // common glyph definitions for all the rasterfonts
    std::map<int, int>    id_to_no;
    int                   nbGlyph, maxGlyph;
    font_glyph*           glyphs;

    font_instance(void);
    virtual ~font_instance(void);

    void                 Ref(void);
    void                 Unref(void);

    bool                 IsOutlineFont(void); // utility
    void                 InstallFace(PangoFont* iFace); // utility; should reset the pFont field if loading failed
		                        // in case the PangoFont is a bitmap font, for example. that way, the calling function 
		                        // will be able to check the validity of the font before installing it in loadedFaces
    void                 InitTheFace();

    int                  MapUnicodeChar(gunichar c); // calls the relevant unicode->glyph index function
    void                 LoadGlyph(int glyph_id);    // the main backend-dependent function
		                        // loads the given glyph's info
		
		// nota: all coordinates returned by these functions are on a [0..1] scale; you need to multiply 
		// by the fontsize to get the real sizes
    Path*                Outline(int glyph_id, Path *copyInto=NULL);
		                        // queries the outline of the glyph (in livarot Path form), and copies it into copyInto instead
		                        // of allocating a new Path if copyInto != NULL
    Geom::PathVector*    PathVector(int glyph_id);
                         // returns the 2geom-type pathvector for this glyph. no refcounting needed, it's deallocated when the font_instance dies
    double               Advance(int glyph_id, bool vertical);
		                        // nominal advance of the font.
    bool                 FontMetrics(double &ascent, double &descent, double &leading);
    bool                 FontSlope(double &run, double &rise);
                                // for generating slanted cursors for oblique fonts
    NR::Maybe<NR::Rect>             BBox(int glyph_id);

		// creates a rasterfont for the given style
    raster_font*         RasterFont(NR::Matrix const &trs, double stroke_width,
                                    bool vertical = false, JoinType stroke_join = join_straight,
                                    ButtType stroke_cap = butt_straight, float miter_limit = 4.0);
		// the dashes array in iStyle is copied
    raster_font*         RasterFont(font_style const &iStyle);
		// private use: tells the font_instance that the raster_font 'who' has died
    void                 RemoveRasterFont(raster_font *who);

		// attribute queries
    unsigned             Name(gchar *str, unsigned size);
    unsigned             PSName(gchar *str, unsigned size);
    unsigned             Family(gchar *str, unsigned size);
    unsigned             Attribute(gchar const *key, gchar *str, unsigned size);

private:
    void                 FreeTheFace();

#ifdef USE_PANGO_WIN32
    HFONT                 theFace;
#else
    FT_Face               theFace; 
                // it's a pointer in fact; no worries to ref/unref it, pango does its magic
                // as long as pFont is valid, theFace is too
#endif

};


#endif /* !SEEN_LIBNRTYPE_FONT_INSTANCE_H */

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

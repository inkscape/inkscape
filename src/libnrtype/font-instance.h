#ifndef SEEN_LIBNRTYPE_FONT_INSTANCE_H
#define SEEN_LIBNRTYPE_FONT_INSTANCE_H

#include <map>
#include <pango/pango-types.h>
#include <pango/pango-font.h>
#include <require-config.h>
#include "FontFactory.h"

#include <libnrtype/font-style.h>
#include <2geom/d2.h>

class font_factory;
struct font_glyph;

// the font_instance are the template of several raster_font; they provide metrics and outlines
// that are drawn by the raster_font, so the raster_font needs info relative to the way the
// font need to be drawn. note that fontsize is a scale factor in the transform matrix
// of the style
class font_instance {
public:
    // the real source of the font
    PangoFont*            pFont;
    // depending on the rendering backend, different temporary data

    // that's the font's fingerprint; this particular PangoFontDescription gives the entry at which this font_instance
    // resides in the font_factory loadedFaces unordered_map
    PangoFontDescription* descr;
    // refcount
    int                   refCount;
    // font_factory owning this font_instance
    font_factory*         parent;

    // common glyph definitions for all the rasterfonts
    std::map<int, int>    id_to_no;
    int                   nbGlyph, maxGlyph;
    font_glyph*           glyphs;

    // Map of OpenType tables found in font (convert to std::set?)
    std::map<Glib::ustring, int> openTypeTables;

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

    // Return 2geom pathvector for glyph. Deallocated when font instance dies.
    Geom::PathVector*    PathVector(int glyph_id);

    // Horizontal advance if 'vertical' is false, vertical advance if true.
    double               Advance(int glyph_id, bool vertical);

    double               GetTypoAscent()  { return _ascent; }
    double               GetTypoDescent() { return _descent; }
    double               GetXHeight()     { return _xheight; }
    double               GetMaxAscent()   { return _ascent_max; }
    double               GetMaxDescent()  { return _descent_max; }
    const double*        GetBaselines()   { return _baselines; }

    bool                 FontMetrics(double &ascent, double &descent, double &leading);
    bool                 FontDecoration(double &underline_position, double &underline_thickness,
                                        double &linethrough_position, double &linethrough_thickness);
    bool                 FontSlope(double &run, double &rise);
                                // for generating slanted cursors for oblique fonts
    Geom::OptRect        BBox(int glyph_id);

private:
    void                 FreeTheFace();
    // Find ascent, descent, x-height, and baselines.
    void                 FindFontMetrics();

    // Temp: make public
public:
#ifdef USE_PANGO_WIN32
    HFONT                 theFace;
#else
    FT_Face               theFace;
                // it's a pointer in fact; no worries to ref/unref it, pango does its magic
                // as long as pFont is valid, theFace is too
#endif

private:

    // Font metrics in em-box units
    double  _ascent;       // Typographic ascent.
    double  _descent;      // Typographic descent.
    double  _xheight;      // x-height of font.
    double  _ascent_max;   // Maxiumum ascent of all glyphs in font.
    double  _descent_max;  // Maxiumum descent of all glyphs in font.

    // Baselines
    double _baselines[SP_CSS_BASELINE_SIZE];
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

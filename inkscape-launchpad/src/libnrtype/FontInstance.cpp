/*
 *  FontInstance.cpp
 *  testICU
 *
 *   Authors:
 *     fred
 *     bulia byak <buliabyak@users.sf.net>
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef PANGO_ENABLE_ENGINE
#define PANGO_ENABLE_ENGINE
#endif

#include <ft2build.h>
#include FT_OUTLINE_H
#include FT_BBOX_H
#include FT_TRUETYPE_TAGS_H
#include FT_TRUETYPE_TABLES_H
#include FT_GLYPH_H
#include <pango/pangoft2.h>
#include <2geom/pathvector.h>
#include <2geom/path-sink.h>
#include "libnrtype/font-glyph.h"
#include "libnrtype/font-instance.h"
#include "livarot/Path.h"
#include "util/unordered-containers.h"


struct font_style_hash : public std::unary_function<font_style, size_t> {
    size_t operator()(font_style const &x) const;
};

struct font_style_equal : public std::binary_function<font_style, font_style, bool> {
    bool operator()(font_style const &a, font_style const &b) const;
};

static const double STROKE_WIDTH_THREASHOLD = 0.01;



size_t font_style_hash::operator()(const font_style &x) const {
    int h = 0;
    int n = static_cast<int>(floor(100 * x.stroke_width));
    h *= 12186;
    h += n;
    n = (x.vertical) ? 1:0;
    h *= 12186;
    h += n;
    if ( x.stroke_width >= STROKE_WIDTH_THREASHOLD ) {
        n = x.stroke_cap * 10 + x.stroke_join + static_cast<int>(x.stroke_miter_limit * 100);
        h *= 12186;
        h += n;
        if ( x.nbDash > 0 ) {
            n = x.nbDash;
            h *= 12186;
            h += n;
            n = static_cast<int>(floor(100 * x.dash_offset));
            h *= 12186;
            h += n;
            for (int i = 0; i < x.nbDash; i++) {
                n = static_cast<int>(floor(100 * x.dashes[i]));
                h *= 12186;
                h += n;
            }
        }
    }
    return h;
}

bool font_style_equal::operator()(const font_style &a,const font_style &b) const {
    bool same = true;
    for (int i = 0; (i < 6) && same; i++) {
        same = ( static_cast<int>(100 * a.transform[i]) == static_cast<int>(100 * b.transform[i]) );
    }
    same &= ( a.vertical == b.vertical )
        && ( a.stroke_width > STROKE_WIDTH_THREASHOLD ) == ( b.stroke_width > STROKE_WIDTH_THREASHOLD );
    if ( same && ( a.stroke_width > STROKE_WIDTH_THREASHOLD ) ) {
        same = ( a.stroke_cap == b.stroke_cap )
            && ( a.stroke_join == b.stroke_join )
            && ( static_cast<int>(a.stroke_miter_limit * 100) == static_cast<int>(b.stroke_miter_limit * 100) )
            && ( a.nbDash == b.nbDash );
        if ( same && ( a.nbDash > 0 ) ) {
            same = ( static_cast<int>(floor(100 * a.dash_offset)) == static_cast<int>(floor(100 * b.dash_offset)) );
            for (int i = 0; (i < a.nbDash) && same; i++) {
                same = ( static_cast<int>(floor(100 * a.dashes[i])) == static_cast<int>(floor(100 * b.dashes[i])) );
            }
        }
    }
    return same;
}

#ifndef USE_PANGO_WIN32
/*
 * Outline extraction
 */

struct FT2GeomData {
    FT2GeomData(Geom::PathBuilder &b, double s)
        : builder(b)
        , last(0, 0)
        , scale(s)
    {}
    Geom::PathBuilder &builder;
    Geom::Point last;
    double scale;
};

// Note: Freetype 2.2.1 redefined function signatures for functions to be placed in an
// FT_Outline_Funcs structure.  This is needed to keep backwards compatibility with the
// 2.1.x series.

/* *** BEGIN #if HACK *** */
#if FREETYPE_MAJOR == 2 && FREETYPE_MINOR >= 2
typedef FT_Vector const FREETYPE_VECTOR;
#else
typedef FT_Vector FREETYPE_VECTOR;
#endif

// outline as returned by freetype -> livarot Path
// see nr-type-ft2.cpp for the freetype -> artBPath on which this code is based
static int ft2_move_to(FREETYPE_VECTOR *to, void * i_user)
{
    FT2GeomData *user = (FT2GeomData*)i_user;
    Geom::Point p(to->x, to->y);
    //    printf("m  t=%f %f\n",p[0],p[1]);
    user->builder.moveTo(p * user->scale);
    user->last = p;
    return 0;
}

static int ft2_line_to(FREETYPE_VECTOR *to, void *i_user)
{
    FT2GeomData *user = (FT2GeomData*)i_user;
    Geom::Point p(to->x, to->y);
    //    printf("l  t=%f %f\n",p[0],p[1]);
    user->builder.lineTo(p * user->scale);
    user->last = p;
    return 0;
}

static int ft2_conic_to(FREETYPE_VECTOR *control, FREETYPE_VECTOR *to, void *i_user)
{
    FT2GeomData *user = (FT2GeomData*)i_user;
    Geom::Point p(to->x, to->y), c(control->x, control->y);
    user->builder.quadTo(c * user->scale, p * user->scale);
    //    printf("b c=%f %f  t=%f %f\n",c[0],c[1],p[0],p[1]);
    user->last = p;
    return 0;
}

static int ft2_cubic_to(FREETYPE_VECTOR *control1, FREETYPE_VECTOR *control2, FREETYPE_VECTOR *to, void *i_user)
{
    FT2GeomData *user = (FT2GeomData*)i_user;
    Geom::Point p(to->x, to->y);
    Geom::Point c1(control1->x, control1->y);
    Geom::Point c2(control2->x, control2->y);
    //    printf("c c1=%f %f  c2=%f %f   t=%f %f\n",c1[0],c1[1],c2[0],c2[1],p[0],p[1]);
    //user->theP->CubicTo(p,3*(c1-user->last),3*(p-c2));
    user->builder.curveTo(c1 * user->scale, c2 * user->scale, p * user->scale);
    user->last = p;
    return 0;
}
#endif

/* *** END #if HACK *** */

/*
 *
 */

font_instance::font_instance(void) :
    pFont(0),
    descr(0),
    refCount(0),
    parent(0),
    nbGlyph(0),
    maxGlyph(0),
    glyphs(0),
    theFace(0)
{
    //printf("font instance born\n");
    _ascent  = _ascent_max  = 0.8;
    _descent = _descent_max = 0.2;
    _xheight = 0.5;

    // Default baseline values, alphabetic is reference
    _baselines[ SP_CSS_BASELINE_AUTO             ] =  0.0;
    _baselines[ SP_CSS_BASELINE_ALPHABETIC       ] =  0.0;
    _baselines[ SP_CSS_BASELINE_IDEOGRAPHIC      ] = -_descent;
    _baselines[ SP_CSS_BASELINE_HANGING          ] =  0.8 * _ascent;
    _baselines[ SP_CSS_BASELINE_MATHEMATICAL     ] =  0.8 * _xheight;
    _baselines[ SP_CSS_BASELINE_CENTRAL          ] =  0.5 - _descent;
    _baselines[ SP_CSS_BASELINE_MIDDLE           ] =  0.5 * _xheight;
    _baselines[ SP_CSS_BASELINE_TEXT_BEFORE_EDGE ] = _ascent;
    _baselines[ SP_CSS_BASELINE_TEXT_AFTER_EDGE  ] = -_descent;
}

font_instance::~font_instance(void)
{
    if ( parent ) {
        parent->UnrefFace(this);
        parent = 0;
    }

    //printf("font instance death\n");
    if ( pFont ) {
        FreeTheFace();
        g_object_unref(pFont);
        pFont = 0;
    }

    if ( descr ) {
        pango_font_description_free(descr);
        descr = 0;
    }

    //    if ( theFace ) FT_Done_Face(theFace); // owned by pFont. don't touch
    theFace = 0;

    for (int i=0;i<nbGlyph;i++) {
        if ( glyphs[i].pathvector ) {
            delete glyphs[i].pathvector;
        }
    }
    if ( glyphs ) {
        free(glyphs);
        glyphs = 0;
    }
    nbGlyph = 0;
    maxGlyph = 0;
}

void font_instance::Ref(void)
{
    refCount++;
    //char *tc=pango_font_description_to_string(descr);
    //printf("font %x %s ref'd %i\n",this,tc,refCount);
    //free(tc);
}

void font_instance::Unref(void)
{
    refCount--;
    //char *tc=pango_font_description_to_string(descr);
    //printf("font %x %s unref'd %i\n",this,tc,refCount);
    //free(tc);
    if ( refCount <= 0 ) {
        delete this;
    }
}

void font_instance::InitTheFace()
{
    if (theFace == NULL && pFont != NULL) {
#ifdef USE_PANGO_WIN32
    if ( !theFace ) {
        LOGFONT *lf=pango_win32_font_logfont(pFont);
        g_assert(lf != NULL);
        theFace=pango_win32_font_cache_load(parent->pangoFontCache,lf);
        g_free(lf);
    }
    XFORM identity = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
    SetWorldTransform(parent->hScreenDC, &identity);
    SetGraphicsMode(parent->hScreenDC, GM_COMPATIBLE);
    SelectObject(parent->hScreenDC,theFace);
#else
    theFace=pango_fc_font_lock_face(PANGO_FC_FONT(pFont));
    if ( theFace ) {
        FT_Select_Charmap(theFace,ft_encoding_unicode) && FT_Select_Charmap(theFace,ft_encoding_symbol);
    }
#endif
    FindFontMetrics();
    }
}

void font_instance::FreeTheFace()
{
#ifdef USE_PANGO_WIN32
    SelectObject(parent->hScreenDC,GetStockObject(SYSTEM_FONT));
    pango_win32_font_cache_unload(parent->pangoFontCache,theFace);
#else
    pango_fc_font_unlock_face(PANGO_FC_FONT(pFont));
#endif
    theFace=NULL;
}

void font_instance::InstallFace(PangoFont* iFace)
{
    if ( !iFace ) {
        return;
    }
    pFont=iFace;
    iFace = NULL;

    InitTheFace();

    if ( pFont && IsOutlineFont() == false ) {
        FreeTheFace();
        if ( pFont ) {
            g_object_unref(pFont);
        }
        pFont=NULL;
    }
}

bool font_instance::IsOutlineFont(void)
{
    if ( pFont == NULL ) {
        return false;
    }
    InitTheFace();
#ifdef USE_PANGO_WIN32
    TEXTMETRIC tm;
    return GetTextMetrics(parent->hScreenDC,&tm) && tm.tmPitchAndFamily&(TMPF_TRUETYPE|TMPF_DEVICE);
#else
    return FT_IS_SCALABLE(theFace);
#endif
}

int font_instance::MapUnicodeChar(gunichar c)
{
    int res = 0;
    if ( pFont  ) {
#ifdef USE_PANGO_WIN32
        res = pango_win32_font_get_glyph_index(pFont, c);
#else
        theFace = pango_fc_font_lock_face(PANGO_FC_FONT(pFont));
        if ( c > 0xf0000 ) {
            res = CLAMP(c, 0xf0000, 0x1fffff) - 0xf0000;
        } else {
            res = FT_Get_Char_Index(theFace, c);
        }
        pango_fc_font_unlock_face(PANGO_FC_FONT(pFont));
#endif
    }
    return res;
}


#ifdef USE_PANGO_WIN32
static inline Geom::Point pointfx_to_nrpoint(const POINTFX &p, double scale)
{
    return Geom::Point(*(long*)&p.x / 65536.0 * scale,
                     *(long*)&p.y / 65536.0 * scale);
}
#endif

void font_instance::LoadGlyph(int glyph_id)
{
    if ( pFont == NULL ) {
        return;
    }
    InitTheFace();
#ifndef USE_PANGO_WIN32
    if ( !FT_IS_SCALABLE(theFace) ) {
        return; // bitmap font
    }
#endif

    if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
        Geom::PathBuilder path_builder;

        if ( nbGlyph >= maxGlyph ) {
            maxGlyph=2*nbGlyph+1;
            glyphs=(font_glyph*)realloc(glyphs,maxGlyph*sizeof(font_glyph));
        }
        font_glyph  n_g;
        n_g.pathvector=NULL;
        n_g.bbox[0]=n_g.bbox[1]=n_g.bbox[2]=n_g.bbox[3]=0;
        n_g.h_advance = 0;
        n_g.v_advance = 0;
        n_g.h_width = 0;
        n_g.v_width = 0;
        bool   doAdd=false;

#ifdef USE_PANGO_WIN32

#ifndef GGO_UNHINTED         // For compatibility with old SDKs.
#define GGO_UNHINTED 0x0100
#endif

        MAT2 identity = {{0,1},{0,0},{0,0},{0,1}};
        OUTLINETEXTMETRIC otm;
        GetOutlineTextMetrics(parent->hScreenDC, sizeof(otm), &otm);
        GLYPHMETRICS metrics;
        DWORD bufferSize=GetGlyphOutline (parent->hScreenDC, glyph_id, GGO_GLYPH_INDEX | GGO_NATIVE | GGO_UNHINTED, &metrics, 0, NULL, &identity);
        double scale=1.0/parent->fontSize;
        n_g.h_advance = metrics.gmCellIncX          * scale;
        n_g.v_advance = otm.otmTextMetrics.tmHeight * scale;
        n_g.h_width   = metrics.gmBlackBoxX         * scale;
        n_g.v_width   = metrics.gmBlackBoxY         * scale;
        if ( bufferSize == GDI_ERROR) {
            // shit happened
        } else if ( bufferSize == 0) {
            // character has no visual representation, but is valid (eg whitespace)
            doAdd=true;
        } else {
            char *buffer = new char[bufferSize];
            if ( GetGlyphOutline (parent->hScreenDC, glyph_id, GGO_GLYPH_INDEX | GGO_NATIVE | GGO_UNHINTED, &metrics, bufferSize, buffer, &identity) <= 0 ) {
                // shit happened
            } else {
                // Platform SDK is rubbish, read KB87115 instead
                DWORD polyOffset=0;
                while ( polyOffset < bufferSize ) {
                    TTPOLYGONHEADER const *polyHeader=(TTPOLYGONHEADER const *)(buffer+polyOffset);
                    if (polyOffset+polyHeader->cb > bufferSize) break;

                    if (polyHeader->dwType == TT_POLYGON_TYPE) {
                        path_builder.moveTo(pointfx_to_nrpoint(polyHeader->pfxStart, scale));
                        DWORD curveOffset=polyOffset+sizeof(TTPOLYGONHEADER);

                        while ( curveOffset < polyOffset+polyHeader->cb ) {
                            TTPOLYCURVE const *polyCurve=(TTPOLYCURVE const *)(buffer+curveOffset);
                            POINTFX const *p=polyCurve->apfx;
                            POINTFX const *endp=p+polyCurve->cpfx;

                            switch (polyCurve->wType) {
                            case TT_PRIM_LINE:
                                while ( p != endp )
                                    path_builder.lineTo(pointfx_to_nrpoint(*p++, scale));
                                break;

                            case TT_PRIM_QSPLINE:
                                {
                                    g_assert(polyCurve->cpfx >= 2);

                                    // The list of points specifies one or more control points and ends with the end point.
                                    // The intermediate points (on the curve) are the points between the control points.
                                    Geom::Point this_control = pointfx_to_nrpoint(*p++, scale);
                                    while ( p+1 != endp ) { // Process all "midpoints" (all points except the last)
                                        Geom::Point new_control = pointfx_to_nrpoint(*p++, scale);
                                        path_builder.quadTo(this_control, (new_control+this_control)/2);
                                        this_control = new_control;
                                    }
                                    Geom::Point end = pointfx_to_nrpoint(*p++, scale);
                                    path_builder.quadTo(this_control, end);
                                }
                                break;

                            case 3:  // TT_PRIM_CSPLINE
                                g_assert(polyCurve->cpfx % 3 == 0);
                                while ( p != endp ) {
                                    path_builder.curveTo(pointfx_to_nrpoint(p[0], scale),
                                                         pointfx_to_nrpoint(p[1], scale),
                                                         pointfx_to_nrpoint(p[2], scale));
                                    p += 3;
                                }
                                break;
                            }
                            curveOffset += sizeof(TTPOLYCURVE)+sizeof(POINTFX)*(polyCurve->cpfx-1);
                        }
                    }
                    polyOffset += polyHeader->cb;
                }
                doAdd=true;
            }
            delete [] buffer;
        }
#else
        if (FT_Load_Glyph (theFace, glyph_id, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)) {
            // shit happened
        } else {
            if ( FT_HAS_HORIZONTAL(theFace) ) {
                n_g.h_advance=((double)theFace->glyph->metrics.horiAdvance)/((double)theFace->units_per_EM);
                n_g.h_width=((double)theFace->glyph->metrics.width)/((double)theFace->units_per_EM);
            } else {
                n_g.h_width=n_g.h_advance=((double)(theFace->bbox.xMax-theFace->bbox.xMin))/((double)theFace->units_per_EM);
            }
            if ( FT_HAS_VERTICAL(theFace) ) {
                n_g.v_advance=((double)theFace->glyph->metrics.vertAdvance)/((double)theFace->units_per_EM);
                n_g.v_width=((double)theFace->glyph->metrics.height)/((double)theFace->units_per_EM);
            } else {
                // CSS3 Writing modes dictates that if vertical font metrics are missing we must
                // synthisize them. No method is specified. The SVG 1.1 spec suggests using the em
                // height (which is not theFace->height as that includes leading). The em height
                // is ascender + descender (descender positive).  Note: The "Requirements for
                // Japanese Text Layout" W3C document says that Japanese kanji should be "set
                // solid" which implies that vertical (and horizontal) advance should be 1em.
                n_g.v_width=n_g.v_advance= 1.0;
            }
            if ( theFace->glyph->format == ft_glyph_format_outline ) {
                FT_Outline_Funcs ft2_outline_funcs = {
                    ft2_move_to,
                    ft2_line_to,
                    ft2_conic_to,
                    ft2_cubic_to,
                    0, 0
                };
                FT2GeomData user(path_builder, 1.0/((double)theFace->units_per_EM));
                FT_Outline_Decompose (&theFace->glyph->outline, &ft2_outline_funcs, &user);
            }
            doAdd=true;
        }
#endif
        path_builder.flush();

        if ( doAdd ) {
            Geom::PathVector pv = path_builder.peek();
            // close all paths
            for (Geom::PathVector::iterator i = pv.begin(); i != pv.end(); ++i) {
                i->close();
            }
            if ( !pv.empty() ) {
                n_g.pathvector = new Geom::PathVector(pv);
                Geom::OptRect bounds = bounds_exact(*n_g.pathvector);
                if (bounds) {
                    n_g.bbox[0] = bounds->left();
                    n_g.bbox[1] = bounds->top();
                    n_g.bbox[2] = bounds->right();
                    n_g.bbox[3] = bounds->bottom();
                }
            }
            glyphs[nbGlyph]=n_g;
            id_to_no[glyph_id]=nbGlyph;
            nbGlyph++;
        }
    } else {
    }
}

bool font_instance::FontMetrics(double &ascent,double &descent,double &xheight)
{
    if ( pFont == NULL ) {
        return false;
    }
    InitTheFace();
    if ( theFace == NULL ) {
        return false;
    }

    ascent = _ascent;
    descent = _descent;
    xheight = _xheight;

    return true;
}

bool font_instance::FontDecoration( double &underline_position,   double &underline_thickness,
                                    double &linethrough_position, double &linethrough_thickness)
{
    if ( pFont == NULL ) {
        return false;
    }
    InitTheFace();
    if ( theFace == NULL ) {
        return false;
    }
#ifdef USE_PANGO_WIN32
    OUTLINETEXTMETRIC otm;
    if ( !GetOutlineTextMetrics(parent->hScreenDC,sizeof(otm),&otm) ) {
        return false;
    }
    double scale=1.0/parent->fontSize;
    underline_position    = fabs(otm.otmUnderscorePosition *scale);
    underline_thickness   = fabs(otm.otmUnderscoreSize     *scale);
    linethrough_position  = fabs(otm.otmStrikeoutPosition  *scale);
    linethrough_thickness = fabs(otm.otmStrikeoutSize      *scale);
#else
    if ( theFace->units_per_EM == 0 ) {
        return false; // bitmap font
    }
    underline_position    = fabs(((double)theFace->underline_position )/((double)theFace->units_per_EM));
    underline_thickness   = fabs(((double)theFace->underline_thickness)/((double)theFace->units_per_EM));
    // there is no specific linethrough information, mock it up from other font fields
    linethrough_position  = fabs(((double)theFace->ascender / 3.0     )/((double)theFace->units_per_EM));
    linethrough_thickness = fabs(((double)theFace->underline_thickness)/((double)theFace->units_per_EM));
#endif
    return true;
}


bool font_instance::FontSlope(double &run, double &rise)
{
    run = 0.0;
    rise = 1.0;

    if ( pFont == NULL ) {
        return false;
    }
    InitTheFace();
    if ( theFace == NULL ) {
        return false;
    }

#ifdef USE_PANGO_WIN32
    OUTLINETEXTMETRIC otm;
    if ( !GetOutlineTextMetrics(parent->hScreenDC,sizeof(otm),&otm) ) return false;
    run=otm.otmsCharSlopeRun;
    rise=otm.otmsCharSlopeRise;
#else
    if ( !FT_IS_SCALABLE(theFace) ) {
        return false; // bitmap font
    }

    TT_HoriHeader *hhea = (TT_HoriHeader*)FT_Get_Sfnt_Table(theFace, ft_sfnt_hhea);
    if (hhea == NULL) {
        return false;
    }
    run = hhea->caret_Slope_Run;
    rise = hhea->caret_Slope_Rise;
#endif
    return true;
}

Geom::OptRect font_instance::BBox(int glyph_id)
{
    int no = -1;
    if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
        LoadGlyph(glyph_id);
        if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
            // didn't load
        } else {
            no = id_to_no[glyph_id];
        }
    } else {
        no = id_to_no[glyph_id];
    }
    if ( no < 0 ) {
        return Geom::OptRect();
    } else {
        Geom::Point rmin(glyphs[no].bbox[0],glyphs[no].bbox[1]);
        Geom::Point rmax(glyphs[no].bbox[2],glyphs[no].bbox[3]);
        return Geom::Rect(rmin, rmax);
    }
}

Geom::PathVector* font_instance::PathVector(int glyph_id)
{
    int no = -1;
    if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
        LoadGlyph(glyph_id);
        if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
            // didn't load
        } else {
            no = id_to_no[glyph_id];
        }
    } else {
        no = id_to_no[glyph_id];
    }
    if ( no < 0 ) return NULL;
    return glyphs[no].pathvector;
}

double font_instance::Advance(int glyph_id,bool vertical)
{
    int no = -1;
    if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
        LoadGlyph(glyph_id);
        if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
            // didn't load
        } else {
            no=id_to_no[glyph_id];
        }
    } else {
        no = id_to_no[glyph_id];
    }
    if ( no >= 0 ) {
        if ( vertical ) {
            return glyphs[no].v_advance;
        } else {
            return glyphs[no].h_advance;
        }
    }
    return 0;
}

// Internal function to find baselines
void font_instance::FindFontMetrics() {

    // CSS2 recommends using the OS/2 values sTypoAscender and sTypoDescender for the Typographic
    // ascender and descender values:
    //   http://www.w3.org/TR/CSS2/visudet.html#sTypoAscender
    // On Windows, the typographic ascender and descender are taken from the otmMacAscent and
    // otmMacDescent values:
    //   http://microsoft.public.win32.programmer.gdi.narkive.com/LV6k4BDh/msdn-documentation-outlinetextmetrics-clarification
    // The otmAscent and otmDescent values are the maxiumum ascent and maxiumum descent of all the
    // glyphs in a font.
    if ( theFace ) {

#ifdef USE_PANGO_WIN32
        
        if ( GetOutlineTextMetrics(parent->hScreenDC,sizeof(otm),&otm) ) {
            double scale=1.0/parent->fontSize;
            _ascent      = fabs(otm.otmMacAscent  * scale);
            _descent     = fabs(otm.otmMacDescent * scale);
            _xheight     = fabs(otm.otmXHeight    * scale);
            _ascent_max  = fabs(otm.otmAscent     * scale);
            _descent_max = fabs(otm.otmDescent    * scale);

            // In CSS em size is ascent + descent... which should be 1. If not,
            // adjust so it is.
            double em = _ascent + _descent;
            if( em > 0 ) {
                _ascent /= em;
                _descent /= em;
            }

            // May not be necessary but if OS/2 table missing or not version 2 or higher,
            // xheight might be zero.
            if( _xheight == 0.0 ) {
                _xheight = 0.5;
            }

            // Baselines defined relative to  alphabetic.
            _baselines[ SP_CSS_BASELINE_IDEOGRAPHIC      ] = -_descent;      // Recommendation
            _baselines[ SP_CSS_BASELINE_HANGING          ] = 0.8 * _ascent;  // Guess
            _baselines[ SP_CSS_BASELINE_MATHEMATICAL     ] = 0.8 * _xheight; // Guess
            _baselines[ SP_CSS_BASELINE_CENTRAL          ] = 0.5 - _descent; // Definition
            _baselines[ SP_CSS_BASELINE_MIDDLE           ] = 0.5 * _xheight; // Definition
            _baselines[ SP_CSS_BASELINE_TEXT_BEFORE_EDGE ] = _ascent;        // Definition
            _baselines[ SP_CSS_BASELINE_TEXT_AFTER_EDGE  ] = -_descent;      // Definition


            MAT2 identity = {{0,1},{0,0},{0,0},{0,1}};
            GLYPHMETRICS metrics;
            int retval;

            // Better math baseline:
            // Try center of minus sign
            retval =  GetGlyphOutline (parent->hScreenDC, 0x2212, GGO_NATIVE | GGO_UNHINTED, &metrics, 0, NULL, &identity);
            // If no minus sign, try hyphen
            if( retval <= 0 )
                retval =  GetGlyphOutline (parent->hScreenDC, '-', GGO_NATIVE | GGO_UNHINTED, &metrics, 0, NULL, &identity);

            if( retval > 0 ) {
                double math = (metrics.gmptGlyphOrigin.y + 0.5 * metrics.gmBlackBoxY) * scale;
                _baselines[ SP_CSS_BASELINE_MATHEMATICAL ] = math;
            }

            // Find hanging baseline... assume it is at top of 'म'.
            retval =  GetGlyphOutline (parent->hScreenDC, 0x092E, GGO_NATIVE | GGO_UNHINTED, &metrics, 0, NULL, &identity);
            if( retval > 0 ) {
                double hanging = metrics.gmptGlyphOrigin.y * scale;
                _baselines[ SP_CSS_BASELINE_MATHEMATICAL ] = hanging;
            }
        }

#else

        if ( theFace->units_per_EM != 0 ) {  // If zero then it's a bitmap font.

            TT_OS2*  os2 = (TT_OS2*)FT_Get_Sfnt_Table( theFace, ft_sfnt_os2 );       
            if( os2 ) {
                _ascent  = fabs(((double)os2->sTypoAscender) / ((double)theFace->units_per_EM));
                _descent = fabs(((double)os2->sTypoDescender)/ ((double)theFace->units_per_EM));
            } else {
                _ascent  = fabs(((double)theFace->ascender)  / ((double)theFace->units_per_EM));
                _descent = fabs(((double)theFace->descender) / ((double)theFace->units_per_EM));
            }
            _ascent_max  = fabs(((double)theFace->ascender)  / ((double)theFace->units_per_EM));
            _descent_max = fabs(((double)theFace->descender) / ((double)theFace->units_per_EM));

            // In CSS em size is ascent + descent... which should be 1. If not,
            // adjust so it is.
            double em = _ascent + _descent;
            if( em > 0 ) {
                _ascent /= em;
                _descent /= em;
            }

            // x-height
            if( os2 && os2->version >= 0x0002 && os2->version != 0xffffu ) {
                // Only os/2 version 2 and above have sxHeight, 0xffff marks "old Mac fonts" without table
                _xheight = fabs(((double)os2->sxHeight) / ((double)theFace->units_per_EM));
            } else {
                // Measure 'x' height in font. Recommended option by XSL standard if no sxHeight.
                FT_UInt index = FT_Get_Char_Index( theFace, 'x' );
                if( index != 0 ) {
                    FT_Load_Glyph( theFace, index, FT_LOAD_NO_SCALE );
                    _xheight = (fabs)(((double)theFace->glyph->metrics.height/(double)theFace->units_per_EM));
                } else {
                    // No 'x' in font!
                    _xheight = 0.5;
                }
            }

            // Baselines defined relative to  alphabetic.
            _baselines[ SP_CSS_BASELINE_IDEOGRAPHIC      ] = -_descent;      // Recommendation
            _baselines[ SP_CSS_BASELINE_HANGING          ] = 0.8 * _ascent;  // Guess
            _baselines[ SP_CSS_BASELINE_MATHEMATICAL     ] = 0.8 * _xheight; // Guess
            _baselines[ SP_CSS_BASELINE_CENTRAL          ] = 0.5 - _descent; // Definition
            _baselines[ SP_CSS_BASELINE_MIDDLE           ] = 0.5 * _xheight; // Definition
            _baselines[ SP_CSS_BASELINE_TEXT_BEFORE_EDGE ] = _ascent;        // Definition
            _baselines[ SP_CSS_BASELINE_TEXT_AFTER_EDGE  ] = -_descent;      // Definition

            // Better math baseline:
            // Try center of minus sign
            FT_UInt index = FT_Get_Char_Index( theFace, 0x2212 ); //'−'
            // If no minus sign, try hyphen
            if( index == 0 )
                index = FT_Get_Char_Index( theFace, '-' );

            if( index != 0 ) {
                FT_Load_Glyph( theFace, index, FT_LOAD_NO_SCALE );
                FT_Glyph aglyph;
                FT_Get_Glyph( theFace->glyph, &aglyph );
                FT_BBox acbox;
                FT_Glyph_Get_CBox( aglyph, FT_GLYPH_BBOX_UNSCALED, &acbox );
                double math = (acbox.yMin + acbox.yMax)/2.0/(double)theFace->units_per_EM;
                _baselines[ SP_CSS_BASELINE_MATHEMATICAL ] = math;
                // std::cout << "Math baseline: - bbox: y_min: " << acbox.yMin
                //           << "  y_max: " << acbox.yMax
                //           << "  math: " << math << std::endl;
            }

            // Find hanging baseline... assume it is at top of 'म'.
            index = FT_Get_Char_Index( theFace, 0x092E ); // 'म'
            if( index != 0 ) {
                FT_Load_Glyph( theFace, index, FT_LOAD_NO_SCALE );
                FT_Glyph aglyph;
                FT_Get_Glyph( theFace->glyph, &aglyph );
                FT_BBox acbox;
                FT_Glyph_Get_CBox( aglyph, FT_GLYPH_BBOX_UNSCALED, &acbox );
                double hanging = (double)acbox.yMax/(double)theFace->units_per_EM;
                _baselines[ SP_CSS_BASELINE_HANGING ] = hanging;
                // std::cout << "Hanging baseline:  प: " << hanging << std::endl;
            }
        }
#endif
        // const gchar *family = pango_font_description_get_family(descr);
        // std::cout << "Font: " << (family?family:"null") << std::endl;
        // std::cout << "  ascent:      " << _ascent      << std::endl;
        // std::cout << "  descent:     " << _descent     << std::endl;
        // std::cout << "  x-height:    " << _xheight     << std::endl;
        // std::cout << "  max ascent:  " << _ascent_max  << std::endl;
        // std::cout << "  max descent: " << _descent_max << std::endl;
        // std::cout << " Baselines:" << std::endl;
        // std::cout << "  alphabetic:  " << _baselines[ SP_CSS_BASELINE_ALPHABETIC       ] << std::endl;
        // std::cout << "  ideographic: " << _baselines[ SP_CSS_BASELINE_IDEOGRAPHIC      ] << std::endl;
        // std::cout << "  hanging:     " << _baselines[ SP_CSS_BASELINE_HANGING          ] << std::endl;
        // std::cout << "  math:        " << _baselines[ SP_CSS_BASELINE_MATHEMATICAL     ] << std::endl;
        // std::cout << "  central:     " << _baselines[ SP_CSS_BASELINE_CENTRAL          ] << std::endl;
        // std::cout << "  middle:      " << _baselines[ SP_CSS_BASELINE_MIDDLE           ] << std::endl;
        // std::cout << "  text_before: " << _baselines[ SP_CSS_BASELINE_TEXT_BEFORE_EDGE ] << std::endl;
        // std::cout << "  text_after:  " << _baselines[ SP_CSS_BASELINE_TEXT_AFTER_EDGE  ] << std::endl;
    }
}


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

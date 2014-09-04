/** @file
 * @brief Metafile printing - common functions
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2013 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_EXTENSION_INTERNAL_METAFILE_PRINT_H
#define SEEN_INKSCAPE_EXTENSION_INTERNAL_METAFILE_PRINT_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <map>
#include <stack>
#include <glibmm/ustring.h>
#include <libuemf/uemf.h>
#include <2geom/affine.h>
#include <2geom/pathvector.h>

#include "extension/implementation/implementation.h"
#include "splivarot.h"
#include "display/canvas-bpath.h"

class SPGradient;
class SPObject;

namespace Inkscape {
class Pixbuf;

namespace Extension {
namespace Internal {

enum MFDrawMode {DRAW_PAINT, DRAW_PATTERN, DRAW_IMAGE, DRAW_LINEAR_GRADIENT, DRAW_RADIAL_GRADIENT};

struct FontfixParams {
    double f1;         //Vertical (rotating) offset factor (* font height)
    double f2;         //Vertical (nonrotating) offset factor (* font height)
    double f3;         //Horizontal (nonrotating) offset factor (* font height)
    FontfixParams() : f1(0), f2(0), f3(0) {}
};

class PrintMetafile
    : public Inkscape::Extension::Implementation::Implementation
{
public:
    PrintMetafile() {}
    ~PrintMetafile();

    bool textToPath (Inkscape::Extension::Print * ext);
    unsigned int bind(Inkscape::Extension::Print *module, Geom::Affine const &transform, float opacity);
    unsigned int release(Inkscape::Extension::Print *module);

protected:
    struct GRADVALUES {
        Geom::Point  p1;         // center   or start
        Geom::Point  p2;         // xhandle  or end
        Geom::Point  p3;         // yhandle  or unused
        double       r;          // radius   or unused
        void        *grad;       // to access the stops information
        int          mode;       // DRAW_LINEAR_GRADIENT or DRAW_RADIAL_GRADIENT, if GRADVALUES is valid, else any value
        U_COLORREF   bgc;        // document background color, this is as good a place as any to keep it
        float        rgb[3];     // also background color, but as 0-1 float.
    };

    double  _width;
    double  _height;
    double  _doc_unit_scale;     // to pixels, regardless of the document units
    
    U_RECTL  rc;

    uint32_t htextalignment;
    uint32_t hpolyfillmode;             // used to minimize redundant records that set this
    float    htextcolor_rgb[3];         // used to minimize redundant records that set this

    std::stack<Geom::Affine> m_tr_stack;
    Geom::PathVector fill_pathv;
    Geom::Affine fill_transform;
    bool use_stroke;
    bool use_fill;
    bool simple_shape;
    bool usebk;

    GRADVALUES gv;

    static bool _load_ppt_fontfix_data(); 
    static void _lookup_ppt_fontfix(Glib::ustring const &fontname, FontfixParams &);
    static U_COLORREF _gethexcolor(uint32_t color);
    static uint32_t _translate_weight(unsigned inkweight);

    U_COLORREF avg_stop_color(SPGradient *gr);
    U_COLORREF weight_opacity(U_COLORREF c1);
    U_COLORREF weight_colors(U_COLORREF c1, U_COLORREF c2, double t);

    void        hatch_classify(char *name, int *hatchType, U_COLORREF *hatchColor, U_COLORREF *bkColor);
    void        brush_classify(SPObject *parent, int depth, Inkscape::Pixbuf **epixbuf, int *hatchType, U_COLORREF *hatchColor, U_COLORREF *bkColor);
    static void swapRBinRGBA(char *px, int pixels);

    int         hold_gradient(void *gr, int mode);
    static Geom::PathVector center_ellipse_as_SVG_PathV(Geom::Point ctr, double rx, double ry, double F);
    static Geom::PathVector center_elliptical_ring_as_SVG_PathV(Geom::Point ctr, double rx1, double ry1, double rx2, double ry2, double F);
    static Geom::PathVector center_elliptical_hole_as_SVG_PathV(Geom::Point ctr, double rx, double ry, double F);
    static Geom::PathVector rect_cutter(Geom::Point ctr, Geom::Point pos, Geom::Point neg, Geom::Point width);
    static FillRule    SPWR_to_LVFR(SPWindRule wr);

    virtual int  create_brush(SPStyle const *style, PU_COLORREF fcolor) = 0;
    virtual void destroy_brush() = 0;
    virtual int  create_pen(SPStyle const *style, const Geom::Affine &transform) = 0;
    virtual void destroy_pen() = 0;

private:
    typedef std::map<Glib::ustring, FontfixParams> FontfixMap;
    static FontfixMap _ppt_fixable_fonts;
    static bool _ppt_fontfix_read;
};

} // namespace Internal
} // namespace Extension
} // namespace Inkscape

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

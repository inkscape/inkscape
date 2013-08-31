/** @file
 * @brief Windows Metafile printing - implementation
 */
/* Author:
 *   Ulf Erikson <ulferikson@users.sf.net>
 *
 * Copyright (C) 2006-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef __INKSCAPE_EXTENSION_INTERNAL_PRINT_WMF_H__
#define __INKSCAPE_EXTENSION_INTERNAL_PRINT_WMF_H__


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <libuemf/uwmf.h>

#include "extension/implementation/implementation.h"
//#include "extension/extension.h"

#include <2geom/pathvector.h>
#include "sp-gradient.h"
#include "splivarot.h"             // pieces for union on shapes
#include "display/canvas-bpath.h"  // for SPWindRule

#include <stack>

namespace Inkscape {
namespace Extension {
namespace Internal {

class PrintWmf : public Inkscape::Extension::Implementation::Implementation
{
    double  _width;
    double  _height;
    U_RECTL  rc;

    uint32_t htextalignment;
    uint32_t hbrush, hpen, hpenOld, hbrush_null, hpen_null;
    uint32_t hmiterlimit;               // used to minimize redundant records that set this
    uint32_t hpolyfillmode;             // used to minimize redundant records that set this
    float    htextcolor_rgb[3];         // used to minimize redundant records that set this

    std::stack<Geom::Affine> m_tr_stack;
    Geom::PathVector fill_pathv;
    Geom::Affine fill_transform;
    bool use_stroke;
    bool use_fill;
    bool simple_shape;
    bool usebk;

    unsigned int print_pathv (Geom::PathVector const &pathv, const Geom::Affine &transform);
    bool print_simple_shape (Geom::PathVector const &pathv, const Geom::Affine &transform);

public:
    PrintWmf (void);
    virtual ~PrintWmf (void);

    /* Print functions */
    virtual unsigned int setup (Inkscape::Extension::Print * module);

    virtual unsigned int begin (Inkscape::Extension::Print * module, SPDocument *doc);
    virtual unsigned int finish (Inkscape::Extension::Print * module);

    /* Rendering methods */
    virtual unsigned int bind(Inkscape::Extension::Print *module, Geom::Affine const &transform, float opacity);
    virtual unsigned int release(Inkscape::Extension::Print *module);
    virtual unsigned int fill (Inkscape::Extension::Print *module,
                               Geom::PathVector const &pathv,
                               Geom::Affine const &ctm, SPStyle const *style,
                               Geom::OptRect const &pbox, Geom::OptRect const &dbox,
                               Geom::OptRect const &bbox);
    virtual unsigned int stroke (Inkscape::Extension::Print * module,
                                 Geom::PathVector const &pathv,
                                 Geom::Affine const &ctm, SPStyle const *style,
                                 Geom::OptRect const &pbox, Geom::OptRect const &dbox,
                                 Geom::OptRect const &bbox);
    virtual unsigned int image(Inkscape::Extension::Print *module,
                           unsigned char *px,
                           unsigned int w,
                           unsigned int h,
                           unsigned int rs,
                           Geom::Affine const &transform,
                           SPStyle const *style);
    virtual unsigned int comment(Inkscape::Extension::Print *module, const char * comment);
    virtual unsigned int text(Inkscape::Extension::Print *module, char const *text,
                              Geom::Point const &p, SPStyle const *style);
    bool textToPath (Inkscape::Extension::Print * ext);

    static void init (void);
protected:
    static void  read_system_fflist(void);  //this is not called by any other source files
    static void  search_long_fflist(const char *fontname, double *f1, double *f2, double *f3);
    static void  search_short_fflist(const char *fontname, double *f1, double *f2, double *f3);
    static void  smuggle_adxky_out(const char *string, int16_t **adx, double *ky, int *rtl, int *ndx, float scale);
    U_COLORREF   gethexcolor(uint32_t color);
    uint32_t     transweight(const unsigned int inkweight);

    int create_brush(SPStyle const *style, PU_COLORREF fcolor);
    void destroy_brush();
    int create_pen(SPStyle const *style, const Geom::Affine &transform);
    void destroy_pen();

    void hatch_classify(char *name, int *hatchType, U_COLORREF *hatchColor, U_COLORREF *bkColor);
    void brush_classify(SPObject *parent, int depth, GdkPixbuf **epixbuf, int *hatchType, U_COLORREF *hatchColor, U_COLORREF *bkColor);
    void swapRBinRGBA(char *px, int pixels);
    
    U_COLORREF avg_stop_color(SPGradient *gr);
    
    int hold_gradient(void *gr, int mode);
    
    inline U_COLORREF weight_opacity(U_COLORREF c1);

    U_COLORREF weight_colors(U_COLORREF c1, U_COLORREF c2, double t);

    Geom::PathVector center_ellipse_as_SVG_PathV(Geom::Point ctr, double rx, double ry, double F);

    Geom::PathVector center_elliptical_ring_as_SVG_PathV(Geom::Point ctr, double rx1, double ry1, double rx2, double ry2, double F);

    Geom::PathVector center_elliptical_hole_as_SVG_PathV(Geom::Point ctr, double rx, double ry, double F);

    Geom::PathVector rect_cutter(Geom::Point ctr, Geom::Point pos, Geom::Point neg, Geom::Point width);

    FillRule SPWR_to_LVFR(SPWindRule wr);

};

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */


#endif /* __INKSCAPE_EXTENSION_INTERNAL_PRINT_WMF_H__ */

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

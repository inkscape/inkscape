/** @file
 * @brief Enhanced Metafile printing - implementation
 */
/* Authors:
 *   Ulf Erikson <ulferikson@users.sf.net>
 *   David Mathog
 *
 * Copyright (C) 2006-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef SEEN_INKSCAPE_EXTENSION_INTERNAL_EMF_PRINT_H
#define SEEN_INKSCAPE_EXTENSION_INTERNAL_EMF_PRINT_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <libuemf/uemf.h>
#include "extension/internal/metafile-print.h"

#include "sp-gradient.h"
#include "splivarot.h"             // pieces for union on shapes
#include "display/canvas-bpath.h"  // for SPWindRule

namespace Inkscape {
namespace Extension {
namespace Internal {

class PrintEmf : public PrintMetafile
{
    uint32_t hbrush, hbrushOld, hpen;

    unsigned int print_pathv (Geom::PathVector const &pathv, const Geom::Affine &transform);
    bool print_simple_shape (Geom::PathVector const &pathv, const Geom::Affine &transform);

public:
    PrintEmf();

    /* Print functions */
    virtual unsigned int setup (Inkscape::Extension::Print * module);

    virtual unsigned int begin (Inkscape::Extension::Print * module, SPDocument *doc);
    virtual unsigned int finish (Inkscape::Extension::Print * module);

    /* Rendering methods */
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

    static void init (void);
protected:
    static void  smuggle_adxkyrtl_out(const char *string, uint32_t **adx, double *ky, int *rtl, int *ndx, float scale);

    void        do_clip_if_present(SPStyle const *style);
    Geom::PathVector merge_PathVector_with_group(Geom::PathVector const &combined_pathvector, SPItem const *item, const Geom::Affine &transform);
    Geom::PathVector merge_PathVector_with_shape(Geom::PathVector const &combined_pathvector, SPItem const *item, const Geom::Affine &transform);
    unsigned int draw_pathv_to_EMF(Geom::PathVector const &pathv, const Geom::Affine &transform);
    Geom::Path  pathv_to_simple_polygon(Geom::PathVector const &pathv, int *vertices);
    Geom::Path  pathv_to_rect(Geom::PathVector const &pathv, bool *is_rect, double *angle);
    Geom::Point get_pathrect_corner(Geom::Path pathRect, double angle, int corner);
    U_TRIVERTEX make_trivertex(Geom::Point Pt, U_COLORREF uc);
    int         vector_rect_alignment(double angle, Geom::Point vtest);
    int         create_brush(SPStyle const *style, PU_COLORREF fcolor);
    void        destroy_brush();
    int         create_pen(SPStyle const *style, const Geom::Affine &transform);
    void        destroy_pen();
};

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */


#endif /* __INKSCAPE_EXTENSION_INTERNAL_PRINT_EMF_H__ */

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

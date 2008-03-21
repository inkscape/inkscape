#ifndef __INKSCAPE_EXTENSION_INTERNAL_PRINT_EMF_WIN32_H__
#define __INKSCAPE_EXTENSION_INTERNAL_PRINT_EMF_WIN32_H__

/*
 * Enhanced Metafile Printing.
 *
 * Author:
 *   Ulf Erikson <ulferikson@users.sf.net>
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef WIN32

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "extension/implementation/implementation.h"
#include "extension/extension.h"

#include "svg/stringstream.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-rect.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Inkscape {
namespace Extension {
namespace Internal {

class PrintEmfWin32 : public Inkscape::Extension::Implementation::Implementation
{
    double  _width;
    double  _height;
    HDC    hdc;
    RECT   rc;

    HBRUSH hbrush, hbrushOld;
    HPEN hpen, hpenOld;

    NArtBpath *fill_path;
    NArtBpath *fill_path_copy;
    NR::Matrix fill_transform;
    NRRect fill_pbox;
    NR::Matrix text_transform;

    unsigned int print_bpath (const NArtBpath *bp, const NR::Matrix *transform, NRRect const *pbox);

public:
    PrintEmfWin32 (void);
    virtual ~PrintEmfWin32 (void);

    /* Print functions */
    virtual unsigned int setup (Inkscape::Extension::Print * module);

    virtual unsigned int begin (Inkscape::Extension::Print * module, SPDocument *doc);
    virtual unsigned int finish (Inkscape::Extension::Print * module);

    /* Rendering methods */
    virtual unsigned int bind(Inkscape::Extension::Print *module, NR::Matrix const *transform, float opacity);
    virtual unsigned int release(Inkscape::Extension::Print *module);
    virtual unsigned int fill (Inkscape::Extension::Print * module,
                               const NRBPath *bpath, const NR::Matrix *ctm, const SPStyle *style,
                               const NRRect *pbox, const NRRect *dbox, const NRRect *bbox);
    virtual unsigned int stroke (Inkscape::Extension::Print * module,
                                 const NRBPath *bpath, const NR::Matrix *transform, const SPStyle *style,
                                 const NRRect *pbox, const NRRect *dbox, const NRRect *bbox);
    virtual unsigned int comment(Inkscape::Extension::Print *module, const char * comment);
    virtual unsigned int text(Inkscape::Extension::Print *module, char const *text,
                              NR::Point p, SPStyle const *style);
    bool textToPath (Inkscape::Extension::Print * ext);

    static void init (void);

protected:
    int create_brush(SPStyle const *style);

    void destroy_brush();

    void create_pen(SPStyle const *style, const NR::Matrix *transform);

    void destroy_pen();

    void flush_fill();

    NArtBpath *copy_bpath(const NArtBpath *bp);
    int cmp_bpath(const NArtBpath *bp1, const NArtBpath *bp2);

};

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* WIN32 */

#endif /* __INKSCAPE_EXTENSION_INTERNAL_PRINT_EMF_WIN32_H__ */

/*
  Local Variables:
  mode:cpp
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

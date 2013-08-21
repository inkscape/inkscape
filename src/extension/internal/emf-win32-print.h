/** @file
 * @brief Enhanced Metafile printing - implementation
 */
/* Author:
 *   Ulf Erikson <ulferikson@users.sf.net>
 *
 * Copyright (C) 2006-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef __INKSCAPE_EXTENSION_INTERNAL_PRINT_EMF_WIN32_H__
#define __INKSCAPE_EXTENSION_INTERNAL_PRINT_EMF_WIN32_H__

#ifdef WIN32

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "extension/implementation/implementation.h"
//#include "extension/extension.h"

#include <2geom/pathvector.h>

#include <stack>

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

    std::stack<Geom::Affine> m_tr_stack;
    Geom::PathVector fill_pathv;
    Geom::Affine fill_transform;
    bool stroke_and_fill;
    bool fill_only;
    bool simple_shape;

    unsigned int print_pathv (Geom::PathVector const &pathv, const Geom::Affine &transform);
    bool print_simple_shape (Geom::PathVector const &pathv, const Geom::Affine &transform);

public:
    PrintEmfWin32 (void);
    virtual ~PrintEmfWin32 (void);

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
    virtual unsigned int comment(Inkscape::Extension::Print *module, const char * comment);
    virtual unsigned int text(Inkscape::Extension::Print *module, char const *text,
                              Geom::Point const &p, SPStyle const *style);
    bool textToPath (Inkscape::Extension::Print * ext);

    static void init (void);
protected:
    int create_brush(SPStyle const *style);

    void destroy_brush();

    void create_pen(SPStyle const *style, const Geom::Affine &transform);

    void destroy_pen();

    void flush_fill();

};

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* WIN32 */

#endif /* __INKSCAPE_EXTENSION_INTERNAL_PRINT_EMF_WIN32_H__ */

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

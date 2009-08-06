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

    std::stack<Geom::Matrix> m_tr_stack;
    Geom::PathVector fill_pathv;
    Geom::Matrix fill_transform;
    bool stroke_and_fill;
    bool fill_only;
    bool simple_shape;

    unsigned int print_pathv (Geom::PathVector const &pathv, const Geom::Matrix &transform);
    bool print_simple_shape (Geom::PathVector const &pathv, const Geom::Matrix &transform);

public:
    PrintEmfWin32 (void);
    virtual ~PrintEmfWin32 (void);

    /* Print functions */
    virtual unsigned int setup (Inkscape::Extension::Print * module);

    virtual unsigned int begin (Inkscape::Extension::Print * module, SPDocument *doc);
    virtual unsigned int finish (Inkscape::Extension::Print * module);

    /* Rendering methods */
    virtual unsigned int bind(Inkscape::Extension::Print *module, Geom::Matrix const *transform, float opacity);
    virtual unsigned int release(Inkscape::Extension::Print *module);
    virtual unsigned int fill (Inkscape::Extension::Print * module,
                               Geom::PathVector const &pathv, const Geom::Matrix *ctm, const SPStyle *style,
                               const NRRect *pbox, const NRRect *dbox, const NRRect *bbox);
    virtual unsigned int stroke (Inkscape::Extension::Print * module,
                                 Geom::PathVector const &pathv, const Geom::Matrix *transform, const SPStyle *style,
                                 const NRRect *pbox, const NRRect *dbox, const NRRect *bbox);
    virtual unsigned int comment(Inkscape::Extension::Print *module, const char * comment);
    virtual unsigned int text(Inkscape::Extension::Print *module, char const *text,
                              Geom::Point p, SPStyle const *style);
    bool textToPath (Inkscape::Extension::Print * ext);

    static void init (void);

protected:
    int create_brush(SPStyle const *style);

    void destroy_brush();

    void create_pen(SPStyle const *style, const Geom::Matrix &transform);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

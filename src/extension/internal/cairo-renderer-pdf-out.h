/*
 * A quick hack to use the Cairo renderer to write out a file.  This
 * then makes 'save as...' PDF.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *   Ulf Erikson <ulferikson@users.sf.net>
 *
 * Copyright (C) 2004-2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef EXTENSION_INTERNAL_CAIRO_RENDERER_PDF_OUT_H
#define EXTENSION_INTERNAL_CAIRO_RENDERER_PDF_OUT_H

#include "extension/implementation/implementation.h"

#ifdef HAVE_CAIRO_PDF

namespace Inkscape {
namespace Extension {
namespace Internal {

class CairoRendererPdfOutput : Inkscape::Extension::Implementation::Implementation {

public:
    bool check(Inkscape::Extension::Extension *module);
    void save(Inkscape::Extension::Output *mod,
              SPDocument *doc,
              gchar const *filename);
    static void init();
};

} } }  /* namespace Inkscape, Extension, Internal */

#endif /* HAVE_CAIRO_PDF */

#endif /* !EXTENSION_INTERNAL_CAIRO_RENDERER_PDF_OUT_H */

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

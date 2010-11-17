/*
 * A quick hack to use the print output to write out a file.  This
 * then makes 'save as...' PNG.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *   Ulf Erikson <ulferikson@users.sf.net>
 *
 * Copyright (C) 2004-2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef EXTENSION_INTERNAL_CAIRO_PNG_OUT_H
#define EXTENSION_INTERNAL_CAIRO_PNG_OUT_H

#include "extension/implementation/implementation.h"

#ifdef HAVE_CAIRO_PDF

namespace Inkscape {
namespace Extension {
namespace Internal {

class CairoRendererOutput : Inkscape::Extension::Implementation::Implementation {

public:
    bool check(Inkscape::Extension::Extension *module);
    void save(Inkscape::Extension::Output *mod,
              SPDocument *doc,
              gchar const *filename);
    static void init();
};

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* HAVE_CAIRO_PDF */

#endif /* !EXTENSION_INTERNAL_CAIRO_PNG_OUT_H */

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

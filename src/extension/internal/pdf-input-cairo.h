#ifndef __EXTENSION_INTERNAL_PDFINPUTCAIRO_H__
#define __EXTENSION_INTERNAL_PDFINPUTCAIRO_H__

/*
 * PDF input using libpoppler and Cairo's SVG surface.
 *
 * Authors:
 *   miklos erdelyi
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_POPPLER_GLIB

#include "../implementation/implementation.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

class PdfInputCairo: public Inkscape::Extension::Implementation::Implementation {
    PdfInputCairo () { };
public:
    SPDocument *open( Inkscape::Extension::Input *mod,
                                const gchar *uri );
    static void         init( void );

};

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* HAVE_POPPLER_GLIB */

#endif /* __EXTENSION_INTERNAL_PDFINPUTCAIRO_H__ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

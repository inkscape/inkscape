/*
 * This is the code that moves all of the SVG loading and saving into
 * the module format.  Really Sodipodi is built to handle these formats
 * internally, so this is just calling those internal functions.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __SVG_H__
#define __SVG_H__

#include "../implementation/implementation.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

class Svg : public Inkscape::Extension::Implementation::Implementation {

public:
    virtual void        save( Inkscape::Extension::Output *mod,
                               SPDocument *doc,
                               gchar const *filename );
    virtual SPDocument *open( Inkscape::Extension::Input *mod,
                                const gchar *uri );
    static void         init( void );

};

} } }  /* namespace Inkscape, Extension, Implementation */
#endif /* __SVG_H__ */

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

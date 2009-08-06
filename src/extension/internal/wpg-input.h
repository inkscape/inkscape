/*
 * This code abstracts the libwpg interfaces into the Inkscape
 * input extension interface.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __EXTENSION_INTERNAL_WPGOUTPUT_H__
#define __EXTENSION_INTERNAL_WPGOUTPUT_H__

#ifdef WITH_LIBWPG

#include "../implementation/implementation.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

class WpgInput : public Inkscape::Extension::Implementation::Implementation {
    WpgInput () { };
public:
    SPDocument *open( Inkscape::Extension::Input *mod,
                                const gchar *uri );
    static void         init( void );

};

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* WITH_LIBWPG */
#endif /* __EXTENSION_INTERNAL_WPGOUTPUT_H__ */


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

/*
 * This code abstracts the libwpg interfaces into the Inkscape
 * input extension interface.
 *
 * Authors:
 *   Fridrich Strba (fridrich.strba@bluewin.ch)
 *
 * Copyright (C) 2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __EXTENSION_INTERNAL_CDROUTPUT_H__
#define __EXTENSION_INTERNAL_CDROUTPUT_H__

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef WITH_LIBCDR

#include <gtkmm/dialog.h>

#include "../implementation/implementation.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

class CdrInput : public Inkscape::Extension::Implementation::Implementation {
     CdrInput () { };
public:
     SPDocument *open( Inkscape::Extension::Input *mod,
                       const gchar *uri );
     static void         init( void );

};

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* WITH_LIBCDR */
#endif /* __EXTENSION_INTERNAL_CDROUTPUT_H__ */


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

/*
 * Authors:
 *   Soren Berg <glimmer07@gmail.com>
 *
 * Copyright (C) 2009 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_EXTENSION_DBUS_INIT_H__
#define INKSCAPE_EXTENSION_DBUS_INIT_H__

#include "desktop.h"

namespace Inkscape {
namespace Extension {
namespace Dbus {

/** \brief  Dbus stuff.  For registering objects on the bus. */

void init (void);

gchar * init_document (void);

gchar * init_desktop (void);

gchar * dbus_init_desktop_interface (SPDesktop * dt);

} } }  /* namespace Dbus, Extension, Inkscape */

#endif /* INKSCAPE_EXTENSION_DBUS_INIT_H__ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

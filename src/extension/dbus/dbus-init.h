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

class SPDesktop;

namespace Inkscape {
namespace Extension {
namespace Dbus {

/** \brief  Dbus stuff.  For registering objects on the bus. */

void init (void);

gchar * init_document (void);

gchar * init_active_document ();

gchar * init_desktop (void);

gchar * dbus_init_desktop_interface (SPDesktop * dt);

/** Set the bus name to use. Default is "org.inkscape".
  This function should only be called once, before init(), if a non-default
  bus name is required. */
void dbus_set_bus_name(gchar * bus_name);

/** Get the bus name for this instance. Default is "org.inkscape".
  This function should only be called after init().
  The returned gchar * is owned by this module and should not be freed. */
gchar * dbus_get_bus_name();

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

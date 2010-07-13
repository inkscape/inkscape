/*
 * This is where the implementation of the DBus based application API lives.
 * All the methods in here are designed to be called remotly via DBus.
 * document-interface.cpp has all of the actual manipulation methods.
 * This interface is just for creating new document interfaces.
 *
 * Documentation for these methods is in application-interface.xml
 * which is the "gold standard" as to how the interface should work.
 *
 * Authors:
 *   Soren Berg <Glimmer07@gmail.com>
 *
 * Copyright (C) 2009 Soren Berg
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_EXTENSION_APPLICATION_INTERFACE_H_
#define INKSCAPE_EXTENSION_APPLICATION_INTERFACE_H_

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <dbus/dbus-glib-lowlevel.h>

#define DBUS_APPLICATION_INTERFACE_PATH  "/org/inkscape/application"
        
#define TYPE_APPLICATION_INTERFACE            (application_interface_get_type ())
#define APPLICATION_INTERFACE(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_APPLICATION_INTERFACE, ApplicationInterface))
#define APPLICATION_INTERFACE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_APPLICATION_INTERFACE, ApplicationInterfaceClass))
#define IS_APPLICATION_INTERFACE(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_APPLICATION_INTERFACE))
#define IS_APPLICATION_INTERFACE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_APPLICATION_INTERFACE))
#define APPLICATION_INTERFACE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_APPLICATION_INTERFACE, ApplicationInterfaceClass))

G_BEGIN_DECLS

typedef struct _ApplicationInterface ApplicationInterface;
typedef struct _ApplicationInterfaceClass ApplicationInterfaceClass;

struct _ApplicationInterface {
        GObject parent;
};

struct _ApplicationInterfaceClass {
        GObjectClass parent;
};

/****************************************************************************
     DESKTOP FUNCTIONS
****************************************************************************/

gchar* 
application_interface_desktop_new (ApplicationInterface *object, 
                                   GError **error);

gchar** 
application_interface_get_desktop_list (ApplicationInterface *object);

gchar* 
application_interface_get_active_desktop (ApplicationInterface *object, 
                                          GError **error);

gboolean
application_interface_set_active_desktop (ApplicationInterface *object,
                                          gchar* document_name, 
                                          GError **error);

gboolean
application_interface_desktop_close_all (ApplicationInterface *object, 
                                         GError **error);

gboolean
application_interface_exit (ApplicationInterface *object, GError **error);

/****************************************************************************
     DOCUMENT FUNCTIONS
****************************************************************************/

gchar* 
application_interface_document_new (ApplicationInterface *object, 
                                    GError **error);

gchar** 
application_interface_get_document_list (ApplicationInterface *object);

gboolean
application_interface_document_close_all (ApplicationInterface *object,
                                          GError **error);


/****************************************************************************
     SETUP
****************************************************************************/

ApplicationInterface *application_interface_new (void);
GType application_interface_get_type (void);


G_END_DECLS

#endif // INKSCAPE_EXTENSION_APPLICATION_INTERFACE_H_

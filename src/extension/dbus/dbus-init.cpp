/*
 * This is where Inkscape connects to the DBus when it starts and 
 * registers the main interface.
 *
 * Also where new interfaces are registered when a new document is created.
 * (Not called directly by application-interface but called indirectly.)
 *
 * Authors:
 *   Soren Berg <Glimmer07@gmail.com>
 *
 * Copyright (C) 2009 Soren Berg
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
 
#include <dbus/dbus-glib.h>
#include "dbus-init.h"

#include "application-interface.h"
#include "application-server-glue.h"

#include "document-interface.h"
#include "document-server-glue.h"

#include "inkscape.h"
#include "document.h"
#include "desktop.h"
#include "file.h"
#include "verbs.h"
#include "helper/action.h"

#include <algorithm>
#include <iostream>
#include <sstream>




namespace Inkscape {
namespace Extension {
namespace Dbus {

/* PRIVATE get a connection to the session bus */
DBusGConnection *
dbus_get_connection() {
	GError *error = NULL;
	DBusGConnection *connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (error) {
		fprintf(stderr, "Failed to get connection");
		return NULL;
	}
	else
		return connection;
}

/* PRIVATE create a proxy object for a bus.*/
DBusGProxy *
dbus_get_proxy(DBusGConnection *connection) {
	return dbus_g_proxy_new_for_name (connection,
                DBUS_SERVICE_DBUS,
                DBUS_PATH_DBUS,
                DBUS_INTERFACE_DBUS);
}

/* PRIVATE register an object on a bus */
static gpointer
dbus_register_object (DBusGConnection *connection,
                      DBusGProxy *proxy,
                      GType object_type,
                      const DBusGObjectInfo *info,
                      const gchar *path)
{
        GObject *object = (GObject*)g_object_new (object_type, NULL);
        dbus_g_object_type_install_info (object_type, info);
        dbus_g_connection_register_g_object (connection, path, object);
        return object;
}

/* Initialize a Dbus service */
void 
init (void)
{
        guint   result;
        GError *error = NULL;
        DBusGConnection *connection;
        DBusGProxy *proxy;
	    DocumentInterface *obj;
        connection = dbus_get_connection();
        proxy = dbus_get_proxy(connection);
        org_freedesktop_DBus_request_name (proxy,
                "org.inkscape",
                DBUS_NAME_FLAG_DO_NOT_QUEUE, &result, &error);
        //create interface for application
        dbus_register_object (connection, 
                proxy,
                TYPE_APPLICATION_INTERFACE,
                &dbus_glib_application_interface_object_info,
                DBUS_APPLICATION_INTERFACE_PATH);
} //init

gchar *
init_document (void) {
        guint   result;
        GError *error = NULL;
        DBusGConnection *connection;
        DBusGProxy *proxy;
	SPDocument *doc;

	doc = sp_document_new(NULL, 1, TRUE);

        std::string name("/org/inkscape/");
	name.append(doc->name);
        std::replace(name.begin(), name.end(), ' ', '_');

        connection = dbus_get_connection();
        proxy = dbus_get_proxy(connection);

        dbus_register_object (connection, 
                proxy,
                TYPE_DOCUMENT_INTERFACE,
                &dbus_glib_document_interface_object_info,
                name.c_str());
	return strdup(name.c_str());
} //init_document

gchar *
dbus_init_desktop_interface (SPDesktop * dt)
{
    DBusGConnection *connection;
    DBusGProxy *proxy;
	DocumentInterface *obj;
	dbus_g_error_domain_register (INKSCAPE_ERROR,
				NULL,
				INKSCAPE_TYPE_ERROR);

    std::string name("/org/inkscape/desktop_");
	std::stringstream out;
	out << dt->dkey;
	name.append(out.str());

	//printf("DKEY: %d\n, NUMBER %d\n NAME: %s\n", dt->dkey, dt->number, name.c_str());

    connection = dbus_get_connection();
    proxy = dbus_get_proxy(connection);

    obj = (DocumentInterface*) dbus_register_object (connection, 
          proxy, TYPE_DOCUMENT_INTERFACE,
          &dbus_glib_document_interface_object_info, name.c_str());
	obj->desk = dt;
    obj->updates = TRUE;

    return strdup(name.c_str());
}

gchar *
init_desktop (void) {
    //this function will create a new desktop and call
    //dbus_init_desktop_interface.
	SPDesktop * dt = sp_file_new_default();

    std::string name("/org/inkscape/desktop_");
	std::stringstream out;
	out << dt->dkey;
	name.append(out.str());
    return strdup(name.c_str());
} //init_desktop



} } } /* namespace Inkscape::Extension::Dbus */

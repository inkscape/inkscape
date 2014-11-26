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
// this is reguired so that giomm headers won't barf
#undef DBUS_MESSAGE_TYPE_INVALID
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


namespace
{
    // This stores the bus name to use for this app instance. By default, it
    // will be set to org.inkscape. However, users may provide other names by
    // setting command-line parameters when starting Inkscape, so that more
    // than one instance of Inkscape may be used by external scripts.
    gchar *instance_bus_name = NULL;
}

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

/*
 * PRIVATE register a document interface for the document in the given ActionContext, if none exists.
 * Return the DBus path to the interface (something like /org/inkscape/document_0).
 * Note that while a DocumentInterface could be used either for a document with no desktop, or a
 * document with a desktop, this function is only used for creating interfaces in the former case.
 * Desktop-associated DocumentInterfaces are named /org/inkscape/desktop_0, etc.
 * FIXME: This state of affairs probably needs tidying up at some point in the future.
 */
static gchar *
dbus_register_document(Inkscape::ActionContext const & target)
{
    SPDocument *doc = target.getDocument();
    g_assert(doc != NULL);

    // Document name is not suitable for DBus name, as it might contain invalid chars
    std::string name("/org/inkscape/document_");
    std::stringstream ss;
    ss << doc->serial();
    name.append(ss.str());
    
    DBusGConnection *connection = dbus_get_connection();
    DBusGProxy *proxy = dbus_get_proxy(connection);

    // Has the document already been registered?
    if (!dbus_g_connection_lookup_g_object(connection, name.c_str())) {
        // No - register it
        DocumentInterface *doc_interface = (DocumentInterface*) dbus_register_object (connection, 
            proxy,
            TYPE_DOCUMENT_INTERFACE,
            &dbus_glib_document_interface_object_info,
            name.c_str());

        // Set the document info for this interface
        doc_interface->target = target;
    }
    return strdup(name.c_str());
}

/* Initialize a Dbus service */
void 
init (void)
{
        if (instance_bus_name == NULL) {
            // Set the bus name to the default
            instance_bus_name = strdup("org.inkscape");
        }

        guint   result;
        GError *error = NULL;
        DBusGConnection *connection;
        DBusGProxy *proxy;
        connection = dbus_get_connection();
        proxy = dbus_get_proxy(connection);
        org_freedesktop_DBus_request_name (proxy,
                instance_bus_name,
                DBUS_NAME_FLAG_DO_NOT_QUEUE, &result, &error);
        //create interface for application
        dbus_register_object (connection, 
                proxy,
                TYPE_APPLICATION_INTERFACE,
                &dbus_glib_application_interface_object_info,
                DBUS_APPLICATION_INTERFACE_PATH);
}

gchar *
init_document (void)
{
    // This is for command-line use only
    g_assert(!INKSCAPE.use_gui());

    // Create a blank document and get its selection model etc in an ActionContext
    SPDocument *doc = SPDocument::createNewDoc(NULL, 1, TRUE);
    INKSCAPE.add_document(doc);
    return dbus_register_document(INKSCAPE.action_context_for_document(doc));
}

gchar *
init_active_document()
{
    SPDocument *doc = INKSCAPE.active_document();
    if (!doc) {
        return NULL;
    }
    
    return dbus_register_document(INKSCAPE.active_action_context());
}

gchar *
dbus_init_desktop_interface (SPDesktop * dt)
{
    DBusGConnection *connection;
    DBusGProxy *proxy;

    std::string name("/org/inkscape/desktop_");
	std::stringstream out;
	out << dt->dkey;
	name.append(out.str());

	//printf("DKEY: %d\n, NUMBER %d\n NAME: %s\n", dt->dkey, dt->number, name.c_str());

    connection = dbus_get_connection();
    proxy = dbus_get_proxy(connection);

    DocumentInterface *doc_interface = (DocumentInterface*) dbus_register_object (connection, 
          proxy, TYPE_DOCUMENT_INTERFACE,
          &dbus_glib_document_interface_object_info, name.c_str());
    doc_interface->target = Inkscape::ActionContext(dt);
    doc_interface->updates = TRUE;
    dt->dbus_document_interface=doc_interface;
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
}

void
dbus_set_bus_name(gchar * bus_name)
{
    g_assert(bus_name != NULL);
    g_assert(instance_bus_name == NULL);
    instance_bus_name = strdup(bus_name);
}

gchar *
dbus_get_bus_name()
{
    g_assert(instance_bus_name != NULL);
    return instance_bus_name;
}

} } } /* namespace Inkscape::Extension::Dbus */

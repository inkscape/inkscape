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
 
#include "application-interface.h"
#include <string.h>
#include "dbus-init.h"

G_DEFINE_TYPE(ApplicationInterface, application_interface, G_TYPE_OBJECT)

static void
application_interface_finalize (GObject *object)
{
        G_OBJECT_CLASS (application_interface_parent_class)->finalize (object);
}


static void
application_interface_class_init (ApplicationInterfaceClass *klass)
{
        GObjectClass *object_class;
        object_class = G_OBJECT_CLASS (klass);
        object_class->finalize = application_interface_finalize;
}

static void
application_interface_init (ApplicationInterface *object)
{
}


ApplicationInterface *
application_interface_new (void)
{
        return (ApplicationInterface*)g_object_new (TYPE_APPLICATION_INTERFACE, NULL);
}

/****************************************************************************
     DESKTOP FUNCTIONS
****************************************************************************/

gchar* 
application_interface_desktop_new (ApplicationInterface *object, 
                                   GError **error) 
{
  return (gchar*)Inkscape::Extension::Dbus::init_desktop();
}

gchar** 
application_interface_get_desktop_list (ApplicationInterface *object)
{
  return NULL;
}

gchar* 
application_interface_get_active_desktop (ApplicationInterface *object,
                                          GError **error)
{
  return NULL;
}

gboolean
application_interface_set_active_desktop (ApplicationInterface *object, 
                                          gchar* document_name,
                                          GError **error)
{
  return TRUE;
}

gboolean
application_interface_desktop_close_all (ApplicationInterface *object,
                                          GError **error) 
{
  return TRUE;
}

gboolean
application_interface_exit (ApplicationInterface *object, GError **error)
{
    return TRUE;
}

/****************************************************************************
     DOCUMENT FUNCTIONS
****************************************************************************/

gchar* application_interface_document_new (ApplicationInterface *object,
                                           GError **error)
{
  return (gchar*)Inkscape::Extension::Dbus::init_document();
}

gchar** 
application_interface_get_document_list (ApplicationInterface *object)
{
  return NULL;
}

gboolean
application_interface_document_close_all (ApplicationInterface *object,
                                          GError **error) 
{
  return TRUE;
}

/* INTERESTING FUNCTIONS
    SPDesktop  *desktop = SP_ACTIVE_DESKTOP;
    g_assert(desktop != NULL);

    SPDocument *doc = sp_desktop_document(desktop);
    g_assert(doc != NULL);

    Inkscape::XML::Node     *repr = sp_document_repr_root(doc);
    g_assert(repr != NULL);
*/


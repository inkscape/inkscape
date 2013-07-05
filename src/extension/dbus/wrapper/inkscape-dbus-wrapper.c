#include "inkscape-dbus-wrapper.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>



#include "document-client-glue.h"
#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>

// (static.*(\n[^}]*)*(async)+.*(\n[^}]*)*})|typedef void .*;
// http://www.josephkahn.com/music/index.xml

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

#if 0
/* PRIVATE register an object on a bus */
static gpointer
dbus_register_object (DBusGConnection *connection,
                      DBusGProxy * proxy,
                      GType object_type,
                      const DBusGObjectInfo *info,
                      const gchar *path)
{
        GObject *object = (GObject*)g_object_new (object_type, NULL);
        dbus_g_object_type_install_info (object_type, info);
        dbus_g_connection_register_g_object (connection, path, object);
        return object;
}
#endif

/****************************************************************************
     DOCUMENT INTERFACE CLASS STUFF
****************************************************************************/

struct _DocumentInterface {
	GObject parent;
	DBusGProxy * proxy;
};

G_DEFINE_TYPE(DocumentInterface, document_interface, G_TYPE_OBJECT)

static void
document_interface_finalize (GObject *object)
{
        G_OBJECT_CLASS (document_interface_parent_class)->finalize (object);
}


static void
document_interface_class_init (DocumentInterfaceClass *klass)
{
        GObjectClass *object_class;
        object_class = G_OBJECT_CLASS (klass);
        object_class->finalize = document_interface_finalize;
}

static void
document_interface_init (DocumentInterface *object)
{
	object->proxy = NULL;
}


DocumentInterface *
document_interface_new (void)
{
        return (DocumentInterface*)g_object_new (TYPE_DOCUMENT_INTERFACE, NULL);
}

DocumentInterface *
inkscape_desktop_init_dbus ()
{
    DBusGConnection *connection;
    GError *error;
    DBusGProxy *proxy;
  
#if !GLIB_CHECK_VERSION(2,36,0)
    g_type_init ();
#endif

    error = NULL;
    connection = dbus_g_bus_get (DBUS_BUS_SESSION,
                               &error);
    if (connection == NULL)
        {
            g_printerr ("Failed to open connection to bus: %s\n",
                  error->message);
            g_error_free (error);
            exit (1);
        }
  
    proxy = dbus_g_proxy_new_for_name (connection,
                                     "org.inkscape",
                                     "/org/inkscape/desktop_0",
                                     "org.inkscape.document");

     DocumentInterface * inkdesk = (DocumentInterface *)g_object_new (TYPE_DOCUMENT_INTERFACE, NULL);
    inkdesk->proxy = proxy;
    return inkdesk;
}


//static
gboolean
inkscape_delete_all (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
    return org_inkscape_document_delete_all (proxy, error);
}

//static
gboolean
inkscape_call_verb (DocumentInterface *doc, const char * IN_verbid, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_call_verb(proxy, IN_verbid, error);
}

 
//static
gchar *
inkscape_rectangle (DocumentInterface *doc, const gint IN_x, const gint IN_y, const gint IN_width, const gint IN_height, GError **error)
{
  char * OUT_object_name;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_rectangle (proxy, IN_x, IN_y, IN_width, IN_height, &OUT_object_name, error);
  return OUT_object_name;
}

//static
char *
inkscape_ellipse (DocumentInterface *doc, const gint IN_x, const gint IN_y, const gint IN_width, const gint IN_height, GError **error)
{
  char * OUT_object_name;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_ellipse (proxy, IN_x, IN_y, IN_width, IN_height, &OUT_object_name, error);
  return OUT_object_name;
}

//static
char *
inkscape_polygon (DocumentInterface *doc, const gint IN_cx, const gint IN_cy, const gint IN_radius, const gint IN_rotation, const gint IN_sides, GError **error)
{
  char * OUT_object_name;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_polygon (proxy, IN_cx, IN_cy, IN_radius, IN_rotation, IN_sides, &OUT_object_name, error);
  return OUT_object_name;
}

//static
char *
inkscape_star (DocumentInterface *doc, const gint IN_cx, const gint IN_cy, const gint IN_r1, const gint IN_r2, const gdouble IN_arg1, const gdouble IN_arg2, const gint IN_sides, const gdouble IN_rounded, GError **error)
{
  char * OUT_object_name;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_star (proxy, IN_cx, IN_cy, IN_r1, IN_r2, IN_arg1, IN_arg2, IN_sides, IN_rounded, &OUT_object_name, error);
  return OUT_object_name;
}

//static
char *
inkscape_spiral (DocumentInterface *doc, const gint IN_cx, const gint IN_cy, const gint IN_r, const gint IN_revolutions, GError **error)
{
  char * OUT_object_name;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_spiral (proxy, IN_cx, IN_cy, IN_r, IN_revolutions, &OUT_object_name, error);
  return OUT_object_name;
}

//static
char *
inkscape_line (DocumentInterface *doc, const gint IN_x, const gint IN_y, const gint IN_x2, const gint IN_y2, GError **error)
{
  char * OUT_object_name;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_line (proxy, IN_x, IN_y, IN_x2, IN_y2, &OUT_object_name, error);
  return OUT_object_name;
}

//static
char *
inkscape_text (DocumentInterface *doc, const gint IN_x, const gint IN_y, const char * IN_text, GError **error)
{
  char * OUT_object_name;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_text (proxy, IN_x, IN_y, IN_text, &OUT_object_name, error);
  return OUT_object_name;
}

//static
char *
inkscape_image (DocumentInterface *doc, const gint IN_x, const gint IN_y, const char * IN_text, GError **error)
{
  char * OUT_object_name;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_image (proxy, IN_x, IN_y, IN_text, &OUT_object_name, error);
  return OUT_object_name;
}

//static
char *
inkscape_node (DocumentInterface *doc, const char * IN_svgtype, GError **error)
{
  char *OUT_node_name;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_node (proxy, IN_svgtype, &OUT_node_name, error);
  return OUT_node_name;
}

//static
gdouble
inkscape_document_get_width (DocumentInterface *doc, GError **error)
{
  gdouble OUT_val;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_document_get_width (proxy, &OUT_val, error);
  return OUT_val;
}

//static
gdouble
inkscape_document_get_height (DocumentInterface *doc, GError **error)
{
  gdouble OUT_val;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_document_get_height (proxy, &OUT_val, error);
  return OUT_val;
}

//static
char *
inkscape_document_get_css (DocumentInterface *doc, GError **error)
{
  char * OUT_css;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_document_get_css (proxy, &OUT_css, error);
  return OUT_css;
}

//static
gboolean
inkscape_document_set_css (DocumentInterface *doc, const char * IN_stylestring, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_document_set_css (proxy, IN_stylestring, error);
}

//static
gboolean
inkscape_document_merge_css (DocumentInterface *doc, const char * IN_stylestring, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_document_merge_css (proxy, IN_stylestring, error);
}

//static
gboolean
inkscape_document_resize_to_fit_selection (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_document_resize_to_fit_selection (proxy, error);
}

//static
gboolean
inkscape_set_attribute (DocumentInterface *doc, const char * IN_shape, const char * IN_attribute, const char * IN_newval, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_set_attribute (proxy, IN_shape, IN_attribute, IN_newval, error);
}

//static
gboolean
inkscape_set_int_attribute (DocumentInterface *doc, const char * IN_shape, const char * IN_attribute, const gint IN_newval, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_set_int_attribute (proxy, IN_shape, IN_attribute, IN_newval, error);
}

//static
gboolean
inkscape_set_double_attribute (DocumentInterface *doc, const char * IN_shape, const char * IN_attribute, const gdouble IN_newval, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_set_double_attribute (proxy, IN_shape, IN_attribute, IN_newval, error);
}

//static
char *
inkscape_get_attribute (DocumentInterface *doc, const char * IN_shape, const char * IN_attribute, GError **error)
{
  char * OUT_val;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_get_attribute (proxy, IN_shape, IN_attribute, &OUT_val, error);
  return OUT_val;
}

//static
gboolean
inkscape_move (DocumentInterface *doc, const char * IN_shape, const gdouble IN_x, const gdouble IN_y, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_move (proxy, IN_shape, IN_x, IN_y, error);
}

//static
gboolean
inkscape_move_to (DocumentInterface *doc, const char * IN_shape, const gdouble IN_x, const gdouble IN_y, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_move_to (proxy, IN_shape, IN_x, IN_y, error);
}

//static
gboolean
inkscape_object_to_path (DocumentInterface *doc, const char * IN_objectname, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_object_to_path (proxy, IN_objectname, error);
}

//static
char *
inkscape_get_path (DocumentInterface *doc, const char * IN_shape, GError **error)
{
  char * OUT_val;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_get_path (proxy, IN_shape, &OUT_val, error);
  return OUT_val;
}

//static
gboolean
inkscape_transform (DocumentInterface *doc, const char * IN_shape, const char * IN_transformstr, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_transform (proxy, IN_shape, IN_transformstr, error);
}

//static
char *
inkscape_get_css (DocumentInterface *doc, const char * IN_shape, GError **error)
{
  char * OUT_css;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_get_css (proxy, IN_shape, &OUT_css, error);
  return OUT_css;
}

//static
gboolean
inkscape_modify_css (DocumentInterface *doc, const char * IN_shape, const char * IN_cssattrib, const char * IN_newval, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_modify_css (proxy, IN_shape, IN_cssattrib, IN_newval, error);
}

//static
gboolean
inkscape_merge_css (DocumentInterface *doc, const char * IN_shape, const char * IN_stylestring, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_merge_css (proxy, IN_shape, IN_stylestring, error);
}

//static
gboolean
inkscape_set_color (DocumentInterface *doc, const char * IN_shape, const gint IN_red, const gint IN_green, const gint IN_blue, const gboolean IN_fill, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_set_color (proxy, IN_shape, IN_red, IN_green, IN_blue, IN_fill, error);
}

//static
gboolean
inkscape_move_to_layer (DocumentInterface *doc, const char * IN_objectname, const char * IN_layername, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_move_to_layer (proxy, IN_objectname, IN_layername, error);
}

//static
GArray*
inkscape_get_node_coordinates (DocumentInterface *doc, const char * IN_shape, GError **error)
{
  GArray* OUT_points;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_get_node_coordinates (proxy, IN_shape, &OUT_points, error);
  return OUT_points;
}

//static
gboolean
inkscape_save (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_save (proxy, error);
}

//static
gboolean
inkscape_save_as (DocumentInterface *doc, const char * IN_pathname, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_save_as (proxy, IN_pathname, error);
}

//static
gboolean
inkscape_load (DocumentInterface *doc, const char * IN_pathname, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_load (proxy, IN_pathname, error);
}

//static
gboolean
inkscape_mark_as_unmodified (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_mark_as_unmodified (proxy, error);
}

//static
gboolean
inkscape_close (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_close (proxy, error);
}

//static
gboolean
inkscape_inkscape_exit (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_exit (proxy, error);
}

//static
gboolean
inkscape_undo (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_undo (proxy, error);
}

//static
gboolean
inkscape_redo (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_redo (proxy, error);
}

//static
gboolean
inkscape_pause_updates (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_pause_updates (proxy, error);
}

//static
gboolean
inkscape_resume_updates (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_resume_updates (proxy, error);
}

//static
gboolean
inkscape_update (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_update (proxy, error);
}

//static
char **
inkscape_selection_get (DocumentInterface *doc, GError **error)
{
  char ** OUT_listy;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_selection_get (proxy, &OUT_listy, error);
  return OUT_listy;
}

//static
gboolean
inkscape_selection_add (DocumentInterface *doc, const char * IN_name, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_add (proxy, IN_name, error);
}

//static
gboolean
inkscape_selection_add_list (DocumentInterface *doc, const char ** IN_name, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_add_list (proxy, IN_name, error);
}

//static
gboolean
inkscape_selection_set (DocumentInterface *doc, const char * IN_name, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_set (proxy, IN_name, error);
}

//static
gboolean
inkscape_selection_set_list (DocumentInterface *doc, const char ** IN_name, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_set_list (proxy, IN_name, error);
}

//static
gboolean
inkscape_selection_rotate (DocumentInterface *doc, const gint IN_angle, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_rotate (proxy, IN_angle, error);
}

//static
gboolean
inkscape_selection_delete (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_delete (proxy, error);
}

//static
gboolean
inkscape_selection_clear (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_clear (proxy, error);
}

//static
gboolean
inkscape_select_all (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_select_all (proxy, error);
}

//static
gboolean
inkscape_select_all_in_all_layers (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_select_all_in_all_layers (proxy, error);
}

//static
gboolean
inkscape_selection_box (DocumentInterface *doc, const gint IN_x, const gint IN_y, const gint IN_x2, const gint IN_y2, const gboolean IN_replace, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_box (proxy, IN_x, IN_y, IN_x2, IN_y2, IN_replace, error);
}

//static
gboolean
inkscape_selection_invert (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_invert (proxy, error);
}

//static
gboolean
inkscape_selection_group (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_group (proxy, error);
}

//static
gboolean
inkscape_selection_ungroup (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_ungroup (proxy, error);
}

//static
gboolean
inkscape_selection_cut (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_cut (proxy, error);
}

//static
gboolean
inkscape_selection_copy (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_copy (proxy, error);
}

//static
gboolean
inkscape_selection_paste (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_paste (proxy, error);
}

//static
gboolean
inkscape_selection_scale (DocumentInterface *doc, const gdouble IN_grow, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_scale (proxy, IN_grow, error);
}

//static
gboolean
inkscape_selection_move (DocumentInterface *doc, const gdouble IN_x, const gdouble IN_y, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_move (proxy, IN_x, IN_y, error);
}

//static
gboolean
inkscape_selection_move_to (DocumentInterface *doc, const gdouble IN_x, const gdouble IN_y, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_move_to (proxy, IN_x, IN_y, error);
}

//static
gboolean
inkscape_selection_move_to_layer (DocumentInterface *doc, const char * IN_layer, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_move_to_layer (proxy, IN_layer, error);
}

//static
GArray *
inkscape_selection_get_center (DocumentInterface *doc, GError **error)
{
  GArray* OUT_centerpoint;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_selection_get_center (proxy, &OUT_centerpoint, error);
  return OUT_centerpoint;
}

//static
gboolean
inkscape_selection_to_path (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_selection_to_path (proxy, error);
}

//static
char **
inkscape_selection_combine (DocumentInterface *doc, const char * IN_type, GError **error)
{
  char ** OUT_newpaths;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_selection_combine (proxy, IN_type, &OUT_newpaths, error);
  return OUT_newpaths;
}

//static
gboolean
inkscape_selection_change_level (DocumentInterface *doc, const char * IN_command, GError **error)
{
  gboolean OUT_objectsmoved; 
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_selection_change_level (proxy, IN_command, &OUT_objectsmoved, error);
  return OUT_objectsmoved;
}

//static
char *
inkscape_layer_new (DocumentInterface *doc, GError **error)
{
  char * OUT_layername;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_layer_new (proxy, &OUT_layername, error);
  return OUT_layername;
}

//static
gboolean
inkscape_layer_set (DocumentInterface *doc, const char * IN_layer, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_layer_set (proxy, IN_layer, error);
}

//static
char **
inkscape_layer_get_all (DocumentInterface *doc, GError **error)
{
  char ** OUT_layers;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_layer_get_all (proxy, &OUT_layers, error);
  return OUT_layers;
}

//static
gboolean
inkscape_layer_change_level (DocumentInterface *doc, const char * IN_command, GError **error)
{
  gboolean OUT_layermoved;
  DBusGProxy *proxy = doc->proxy;
  org_inkscape_document_layer_change_level (proxy, IN_command, &OUT_layermoved, error);
  return OUT_layermoved;
}

//static
gboolean
inkscape_layer_next (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_layer_next (proxy, error);
}

//static
gboolean
inkscape_layer_previous (DocumentInterface *doc, GError **error)
{
  DBusGProxy *proxy = doc->proxy;
  return org_inkscape_document_layer_previous (proxy, error);
}

/*
int
main (int argc, char** argv) 
{
    gchar * result;
    GError *error = NULL;
    DocumentInterface * doc = inkscape_desktop_init_dbus ();
    result = rectangle (doc->proxy, 10, 10, 100, 100, &error);
    printf("RESULT: %s\n", result);
     
    //dbus_g_proxy_call(doc->proxy, "rectangle", &error, G_TYPE_INT, 100, G_TYPE_INT, 100, G_TYPE_INT, 100, G_TYPE_INT, 100, G_TYPE_INVALID, G_TYPE_INVALID);
    printf("yes\n");
}
*/


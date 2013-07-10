#ifndef INKSCAPE_EXTENSION_DOCUMENT_INTERFACE_H_
#define INKSCAPE_EXTENSION_DOCUMENT_INTERFACE_H_

#include <glib.h>
#include <glib-object.h>

//#include "document-client-glue-mod.h"

//#include <dbus/dbus-glib-bindings.h>
//#include <dbus/dbus-glib-lowlevel.h>
        
#define TYPE_DOCUMENT_INTERFACE            (document_interface_get_type ())
#define DOCUMENT_INTERFACE(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_DOCUMENT_INTERFACE, DocumentInterface))
#define DOCUMENT_INTERFACE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DOCUMENT_INTERFACE, DocumentInterfaceClass))
#define IS_DOCUMENT_INTERFACE(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_DOCUMENT_INTERFACE))
#define IS_DOCUMENT_INTERFACE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DOCUMENT_INTERFACE))
#define DOCUMENT_INTERFACE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DOCUMENT_INTERFACE, DocumentInterfaceClass))

G_BEGIN_DECLS

typedef struct _DocumentInterface DocumentInterface;
typedef struct _DocumentInterfaceClass DocumentInterfaceClass;

struct _DocumentInterface;

struct _DocumentInterfaceClass {
	GObjectClass parent;
};


DocumentInterface *document_interface_new (void);
GType document_interface_get_type (void);



DocumentInterface * 
inkscape_desktop_init_dbus ();

//static
gboolean
inkscape_delete_all (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_call_verb (DocumentInterface *doc, const char * IN_verbid, GError **error);

 
//static
gchar *
inkscape_rectangle (DocumentInterface *doc, const gint IN_x, const gint IN_y, const gint IN_width, const gint IN_height, GError **error);

//static
char *
inkscape_ellipse (DocumentInterface *doc, const gint IN_x, const gint IN_y, const gint IN_width, const gint IN_height, GError **error);

//static
char *
inkscape_polygon (DocumentInterface *doc, const gint IN_cx, const gint IN_cy, const gint IN_radius, const gint IN_rotation, const gint IN_sides, GError **error);

//static
char *
inkscape_star (DocumentInterface *doc, const gint IN_cx, const gint IN_cy, const gint IN_r1, const gint IN_r2, const gdouble IN_arg1, const gdouble IN_arg2, const gint IN_sides, const gdouble IN_rounded, GError **error);

//static
char *
inkscape_spiral (DocumentInterface *doc, const gint IN_cx, const gint IN_cy, const gint IN_r, const gint IN_revolutions, GError **error);

//static
char *
inkscape_line (DocumentInterface *doc, const gint IN_x, const gint IN_y, const gint IN_x2, const gint IN_y2, GError **error);

//static
char *
inkscape_text (DocumentInterface *doc, const gint IN_x, const gint IN_y, const char * IN_text, GError **error);

//static
char *
inkscape_image (DocumentInterface *doc, const gint IN_x, const gint IN_y, const char * IN_text, GError **error);

//static
char *
inkscape_node (DocumentInterface *doc, const char * IN_svgtype, GError **error);

//static
gdouble
inkscape_document_get_width (DocumentInterface *doc, GError **error);

//static
gdouble
inkscape_document_get_height (DocumentInterface *doc, GError **error);

//static
char *
inkscape_document_get_css (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_document_set_css (DocumentInterface *doc, const char * IN_stylestring, GError **error);

//static
gboolean
inkscape_document_merge_css (DocumentInterface *doc, const char * IN_stylestring, GError **error);

//static
gboolean
inkscape_document_resize_to_fit_selection (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_set_attribute (DocumentInterface *doc, const char * IN_shape, const char * IN_attribute, const char * IN_newval, GError **error);

//static
gboolean
inkscape_set_int_attribute (DocumentInterface *doc, const char * IN_shape, const char * IN_attribute, const gint IN_newval, GError **error);

//static
gboolean
inkscape_set_double_attribute (DocumentInterface *doc, const char * IN_shape, const char * IN_attribute, const gdouble IN_newval, GError **error);

//static
char *
inkscape_get_attribute (DocumentInterface *doc, const char * IN_shape, const char * IN_attribute, GError **error);

//static
gboolean
inkscape_move (DocumentInterface *doc, const char * IN_shape, const gdouble IN_x, const gdouble IN_y, GError **error);

//static
gboolean
inkscape_move_to (DocumentInterface *doc, const char * IN_shape, const gdouble IN_x, const gdouble IN_y, GError **error);

//static
gboolean
inkscape_object_to_path (DocumentInterface *doc, const char * IN_objectname, GError **error);

//static
char *
inkscape_get_path (DocumentInterface *doc, const char * IN_shape, GError **error);

//static
gboolean
inkscape_transform (DocumentInterface *doc, const char * IN_shape, const char * IN_transformstr, GError **error);

//static
char *
inkscape_get_css (DocumentInterface *doc, const char * IN_shape, GError **error);

//static
gboolean
inkscape_modify_css (DocumentInterface *doc, const char * IN_shape, const char * IN_cssattrib, const char * IN_newval, GError **error);

//static
gboolean
inkscape_inkscape_merge_css (DocumentInterface *doc, const char * IN_shape, const char * IN_stylestring, GError **error);

//static
gboolean
inkscape_set_color (DocumentInterface *doc, const char * IN_shape, const gint IN_red, const gint IN_green, const gint IN_blue, const gboolean IN_fill, GError **error);

//static
gboolean
inkscape_move_to_layer (DocumentInterface *doc, const char * IN_objectname, const char * IN_layername, GError **error);

//static
GArray*
inkscape_get_node_coordinates (DocumentInterface *doc, const char * IN_shape, GError **error);

//static
gboolean
inkscape_save (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_save_as (DocumentInterface *doc, const char * IN_pathname, GError **error);

//static
gboolean
inkscape_load (DocumentInterface *doc, const char * IN_pathname, GError **error);

//static
gboolean
inkscape_mark_as_unmodified (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_close (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_inkscape_exit (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_undo (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_redo (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_pause_updates (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_resume_updates (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_update (DocumentInterface *doc, GError **error);

//static
char **
inkscape_selection_get (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_selection_add (DocumentInterface *doc, const char * IN_name, GError **error);

//static
gboolean
inkscape_selection_add_list (DocumentInterface *doc, const char ** IN_name, GError **error);

//static
gboolean
inkscape_selection_set (DocumentInterface *doc, const char * IN_name, GError **error);

//static
gboolean
inkscape_selection_set_list (DocumentInterface *doc, const char ** IN_name, GError **error);

//static
gboolean
inkscape_selection_rotate (DocumentInterface *doc, const gint IN_angle, GError **error);

//static
gboolean
inkscape_selection_delete (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_selection_clear (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_select_all (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_select_all_in_all_layers (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_selection_box (DocumentInterface *doc, const gint IN_x, const gint IN_y, const gint IN_x2, const gint IN_y2, const gboolean IN_replace, GError **error);

//static
gboolean
inkscape_selection_invert (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_selection_group (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_selection_ungroup (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_selection_cut (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_selection_copy (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_selection_paste (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_selection_scale (DocumentInterface *doc, const gdouble IN_grow, GError **error);

//static
gboolean
inkscape_selection_move (DocumentInterface *doc, const gdouble IN_x, const gdouble IN_y, GError **error);

//static
gboolean
inkscape_selection_move_to (DocumentInterface *doc, const gdouble IN_x, const gdouble IN_y, GError **error);

//static
gboolean
inkscape_selection_move_to_layer (DocumentInterface *doc, const char * IN_layer, GError **error);

//static
GArray *
inkscape_selection_get_center (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_selection_to_path (DocumentInterface *doc, GError **error);

//static
char **
inkscape_selection_combine (DocumentInterface *doc, const char * IN_type, GError **error);

//static
gboolean
inkscape_selection_change_level (DocumentInterface *doc, const char * IN_command, GError **error);

//static
char *
inkscape_layer_new (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_layer_set (DocumentInterface *doc, const char * IN_layer, GError **error);

//static
char **
inkscape_layer_get_all (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_layer_change_level (DocumentInterface *doc, const char * IN_command, GError **error);

//static
gboolean
inkscape_layer_next (DocumentInterface *doc, GError **error);

//static
gboolean
inkscape_layer_previous (DocumentInterface *doc, GError **error);

G_END_DECLS

#endif // INKSCAPE_EXTENSION_DOCUMENT_INTERFACE_H_

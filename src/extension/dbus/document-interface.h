/*
 * This is where the implementation of the DBus based document API lives.
 * All the methods in here (except in the helper section) are 
 * designed to be called remotly via DBus. application-interface.cpp
 * has the methods used to connect to the bus and get a document instance.
 *
 * Documentation for these methods is in document-interface.xml
 * which is the "gold standard" as to how the interface should work.
 *
 * Authors:
 *   Soren Berg <Glimmer07@gmail.com>
 *
 * Copyright (C) 2009 Soren Berg
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
 
#ifndef INKSCAPE_EXTENSION_DOCUMENT_INTERFACE_H_
#define INKSCAPE_EXTENSION_DOCUMENT_INTERFACE_H_

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <dbus/dbus-glib-lowlevel.h>
#include "desktop.h"

#define DBUS_DOCUMENT_INTERFACE_PATH  "/org/inkscape/document"
        
#define TYPE_DOCUMENT_INTERFACE            (document_interface_get_type ())
#define DOCUMENT_INTERFACE(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_DOCUMENT_INTERFACE, DocumentInterface))
#define DOCUMENT_INTERFACE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DOCUMENT_INTERFACE, DocumentInterfaceClass))
#define IS_DOCUMENT_INTERFACE(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_DOCUMENT_INTERFACE))
#define IS_DOCUMENT_INTERFACE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DOCUMENT_INTERFACE))
#define DOCUMENT_INTERFACE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DOCUMENT_INTERFACE, DocumentInterfaceClass))

G_BEGIN_DECLS

typedef struct _DocumentInterface DocumentInterface;
typedef struct _DocumentInterfaceClass DocumentInterfaceClass;

struct _DocumentInterface {
        GObject parent;
	SPDesktop *desk;
    gboolean updates;
};

struct _DocumentInterfaceClass {
        GObjectClass parent;
};

typedef enum
{
  INKSCAPE_ERROR_SELECTION,
  INKSCAPE_ERROR_OBJECT,
  INKSCAPE_ERROR_VERB,
  INKSCAPE_ERROR_OTHER
} InkscapeError;

#define INKSCAPE_ERROR (inkscape_error_quark ())
#define INKSCAPE_TYPE_ERROR (inkscape_error_get_type ()) 

GQuark inkscape_error_quark (void);
GType inkscape_error_get_type (void);

struct DBUSPoint {
    int x;
    int y;
};
/****************************************************************************
     MISC FUNCTIONS
****************************************************************************/

gboolean 
document_interface_delete_all (DocumentInterface *object, GError **error);

gboolean
document_interface_call_verb (DocumentInterface *object, 
                              gchar *verbid, GError **error);

/****************************************************************************
     CREATION FUNCTIONS
****************************************************************************/

gchar* 
document_interface_rectangle (DocumentInterface *object, int x, int y, 
                              int width, int height, GError **error);

gchar* 
document_interface_ellipse (DocumentInterface *object, int x, int y, 
                              int width, int height, GError **error);

gchar* 
document_interface_polygon (DocumentInterface *object, int cx, int cy, 
                            int radius, int rotation, int sides, 
                            GError **error);

gchar* 
document_interface_star (DocumentInterface *object, int cx, int cy, 
                         int r1, int r2, int sides, gdouble rounded,
                         gdouble arg1, gdouble arg2, GError **error);

gchar* 
document_interface_spiral (DocumentInterface *object, int cx, int cy, 
                                   int r, int revolutions, GError **error);

gchar* 
document_interface_line (DocumentInterface *object, int x, int y, 
                              int x2, int y2, GError **error);

gboolean
document_interface_text (DocumentInterface *object, int x, int y, 
                         gchar *text, GError **error);
                         
gchar *
document_interface_image (DocumentInterface *object, int x, int y, 
                          gchar *filename, GError **error);

gchar* 
document_interface_node (DocumentInterface *object, gchar *svgtype, 
                             GError **error);


/****************************************************************************
     ENVIORNMENT FUNCTIONS
****************************************************************************/
gdouble
document_interface_document_get_width (DocumentInterface *object);

gdouble
document_interface_document_get_height (DocumentInterface *object);

gchar *
document_interface_document_get_css (DocumentInterface *object, GError **error);

gboolean 
document_interface_document_merge_css (DocumentInterface *object,
                                       gchar *stylestring, GError **error);

gboolean 
document_interface_document_set_css (DocumentInterface *object,
                                     gchar *stylestring, GError **error);

gboolean 
document_interface_document_resize_to_fit_selection (DocumentInterface *object,
                                                     GError **error);

/****************************************************************************
     OBJECT FUNCTIONS
****************************************************************************/

gboolean
document_interface_set_attribute (DocumentInterface *object, 
                                  char *shape, char *attribute, 
                                  char *newval, GError **error);

gboolean
document_interface_set_int_attribute (DocumentInterface *object, 
                                      char *shape, char *attribute, 
                                      int newval, GError **error);

gboolean
document_interface_set_double_attribute (DocumentInterface *object, 
                                         char *shape, char *attribute, 
                                         double newval, GError **error);

gchar * 
document_interface_get_attribute (DocumentInterface *object, 
                                  char *shape, char *attribute, GError **error);

gboolean 
document_interface_move (DocumentInterface *object, gchar *name, 
                         gdouble x, gdouble y, GError **error);

gboolean 
document_interface_move_to (DocumentInterface *object, gchar *name, 
                            gdouble x, gdouble y, GError **error);

gboolean
document_interface_object_to_path (DocumentInterface *object, 
                                   char *shape, GError **error);

gchar *
document_interface_get_path (DocumentInterface *object, 
                             char *pathname, GError **error);

gboolean 
document_interface_transform (DocumentInterface *object, gchar *shape,
                              gchar *transformstr, GError **error);

gchar *
document_interface_get_css (DocumentInterface *object, gchar *shape,
                            GError **error);

gboolean 
document_interface_modify_css (DocumentInterface *object, gchar *shape,
                               gchar *cssattrb, gchar *newval, GError **error);

gboolean 
document_interface_merge_css (DocumentInterface *object, gchar *shape,
                               gchar *stylestring, GError **error);

gboolean 
document_interface_set_color (DocumentInterface *object, gchar *shape,
                              int r, int g, int b, gboolean fill, GError **error);

gboolean 
document_interface_move_to_layer (DocumentInterface *object, gchar *shape, 
                              gchar *layerstr, GError **error);


GArray *
document_interface_get_node_coordinates (DocumentInterface *object, gchar *shape);

/****************************************************************************
     FILE I/O FUNCTIONS
****************************************************************************/

gboolean 
document_interface_save (DocumentInterface *object, GError **error);

gboolean 
document_interface_load (DocumentInterface *object, 
                        gchar *filename, GError **error);

gboolean 
document_interface_save_as (DocumentInterface *object, 
                           gchar *filename, GError **error);

gboolean 
document_interface_mark_as_unmodified (DocumentInterface *object, GError **error);
/*
gboolean 
document_interface_print_to_file (DocumentInterface *object, GError **error);
*/

/****************************************************************************
     PROGRAM CONTROL FUNCTIONS
****************************************************************************/

gboolean
document_interface_close (DocumentInterface *object, GError **error);

gboolean
document_interface_exit (DocumentInterface *object, GError **error);

gboolean
document_interface_undo (DocumentInterface *object, GError **error);

gboolean
document_interface_redo (DocumentInterface *object, GError **error);


/****************************************************************************
     UPDATE FUNCTIONS
****************************************************************************/
void
document_interface_pause_updates (DocumentInterface *object, GError **error);

void
document_interface_resume_updates (DocumentInterface *object, GError **error);

void
document_interface_update (DocumentInterface *object, GError **error);

/****************************************************************************
     SELECTION FUNCTIONS
****************************************************************************/
gboolean
document_interface_selection_get (DocumentInterface *object, char ***out, GError **error);

gboolean
document_interface_selection_add (DocumentInterface *object, 
                                  char *name, GError **error);

gboolean
document_interface_selection_add_list (DocumentInterface *object, 
                                       char **names, GError **error);

gboolean
document_interface_selection_set (DocumentInterface *object, 
                                  char *name, GError **error);

gboolean
document_interface_selection_set_list (DocumentInterface *object, 
                                       gchar **names, GError **error);

gboolean
document_interface_selection_rotate (DocumentInterface *object, 
                                     int angle, GError **error);

gboolean
document_interface_selection_delete(DocumentInterface *object, GError **error);

gboolean
document_interface_selection_clear(DocumentInterface *object, GError **error);

gboolean
document_interface_select_all(DocumentInterface *object, GError **error);

gboolean
document_interface_select_all_in_all_layers(DocumentInterface *object, 
                                            GError **error);

gboolean
document_interface_selection_box (DocumentInterface *object, int x, int y,
                                  int x2, int y2, gboolean replace, 
                                  GError **error);

gboolean
document_interface_selection_invert (DocumentInterface *object, GError **error);

gboolean
document_interface_selection_group(DocumentInterface *object, GError **error);

gboolean
document_interface_selection_ungroup(DocumentInterface *object, GError **error);

gboolean
document_interface_selection_cut(DocumentInterface *object, GError **error);

gboolean
document_interface_selection_copy(DocumentInterface *object, GError **error);

gboolean
document_interface_selection_paste(DocumentInterface *object, GError **error);

gboolean
document_interface_selection_scale (DocumentInterface *object, 
                                    gdouble grow, GError **error);

gboolean
document_interface_selection_move (DocumentInterface *object, gdouble x, 
                                   gdouble y, GError **error);

gboolean
document_interface_selection_move_to (DocumentInterface *object, gdouble x, 
                                      gdouble y, GError **error);

gboolean 
document_interface_selection_move_to_layer (DocumentInterface *object,
                                            gchar *layerstr, GError **error);

GArray * 
document_interface_selection_get_center (DocumentInterface *object);

gboolean 
document_interface_selection_to_path (DocumentInterface *object, GError **error);

gchar *
document_interface_selection_combine (DocumentInterface *object, gchar *cmd,
                                      GError **error);

gboolean
document_interface_selection_divide (DocumentInterface *object, 
                                     char ***out, GError **error);


gboolean
document_interface_selection_change_level (DocumentInterface *object, gchar *cmd,
                                      GError **error);

/****************************************************************************
     LAYER FUNCTIONS
****************************************************************************/

gchar *
document_interface_layer_new (DocumentInterface *object, GError **error);

gboolean 
document_interface_layer_set (DocumentInterface *object,
                              gchar *layerstr, GError **error);

gchar **
document_interface_layer_get_all (DocumentInterface *object);

gboolean 
document_interface_layer_change_level (DocumentInterface *object,
                                       gchar *cmd, GError **error);

gboolean 
document_interface_layer_next (DocumentInterface *object, GError **error);

gboolean 
document_interface_layer_previous (DocumentInterface *object, GError **error);








DocumentInterface *document_interface_new (void);
GType document_interface_get_type (void);


G_END_DECLS

#endif // INKSCAPE_EXTENSION_DOCUMENT_INTERFACE_H_

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

// this is reguired so that giomm headers won't barf
#undef DBUS_MESSAGE_TYPE_INVALID
#undef DBUS_MESSAGE_TYPE_METHOD_CALL
#undef DBUS_MESSAGE_TYPE_METHOD_RETURN
#undef DBUS_MESSAGE_TYPE_ERROR
#undef DBUS_MESSAGE_TYPE_SIGNAL

#include "helper/action-context.h"

class SPDesktop;
class SPItem;
        
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
    Inkscape::ActionContext target; ///< stores information about which document, selection, desktop etc this interface is linked to
    gboolean updates;
};

struct _DocumentInterfaceClass {
        GObjectClass parent;
};



struct DBUSPoint {
    int x;
    int y;
};
/****************************************************************************
     MISC FUNCTIONS
****************************************************************************/

gboolean 
document_interface_delete_all (DocumentInterface *doc_interface, GError **error);

gboolean
document_interface_call_verb (DocumentInterface *doc_interface, 
                              gchar *verbid, GError **error);

/****************************************************************************
     CREATION FUNCTIONS
****************************************************************************/

gchar* 
document_interface_rectangle (DocumentInterface *doc_interface, int x, int y, 
                              int width, int height, GError **error);

gchar* 
document_interface_ellipse (DocumentInterface *doc_interface, int x, int y, 
                              int width, int height, GError **error);

gchar* 
document_interface_polygon (DocumentInterface *doc_interface, int cx, int cy, 
                            int radius, int rotation, int sides, 
                            GError **error);

gchar* 
document_interface_star (DocumentInterface *doc_interface, int cx, int cy, 
                         int r1, int r2, int sides, gdouble rounded,
                         gdouble arg1, gdouble arg2, GError **error);

gchar* 
document_interface_spiral (DocumentInterface *doc_interface, int cx, int cy, 
                                   int r, int revolutions, GError **error);

gchar* 
document_interface_line (DocumentInterface *doc_interface, int x, int y, 
                              int x2, int y2, GError **error);

gchar* 
document_interface_text (DocumentInterface *doc_interface, int x, int y, 
                         gchar *text, GError **error);
gboolean
document_interface_set_text (DocumentInterface *doc_interface, gchar *name,
                             gchar *text, GError **error);
gboolean
document_interface_text_apply_style (DocumentInterface *doc_interface, gchar *name,
                                     int start_pos, int end_pos,  gchar *style, gchar *styleval,
                                     GError **error);

gchar *
document_interface_image (DocumentInterface *doc_interface, int x, int y, 
                          gchar *filename, GError **error);

gchar* 
document_interface_node (DocumentInterface *doc_interface, gchar *svgtype, 
                             GError **error);


/****************************************************************************
     ENVIORNMENT FUNCTIONS
****************************************************************************/
gdouble
document_interface_document_get_width (DocumentInterface *doc_interface);

gdouble
document_interface_document_get_height (DocumentInterface *doc_interface);

gchar *
document_interface_document_get_css (DocumentInterface *doc_interface, GError **error);

gboolean 
document_interface_document_merge_css (DocumentInterface *doc_interface,
                                       gchar *stylestring, GError **error);

gboolean 
document_interface_document_set_css (DocumentInterface *doc_interface,
                                     gchar *stylestring, GError **error);

gboolean 
document_interface_document_resize_to_fit_selection (DocumentInterface *doc_interface,
                                                     GError **error);
gboolean
document_interface_document_set_display_area (DocumentInterface *doc_interface,
                                              double x0,
                                              double y0,
                                              double x1,
                                              double y1,
                                              double border,
                                              GError **error);
GArray *
document_interface_document_get_display_area (DocumentInterface *doc_interface);

/****************************************************************************
     OBJECT FUNCTIONS
****************************************************************************/

gboolean
document_interface_set_attribute (DocumentInterface *doc_interface, 
                                  char *shape, char *attribute, 
                                  char *newval, GError **error);

gboolean
document_interface_set_int_attribute (DocumentInterface *doc_interface, 
                                      char *shape, char *attribute, 
                                      int newval, GError **error);

gboolean
document_interface_set_double_attribute (DocumentInterface *doc_interface, 
                                         char *shape, char *attribute, 
                                         double newval, GError **error);

gchar * 
document_interface_get_attribute (DocumentInterface *doc_interface, 
                                  char *shape, char *attribute, GError **error);

gboolean 
document_interface_move (DocumentInterface *doc_interface, gchar *name, 
                         gdouble x, gdouble y, GError **error);

gboolean 
document_interface_move_to (DocumentInterface *doc_interface, gchar *name, 
                            gdouble x, gdouble y, GError **error);

gboolean
document_interface_object_to_path (DocumentInterface *doc_interface, 
                                   char *shape, GError **error);

gchar *
document_interface_get_path (DocumentInterface *doc_interface, 
                             char *pathname, GError **error);

gboolean 
document_interface_transform (DocumentInterface *doc_interface, gchar *shape,
                              gchar *transformstr, GError **error);

gchar *
document_interface_get_css (DocumentInterface *doc_interface, gchar *shape,
                            GError **error);

gboolean 
document_interface_modify_css (DocumentInterface *doc_interface, gchar *shape,
                               gchar *cssattrb, gchar *newval, GError **error);

gboolean 
document_interface_merge_css (DocumentInterface *doc_interface, gchar *shape,
                               gchar *stylestring, GError **error);

gboolean 
document_interface_set_color (DocumentInterface *doc_interface, gchar *shape,
                              int r, int g, int b, gboolean fill, GError **error);

gboolean 
document_interface_move_to_layer (DocumentInterface *doc_interface, gchar *shape, 
                              gchar *layerstr, GError **error);


GArray *
document_interface_get_node_coordinates (DocumentInterface *doc_interface, gchar *shape);

/****************************************************************************
     FILE I/O FUNCTIONS
****************************************************************************/

gboolean 
document_interface_save (DocumentInterface *doc_interface, GError **error);

gboolean 
document_interface_load (DocumentInterface *doc_interface, 
                        gchar *filename, GError **error);

gboolean 
document_interface_save_as (DocumentInterface *doc_interface, 
                           const gchar *filename, GError **error);

gboolean 
document_interface_mark_as_unmodified (DocumentInterface *doc_interface, GError **error);
/*
gboolean 
document_interface_print_to_file (DocumentInterface *doc_interface, GError **error);
*/

/****************************************************************************
     PROGRAM CONTROL FUNCTIONS
****************************************************************************/

gboolean
document_interface_close (DocumentInterface *doc_interface, GError **error);

gboolean
document_interface_exit (DocumentInterface *doc_interface, GError **error);

gboolean
document_interface_undo (DocumentInterface *doc_interface, GError **error);

gboolean
document_interface_redo (DocumentInterface *doc_interface, GError **error);


/****************************************************************************
     UPDATE FUNCTIONS
****************************************************************************/
void
document_interface_pause_updates (DocumentInterface *doc_interface, GError **error);

void
document_interface_resume_updates (DocumentInterface *doc_interface, GError **error);

void
document_interface_update (DocumentInterface *doc_interface, GError **error);

/****************************************************************************
     SELECTION FUNCTIONS
****************************************************************************/
gboolean
document_interface_selection_get (DocumentInterface *doc_interface, char ***out, GError **error);

gboolean
document_interface_selection_add (DocumentInterface *doc_interface, 
                                  char *name, GError **error);

gboolean
document_interface_selection_add_list (DocumentInterface *doc_interface, 
                                       char **names, GError **error);

gboolean
document_interface_selection_set (DocumentInterface *doc_interface, 
                                  char *name, GError **error);

gboolean
document_interface_selection_set_list (DocumentInterface *doc_interface, 
                                       gchar **names, GError **error);

gboolean
document_interface_selection_rotate (DocumentInterface *doc_interface, 
                                     int angle, GError **error);

gboolean
document_interface_selection_delete(DocumentInterface *doc_interface, GError **error);

gboolean
document_interface_selection_clear(DocumentInterface *doc_interface, GError **error);

gboolean
document_interface_select_all(DocumentInterface *doc_interface, GError **error);

gboolean
document_interface_select_all_in_all_layers(DocumentInterface *doc_interface, 
                                            GError **error);

gboolean
document_interface_selection_box (DocumentInterface *doc_interface, int x, int y,
                                  int x2, int y2, gboolean replace, 
                                  GError **error);

gboolean
document_interface_selection_invert (DocumentInterface *doc_interface, GError **error);

gboolean
document_interface_selection_group(DocumentInterface *doc_interface, GError **error);

gboolean
document_interface_selection_ungroup(DocumentInterface *doc_interface, GError **error);

gboolean
document_interface_selection_cut(DocumentInterface *doc_interface, GError **error);

gboolean
document_interface_selection_copy(DocumentInterface *doc_interface, GError **error);

gboolean
document_interface_selection_paste(DocumentInterface *doc_interface, GError **error);

gboolean
document_interface_selection_scale (DocumentInterface *doc_interface, 
                                    gdouble grow, GError **error);

gboolean
document_interface_selection_move (DocumentInterface *doc_interface, gdouble x, 
                                   gdouble y, GError **error);

gboolean
document_interface_selection_move_to (DocumentInterface *doc_interface, gdouble x, 
                                      gdouble y, GError **error);

gboolean 
document_interface_selection_move_to_layer (DocumentInterface *doc_interface,
                                            gchar *layerstr, GError **error);

GArray * 
document_interface_selection_get_center (DocumentInterface *doc_interface);

gboolean 
document_interface_selection_to_path (DocumentInterface *doc_interface, GError **error);

gboolean
document_interface_selection_combine (DocumentInterface *doc_interface, gchar *cmd, char ***newpaths,
                                      GError **error);

gboolean
document_interface_selection_change_level (DocumentInterface *doc_interface, gchar *cmd,
                                      GError **error);

/****************************************************************************
     LAYER FUNCTIONS
****************************************************************************/

gchar *
document_interface_layer_new (DocumentInterface *doc_interface, GError **error);

gboolean 
document_interface_layer_set (DocumentInterface *doc_interface,
                              gchar *layerstr, GError **error);

gchar **
document_interface_layer_get_all (DocumentInterface *doc_interface);

gboolean 
document_interface_layer_change_level (DocumentInterface *doc_interface,
                                       gchar *cmd, GError **error);

gboolean 
document_interface_layer_next (DocumentInterface *doc_interface, GError **error);

gboolean 
document_interface_layer_previous (DocumentInterface *doc_interface, GError **error);








DocumentInterface *document_interface_new (void);
GType document_interface_get_type (void);

extern DocumentInterface *fugly;
gboolean dbus_send_ping (SPDesktop* desk,     SPItem *item);

gboolean
document_interface_get_children (DocumentInterface *doc_interface,  char *name, char ***out, GError **error);

gchar* 
document_interface_get_parent (DocumentInterface *doc_interface,  char *name, GError **error);

gchar*
document_interface_import (DocumentInterface *doc_interface, 
                           gchar *filename, GError **error);

G_END_DECLS

#endif // INKSCAPE_EXTENSION_DOCUMENT_INTERFACE_H_

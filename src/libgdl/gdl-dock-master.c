/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- 
 *
 * gdl-dock-master.c - Object which manages a dock ring
 *
 * This file is part of the GNOME Devtools Libraries.
 *
 * Copyright (C) 2002 Gustavo Giráldez <gustavo.giraldez@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gdl-i18n.h"

#include "gdl-dock-master.h"
#include "gdl-dock.h"
#include "gdl-dock-item.h"
#include "gdl-dock-notebook.h"
#include "gdl-switcher.h"
#include "libgdlmarshal.h"
#include "libgdltypebuiltins.h"
#ifdef WIN32
#include "gdl-win32.h"
#endif

/* ----- Private prototypes ----- */

static void     gdl_dock_master_class_init    (GdlDockMasterClass *klass);

static void     gdl_dock_master_dispose       (GObject            *g_object);
static void     gdl_dock_master_set_property  (GObject            *object,
                                               guint               prop_id,
                                               const GValue       *value,
                                               GParamSpec         *pspec);
static void     gdl_dock_master_get_property  (GObject            *object,
                                               guint               prop_id,
                                               GValue             *value,
                                               GParamSpec         *pspec);

static void     _gdl_dock_master_remove       (GdlDockObject      *object,
                                               GdlDockMaster      *master);

static void     gdl_dock_master_drag_begin    (GdlDockItem        *item, 
                                               gpointer            data);
static void     gdl_dock_master_drag_end      (GdlDockItem        *item,
                                               gboolean            cancelled,
                                               gpointer            data);
static void     gdl_dock_master_drag_motion   (GdlDockItem        *item, 
                                               gint                x, 
                                               gint                y,
                                               gpointer            data);

static void     _gdl_dock_master_foreach      (gpointer            key,
                                               gpointer            value,
                                               gpointer            user_data);

static void     gdl_dock_master_xor_rect      (GdlDockMaster      *master);

static void     gdl_dock_master_layout_changed (GdlDockMaster     *master);

static void gdl_dock_master_set_switcher_style (GdlDockMaster *master,
                                                GdlSwitcherStyle switcher_style);

/* ----- Private data types and variables ----- */

enum {
    PROP_0,
    PROP_DEFAULT_TITLE,
    PROP_LOCKED,
    PROP_SWITCHER_STYLE
};

enum {
    LAYOUT_CHANGED,
    LAST_SIGNAL
};

struct _GdlDockMasterPrivate {
    gint            number;             /* for naming nameless manual objects */
    gchar          *default_title;
    
    GdkGC          *root_xor_gc;
    gboolean        rect_drawn;
    GdlDock        *rect_owner;
    
    GdlDockRequest *drag_request;

    /* source id for the idle handler to emit a layout_changed signal */
    guint           idle_layout_changed_id;

    /* hashes to quickly calculate the overall locked status: i.e.
     * if size(unlocked_items) == 0 then locked = 1
     * else if size(locked_items) == 0 then locked = 0
     * else locked = -1
     */
    GHashTable     *locked_items;
    GHashTable     *unlocked_items;
    
    GdlSwitcherStyle switcher_style;
};

#define COMPUTE_LOCKED(master)                                          \
    (g_hash_table_size ((master)->_priv->unlocked_items) == 0 ? 1 :     \
     (g_hash_table_size ((master)->_priv->locked_items) == 0 ? 0 : -1))

static guint master_signals [LAST_SIGNAL] = { 0 };


/* ----- Private interface ----- */

G_DEFINE_TYPE (GdlDockMaster, gdl_dock_master, G_TYPE_OBJECT);

static void
gdl_dock_master_class_init (GdlDockMasterClass *klass)
{
    GObjectClass      *g_object_class;

    g_object_class = G_OBJECT_CLASS (klass);

    g_object_class->dispose = gdl_dock_master_dispose;
    g_object_class->set_property = gdl_dock_master_set_property;
    g_object_class->get_property = gdl_dock_master_get_property;

    g_object_class_install_property (
        g_object_class, PROP_DEFAULT_TITLE,
        g_param_spec_string ("default-title", _("Default title"),
                             _("Default title for newly created floating docks"),
                             NULL,
                             G_PARAM_READWRITE));
    
    g_object_class_install_property (
        g_object_class, PROP_LOCKED,
        g_param_spec_int ("locked", _("Locked"),
                          _("If is set to 1, all the dock items bound to the master "
                            "are locked; if it's 0, all are unlocked; -1 indicates "
                            "inconsistency among the items"),
                          -1, 1, 0,
                          G_PARAM_READWRITE));

    g_object_class_install_property (
        g_object_class, PROP_SWITCHER_STYLE,
        g_param_spec_enum ("switcher-style", _("Switcher Style"),
                           _("Switcher buttons style"),
                           GDL_TYPE_SWITCHER_STYLE,
                           GDL_SWITCHER_STYLE_BOTH,
                           G_PARAM_READWRITE));

    master_signals [LAYOUT_CHANGED] = 
        g_signal_new ("layout-changed", 
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GdlDockMasterClass, layout_changed),
                      NULL, /* accumulator */
                      NULL, /* accu_data */
                      gdl_marshal_VOID__VOID,
                      G_TYPE_NONE, /* return type */
                      0);

    klass->layout_changed = gdl_dock_master_layout_changed;
}

static void
gdl_dock_master_init (GdlDockMaster *master)
{
    master->dock_objects = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                  g_free, NULL);
    master->toplevel_docks = NULL;
    master->controller = NULL;
    master->dock_number = 1;
    
    master->_priv = g_new0 (GdlDockMasterPrivate, 1);
    master->_priv->number = 1;
    master->_priv->switcher_style = GDL_SWITCHER_STYLE_BOTH;
    master->_priv->locked_items = g_hash_table_new (g_direct_hash, g_direct_equal);
    master->_priv->unlocked_items = g_hash_table_new (g_direct_hash, g_direct_equal);
}

static void
_gdl_dock_master_remove (GdlDockObject *object,
                         GdlDockMaster *master)
{
    g_return_if_fail (master != NULL && object != NULL);

    if (GDL_IS_DOCK (object)) {
        GList *found_link;

        found_link = g_list_find (master->toplevel_docks, object);
        if (found_link)
            master->toplevel_docks = g_list_delete_link (master->toplevel_docks,
                                                         found_link);
        if (object == master->controller) {
            GList *last;
            GdlDockObject *new_controller = NULL;
            
            /* now find some other non-automatic toplevel to use as a
               new controller.  start from the last dock, since it's
               probably a non-floating and manual */
            last = g_list_last (master->toplevel_docks);
            while (last) {
                if (!GDL_DOCK_OBJECT_AUTOMATIC (last->data)) {
                    new_controller = GDL_DOCK_OBJECT (last->data);
                    break;
                }
                last = last->prev;
            };

            if (new_controller) {
                /* the new controller gets the ref (implicitly of course) */
                master->controller = new_controller;
            } else {
                master->controller = NULL;
                /* no controller, no master */
                g_object_unref (master);
            }
        }
    }
    /* disconnect dock object signals */
    g_signal_handlers_disconnect_matched (object, G_SIGNAL_MATCH_DATA, 
                                          0, 0, NULL, NULL, master);

    /* unref the object from the hash if it's there */
    if (object->name) {
        GdlDockObject *found_object;
        found_object = g_hash_table_lookup (master->dock_objects, object->name);
        if (found_object == object) {
            g_hash_table_remove (master->dock_objects, object->name);
            g_object_unref (object);
        }
    }
}

static void
ht_foreach_build_slist (gpointer  key,
                        gpointer  value,
                        GSList  **slist)
{
    *slist = g_slist_prepend (*slist, value);
}

static void
gdl_dock_master_dispose (GObject *g_object)
{
    GdlDockMaster *master;
    
    g_return_if_fail (GDL_IS_DOCK_MASTER (g_object));

    master = GDL_DOCK_MASTER (g_object);

    if (master->toplevel_docks) {
        g_list_foreach (master->toplevel_docks,
                        (GFunc) gdl_dock_object_unbind, NULL);
        g_list_free (master->toplevel_docks);
        master->toplevel_docks = NULL;
    }
    
    if (master->dock_objects) {
        GSList *alive_docks = NULL;
        g_hash_table_foreach (master->dock_objects,
                              (GHFunc) ht_foreach_build_slist, &alive_docks);
        while (alive_docks) {
            gdl_dock_object_unbind (GDL_DOCK_OBJECT (alive_docks->data));
            alive_docks = g_slist_delete_link (alive_docks, alive_docks);
        }
        
        g_hash_table_destroy (master->dock_objects);
        master->dock_objects = NULL;
    }
    
    if (master->_priv) {
        if (master->_priv->idle_layout_changed_id)
            g_source_remove (master->_priv->idle_layout_changed_id);
        
        if (master->_priv->root_xor_gc) {
            g_object_unref (master->_priv->root_xor_gc);
            master->_priv->root_xor_gc = NULL;
        }
        if (master->_priv->drag_request) {
            if (G_IS_VALUE (&master->_priv->drag_request->extra))
                g_value_unset (&master->_priv->drag_request->extra);
            g_free (master->_priv->drag_request);
            master->_priv->drag_request = NULL;
        }
        g_free (master->_priv->default_title);
        master->_priv->default_title = NULL;

        g_hash_table_destroy (master->_priv->locked_items);
        master->_priv->locked_items = NULL;
        g_hash_table_destroy (master->_priv->unlocked_items);
        master->_priv->unlocked_items = NULL;
        
        g_free (master->_priv);
        master->_priv = NULL;
    }

    G_OBJECT_CLASS (gdl_dock_master_parent_class)->dispose (g_object);
}

static void 
foreach_lock_unlock (GdlDockItem *item,
                     gboolean     locked)
{
    if (!GDL_IS_DOCK_ITEM (item))
        return;
    
    g_object_set (item, "locked", locked, NULL);
    if (gdl_dock_object_is_compound (GDL_DOCK_OBJECT (item)))
        gtk_container_foreach (GTK_CONTAINER (item),
                               (GtkCallback) foreach_lock_unlock,
                               GINT_TO_POINTER (locked));
}

static void
gdl_dock_master_lock_unlock (GdlDockMaster *master,
                             gboolean       locked)
{
    GList *l;
    
    for (l = master->toplevel_docks; l; l = l->next) {
        GdlDock *dock = GDL_DOCK (l->data);
        if (dock->root)
            foreach_lock_unlock (GDL_DOCK_ITEM (dock->root), locked);
    }

    /* just to be sure hidden items are set too */
    gdl_dock_master_foreach (master,
                             (GFunc) foreach_lock_unlock,
                             GINT_TO_POINTER (locked));
}

static void
gdl_dock_master_set_property  (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
    GdlDockMaster *master = GDL_DOCK_MASTER (object);

    switch (prop_id) {
        case PROP_DEFAULT_TITLE:
            g_free (master->_priv->default_title);
            master->_priv->default_title = g_value_dup_string (value);
            break;
        case PROP_LOCKED:
            if (g_value_get_int (value) >= 0)
                gdl_dock_master_lock_unlock (master, (g_value_get_int (value) > 0));
            break;
        case PROP_SWITCHER_STYLE:
            gdl_dock_master_set_switcher_style (master, g_value_get_enum (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
gdl_dock_master_get_property  (GObject      *object,
                               guint         prop_id,
                               GValue       *value,
                               GParamSpec   *pspec)
{
    GdlDockMaster *master = GDL_DOCK_MASTER (object);

    switch (prop_id) {
        case PROP_DEFAULT_TITLE:
            g_value_set_string (value, master->_priv->default_title);
            break;
        case PROP_LOCKED:
            g_value_set_int (value, COMPUTE_LOCKED (master));
            break;
        case PROP_SWITCHER_STYLE:
            g_value_set_enum (value, master->_priv->switcher_style);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
gdl_dock_master_drag_begin (GdlDockItem *item,
                            gpointer     data)
{
    GdlDockMaster  *master;
    GdlDockRequest *request;
    
    g_return_if_fail (data != NULL);
    g_return_if_fail (item != NULL);

    master = GDL_DOCK_MASTER (data);

    if (!master->_priv->drag_request)
        master->_priv->drag_request = g_new0 (GdlDockRequest, 1);

    request = master->_priv->drag_request;
    
    /* Set the target to itself so it won't go floating with just a click. */
    request->applicant = GDL_DOCK_OBJECT (item);
    request->target = GDL_DOCK_OBJECT (item);
    request->position = GDL_DOCK_FLOATING;
    if (G_IS_VALUE (&request->extra))
        g_value_unset (&request->extra);

    master->_priv->rect_drawn = FALSE;
    master->_priv->rect_owner = NULL;
}

static void
gdl_dock_master_drag_end (GdlDockItem *item, 
                          gboolean     cancelled,
                          gpointer     data)
{
    GdlDockMaster  *master;
    GdlDockRequest *request;
    
    g_return_if_fail (data != NULL);
    g_return_if_fail (item != NULL);

    master = GDL_DOCK_MASTER (data);
    request = master->_priv->drag_request;
    
    g_return_if_fail (GDL_DOCK_OBJECT (item) == request->applicant);
    
    /* Erase previously drawn rectangle */
    if (master->_priv->rect_drawn)
        gdl_dock_master_xor_rect (master);
    
    /* cancel conditions */
    if (cancelled || request->applicant == request->target)
        return;
    
    /* dock object to the requested position */
    gdl_dock_object_dock (request->target,
                          request->applicant,
                          request->position,
                          &request->extra);
    
    g_signal_emit (master, master_signals [LAYOUT_CHANGED], 0);
}

static void
gdl_dock_master_drag_motion (GdlDockItem *item, 
                             gint         root_x, 
                             gint         root_y,
                             gpointer     data)
{
    GdlDockMaster  *master;
    GdlDockRequest  my_request, *request;
    GdkWindow      *window;
    GdkWindow      *widget_window;
    gint            win_x, win_y;
    gint            x, y;
    GdlDock        *dock = NULL;
    gboolean        may_dock = FALSE;
    
    g_return_if_fail (item != NULL && data != NULL);

    master = GDL_DOCK_MASTER (data);
    request = master->_priv->drag_request;

    g_return_if_fail (GDL_DOCK_OBJECT (item) == request->applicant);
    
    my_request = *request;

    /* first look under the pointer */
    window = gdk_window_at_pointer (&win_x, &win_y);
    if (window) {
        GtkWidget *widget;
        /* ok, now get the widget who owns that window and see if we can
           get to a GdlDock by walking up the hierarchy */
        gdk_window_get_user_data (window, (gpointer) &widget);
        if (GTK_IS_WIDGET (widget)) {
            while (widget && (!GDL_IS_DOCK (widget) || 
	           GDL_DOCK_OBJECT_GET_MASTER (widget) != master))
                widget = gtk_widget_get_parent (widget);
            if (widget) {
                gint win_w, win_h;
                
                widget_window = gtk_widget_get_window (widget);

                /* verify that the pointer is still in that dock
                   (the user could have moved it) */
                gdk_window_get_geometry (widget_window,
                                         NULL, NULL, &win_w, &win_h, NULL);
                gdk_window_get_origin (widget_window, &win_x, &win_y);
                if (root_x >= win_x && root_x < win_x + win_w &&
                    root_y >= win_y && root_y < win_y + win_h)
                    dock = GDL_DOCK (widget);
            }
        }
    }

    if (dock) {
        GdkWindow *dock_window = gtk_widget_get_window (GTK_WIDGET (dock));

        /* translate root coordinates into dock object coordinates
           (i.e. widget coordinates) */
        gdk_window_get_origin (dock_window, &win_x, &win_y);
        x = root_x - win_x;
        y = root_y - win_y;
        may_dock = gdl_dock_object_dock_request (GDL_DOCK_OBJECT (dock),
                                                 x, y, &my_request);
    }
    else {
        GList *l;

        /* try to dock the item in all the docks in the ring in turn */
        for (l = master->toplevel_docks; l; l = l->next) {
            GdkWindow *dock_window;
            dock = GDL_DOCK (l->data);
            dock_window = gtk_widget_get_window (GTK_WIDGET (dock));
            /* translate root coordinates into dock object coordinates
               (i.e. widget coordinates) */
            gdk_window_get_origin (dock_window, &win_x, &win_y);
            x = root_x - win_x;
            y = root_y - win_y;
            may_dock = gdl_dock_object_dock_request (GDL_DOCK_OBJECT (dock),
                                                     x, y, &my_request);
            if (may_dock)
                break;
        }
    }

  
    if (!may_dock) {
        GtkRequisition req;
	/* Special case for GdlDockItems : they must respect the flags */
	if(GDL_IS_DOCK_ITEM(item)
	&& GDL_DOCK_ITEM(item)->behavior & GDL_DOCK_ITEM_BEH_NEVER_FLOATING)
	    return;

        dock = NULL;
        my_request.target = GDL_DOCK_OBJECT (
            gdl_dock_object_get_toplevel (request->applicant));
        my_request.position = GDL_DOCK_FLOATING;

        gdl_dock_item_preferred_size (GDL_DOCK_ITEM (request->applicant), &req);
        my_request.rect.width = req.width;
        my_request.rect.height = req.height;

        my_request.rect.x = root_x - GDL_DOCK_ITEM (request->applicant)->dragoff_x;
        my_request.rect.y = root_y - GDL_DOCK_ITEM (request->applicant)->dragoff_y;

        /* setup extra docking information */
        if (G_IS_VALUE (&my_request.extra))
            g_value_unset (&my_request.extra);

        g_value_init (&my_request.extra, GDK_TYPE_RECTANGLE);
        g_value_set_boxed (&my_request.extra, &my_request.rect);
    }
    /* if we want to enforce GDL_DOCK_ITEM_BEH_NEVER_FLOATING		*/
    /* the item must remain attached to the controller, otherwise	*/
    /* it could be inserted in another floating dock			*/
    /* so check for the flag at this moment				*/
    else if(GDL_IS_DOCK_ITEM(item)
	&& GDL_DOCK_ITEM(item)->behavior & GDL_DOCK_ITEM_BEH_NEVER_FLOATING
	&& dock != GDL_DOCK(master->controller))
	    return;

    if (!(my_request.rect.x == request->rect.x &&
          my_request.rect.y == request->rect.y &&
          my_request.rect.width == request->rect.width &&
          my_request.rect.height == request->rect.height &&
          dock == master->_priv->rect_owner)) {

        /* erase the previous rectangle */
        if (master->_priv->rect_drawn)
            gdl_dock_master_xor_rect (master);
    }

    /* set the new values */
    *request = my_request;
    master->_priv->rect_owner = dock;
    
    /* draw the previous rectangle */
    if (~master->_priv->rect_drawn)
        gdl_dock_master_xor_rect (master);
}

static void
_gdl_dock_master_foreach (gpointer key,
                          gpointer value,
                          gpointer user_data)
{
    struct {
        GFunc    function;
        gpointer user_data;
    } *data = user_data;

    (* data->function) (GTK_WIDGET (value), data->user_data);
}

static void
gdl_dock_master_xor_rect (GdlDockMaster *master)
{
    gint8         dash_list [2];
    GdkWindow    *window;
    GdkRectangle *rect;
    
    if (!master->_priv || !master->_priv->drag_request)
        return;
    
    master->_priv->rect_drawn = ~master->_priv->rect_drawn;
    
    if (master->_priv->rect_owner) {
        gdl_dock_xor_rect (master->_priv->rect_owner,
                           &master->_priv->drag_request->rect);
        return;
    }
    
    rect = &master->_priv->drag_request->rect;
    window = gdk_get_default_root_window ();

    if (!master->_priv->root_xor_gc) {
        GdkGCValues values;

        values.function = GDK_INVERT;
        values.subwindow_mode = GDK_INCLUDE_INFERIORS;
        master->_priv->root_xor_gc = gdk_gc_new_with_values (
            window, &values, GDK_GC_FUNCTION | GDK_GC_SUBWINDOW);
    };

#ifdef WIN32    
    GdkLineStyle lineStyle = GDK_LINE_ON_OFF_DASH;
    if (is_os_vista())
    {
        // On Vista the dash-line is increadibly slow to draw, it takes several minutes to draw the tracking lines
        // With GDK_LINE_SOLID it is parts of a second
        // No performance issue on WinXP
        lineStyle = GDK_LINE_SOLID;
    }
#else
    GdkLineStyle lineStyle = GDK_LINE_ON_OFF_DASH;
#endif
    gdk_gc_set_line_attributes (master->_priv->root_xor_gc, 1,
                                lineStyle,
                                GDK_CAP_NOT_LAST,
                                GDK_JOIN_BEVEL);
    
    dash_list[0] = 1;
    dash_list[1] = 1;
    gdk_gc_set_dashes (master->_priv->root_xor_gc, 1, dash_list, 2);

    gdk_draw_rectangle (window, master->_priv->root_xor_gc, 0, 
                        rect->x, rect->y,
                        rect->width, rect->height);

    gdk_gc_set_dashes (master->_priv->root_xor_gc, 0, dash_list, 2);

    gdk_draw_rectangle (window, master->_priv->root_xor_gc, 0, 
                        rect->x + 1, rect->y + 1,
                        rect->width - 2, rect->height - 2);
}

static void
gdl_dock_master_layout_changed (GdlDockMaster *master)
{
    g_return_if_fail (GDL_IS_DOCK_MASTER (master));

    /* emit "layout-changed" on the controller to notify the user who
     * normally shouldn't have access to us */
    if (master->controller)
        g_signal_emit_by_name (master->controller, "layout-changed");

    /* remove the idle handler if there is one */
    if (master->_priv->idle_layout_changed_id) {
        g_source_remove (master->_priv->idle_layout_changed_id);
        master->_priv->idle_layout_changed_id = 0;
    }
}

static gboolean
idle_emit_layout_changed (gpointer user_data)
{
    GdlDockMaster *master = user_data;

    g_return_val_if_fail (master && GDL_IS_DOCK_MASTER (master), FALSE);

    master->_priv->idle_layout_changed_id = 0;
    g_signal_emit (master, master_signals [LAYOUT_CHANGED], 0);
    
    return FALSE;
}

static void 
item_dock_cb (GdlDockObject    *object,
              GdlDockObject    *requestor,
              GdlDockPlacement  position,
              GValue           *other_data,
              gpointer          user_data)
{
    GdlDockMaster *master = user_data;
    
    g_return_if_fail (requestor && GDL_IS_DOCK_OBJECT (requestor));
    g_return_if_fail (master && GDL_IS_DOCK_MASTER (master));

    /* here we are in fact interested in the requestor, since it's
     * assumed that object will not change its visibility... for the
     * requestor, however, could mean that it's being shown */
    if (!GDL_DOCK_OBJECT_IN_REFLOW (requestor) &&
        !GDL_DOCK_OBJECT_AUTOMATIC (requestor)) {
        if (!master->_priv->idle_layout_changed_id)
            master->_priv->idle_layout_changed_id =
                g_idle_add (idle_emit_layout_changed, master);
    }
}

static void 
item_detach_cb (GdlDockObject *object,
                gboolean       recursive,
                gpointer       user_data)
{
    GdlDockMaster *master = user_data;
    
    g_return_if_fail (object && GDL_IS_DOCK_OBJECT (object));
    g_return_if_fail (master && GDL_IS_DOCK_MASTER (master));

    if (!GDL_DOCK_OBJECT_IN_REFLOW (object) &&
        !GDL_DOCK_OBJECT_AUTOMATIC (object)) {
        if (!master->_priv->idle_layout_changed_id)
            master->_priv->idle_layout_changed_id =
                g_idle_add (idle_emit_layout_changed, master);
    }
}

static void
item_notify_cb (GdlDockObject *object,
                GParamSpec    *pspec,
                gpointer       user_data)
{
    GdlDockMaster *master = user_data;
    gint locked = COMPUTE_LOCKED (master);
    gboolean item_locked;
    
    g_object_get (object, "locked", &item_locked, NULL);

    if (item_locked) {
        g_hash_table_remove (master->_priv->unlocked_items, object);
        g_hash_table_insert (master->_priv->locked_items, object, NULL);
    } else {
        g_hash_table_remove (master->_priv->locked_items, object);
        g_hash_table_insert (master->_priv->unlocked_items, object, NULL);
    }
    
    if (COMPUTE_LOCKED (master) != locked)
        g_object_notify (G_OBJECT (master), "locked");
}

/* ----- Public interface ----- */

void
gdl_dock_master_add (GdlDockMaster *master,
                     GdlDockObject *object)
{
    g_return_if_fail (master != NULL && object != NULL);

    if (!GDL_DOCK_OBJECT_AUTOMATIC (object)) {
        GdlDockObject *found_object;
        
        /* create a name for the object if it doesn't have one */
        if (!object->name)
            /* directly set the name, since it's a construction only
               property */
            object->name = g_strdup_printf ("__dock_%u", master->_priv->number++);
        
        /* add the object to our hash list */
        if ((found_object = g_hash_table_lookup (master->dock_objects, object->name))) {
            g_warning (_("master %p: unable to add object %p[%s] to the hash.  "
                         "There already is an item with that name (%p)."),
                       master, object, object->name, found_object);
        }
        else {
            g_object_ref_sink (object);
            g_hash_table_insert (master->dock_objects, g_strdup (object->name), object);
        }
    }
    
    if (GDL_IS_DOCK (object)) {
        gboolean floating;
        
        /* if this is the first toplevel we are adding, name it controller */
        if (!master->toplevel_docks)
            /* the dock should already have the ref */
            master->controller = object;
        
        /* add dock to the toplevel list */
        g_object_get (object, "floating", &floating, NULL);
        if (floating)
            master->toplevel_docks = g_list_prepend (master->toplevel_docks, object);
        else
            master->toplevel_docks = g_list_append (master->toplevel_docks, object);

        /* we are interested in the dock request this toplevel
         * receives to update the layout */
        g_signal_connect (object, "dock",
                          G_CALLBACK (item_dock_cb), master);

    }
    else if (GDL_IS_DOCK_ITEM (object)) {
        /* we need to connect the item's signals */
        g_signal_connect (object, "dock_drag_begin",
                          G_CALLBACK (gdl_dock_master_drag_begin), master);
        g_signal_connect (object, "dock_drag_motion",
                          G_CALLBACK (gdl_dock_master_drag_motion), master);
        g_signal_connect (object, "dock_drag_end",
                          G_CALLBACK (gdl_dock_master_drag_end), master);
        g_signal_connect (object, "dock",
                          G_CALLBACK (item_dock_cb), master);
        g_signal_connect (object, "detach",
                          G_CALLBACK (item_detach_cb), master);

        /* register to "locked" notification if the item has a grip,
         * and add the item to the corresponding hash */
        if (GDL_DOCK_ITEM_HAS_GRIP (GDL_DOCK_ITEM (object))) {
            g_signal_connect (object, "notify::locked",
                              G_CALLBACK (item_notify_cb), master);
            item_notify_cb (object, NULL, master);
        }
        
        /* If the item is notebook, set the switcher style */
        if (GDL_IS_DOCK_NOTEBOOK (object) &&
            GDL_IS_SWITCHER (GDL_DOCK_ITEM (object)->child))
        {
            g_object_set (GDL_DOCK_ITEM (object)->child, "switcher-style",
                          master->_priv->switcher_style, NULL);
        }
        
        /* post a layout_changed emission if the item is not automatic
         * (since it should be added to the items model) */
        if (!GDL_DOCK_OBJECT_AUTOMATIC (object)) {
            if (!master->_priv->idle_layout_changed_id)
                master->_priv->idle_layout_changed_id =
                    g_idle_add (idle_emit_layout_changed, master);
        }
    }
}

void
gdl_dock_master_remove (GdlDockMaster *master,
                        GdlDockObject *object)
{
    g_return_if_fail (master != NULL && object != NULL);

    /* remove from locked/unlocked hashes and property change if
     * that's the case */
    if (GDL_IS_DOCK_ITEM (object) && GDL_DOCK_ITEM_HAS_GRIP (GDL_DOCK_ITEM (object))) {
        gint locked = COMPUTE_LOCKED (master);
        if (g_hash_table_remove (master->_priv->locked_items, object) ||
            g_hash_table_remove (master->_priv->unlocked_items, object)) {
            if (COMPUTE_LOCKED (master) != locked)
                g_object_notify (G_OBJECT (master), "locked");
        }
    }
        
    /* ref the master, since removing the controller could cause master disposal */
    g_object_ref (master);
    
    /* all the interesting stuff happens in _gdl_dock_master_remove */
    _gdl_dock_master_remove (object, master);

    /* post a layout_changed emission if the item is not automatic
     * (since it should be removed from the items model) */
    if (!GDL_DOCK_OBJECT_AUTOMATIC (object)) {
        if (!master->_priv->idle_layout_changed_id)
            master->_priv->idle_layout_changed_id =
                g_idle_add (idle_emit_layout_changed, master);
    }
    
    /* balance ref count */
    g_object_unref (master);
}

void
gdl_dock_master_foreach (GdlDockMaster *master,
                         GFunc          function,
                         gpointer       user_data)
{
    struct {
        GFunc    function;
        gpointer user_data;
    } data;

    g_return_if_fail (master != NULL && function != NULL);

    data.function = function;
    data.user_data = user_data;
    g_hash_table_foreach (master->dock_objects, _gdl_dock_master_foreach, &data);
}

void
gdl_dock_master_foreach_toplevel (GdlDockMaster *master,
                                  gboolean       include_controller,
                                  GFunc          function,
                                  gpointer       user_data)
{
    GList *l;
    
    g_return_if_fail (master != NULL && function != NULL);

    for (l = master->toplevel_docks; l; ) {
        GdlDockObject *object = GDL_DOCK_OBJECT (l->data);
        l = l->next;
        if (object != master->controller || include_controller)
            (* function) (GTK_WIDGET (object), user_data);
    }
}

GdlDockObject *
gdl_dock_master_get_object (GdlDockMaster *master,
                            const gchar   *nick_name)
{
    gpointer *found;
    
    g_return_val_if_fail (master != NULL, NULL);

    if (!nick_name)
        return NULL;

    found = g_hash_table_lookup (master->dock_objects, nick_name);

    return found ? GDL_DOCK_OBJECT (found) : NULL;
}

GdlDockObject *
gdl_dock_master_get_controller (GdlDockMaster *master)
{
    g_return_val_if_fail (master != NULL, NULL);

    return master->controller;
}

void
gdl_dock_master_set_controller (GdlDockMaster *master,
                                GdlDockObject *new_controller)
{
    g_return_if_fail (master != NULL);

    if (new_controller) {
        if (GDL_DOCK_OBJECT_AUTOMATIC (new_controller))
            g_warning (_("The new dock controller %p is automatic.  Only manual "
                         "dock objects should be named controller."), new_controller);
        
        /* check that the controller is in the toplevel list */
        if (!g_list_find (master->toplevel_docks, new_controller))
            gdl_dock_master_add (master, new_controller);
        master->controller = new_controller;

    } else {
        master->controller = NULL;
        /* no controller, no master */
        g_object_unref (master);
    }
}

static void
set_switcher_style_foreach (GtkWidget *obj, gpointer user_data)
{
    GdlSwitcherStyle style = GPOINTER_TO_INT (user_data);
    
    if (!GDL_IS_DOCK_ITEM (obj))
        return;
    
    if (GDL_IS_DOCK_NOTEBOOK (obj)) {
        
        GtkWidget *child = GDL_DOCK_ITEM (obj)->child;
        if (GDL_IS_SWITCHER (child)) {
            
            g_object_set (child, "switcher-style", style, NULL);
        }
    } else if (gdl_dock_object_is_compound (GDL_DOCK_OBJECT (obj))) {
        
        gtk_container_foreach (GTK_CONTAINER (obj),
                               set_switcher_style_foreach,
                               user_data);
    }
}

static void
gdl_dock_master_set_switcher_style (GdlDockMaster *master,
                                    GdlSwitcherStyle switcher_style)
{
    GList *l;
    g_return_if_fail (GDL_IS_DOCK_MASTER (master));
    
    master->_priv->switcher_style = switcher_style;
    for (l = master->toplevel_docks; l; l = l->next) {
        GdlDock *dock = GDL_DOCK (l->data);
        if (dock->root)
            set_switcher_style_foreach (GTK_WIDGET (dock->root),
                                        GINT_TO_POINTER (switcher_style));
    }

    /* just to be sure hidden items are set too */
    gdl_dock_master_foreach (master, (GFunc) set_switcher_style_foreach,
                             GINT_TO_POINTER (switcher_style));
}

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- 
 *
 * This file is part of the GNOME Devtools Libraries.
 *
 * Copyright (C) 2002 Gustavo Giráldez <gustavo.giraldez@gmx.net>
 *               2007 Naba Kumar  <naba@gnome.org>
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
#include <stdlib.h>
#include <string.h>

#include "gdl-dock.h"
#include "gdl-dock-master.h"
#include "gdl-dock-paned.h"
#include "gdl-dock-notebook.h"
#include "gdl-dock-placeholder.h"

#include "libgdlmarshal.h"

#ifndef __FUNCTION__
#define __FUNCTION__ __func__
#endif

/* ----- Private prototypes ----- */

static void  gdl_dock_class_init      (GdlDockClass *class);

static GObject *gdl_dock_constructor  (GType                  type,
                                       guint                  n_construct_properties,
                                       GObjectConstructParam *construct_param);
static void  gdl_dock_set_property    (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec);
static void  gdl_dock_get_property    (GObject      *object,
                                       guint         prop_id,
                                       GValue       *value,
                                       GParamSpec   *pspec);
static void  gdl_dock_notify_cb       (GObject      *object,
                                       GParamSpec   *pspec,
                                       gpointer      user_data);

static void  gdl_dock_set_title       (GdlDock      *dock);

static void  gdl_dock_destroy         (GtkObject    *object);

static void  gdl_dock_size_request    (GtkWidget      *widget,
                                       GtkRequisition *requisition);
static void  gdl_dock_size_allocate   (GtkWidget      *widget,
                                       GtkAllocation  *allocation);
static void  gdl_dock_map             (GtkWidget      *widget);
static void  gdl_dock_unmap           (GtkWidget      *widget);
static void  gdl_dock_show            (GtkWidget      *widget);
static void  gdl_dock_hide            (GtkWidget      *widget);

static void  gdl_dock_add             (GtkContainer *container,
                                       GtkWidget    *widget);
static void  gdl_dock_remove          (GtkContainer *container,
                                       GtkWidget    *widget);
static void  gdl_dock_forall          (GtkContainer *container,
                                       gboolean      include_internals,
                                       GtkCallback   callback,
                                       gpointer      callback_data);
static GType  gdl_dock_child_type   (GtkContainer *container);

static void     gdl_dock_detach       (GdlDockObject    *object,
                                       gboolean          recursive);
static void     gdl_dock_reduce       (GdlDockObject    *object);
static gboolean gdl_dock_dock_request (GdlDockObject    *object,
                                       gint              x,
                                       gint              y,
                                       GdlDockRequest   *request);
static void     gdl_dock_dock         (GdlDockObject    *object,
                                       GdlDockObject    *requestor,
                                       GdlDockPlacement  position,
                                       GValue           *other_data);
static gboolean gdl_dock_reorder      (GdlDockObject    *object,
                                       GdlDockObject    *requestor,
                                       GdlDockPlacement  new_position,
                                       GValue           *other_data);

static gboolean gdl_dock_floating_window_delete_event_cb (GtkWidget *widget);

static gboolean gdl_dock_child_placement  (GdlDockObject    *object,
                                           GdlDockObject    *child,
                                           GdlDockPlacement *placement);

static void     gdl_dock_present          (GdlDockObject    *object,
                                           GdlDockObject    *child);


/* ----- Class variables and definitions ----- */

struct _GdlDockPrivate
{
    /* for floating docks */
    gboolean            floating;
    GtkWidget          *window;
    gboolean            auto_title;
    
    gint                float_x;
    gint                float_y;
    gint                width;
    gint                height;

    /* auxiliary fields */
    GdkGC              *xor_gc;
};

enum {
    LAYOUT_CHANGED,
    LAST_SIGNAL
};

enum {
    PROP_0,
    PROP_FLOATING,
    PROP_DEFAULT_TITLE,
    PROP_WIDTH,
    PROP_HEIGHT,
    PROP_FLOAT_X,
    PROP_FLOAT_Y
};

static guint dock_signals [LAST_SIGNAL] = { 0 };

#define SPLIT_RATIO  0.3


/* ----- Private functions ----- */

G_DEFINE_TYPE (GdlDock, gdl_dock, GDL_TYPE_DOCK_OBJECT);

static void
gdl_dock_class_init (GdlDockClass *klass)
{
    GObjectClass       *g_object_class;
    GtkObjectClass     *gtk_object_class;
    GtkWidgetClass     *widget_class;
    GtkContainerClass  *container_class;
    GdlDockObjectClass *object_class;
    
    g_object_class = G_OBJECT_CLASS (klass);
    gtk_object_class = GTK_OBJECT_CLASS (klass);
    widget_class = GTK_WIDGET_CLASS (klass);
    container_class = GTK_CONTAINER_CLASS (klass);
    object_class = GDL_DOCK_OBJECT_CLASS (klass);
    
    g_object_class->constructor = gdl_dock_constructor;
    g_object_class->set_property = gdl_dock_set_property;
    g_object_class->get_property = gdl_dock_get_property;
    
    /* properties */

    g_object_class_install_property (
        g_object_class, PROP_FLOATING,
        g_param_spec_boolean ("floating", _("Floating"),
                              _("Whether the dock is floating in its own window"),
                              FALSE,
                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                              GDL_DOCK_PARAM_EXPORT));
    
    g_object_class_install_property (
        g_object_class, PROP_DEFAULT_TITLE,
        g_param_spec_string ("default-title", _("Default title"),
                             _("Default title for the newly created floating docks"),
                             NULL,
                             G_PARAM_READWRITE));
    
    g_object_class_install_property (
        g_object_class, PROP_WIDTH,
        g_param_spec_int ("width", _("Width"),
                          _("Width for the dock when it's of floating type"),
                          -1, G_MAXINT, -1,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
                          GDL_DOCK_PARAM_EXPORT));
    
    g_object_class_install_property (
        g_object_class, PROP_HEIGHT,
        g_param_spec_int ("height", _("Height"),
                          _("Height for the dock when it's of floating type"),
                          -1, G_MAXINT, -1,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
                          GDL_DOCK_PARAM_EXPORT));
    
    g_object_class_install_property (
        g_object_class, PROP_FLOAT_X,
        g_param_spec_int ("floatx", _("Float X"),
                          _("X coordinate for a floating dock"),
                          G_MININT, G_MAXINT, 0,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
                          GDL_DOCK_PARAM_EXPORT));
    
    g_object_class_install_property (
        g_object_class, PROP_FLOAT_Y,
        g_param_spec_int ("floaty", _("Float Y"),
                          _("Y coordinate for a floating dock"),
                          G_MININT, G_MAXINT, 0,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
                          GDL_DOCK_PARAM_EXPORT));
    
    gtk_object_class->destroy = gdl_dock_destroy;

    widget_class->size_request = gdl_dock_size_request;
    widget_class->size_allocate = gdl_dock_size_allocate;
    widget_class->map = gdl_dock_map;
    widget_class->unmap = gdl_dock_unmap;
    widget_class->show = gdl_dock_show;
    widget_class->hide = gdl_dock_hide;
    
    container_class->add = gdl_dock_add;
    container_class->remove = gdl_dock_remove;
    container_class->forall = gdl_dock_forall;
    container_class->child_type = gdl_dock_child_type;
    
    object_class->is_compound = TRUE;
    
    object_class->detach = gdl_dock_detach;
    object_class->reduce = gdl_dock_reduce;
    object_class->dock_request = gdl_dock_dock_request;
    object_class->dock = gdl_dock_dock;
    object_class->reorder = gdl_dock_reorder;    
    object_class->child_placement = gdl_dock_child_placement;
    object_class->present = gdl_dock_present;
    
    /* signals */

    dock_signals [LAYOUT_CHANGED] = 
        g_signal_new ("layout-changed", 
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GdlDockClass, layout_changed),
                      NULL, /* accumulator */
                      NULL, /* accu_data */
                      gdl_marshal_VOID__VOID,
                      G_TYPE_NONE, /* return type */
                      0);

    klass->layout_changed = NULL;
}

static void
gdl_dock_init (GdlDock *dock)
{
    gtk_widget_set_has_window (GTK_WIDGET (dock), FALSE);

    dock->root = NULL;
    dock->_priv = g_new0 (GdlDockPrivate, 1);
    dock->_priv->width = -1;
    dock->_priv->height = -1;
}

static gboolean 
gdl_dock_floating_configure_event_cb (GtkWidget         *widget,
                                      GdkEventConfigure *event,
                                      gpointer           user_data)
{
    GdlDock *dock;
    
    g_return_val_if_fail (user_data != NULL && GDL_IS_DOCK (user_data), TRUE);

    dock = GDL_DOCK (user_data);
    dock->_priv->float_x = event->x;
    dock->_priv->float_y = event->y;
    dock->_priv->width = event->width;
    dock->_priv->height = event->height;

    return FALSE;
}

static GObject *
gdl_dock_constructor (GType                  type,
                      guint                  n_construct_properties,
                      GObjectConstructParam *construct_param)
{
    GObject *g_object;

    g_object = G_OBJECT_CLASS (gdl_dock_parent_class)-> constructor (type,
                                                                     n_construct_properties,
                                                                     construct_param);
    if (g_object) {
        GdlDock *dock = GDL_DOCK (g_object);
        GdlDockMaster *master;

        /* create a master for the dock if none was provided in the construction */
        master = GDL_DOCK_OBJECT_GET_MASTER (GDL_DOCK_OBJECT (dock));
        if (!master) {
            GDL_DOCK_OBJECT_UNSET_FLAGS (dock, GDL_DOCK_AUTOMATIC);
            master = g_object_new (GDL_TYPE_DOCK_MASTER, NULL);
            /* the controller owns the master ref */
            gdl_dock_object_bind (GDL_DOCK_OBJECT (dock), G_OBJECT (master));
        }

        if (dock->_priv->floating) {
            /* create floating window for this dock */
            dock->_priv->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
            g_object_set_data (G_OBJECT (dock->_priv->window), "dock", dock);

            /* set position and default size */
            gtk_window_set_position (GTK_WINDOW (dock->_priv->window),
                                     GTK_WIN_POS_MOUSE);
            gtk_window_set_default_size (GTK_WINDOW (dock->_priv->window),
                                         dock->_priv->width,
                                         dock->_priv->height);
            gtk_window_set_type_hint (GTK_WINDOW (dock->_priv->window),
                                      GDK_WINDOW_TYPE_HINT_NORMAL);
            
            gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dock->_priv->window), 
                                              TRUE);

            /* metacity ignores this */
            gtk_window_move (GTK_WINDOW (dock->_priv->window),
                             dock->_priv->float_x,
                             dock->_priv->float_y);

            /* connect to the configure event so we can track down window geometry */
            g_signal_connect (dock->_priv->window, "configure_event",
                              (GCallback) gdl_dock_floating_configure_event_cb,
                              dock);

            /* set the title and connect to the long_name notify queue
             so we can reset the title when this prop changes */
            gdl_dock_set_title (dock);
            g_signal_connect (dock, "notify::long-name",
                              (GCallback) gdl_dock_notify_cb, NULL);

            gtk_container_add (GTK_CONTAINER (dock->_priv->window), GTK_WIDGET (dock));

            g_signal_connect (dock->_priv->window, "delete_event",
                              G_CALLBACK (gdl_dock_floating_window_delete_event_cb), 
                              NULL);
        }
        GDL_DOCK_OBJECT_SET_FLAGS (dock, GDL_DOCK_ATTACHED);
    }

    return g_object;
}

static void
gdl_dock_set_property  (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
    GdlDock *dock = GDL_DOCK (object);
    
    switch (prop_id) {
        case PROP_FLOATING:
            dock->_priv->floating = g_value_get_boolean (value);
            break;
        case PROP_DEFAULT_TITLE:
            if (GDL_DOCK_OBJECT (object)->master)
                g_object_set (GDL_DOCK_OBJECT (object)->master,
                              "default-title", g_value_get_string (value),
                              NULL);
            break;
        case PROP_WIDTH:
            dock->_priv->width = g_value_get_int (value);
            break;
        case PROP_HEIGHT:
            dock->_priv->height = g_value_get_int (value);
            break;
        case PROP_FLOAT_X:
            dock->_priv->float_x = g_value_get_int (value);
            break;
        case PROP_FLOAT_Y:
            dock->_priv->float_y = g_value_get_int (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }

    switch (prop_id) {
        case PROP_WIDTH:
        case PROP_HEIGHT:
        case PROP_FLOAT_X:
        case PROP_FLOAT_Y:
            if (dock->_priv->floating && dock->_priv->window) {
                gtk_window_resize (GTK_WINDOW (dock->_priv->window),
                                   dock->_priv->width,
                                   dock->_priv->height);
            }
            break;
    }
}

static void
gdl_dock_get_property  (GObject      *object,
                        guint         prop_id,
                        GValue       *value,
                        GParamSpec   *pspec)
{
    GdlDock *dock = GDL_DOCK (object);

    switch (prop_id) {
        case PROP_FLOATING:
            g_value_set_boolean (value, dock->_priv->floating);
            break;
        case PROP_DEFAULT_TITLE:
            if (GDL_DOCK_OBJECT (object)->master) {
                gchar *default_title;
                g_object_get (GDL_DOCK_OBJECT (object)->master,
                              "default-title", &default_title,
                              NULL);
#if GLIB_CHECK_VERSION(2,3,0)
                g_value_take_string (value, default_title);
#else
                g_value_set_string_take_ownership (value, default_title);
#endif
            }
            else
                g_value_set_string (value, NULL);
            break;
        case PROP_WIDTH:
            g_value_set_int (value, dock->_priv->width);
            break;
        case PROP_HEIGHT:
            g_value_set_int (value, dock->_priv->height);
            break;
        case PROP_FLOAT_X:
            g_value_set_int (value, dock->_priv->float_x);
            break;
        case PROP_FLOAT_Y:
            g_value_set_int (value, dock->_priv->float_y);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
gdl_dock_set_title (GdlDock *dock)
{
    GdlDockObject *object = GDL_DOCK_OBJECT (dock);
    gchar         *title = NULL;
    
    if (!dock->_priv->window)
        return;
    
    if (!dock->_priv->auto_title && object->long_name) {
        title = object->long_name;
    }
    else if (object->master) {
        g_object_get (object->master, "default-title", &title, NULL);
    }

    if (!title && dock->root) {
        g_object_get (dock->root, "long-name", &title, NULL);
    }
    
    if (!title) {
        /* set a default title in the long_name */
        dock->_priv->auto_title = TRUE;
        title = g_strdup_printf (
            _("Dock #%d"), GDL_DOCK_MASTER (object->master)->dock_number++);
    }

    gtk_window_set_title (GTK_WINDOW (dock->_priv->window), title);

    g_free (title);
}

static void
gdl_dock_notify_cb (GObject    *object,
                    GParamSpec *pspec,
                    gpointer    user_data)
{
    GdlDock *dock;
    gchar* long_name;
    
    g_return_if_fail (object != NULL || GDL_IS_DOCK (object));

    g_object_get (object, "long-name", &long_name, NULL);

    if (long_name)
    {
        dock = GDL_DOCK (object);
        dock->_priv->auto_title = FALSE;
        gdl_dock_set_title (dock);
    }
    g_free (long_name);
}

static void
gdl_dock_destroy (GtkObject *object)
{
    GdlDock *dock = GDL_DOCK (object);

    if (dock->_priv) {
        GdlDockPrivate *priv = dock->_priv;
        dock->_priv = NULL;

        if (priv->window) {
            gtk_widget_destroy (priv->window);
            priv->floating = FALSE;
            priv->window = NULL;
        }
        
        /* destroy the xor gc */
        if (priv->xor_gc) {
            g_object_unref (priv->xor_gc);
            priv->xor_gc = NULL;
        }

        g_free (priv);
    }
    
   GTK_OBJECT_CLASS (gdl_dock_parent_class)->destroy (object);
}

static void
gdl_dock_size_request (GtkWidget      *widget,
                       GtkRequisition *requisition)
{
    GdlDock       *dock;
    GtkContainer  *container;
    guint          border_width;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GDL_IS_DOCK (widget));

    dock = GDL_DOCK (widget);
    container = GTK_CONTAINER (widget);
    border_width = gtk_container_get_border_width (container);

    /* make request to root */
    if (dock->root && gtk_widget_get_visible (GTK_WIDGET (dock->root)))
        gtk_widget_size_request (GTK_WIDGET (dock->root), requisition);
    else {
        requisition->width = 0;
        requisition->height = 0;
    };

    requisition->width += 2 * border_width;
    requisition->height += 2 * border_width;

    //gtk_widget_size_request (widget, requisition);    
}

static void
gdl_dock_size_allocate (GtkWidget     *widget,
                        GtkAllocation *allocation)
{
    GdlDock       *dock;
    GtkContainer  *container;
    guint          border_width;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GDL_IS_DOCK (widget));
    
    dock = GDL_DOCK (widget);
    container = GTK_CONTAINER (widget);
    border_width = gtk_container_get_border_width (container);

    gtk_widget_set_allocation (widget, allocation);

    /* reduce allocation by border width */
    allocation->x += border_width;
    allocation->y += border_width;
    allocation->width = MAX (1, allocation->width - 2 * border_width);
    allocation->height = MAX (1, allocation->height - 2 * border_width);

    if (dock->root && gtk_widget_get_visible (GTK_WIDGET (dock->root)))
        gtk_widget_size_allocate (GTK_WIDGET (dock->root), allocation);
}

static void
gdl_dock_map (GtkWidget *widget)
{
    GtkWidget *child;
    GdlDock   *dock;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GDL_IS_DOCK (widget));

    dock = GDL_DOCK (widget);

    GTK_WIDGET_CLASS (gdl_dock_parent_class)->map (widget);

    if (dock->root) {
        child = GTK_WIDGET (dock->root);
        if (gtk_widget_get_visible (child) && !gtk_widget_get_mapped (child))
            gtk_widget_map (child);
    }
}

static void
gdl_dock_unmap (GtkWidget *widget)
{
    GtkWidget *child;
    GdlDock   *dock;
    
    g_return_if_fail (widget != NULL);
    g_return_if_fail (GDL_IS_DOCK (widget));

    dock = GDL_DOCK (widget);

    GTK_WIDGET_CLASS (gdl_dock_parent_class)->unmap (widget);

    if (dock->root) {
        child = GTK_WIDGET (dock->root);
        if (gtk_widget_get_visible (child) && gtk_widget_get_mapped (child))
            gtk_widget_unmap (child);
    }
    
    if (dock->_priv->window)
        gtk_widget_unmap (dock->_priv->window);
}

static void
gdl_dock_foreach_automatic (GdlDockObject *object,
                            gpointer       user_data)
{
    void (* function) (GtkWidget *) = user_data;

    if (GDL_DOCK_OBJECT_AUTOMATIC (object))
        (* function) (GTK_WIDGET (object));
}

static void
gdl_dock_show (GtkWidget *widget)
{
    GdlDock *dock;
    
    g_return_if_fail (widget != NULL);
    g_return_if_fail (GDL_IS_DOCK (widget));
    
    GTK_WIDGET_CLASS (gdl_dock_parent_class)->show (widget);
    
    dock = GDL_DOCK (widget);
    if (dock->_priv->floating && dock->_priv->window)
        gtk_widget_show (dock->_priv->window);

    if (GDL_DOCK_IS_CONTROLLER (dock)) {
        gdl_dock_master_foreach_toplevel (GDL_DOCK_OBJECT_GET_MASTER (dock),
                                          FALSE, (GFunc) gdl_dock_foreach_automatic,
                                          gtk_widget_show);
    }
}

static void
gdl_dock_hide (GtkWidget *widget)
{
    GdlDock *dock;
    
    g_return_if_fail (widget != NULL);
    g_return_if_fail (GDL_IS_DOCK (widget));
    
    GTK_WIDGET_CLASS (gdl_dock_parent_class)->hide (widget);
    
    dock = GDL_DOCK (widget);
    if (dock->_priv->floating && dock->_priv->window)
        gtk_widget_hide (dock->_priv->window);

    if (GDL_DOCK_IS_CONTROLLER (dock)) {
        gdl_dock_master_foreach_toplevel (GDL_DOCK_OBJECT_GET_MASTER (dock),
                                          FALSE, (GFunc) gdl_dock_foreach_automatic,
                                          gtk_widget_hide);
    }
}

static void
gdl_dock_add (GtkContainer *container,
              GtkWidget    *widget)
{
    g_return_if_fail (container != NULL);
    g_return_if_fail (GDL_IS_DOCK (container));
    g_return_if_fail (GDL_IS_DOCK_ITEM (widget));

    gdl_dock_add_item (GDL_DOCK (container), 
                       GDL_DOCK_ITEM (widget), 
                       GDL_DOCK_TOP);  /* default position */
}

static void
gdl_dock_remove (GtkContainer *container,
                 GtkWidget    *widget)
{
    GdlDock  *dock;
    gboolean  was_visible;

    g_return_if_fail (container != NULL);
    g_return_if_fail (widget != NULL);

    dock = GDL_DOCK (container);
    was_visible = gtk_widget_get_visible (widget);

    if (GTK_WIDGET (dock->root) == widget) {
        dock->root = NULL;
        GDL_DOCK_OBJECT_UNSET_FLAGS (widget, GDL_DOCK_ATTACHED);
        gtk_widget_unparent (widget);

        if (was_visible && gtk_widget_get_visible (GTK_WIDGET (container)))
            gtk_widget_queue_resize (GTK_WIDGET (dock));
    }
}

static void
gdl_dock_forall (GtkContainer *container,
                 gboolean      include_internals,
                 GtkCallback   callback,
                 gpointer      callback_data)
{
    GdlDock *dock;

    g_return_if_fail (container != NULL);
    g_return_if_fail (GDL_IS_DOCK (container));
    g_return_if_fail (callback != NULL);

    dock = GDL_DOCK (container);

    if (dock->root)
        (*callback) (GTK_WIDGET (dock->root), callback_data);
}

static GType
gdl_dock_child_type (GtkContainer *container)
{
    return GDL_TYPE_DOCK_ITEM;
}

static void
gdl_dock_detach (GdlDockObject *object,
                 gboolean       recursive)
{
    GdlDock *dock = GDL_DOCK (object);
    
    /* detach children */
    if (recursive && dock->root) {
        gdl_dock_object_detach (dock->root, recursive);
    }
    GDL_DOCK_OBJECT_UNSET_FLAGS (object, GDL_DOCK_ATTACHED);
}

static void
gdl_dock_reduce (GdlDockObject *object)
{
    GdlDock *dock = GDL_DOCK (object);
    GtkWidget *parent;
    
    if (dock->root)
        return;
    
    if (GDL_DOCK_OBJECT_AUTOMATIC (dock)) {
        gtk_widget_destroy (GTK_WIDGET (dock));

    } else if (!GDL_DOCK_OBJECT_ATTACHED (dock)) {
        /* if the user explicitly detached the object */
        if (dock->_priv->floating)
            gtk_widget_hide (GTK_WIDGET (dock));
        else {
            GtkWidget *widget = GTK_WIDGET (object);
            parent = gtk_widget_get_parent (widget);
            if (parent)
                gtk_container_remove (GTK_CONTAINER (parent), widget);
        }
    }
}

static gboolean
gdl_dock_dock_request (GdlDockObject  *object,
                       gint            x,
                       gint            y,
                       GdlDockRequest *request)
{
    GdlDock            *dock;
    guint               bw;
    gint                rel_x, rel_y;
    GtkAllocation       alloc;
    gboolean            may_dock = FALSE;
    GdlDockRequest      my_request;

    g_return_val_if_fail (GDL_IS_DOCK (object), FALSE);

    /* we get (x,y) in our allocation coordinates system */
    
    dock = GDL_DOCK (object);
    
    /* Get dock size. */
    gtk_widget_get_allocation (GTK_WIDGET (dock), &alloc);
    bw = gtk_container_get_border_width (GTK_CONTAINER (dock));

    /* Get coordinates relative to our allocation area. */
    rel_x = x - alloc.x;
    rel_y = y - alloc.y;

    if (request)
        my_request = *request;
        
    /* Check if coordinates are in GdlDock widget. */
    if (rel_x > 0 && rel_x < alloc.width &&
        rel_y > 0 && rel_y < alloc.height) {

        /* It's inside our area. */
        may_dock = TRUE;

	/* Set docking indicator rectangle to the GdlDock size. */
        my_request.rect.x = alloc.x + bw;
        my_request.rect.y = alloc.y + bw;
        my_request.rect.width = alloc.width - 2*bw;
        my_request.rect.height = alloc.height - 2*bw;

	/* If GdlDock has no root item yet, set the dock itself as 
	   possible target. */
        if (!dock->root) {
            my_request.position = GDL_DOCK_TOP;
            my_request.target = object;
        } else {
            my_request.target = dock->root;

            /* See if it's in the border_width band. */
            if (rel_x < (gint)bw) {
                my_request.position = GDL_DOCK_LEFT;
                my_request.rect.width *= SPLIT_RATIO;
            } else if (rel_x > alloc.width - (gint)bw) {
                my_request.position = GDL_DOCK_RIGHT;
                my_request.rect.x += my_request.rect.width * (1 - SPLIT_RATIO);
                my_request.rect.width *= SPLIT_RATIO;
            } else if (rel_y < (gint)bw) {
                my_request.position = GDL_DOCK_TOP;
                my_request.rect.height *= SPLIT_RATIO;
            } else if (rel_y > alloc.height - (gint)bw) {
                my_request.position = GDL_DOCK_BOTTOM;
                my_request.rect.y += my_request.rect.height * (1 - SPLIT_RATIO);
                my_request.rect.height *= SPLIT_RATIO;
            } else {
                /* Otherwise try our children. */
                /* give them allocation coordinates (we are a
                   GTK_NO_WINDOW) widget */
                may_dock = gdl_dock_object_dock_request (GDL_DOCK_OBJECT (dock->root), 
                                                         x, y, &my_request);
            }
        }
    }

    if (may_dock && request)
        *request = my_request;
    
    return may_dock;
}

static void
gdl_dock_dock (GdlDockObject    *object,
               GdlDockObject    *requestor,
               GdlDockPlacement  position,
               GValue           *user_data)
{
    GdlDock *dock;
    
    g_return_if_fail (GDL_IS_DOCK (object));
    /* only dock items allowed at this time */
    g_return_if_fail (GDL_IS_DOCK_ITEM (requestor));

    dock = GDL_DOCK (object);
    
    if (position == GDL_DOCK_FLOATING) {
        GdlDockItem *item = GDL_DOCK_ITEM (requestor);
        gint x, y, width, height;

        if (user_data && G_VALUE_HOLDS (user_data, GDK_TYPE_RECTANGLE)) {
            GdkRectangle *rect;

            rect = g_value_get_boxed (user_data);
            x = rect->x;
            y = rect->y;
            width = rect->width;
            height = rect->height;
        }
        else {
            x = y = 0;
            width = height = -1;
        }
        
        gdl_dock_add_floating_item (dock, item,
                                    x, y, width, height);
    }
    else if (dock->root) {
        /* This is somewhat a special case since we know which item to
           pass the request on because we only have on child */
        gdl_dock_object_dock (dock->root, requestor, position, NULL);
        gdl_dock_set_title (dock);
        
    }
    else { /* Item about to be added is root item. */
        GtkWidget *widget = GTK_WIDGET (requestor);
        
        dock->root = requestor;
        GDL_DOCK_OBJECT_SET_FLAGS (requestor, GDL_DOCK_ATTACHED);
        gtk_widget_set_parent (widget, GTK_WIDGET (dock));
        
        gdl_dock_item_show_grip (GDL_DOCK_ITEM (requestor));

        /* Realize the item (create its corresponding GdkWindow) when 
           GdlDock has been realized. */
        if (gtk_widget_get_realized (GTK_WIDGET (dock)))
            gtk_widget_realize (widget);
        
        /* Map the widget if it's visible and the parent is visible and has 
           been mapped. This is done to make sure that the GdkWindow is 
           visible. */
        if (gtk_widget_get_visible (GTK_WIDGET (dock)) && 
            gtk_widget_get_visible (widget)) {
            if (gtk_widget_get_mapped (GTK_WIDGET (dock)))
                gtk_widget_map (widget);
            
            /* Make the widget resize. */
            gtk_widget_queue_resize (widget);
        }
        gdl_dock_set_title (dock);
    }
}
    
static gboolean
gdl_dock_floating_window_delete_event_cb (GtkWidget *widget)
{
    GdlDock *dock;
    
    g_return_val_if_fail (GTK_IS_WINDOW (widget), FALSE);
    
    dock = GDL_DOCK (g_object_get_data (G_OBJECT (widget), "dock"));
    if (dock->root) {
        /* this will call reduce on ourselves, hiding the window if appropiate */
        gdl_dock_item_hide_item (GDL_DOCK_ITEM (dock->root));
    }

    return TRUE;
}

static void
_gdl_dock_foreach_build_list (GdlDockObject *object,
                              gpointer       user_data)
{
    GList **l = (GList **) user_data;

    if (GDL_IS_DOCK_ITEM (object))
        *l = g_list_prepend (*l, object);
}

static gboolean
gdl_dock_reorder (GdlDockObject    *object,
                  GdlDockObject    *requestor,
                  GdlDockPlacement  new_position,
                  GValue           *other_data)
{
    GdlDock *dock = GDL_DOCK (object);
    gboolean handled = FALSE;
    
    if (dock->_priv->floating &&
        new_position == GDL_DOCK_FLOATING &&
        dock->root == requestor) {
        
        if (other_data && G_VALUE_HOLDS (other_data, GDK_TYPE_RECTANGLE)) {
            GdkRectangle *rect;

            rect = g_value_get_boxed (other_data);
            gtk_window_move (GTK_WINDOW (dock->_priv->window),
                             rect->x,
                             rect->y);
            handled = TRUE;
        }
    }
    
    return handled;
}

static gboolean 
gdl_dock_child_placement (GdlDockObject    *object,
                          GdlDockObject    *child,
                          GdlDockPlacement *placement)
{
    GdlDock *dock = GDL_DOCK (object);
    gboolean retval = TRUE;
    
    if (dock->root == child) {
        if (placement) {
            if (*placement == GDL_DOCK_NONE || *placement == GDL_DOCK_FLOATING)
                *placement = GDL_DOCK_TOP;
        }
    } else 
        retval = FALSE;

    return retval;
}

static void 
gdl_dock_present (GdlDockObject *object,
                  GdlDockObject *child)
{
    GdlDock *dock = GDL_DOCK (object);

    if (dock->_priv->floating)
        gtk_window_present (GTK_WINDOW (dock->_priv->window));
}


/* ----- Public interface ----- */

GtkWidget *
gdl_dock_new (void)
{
    GObject *dock;

    dock = g_object_new (GDL_TYPE_DOCK, NULL);
    GDL_DOCK_OBJECT_UNSET_FLAGS (dock, GDL_DOCK_AUTOMATIC);
    
    return GTK_WIDGET (dock);
}

GtkWidget *
gdl_dock_new_from (GdlDock  *original,
                   gboolean  floating)
{
    GObject *new_dock;
    
    g_return_val_if_fail (original != NULL, NULL);
    
    new_dock = g_object_new (GDL_TYPE_DOCK, 
                             "master", GDL_DOCK_OBJECT_GET_MASTER (original), 
                             "floating", floating,
                             NULL);
    GDL_DOCK_OBJECT_UNSET_FLAGS (new_dock, GDL_DOCK_AUTOMATIC);
    
    return GTK_WIDGET (new_dock);
}

/* Depending on where the dock item (where new item will be docked) locates
 * in the dock, we might need to change the docking placement. If the
 * item is does not touches the center of dock, the new-item-to-dock would
 * require a center dock on this item.
 */
static GdlDockPlacement
gdl_dock_refine_placement (GdlDock *dock, GdlDockItem *dock_item,
                           GdlDockPlacement placement)
{
    GtkAllocation allocation;
    GtkRequisition object_size;
    
    gdl_dock_item_preferred_size (dock_item, &object_size);
    gtk_widget_get_allocation (GTK_WIDGET (dock), &allocation);

    g_return_val_if_fail (allocation.width > 0, placement);
    g_return_val_if_fail (allocation.height > 0, placement);
    g_return_val_if_fail (object_size.width > 0, placement);
    g_return_val_if_fail (object_size.height > 0, placement);

    if (placement == GDL_DOCK_LEFT || placement == GDL_DOCK_RIGHT) {
        /* Check if dock_object touches center in terms of width */
        if (allocation.width/2 > object_size.width) {
            return GDL_DOCK_CENTER;
        }
    } else if (placement == GDL_DOCK_TOP || placement == GDL_DOCK_BOTTOM) {
        /* Check if dock_object touches center in terms of height */
        if (allocation.height/2 > object_size.height) {
            return GDL_DOCK_CENTER;
        }
    }
    return placement;
}

/* Determines the larger item of the two based on the placement:
 * for left/right placement, height determines it.
 * for top/bottom placement, width determines it.
 * for center placement, area determines it.
 */
static GdlDockItem*
gdl_dock_select_larger_item (GdlDockItem *dock_item_1,
                             GdlDockItem *dock_item_2,
                             GdlDockPlacement placement,
                             gint level /* for debugging */)
{
    GtkRequisition size_1, size_2;
    
    g_return_val_if_fail (dock_item_1 != NULL, dock_item_2);
    g_return_val_if_fail (dock_item_2 != NULL, dock_item_1);
    
    gdl_dock_item_preferred_size (dock_item_1, &size_1);
    gdl_dock_item_preferred_size (dock_item_2, &size_2);
    
    g_return_val_if_fail (size_1.width > 0, dock_item_2);
    g_return_val_if_fail (size_1.height > 0, dock_item_2);
    g_return_val_if_fail (size_2.width > 0, dock_item_1);
    g_return_val_if_fail (size_2.height > 0, dock_item_1);
    
    if (placement == GDL_DOCK_LEFT || placement == GDL_DOCK_RIGHT)
    {
        /* For left/right placement, height is what matters */
        return (size_1.height >= size_2.height?
                    dock_item_1 : dock_item_2);
    } else if (placement == GDL_DOCK_TOP || placement == GDL_DOCK_BOTTOM)
    {
        /* For top/bottom placement, width is what matters */
        return (size_1.width >= size_2.width?
                    dock_item_1 : dock_item_2);
    } else if (placement == GDL_DOCK_CENTER) {
        /* For center place, area is what matters */
        return ((size_1.width * size_1.height)
                    >= (size_2.width * size_2.height)?
                    dock_item_1 : dock_item_2);
    } else if (placement == GDL_DOCK_NONE) {
	return dock_item_1;
    } else {
        g_warning ("Should not reach here: %s:%d", __FUNCTION__, __LINE__);
    }
    return dock_item_1;
}

/* Determines the best dock item to dock a new item with the given placement.
 * It traverses the dock tree and (based on the placement) tries to find
 * the best located item wrt to the placement. The approach is to find the
 * largest item on/around the placement side (for side placements) and to
 * find the largest item for center placement. In most situations, this is
 * what user wants and the heuristic should be therefore sufficient.
 */
static GdlDockItem*
gdl_dock_find_best_placement_item (GdlDockItem *dock_item,
                                   GdlDockPlacement placement,
                                   gint level /* for debugging */)
{
    GdlDockItem *ret_item = NULL;
    
    if (GDL_IS_DOCK_PANED (dock_item))
    {
        GtkOrientation orientation;
        GdlDockItem *dock_item_1, *dock_item_2;
        GList* children;
        
        children = gtk_container_get_children (GTK_CONTAINER (dock_item));
        
        g_assert (g_list_length (children) == 2);
        
        g_object_get (dock_item, "orientation", &orientation, NULL);
        if ((orientation == GTK_ORIENTATION_HORIZONTAL &&
             placement == GDL_DOCK_LEFT) ||
            (orientation == GTK_ORIENTATION_VERTICAL &&
             placement == GDL_DOCK_TOP)) {
            /* Return left or top pane widget */
            ret_item =
                gdl_dock_find_best_placement_item (GDL_DOCK_ITEM
                                                   (children->data),
                                                   placement, level + 1);
        } else if ((orientation == GTK_ORIENTATION_HORIZONTAL &&
                    placement == GDL_DOCK_RIGHT) ||
                   (orientation == GTK_ORIENTATION_VERTICAL &&
                    placement == GDL_DOCK_BOTTOM)) {
                        /* Return right or top pane widget */
            ret_item =
                gdl_dock_find_best_placement_item (GDL_DOCK_ITEM
                                                   (children->next->data),
                                                   placement, level + 1);
        } else {
            /* Evaluate which of the two sides is bigger */
            dock_item_1 =
                gdl_dock_find_best_placement_item (GDL_DOCK_ITEM
                                                   (children->data),
                                                   placement, level + 1);
            dock_item_2 =
                gdl_dock_find_best_placement_item (GDL_DOCK_ITEM
                                                   (children->next->data),
                                                   placement, level + 1);
            ret_item = gdl_dock_select_larger_item (dock_item_1,
                                                    dock_item_2,
                                                    placement, level);
        }
        g_list_free (children);
    }
    else if (GDL_IS_DOCK_ITEM (dock_item))
    {
        ret_item = dock_item;
    }
    else
    {
        /* should not be here */
        g_warning ("Should not reach here: %s:%d", __FUNCTION__, __LINE__);
    }
    return ret_item;
}

void
gdl_dock_add_item (GdlDock          *dock,
                   GdlDockItem      *item,
                   GdlDockPlacement  placement)
{
    g_return_if_fail (dock != NULL);
    g_return_if_fail (item != NULL);

    if (placement == GDL_DOCK_FLOATING)
        /* Add the item to a new floating dock */
        gdl_dock_add_floating_item (dock, item, 0, 0, -1, -1);

    else {
        GdlDockItem *best_dock_item;
        /* Non-floating item. */
        if (dock->root) {
            GdlDockPlacement local_placement;
            
            best_dock_item =
                gdl_dock_find_best_placement_item (GDL_DOCK_ITEM (dock->root),
                                                   placement, 0);
            local_placement = gdl_dock_refine_placement (dock, best_dock_item,
                                                         placement);
            gdl_dock_object_dock (GDL_DOCK_OBJECT (best_dock_item),
                                  GDL_DOCK_OBJECT (item),
                                  local_placement, NULL);
        } else {
            gdl_dock_object_dock (GDL_DOCK_OBJECT (dock),
                                  GDL_DOCK_OBJECT (item),
                                  placement, NULL);
        }
    }
}

void
gdl_dock_add_floating_item (GdlDock        *dock,
                            GdlDockItem    *item,
                            gint            x,
                            gint            y,
                            gint            width,
                            gint            height)
{
    GdlDock *new_dock;
    
    g_return_if_fail (dock != NULL);
    g_return_if_fail (item != NULL);
    
    new_dock = GDL_DOCK (g_object_new (GDL_TYPE_DOCK, 
                                       "master", GDL_DOCK_OBJECT_GET_MASTER (dock), 
                                       "floating", TRUE,
                                       "width", width,
                                       "height", height,
                                       "floatx", x,
                                       "floaty", y,
                                       NULL));
    
    if (gtk_widget_get_visible (GTK_WIDGET (dock))) {
        gtk_widget_show (GTK_WIDGET (new_dock));
        if (gtk_widget_get_mapped (GTK_WIDGET (dock)))
            gtk_widget_map (GTK_WIDGET (new_dock));
        
        /* Make the widget resize. */
        gtk_widget_queue_resize (GTK_WIDGET (new_dock));
    }

    gdl_dock_add_item (GDL_DOCK (new_dock), item, GDL_DOCK_TOP);
}

GdlDockItem *
gdl_dock_get_item_by_name (GdlDock     *dock,
                           const gchar *name)
{
    GdlDockObject *found;
    
    g_return_val_if_fail (dock != NULL && name != NULL, NULL);
    
    /* proxy the call to our master */
    found = gdl_dock_master_get_object (GDL_DOCK_OBJECT_GET_MASTER (dock), name);

    return (found && GDL_IS_DOCK_ITEM (found)) ? GDL_DOCK_ITEM (found) : NULL;
}

GdlDockPlaceholder *
gdl_dock_get_placeholder_by_name (GdlDock     *dock,
                                  const gchar *name)
{
    GdlDockObject *found;
    
    g_return_val_if_fail (dock != NULL && name != NULL, NULL);
    
    /* proxy the call to our master */
    found = gdl_dock_master_get_object (GDL_DOCK_OBJECT_GET_MASTER (dock), name);

    return (found && GDL_IS_DOCK_PLACEHOLDER (found)) ?
        GDL_DOCK_PLACEHOLDER (found) : NULL;
}

GList *
gdl_dock_get_named_items (GdlDock *dock)
{
    GList *list = NULL;
    
    g_return_val_if_fail (dock != NULL, NULL);

    gdl_dock_master_foreach (GDL_DOCK_OBJECT_GET_MASTER (dock),
                             (GFunc) _gdl_dock_foreach_build_list, &list);

    return list;
}

GdlDock *
gdl_dock_object_get_toplevel (GdlDockObject *object)
{
    GdlDockObject *parent = object;
    
    g_return_val_if_fail (object != NULL, NULL);

    while (parent && !GDL_IS_DOCK (parent))
        parent = gdl_dock_object_get_parent_object (parent);

    return parent ? GDL_DOCK (parent) : NULL;
}

void
gdl_dock_xor_rect (GdlDock      *dock,
                   GdkRectangle *rect)
{
    GtkWidget *widget;
    GdkWindow *window;
    gint8      dash_list [2];

    widget = GTK_WIDGET (dock);

    if (!dock->_priv->xor_gc) {
        if (gtk_widget_get_realized (widget)) {
            GdkGCValues values;

            values.function = GDK_INVERT;
            values.subwindow_mode = GDK_INCLUDE_INFERIORS;
            dock->_priv->xor_gc = gdk_gc_new_with_values 
                (gtk_widget_get_window (widget), &values, GDK_GC_FUNCTION | GDK_GC_SUBWINDOW);
        } else 
            return;
    };

    gdk_gc_set_line_attributes (dock->_priv->xor_gc, 1,
                                GDK_LINE_ON_OFF_DASH,
                                GDK_CAP_NOT_LAST,
                                GDK_JOIN_BEVEL);

    window = gtk_widget_get_window (widget);

    dash_list [0] = 1;
    dash_list [1] = 1;
    
    gdk_gc_set_dashes (dock->_priv->xor_gc, 1, dash_list, 2);

    gdk_draw_rectangle (window, dock->_priv->xor_gc, FALSE,
                        rect->x, rect->y,
                        rect->width, rect->height);

    gdk_gc_set_dashes (dock->_priv->xor_gc, 0, dash_list, 2);

    gdk_draw_rectangle (window, dock->_priv->xor_gc, FALSE,
                        rect->x + 1, rect->y + 1,
                        rect->width - 2, rect->height - 2);
}

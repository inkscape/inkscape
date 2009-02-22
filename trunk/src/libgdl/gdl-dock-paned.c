/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * gdl-dock-paned.h
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
#include <string.h>
#include <gtk/gtkhpaned.h>
#include <gtk/gtkvpaned.h>

#include "gdl-tools.h"
#include "gdl-dock-paned.h"


/* Private prototypes */

static void     gdl_dock_paned_class_init     (GdlDockPanedClass *klass);
static void     gdl_dock_paned_instance_init  (GdlDockPaned      *paned);
static GObject *gdl_dock_paned_constructor    (GType              type,
                                               guint              n_construct_properties,
                                               GObjectConstructParam *construct_param);
static void     gdl_dock_paned_set_property   (GObject           *object,
                                               guint              prop_id,
                                               const GValue      *value,
                                               GParamSpec        *pspec);
static void     gdl_dock_paned_get_property   (GObject           *object,
                                               guint              prop_id,
                                               GValue            *value,
                                               GParamSpec        *pspec);

static void     gdl_dock_paned_destroy        (GtkObject         *object);

static void     gdl_dock_paned_add            (GtkContainer      *container,
                                               GtkWidget         *widget);
static void     gdl_dock_paned_forall         (GtkContainer      *container,
                                               gboolean           include_internals,
                                               GtkCallback        callback,
                                               gpointer           callback_data);
static GType    gdl_dock_paned_child_type     (GtkContainer      *container);

static gboolean gdl_dock_paned_dock_request   (GdlDockObject     *object, 
                                               gint               x,
                                               gint               y, 
                                               GdlDockRequest    *request);
static void     gdl_dock_paned_dock           (GdlDockObject    *object,
                                               GdlDockObject    *requestor,
                                               GdlDockPlacement  position,
                                               GValue           *other_data);

static void     gdl_dock_paned_set_orientation (GdlDockItem    *item,
                                                GtkOrientation  orientation);

static gboolean gdl_dock_paned_child_placement (GdlDockObject    *object,
                                                GdlDockObject    *child,
                                                GdlDockPlacement *placement);


/* ----- Class variables and definitions ----- */

#define SPLIT_RATIO  0.3

enum {
    PROP_0,
    PROP_POSITION
};


/* ----- Private functions ----- */

GDL_CLASS_BOILERPLATE (GdlDockPaned, gdl_dock_paned, GdlDockItem, GDL_TYPE_DOCK_ITEM);

static void
gdl_dock_paned_class_init (GdlDockPanedClass *klass)
{
    GObjectClass       *g_object_class;
    GtkObjectClass     *gtk_object_class;
    GtkWidgetClass     *widget_class;
    GtkContainerClass  *container_class;
    GdlDockObjectClass *object_class;
    GdlDockItemClass   *item_class;

    g_object_class = G_OBJECT_CLASS (klass);
    gtk_object_class = GTK_OBJECT_CLASS (klass);
    widget_class = GTK_WIDGET_CLASS (klass);
    container_class = GTK_CONTAINER_CLASS (klass);
    object_class = GDL_DOCK_OBJECT_CLASS (klass);
    item_class = GDL_DOCK_ITEM_CLASS (klass);

    g_object_class->set_property = gdl_dock_paned_set_property;
    g_object_class->get_property = gdl_dock_paned_get_property;
    g_object_class->constructor = gdl_dock_paned_constructor;
    
    gtk_object_class->destroy = gdl_dock_paned_destroy;

    container_class->add = gdl_dock_paned_add;
    container_class->forall = gdl_dock_paned_forall;
    container_class->child_type = gdl_dock_paned_child_type;
    
    object_class->is_compound = TRUE;
    
    object_class->dock_request = gdl_dock_paned_dock_request;
    object_class->dock = gdl_dock_paned_dock;
    object_class->child_placement = gdl_dock_paned_child_placement;
    
    item_class->has_grip = FALSE;
    item_class->set_orientation = gdl_dock_paned_set_orientation;    

    g_object_class_install_property (
        g_object_class, PROP_POSITION,
        g_param_spec_uint ("position", _("Position"),
                           _("Position of the divider in pixels"),
                           0, G_MAXINT, 0,
                           G_PARAM_READWRITE |
                           GDL_DOCK_PARAM_EXPORT | GDL_DOCK_PARAM_AFTER));
}

static void
gdl_dock_paned_instance_init (GdlDockPaned *paned)
{
    paned->position_changed = FALSE;
    paned->in_drag = FALSE;
    paned->last_drag_position = -1;
}

static void
gdl_dock_paned_resize_paned_ancestors (GdlDockPaned       *paned, 
                                       GdlDockExpansionDirection  expansion_direction,
                                       gint                diff)
{
     GtkWidget *widget, *last_widget;

     g_return_if_fail (GDL_IS_DOCK_PANED (paned));

     last_widget = GTK_WIDGET (paned);

     /* make sure resizing only can be done in the drag direction */
     if (expansion_direction == GDL_DOCK_EXPANSION_DIRECTION_NONE ||
         ((expansion_direction == GDL_DOCK_EXPANSION_DIRECTION_DOWN || 
           expansion_direction == GDL_DOCK_EXPANSION_DIRECTION_UP) && 
          !GTK_IS_VPANED (GDL_DOCK_ITEM (paned)->child)) ||
         ((expansion_direction == GDL_DOCK_EXPANSION_DIRECTION_LEFT || 
           expansion_direction == GDL_DOCK_EXPANSION_DIRECTION_RIGHT) && 
          !GTK_IS_HPANED (GDL_DOCK_ITEM (paned)->child)))
     {         
         return;
     }

     for (widget = GTK_WIDGET (paned)->parent; widget != NULL; 
          widget = widget->parent) {

         if (GTK_IS_PANED (widget)) {

             GtkPaned *paned = GTK_PANED (widget);

                 if (last_widget == paned->child1) {

                     if (!GDL_IS_DOCK_OBJECT(widget->parent)) {
                         GtkRequisition requisition;
                         GtkAllocation allocation = paned->child1->allocation;

                         gtk_widget_size_request (paned->child1, &requisition);

                         gint new_height =
                             (allocation.height > requisition.height && diff > 0 ?
                              allocation.height : requisition.height) + diff;

                         gtk_widget_set_size_request (paned->child1, -1, new_height);
                     }
                     
                     gtk_paned_set_position (paned, gtk_paned_get_position (paned) + diff);
                 }
             
         } else if (!GDL_IS_DOCK_OBJECT (widget)) {
             break;
         }

         last_widget = widget;
     }
}

static void 
gdl_dock_paned_notify_cb (GObject    *g_object,
                          GParamSpec *pspec,
                          gpointer    user_data) 
{
    GdlDockPaned *paned;

    g_return_if_fail (user_data != NULL && GDL_IS_DOCK_PANED (user_data));
    
    /* chain the notification to the GdlDockPaned */
    g_object_notify (G_OBJECT (user_data), pspec->name);
    
    paned = GDL_DOCK_PANED (user_data);
    
    if (GDL_DOCK_ITEM_USER_ACTION (user_data) && !strcmp (pspec->name, "position")) {

        GdlDockExpansionDirection expansion_direction;

        paned->position_changed = TRUE;

        g_object_get (GDL_DOCK_OBJECT(paned)->master,
                      "expand-direction", &expansion_direction,
                      NULL);

        switch (expansion_direction) {
        case GDL_DOCK_EXPANSION_DIRECTION_DOWN:
            if (paned->in_drag) {
                gint max_position, position, diff;
                g_object_get (GDL_DOCK_ITEM (paned)->child,
                              "max-position", &max_position,
                              "position", &position,
                              NULL);
 
                diff = position - paned->last_drag_position;
                paned->last_drag_position = position;

                if (diff > 0 && position == max_position) {
                    gdl_dock_paned_resize_paned_ancestors (paned, expansion_direction, diff + 1);
                } else {
                    gdl_dock_paned_resize_paned_ancestors (paned, expansion_direction, diff);
                }
                 
            }
            break;
        default:
            ;
        }
    }
}

static gboolean 
gdl_dock_paned_button_cb (GtkWidget      *widget,
                          GdkEventButton *event,
                          gpointer        user_data)
{
    GdlDockPaned *paned;
    
    g_return_val_if_fail (user_data != NULL && GDL_IS_DOCK_PANED (user_data), FALSE);
    
    paned = GDL_DOCK_PANED (user_data);
    if (event->button == 1) {
	if (event->type == GDK_BUTTON_PRESS) {

            GdlDockExpansionDirection expansion_direction;

            GDL_DOCK_ITEM_SET_FLAGS (user_data, GDL_DOCK_USER_ACTION);

            paned->in_drag = TRUE;

            g_object_get (GDL_DOCK_OBJECT (paned)->master,
                          "expand-direction", &expansion_direction,
                          NULL);

            switch (expansion_direction) {
            case GDL_DOCK_EXPANSION_DIRECTION_DOWN:
            {
                gint max_position, position;
                g_object_get (GDL_DOCK_ITEM (paned)->child,
                              "max-position", &max_position,
                              "position", &position,
                              NULL);

                paned->last_drag_position = position;
                /* expand the paned's ancestors a bit if the separator is in max position
                 * to allow dragging in all directions */
                if (position == max_position)
                    gdl_dock_paned_resize_paned_ancestors (paned, expansion_direction, 1);
                break;
            }
            default:
                ;
            }
        }
        else if (event->type == GDK_BUTTON_RELEASE) {
	     paned->last_drag_position = -1;
	     paned->in_drag = FALSE;
        }
        else {
            GDL_DOCK_ITEM_UNSET_FLAGS (user_data, GDL_DOCK_USER_ACTION);
            if (paned->position_changed) {
                /* emit pending layout changed signal to track separator position */
                if (GDL_DOCK_OBJECT (paned)->master)
                    g_signal_emit_by_name (GDL_DOCK_OBJECT (paned)->master, "layout-changed");
                paned->position_changed = FALSE;
            }
        }
    }
    
    return FALSE;
}

static void 
gdl_dock_paned_create_child (GdlDockPaned   *paned,
                             GtkOrientation  orientation) 
{
    GdlDockItem *item;
    
    item = GDL_DOCK_ITEM (paned);
    
    if (item->child)
        gtk_widget_unparent (GTK_WIDGET (item->child));
    
    /* create the container paned */
    if (orientation == GTK_ORIENTATION_HORIZONTAL)
        item->child = gtk_hpaned_new ();
    else
        item->child = gtk_vpaned_new ();
    
    /* get notification for propagation */
    g_signal_connect (item->child, "notify::position",
                      (GCallback) gdl_dock_paned_notify_cb, (gpointer) item);
    g_signal_connect (item->child, "button-press-event",
                      (GCallback) gdl_dock_paned_button_cb, (gpointer) item);
    g_signal_connect (item->child, "button-release-event",
                      (GCallback) gdl_dock_paned_button_cb, (gpointer) item);
    
    gtk_widget_set_parent (item->child, GTK_WIDGET (item));
    gtk_widget_show (item->child);
}

static GObject *
gdl_dock_paned_constructor (GType                  type,
                            guint                  n_construct_properties,
                            GObjectConstructParam *construct_param)
{
    GObject *g_object;
    
    g_object = GDL_CALL_PARENT_WITH_DEFAULT (G_OBJECT_CLASS, 
                                               constructor, 
                                               (type,
                                                n_construct_properties,
                                                construct_param),
                                               NULL);
    if (g_object) {
        GdlDockItem *item = GDL_DOCK_ITEM (g_object);
        
        if (!item->child)
            gdl_dock_paned_create_child (GDL_DOCK_PANED (g_object),
                                         item->orientation);
        /* otherwise, the orientation was set as a construction
           parameter and the child is already created */
    }
    
    return g_object;
}

static void
gdl_dock_paned_set_property (GObject        *object,
                             guint           prop_id,
                             const GValue   *value,
                             GParamSpec     *pspec)
{
    GdlDockItem *item = GDL_DOCK_ITEM (object);
      
    switch (prop_id) {
        case PROP_POSITION:
            if (item->child && GTK_IS_PANED (item->child))
                gtk_paned_set_position (GTK_PANED (item->child),
                                        g_value_get_uint (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
gdl_dock_paned_get_property (GObject        *object,
                             guint           prop_id,
                             GValue         *value,
                             GParamSpec     *pspec)
{
    GdlDockItem *item = GDL_DOCK_ITEM (object);
      
    switch (prop_id) {
        case PROP_POSITION:
            if (item->child && GTK_IS_PANED (item->child))
                g_value_set_uint (value,
                                  gtk_paned_get_position (GTK_PANED (item->child)));
            else
                g_value_set_uint (value, 0);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
gdl_dock_paned_destroy (GtkObject *object)
{
    GdlDockItem *item = GDL_DOCK_ITEM (object);

    /* we need to call the virtual first, since in GdlDockDestroy our
       children dock objects are detached */
    GDL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));

    /* after that we can remove the GtkNotebook */
    if (item->child) {
        gtk_widget_unparent (item->child);
        item->child = NULL;
    };
}

static void
gdl_dock_paned_add (GtkContainer *container,
                    GtkWidget    *widget)
{
    GdlDockItem     *item;
    GtkPaned        *paned;
    GdlDockPlacement pos = GDL_DOCK_NONE;
    
    g_return_if_fail (container != NULL && widget != NULL);
    g_return_if_fail (GDL_IS_DOCK_PANED (container));
    g_return_if_fail (GDL_IS_DOCK_ITEM (widget));

    item = GDL_DOCK_ITEM (container);
    g_return_if_fail (item->child != NULL);
    paned = GTK_PANED (item->child);
    g_return_if_fail (!paned->child1 || !paned->child2);

    if (!paned->child1)
        pos = item->orientation == GTK_ORIENTATION_HORIZONTAL ?
            GDL_DOCK_LEFT : GDL_DOCK_TOP;
    else if (!paned->child2)
        pos = item->orientation == GTK_ORIENTATION_HORIZONTAL ?
            GDL_DOCK_RIGHT : GDL_DOCK_BOTTOM;

    if (pos != GDL_DOCK_NONE)
        gdl_dock_object_dock (GDL_DOCK_OBJECT (container),
                              GDL_DOCK_OBJECT (widget),
                              pos, NULL);
}

static void
gdl_dock_paned_forall (GtkContainer *container,
                       gboolean      include_internals,
                       GtkCallback   callback,
                       gpointer      callback_data)
{
    GdlDockItem *item;

    g_return_if_fail (container != NULL);
    g_return_if_fail (GDL_IS_DOCK_PANED (container));
    g_return_if_fail (callback != NULL);

    if (include_internals) {
        /* use GdlDockItem's forall */
        GDL_CALL_PARENT (GTK_CONTAINER_CLASS, forall, 
                           (container, include_internals, callback, callback_data));
    }
    else {
        item = GDL_DOCK_ITEM (container);
        if (item->child)
            gtk_container_foreach (GTK_CONTAINER (item->child), callback, callback_data);
    }
}

static GType
gdl_dock_paned_child_type (GtkContainer *container)
{
    GdlDockItem *item = GDL_DOCK_ITEM (container);

    if (gtk_container_child_type (GTK_CONTAINER (item->child)) == G_TYPE_NONE)
        return G_TYPE_NONE;
    else
        return GDL_TYPE_DOCK_ITEM;
}

static void
gdl_dock_paned_request_foreach (GdlDockObject *object,
                                gpointer       user_data)
{
    struct {
        gint            x, y;
        GdlDockRequest *request;
        gboolean        may_dock;
    } *data = user_data;
    
    GdlDockRequest my_request;
    gboolean       may_dock;
    
    my_request = *data->request;
    may_dock = gdl_dock_object_dock_request (object, data->x, data->y, &my_request);
    if (may_dock) {
        data->may_dock = TRUE;
        *data->request = my_request;
    }
}

static gboolean
gdl_dock_paned_dock_request (GdlDockObject  *object, 
                             gint            x,
                             gint            y, 
                             GdlDockRequest *request)
{
    GdlDockItem        *item;
    guint               bw;
    gint                rel_x, rel_y;
    GtkAllocation      *alloc;
    gboolean            may_dock = FALSE;
    GdlDockRequest      my_request;

    g_return_val_if_fail (GDL_IS_DOCK_ITEM (object), FALSE);

    /* we get (x,y) in our allocation coordinates system */
    
    item = GDL_DOCK_ITEM (object);
    
    /* Get item's allocation. */
    alloc = &(GTK_WIDGET (object)->allocation);
    bw = GTK_CONTAINER (object)->border_width;

    /* Get coordinates relative to our window. */
    rel_x = x - alloc->x;
    rel_y = y - alloc->y;

    if (request)
        my_request = *request;
        
    /* Check if coordinates are inside the widget. */
    if (rel_x > 0 && rel_x < alloc->width &&
        rel_y > 0 && rel_y < alloc->height) {
        GtkRequisition my, other;
        gint divider = -1;
        
        gdl_dock_item_preferred_size (GDL_DOCK_ITEM (my_request.applicant), &other);
        gdl_dock_item_preferred_size (GDL_DOCK_ITEM (object), &my);

        /* It's inside our area. */
        may_dock = TRUE;

	/* Set docking indicator rectangle to the widget size. */
        my_request.rect.x = bw;
        my_request.rect.y = bw;
        my_request.rect.width = alloc->width - 2*bw;
        my_request.rect.height = alloc->height - 2*bw;

        my_request.target = object;

        /* See if it's in the border_width band. */
        if (rel_x < bw) {
            my_request.position = GDL_DOCK_LEFT;
            my_request.rect.width *= SPLIT_RATIO;
            divider = other.width;
        } else if (rel_x > alloc->width - bw) {
            my_request.position = GDL_DOCK_RIGHT;
            my_request.rect.x += my_request.rect.width * (1 - SPLIT_RATIO);
            my_request.rect.width *= SPLIT_RATIO;
            divider = MAX (0, my.width - other.width);
        } else if (rel_y < bw) {
            my_request.position = GDL_DOCK_TOP;
            my_request.rect.height *= SPLIT_RATIO;
            divider = other.height;
        } else if (rel_y > alloc->height - bw) {
            my_request.position = GDL_DOCK_BOTTOM;
            my_request.rect.y += my_request.rect.height * (1 - SPLIT_RATIO);
            my_request.rect.height *= SPLIT_RATIO;
            divider = MAX (0, my.height - other.height);
            
        } else { /* Otherwise try our children. */
            struct {
                gint            x, y;
                GdlDockRequest *request;
                gboolean        may_dock;
            } data;

            /* give them coordinates in their allocation system... the
               GtkPaned has no window, so our children allocation
               coordinates are our window coordinates */
            data.x = rel_x;
            data.y = rel_y;
            data.request = &my_request;
            data.may_dock = FALSE;
            
            gtk_container_foreach (GTK_CONTAINER (object),
                                   (GtkCallback) gdl_dock_paned_request_foreach,
                                   &data);

            may_dock = data.may_dock;
            if (!may_dock) {
                /* the pointer is on the handle, so snap to top/bottom
                   or left/right */
                may_dock = TRUE;
                if (item->orientation == GTK_ORIENTATION_HORIZONTAL) {
                    if (rel_y < alloc->height / 2) {
                        my_request.position = GDL_DOCK_TOP;
                        my_request.rect.height *= SPLIT_RATIO;
                        divider = other.height;
                    } else {
                        my_request.position = GDL_DOCK_BOTTOM;
                        my_request.rect.y += my_request.rect.height * (1 - SPLIT_RATIO);
                        my_request.rect.height *= SPLIT_RATIO;
                        divider = MAX (0, my.height - other.height);
                    }
                } else {
                    if (rel_x < alloc->width / 2) {
                        my_request.position = GDL_DOCK_LEFT;
                        my_request.rect.width *= SPLIT_RATIO;
                        divider = other.width;
                    } else {
                        my_request.position = GDL_DOCK_RIGHT;
                        my_request.rect.x += my_request.rect.width * (1 - SPLIT_RATIO);
                        my_request.rect.width *= SPLIT_RATIO;
                        divider = MAX (0, my.width - other.width);
                    }
                }
            }
        }

        if (divider >= 0 && my_request.position != GDL_DOCK_CENTER) {
            if (G_IS_VALUE (&my_request.extra))
                g_value_unset (&my_request.extra);
            g_value_init (&my_request.extra, G_TYPE_UINT);
            g_value_set_uint (&my_request.extra, (guint) divider);
        }
        
        if (may_dock) {
            /* adjust returned coordinates so they are relative to
               our allocation */
            my_request.rect.x += alloc->x;
            my_request.rect.y += alloc->y;
        }
    }

    if (may_dock && request)
        *request = my_request;
    
    return may_dock;
}

static void
gdl_dock_paned_dock (GdlDockObject    *object,
                     GdlDockObject    *requestor,
                     GdlDockPlacement  position,
                     GValue           *other_data)
{
    GtkPaned *paned;
    gboolean  done = FALSE;
    gboolean  hresize = FALSE;
    gboolean  wresize = FALSE;
    gint      temp = 0;
    
    g_return_if_fail (GDL_IS_DOCK_PANED (object));
    g_return_if_fail (GDL_DOCK_ITEM (object)->child != NULL);

    paned = GTK_PANED (GDL_DOCK_ITEM (object)->child);

    if (GDL_IS_DOCK_ITEM (requestor)) {
        g_object_get (G_OBJECT (requestor), "preferred_height", &temp, NULL);
        if (temp == -2)
            hresize = TRUE;
        temp = 0;
        g_object_get (G_OBJECT (requestor), "preferred_width", &temp, NULL);
        if (temp == -2)
            wresize = TRUE;
    }

    /* see if we can dock the item in our paned */
    switch (GDL_DOCK_ITEM (object)->orientation) {
        case GTK_ORIENTATION_HORIZONTAL:
            if (!paned->child1 && position == GDL_DOCK_LEFT) {
                gtk_paned_pack1 (paned, GTK_WIDGET (requestor), FALSE, FALSE);
                done = TRUE;
            } else if (!paned->child2 && position == GDL_DOCK_RIGHT) {
                gtk_paned_pack2 (paned, GTK_WIDGET (requestor), TRUE, FALSE);
                done = TRUE;
            }
            break;
        case GTK_ORIENTATION_VERTICAL:
            if (!paned->child1 && position == GDL_DOCK_TOP) {
                gtk_paned_pack1 (paned, GTK_WIDGET (requestor), hresize, FALSE);
                done = TRUE;
            } else if (!paned->child2 && position == GDL_DOCK_BOTTOM) {
                gtk_paned_pack2 (paned, GTK_WIDGET (requestor), TRUE, FALSE);
                done = TRUE;
            }
            break;
        default:
            break;
    }

    if (!done) {
        /* this will create another paned and reparent us there */
        GDL_CALL_PARENT (GDL_DOCK_OBJECT_CLASS, dock, (object, requestor, position,
                                                       other_data));
    }
    else {
        gdl_dock_item_show_grip (GDL_DOCK_ITEM (requestor));
        GDL_DOCK_OBJECT_SET_FLAGS (requestor, GDL_DOCK_ATTACHED);
    }
}

static void
gdl_dock_paned_set_orientation (GdlDockItem    *item,
                                GtkOrientation  orientation)
{
    GtkPaned    *old_paned = NULL, *new_paned;
    GtkWidget   *child1, *child2;
    
    g_return_if_fail (GDL_IS_DOCK_PANED (item));

    if (item->child) {
        old_paned = GTK_PANED (item->child);
        g_object_ref (old_paned);
        gtk_widget_unparent (GTK_WIDGET (old_paned));
        item->child = NULL;
    }
    
    gdl_dock_paned_create_child (GDL_DOCK_PANED (item), orientation);
    
    if (old_paned) {
        new_paned = GTK_PANED (item->child);
        child1 = old_paned->child1;
        child2 = old_paned->child2;
    
        if (child1) {
            g_object_ref (child1);
            gtk_container_remove (GTK_CONTAINER (old_paned), child1);
            gtk_paned_pack1 (new_paned, child1, TRUE, FALSE);
            g_object_unref (child1);
        }
        if (child2) {
            g_object_ref (child2);
            gtk_container_remove (GTK_CONTAINER (old_paned), child2);
            gtk_paned_pack1 (new_paned, child2, TRUE, FALSE);
            g_object_unref (child2);
        }
    }
    
    GDL_CALL_PARENT (GDL_DOCK_ITEM_CLASS, set_orientation, (item, orientation));
}

static gboolean 
gdl_dock_paned_child_placement (GdlDockObject    *object,
                                GdlDockObject    *child,
                                GdlDockPlacement *placement)
{
    GdlDockItem      *item = GDL_DOCK_ITEM (object);
    GtkPaned         *paned;
    GdlDockPlacement  pos = GDL_DOCK_NONE;
    
    if (item->child) {
        paned = GTK_PANED (item->child);
        if (GTK_WIDGET (child) == paned->child1)
            pos = item->orientation == GTK_ORIENTATION_HORIZONTAL ?
                GDL_DOCK_LEFT : GDL_DOCK_TOP;
        else if (GTK_WIDGET (child) == paned->child2)
            pos = item->orientation == GTK_ORIENTATION_HORIZONTAL ?
                GDL_DOCK_RIGHT : GDL_DOCK_BOTTOM;
    }

    if (pos != GDL_DOCK_NONE) {
        if (placement)
            *placement = pos;
        return TRUE;
    }
    else
        return FALSE;
}


/* ----- Public interface ----- */

GtkWidget *
gdl_dock_paned_new (GtkOrientation orientation)
{
    GdlDockPaned *paned;

    paned = GDL_DOCK_PANED (g_object_new (GDL_TYPE_DOCK_PANED,
                                          "orientation", orientation, NULL));
    GDL_DOCK_OBJECT_UNSET_FLAGS (paned, GDL_DOCK_AUTOMATIC);
    
    return GTK_WIDGET (paned);
}

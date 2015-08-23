/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 8 -*- */
/*
 * gdl-dock-item-grip.c
 *
 * Author: Michael Meeks Copyright (C) 2002 Sun Microsystems, Inc.
 *
 * Based on BonoboDockItemGrip.  Original copyright notice follows.
 *
 * Copyright (C) 1998 Ettore Perazzoli
 * Copyright (C) 1998 Elliot Lee
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald 
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gdl-i18n.h"
#include <string.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include "gdl-dock-item.h"
#include "gdl-dock-item-grip.h"
#include "gdl-dock-item-button-image.h"
#include "gdl-switcher.h"

#define ALIGN_BORDER 5
#define DRAG_HANDLE_SIZE 10

enum {
    PROP_0,
    PROP_ITEM
};
 
struct _GdlDockItemGripPrivate {
    GtkWidget   *label;
  
    GtkWidget   *close_button;
    GtkWidget   *iconify_button;
    
    gboolean    handle_shown;
};
 
G_DEFINE_TYPE (GdlDockItemGrip, gdl_dock_item_grip, GTK_TYPE_CONTAINER);
                       
GtkWidget*
gdl_dock_item_create_label_widget(GdlDockItemGrip *grip)
{
    GtkHBox *label_box;
    GtkImage *image;
    GtkLabel *label;
    gchar *stock_id = NULL;
    gchar *title = NULL;
    GdkPixbuf *pixbuf;
  
    label_box = (GtkHBox*)gtk_hbox_new (FALSE, 0);
    
    g_object_get (G_OBJECT (grip->item), "stock-id", &stock_id, NULL);
    g_object_get (G_OBJECT (grip->item), "pixbuf-icon", &pixbuf, NULL);
    if(stock_id) {   
        image = GTK_IMAGE(gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_MENU));
        
        gtk_widget_show (GTK_WIDGET(image));
        gtk_box_pack_start(GTK_BOX(label_box), GTK_WIDGET(image), FALSE, TRUE, 0);
            
        g_free (stock_id);
    }
    else if (pixbuf) {
        image = GTK_IMAGE(gtk_image_new_from_pixbuf (pixbuf));
	    
        gtk_widget_show (GTK_WIDGET(image));
        gtk_box_pack_start(GTK_BOX(label_box), GTK_WIDGET(image), FALSE, TRUE, 0);
    }
        
    g_object_get (G_OBJECT (grip->item), "long-name", &title, NULL);
    if (title) {
        label = GTK_LABEL(gtk_label_new(title));
        gtk_label_set_ellipsize(label, PANGO_ELLIPSIZE_END);
        gtk_label_set_justify(label, GTK_JUSTIFY_LEFT);
        gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);    
        gtk_widget_show (GTK_WIDGET(label));
        
        if (gtk_widget_get_direction (GTK_WIDGET(grip)) == GTK_TEXT_DIR_RTL) {
            gtk_box_pack_end(GTK_BOX(label_box), GTK_WIDGET(label), TRUE, TRUE, 1);
        } else {
            gtk_box_pack_start(GTK_BOX(label_box), GTK_WIDGET(label), TRUE, TRUE, 1);
        }
        
        g_free(title);
    }
    
    return GTK_WIDGET(label_box);
}

static gint
gdl_dock_item_grip_expose (GtkWidget      *widget,
                           GdkEventExpose *event)
{
    GdlDockItemGrip *grip;
    GtkAllocation allocation;
    GdkRectangle handle_area;
    GdkRectangle expose_area;

    grip = GDL_DOCK_ITEM_GRIP (widget);

    if(grip->_priv->handle_shown) {

        gtk_widget_get_allocation (widget, &allocation);

        if (gtk_widget_get_direction (widget) != GTK_TEXT_DIR_RTL) {
            handle_area.x = allocation.x;
            handle_area.y = allocation.y;
            handle_area.width = DRAG_HANDLE_SIZE;
            handle_area.height = allocation.height;
        } else {
            handle_area.x = allocation.x + allocation.width - DRAG_HANDLE_SIZE;
            handle_area.y = allocation.y;
            handle_area.width = DRAG_HANDLE_SIZE;
            handle_area.height = allocation.height;
        }

        if (gdk_rectangle_intersect (&handle_area, &event->area, &expose_area)) {

            gtk_paint_handle (gtk_widget_get_style (widget),
                              gtk_widget_get_window (widget),
                              gtk_widget_get_state (widget),
                              GTK_SHADOW_NONE, &expose_area, widget,
                              "handlebox", handle_area.x, handle_area.y,
                              handle_area.width, handle_area.height,
                              GTK_ORIENTATION_VERTICAL);
        
        }
        
    }
  
/* see bug #950556: may contribute to regression with GTK2/Quartz */
#if !defined(GDK_WINDOWING_QUARTZ)
   if (gdl_dock_item_or_child_has_focus(grip->item)) { 
	   
      gtk_paint_focus (gtk_widget_get_style (widget),
                       gtk_widget_get_window (widget),
                       gtk_widget_get_state (widget),
                       &event->area, widget,
                       NULL, 0, 0, -1, -1);
   }
#endif //GDK_WINDOWING_QUARTZ

    return GTK_WIDGET_CLASS (gdl_dock_item_grip_parent_class)->expose_event (widget, event);
}

static void
gdl_dock_item_grip_item_notify (GObject    *master,
                                GParamSpec *pspec,
                                gpointer    data)
{
    GdlDockItemGrip *grip;
    gboolean cursor;
    
    grip = GDL_DOCK_ITEM_GRIP (data);

    if ((strcmp (pspec->name, "stock-id") == 0) ||
        (strcmp (pspec->name, "long-name") == 0)) {        
        
        gdl_dock_item_grip_set_label (grip,
          gdl_dock_item_create_label_widget(grip));
 
    } else if (strcmp (pspec->name, "behavior") == 0) {
        cursor = FALSE;
        if (grip->_priv->close_button) {
            if (GDL_DOCK_ITEM_CANT_CLOSE (grip->item)) {
                gtk_widget_hide (GTK_WIDGET (grip->_priv->close_button));
            } else {
                gtk_widget_show (GTK_WIDGET (grip->_priv->close_button));
                cursor = TRUE;
            }
        }
        if (grip->_priv->iconify_button) {
            if (GDL_DOCK_ITEM_CANT_ICONIFY (grip->item)) {
                gtk_widget_hide (GTK_WIDGET (grip->_priv->iconify_button));
            } else {
                gtk_widget_show (GTK_WIDGET (grip->_priv->iconify_button));
                cursor = TRUE;
            }
        }
        if (grip->title_window && !cursor)
            gdk_window_set_cursor (grip->title_window, NULL);

    }
}

static void
gdl_dock_item_grip_destroy (GtkObject *object)
{
    GdlDockItemGrip *grip = GDL_DOCK_ITEM_GRIP (object);
        
    if (grip->_priv) {
        GdlDockItemGripPrivate *priv = grip->_priv;

        if (priv->label) {
            gtk_widget_unparent(grip->_priv->label);
            priv->label = NULL;
        }

        if (grip->item)
            g_signal_handlers_disconnect_by_func (grip->item,
                                                  gdl_dock_item_grip_item_notify,
                                                  grip);
        grip->item = NULL;

        grip->_priv = NULL;
        g_free (priv);
    }
    
    GTK_OBJECT_CLASS (gdl_dock_item_grip_parent_class)->destroy (object);
}

static void
gdl_dock_item_grip_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    GdlDockItemGrip *grip;

    g_return_if_fail (GDL_IS_DOCK_ITEM_GRIP (object));

    grip = GDL_DOCK_ITEM_GRIP (object);
    
    switch (prop_id) {
        case PROP_ITEM:
            grip->item = g_value_get_object (value);
            if (grip->item) {
                g_signal_connect (grip->item, "notify::long-name",
                                  G_CALLBACK (gdl_dock_item_grip_item_notify),
                                  grip);
                g_signal_connect (grip->item, "notify::stock-id",
                                  G_CALLBACK (gdl_dock_item_grip_item_notify),
                                  grip);
                g_signal_connect (grip->item, "notify::behavior",
                                  G_CALLBACK (gdl_dock_item_grip_item_notify),
                                  grip);

                if (!GDL_DOCK_ITEM_CANT_CLOSE (grip->item) && grip->_priv->close_button)
                    gtk_widget_show (grip->_priv->close_button);
                if (!GDL_DOCK_ITEM_CANT_ICONIFY (grip->item) && grip->_priv->iconify_button)
                    gtk_widget_show (grip->_priv->iconify_button);
            }
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
gdl_dock_item_grip_close_clicked (GtkWidget       *widget,
                                  GdlDockItemGrip *grip)
{
    (void)widget;
    g_return_if_fail (grip->item != NULL);

    gdl_dock_item_hide_item (grip->item);
}

static void
gdl_dock_item_grip_fix_iconify_button (GdlDockItemGrip *grip)
{
    GtkWidget *iconify_button = grip->_priv->iconify_button;
    GdkWindow *window = NULL;
    GdkEvent  *event = NULL;

    GdkModifierType modifiers;
    gint x = 0, y = 0;
    gboolean ev_ret;

    g_return_if_fail (gtk_widget_get_realized (iconify_button));

    window = gtk_widget_get_parent_window (iconify_button);
    event = gdk_event_new (GDK_LEAVE_NOTIFY);

    g_assert (GDK_IS_WINDOW (window));
    gdk_window_get_pointer (window, &x, &y, &modifiers);

    event->crossing.window = g_object_ref (window);
    event->crossing.send_event = FALSE;
    event->crossing.subwindow = g_object_ref (window);
    event->crossing.time = GDK_CURRENT_TIME;
    event->crossing.x = x;
    event->crossing.y = y;
    event->crossing.x_root = event->crossing.y_root = 0;
    event->crossing.mode = GDK_CROSSING_STATE_CHANGED;
    event->crossing.detail = GDK_NOTIFY_NONLINEAR;
    event->crossing.focus = FALSE;
    event->crossing.state = modifiers;

    //GTK_BUTTON (iconify_button)->in_button = FALSE;
    g_signal_emit_by_name (iconify_button, "leave-notify-event",
                           event, &ev_ret, 0);

    gdk_event_free (event);
}

static void
gdl_dock_item_grip_iconify_clicked (GtkWidget       *widget,
                                    GdlDockItemGrip *grip)
{
    GtkWidget *parent;
    
    g_return_if_fail (grip->item != NULL);

    /* Workaround to unhighlight the iconify button. */
    gdl_dock_item_grip_fix_iconify_button (grip);

    parent = gtk_widget_get_parent (GTK_WIDGET (grip->item));
    if (GDL_IS_SWITCHER (parent))
    {
        /* Note: We can not use gtk_container_foreach (parent) here because
         * during iconificatoin, the internal children changes in parent.
         * Instead we keep a list of items to iconify and iconify them
         * one by one.
         */
        GList *node;
        GList *items =
            gtk_container_get_children (GTK_CONTAINER (parent));
        for (node = items; node != NULL; node = node->next)
        {
            GdlDockItem *item = GDL_DOCK_ITEM (node->data);
            if (!GDL_DOCK_ITEM_CANT_ICONIFY (item))
                gdl_dock_item_iconify_item (item);
        }
        g_list_free (items);
    }
    else
    {
        gdl_dock_item_iconify_item (grip->item);
    }
}
  
static void
gdl_dock_item_grip_init (GdlDockItemGrip *grip)
{
    GtkWidget *image;

    gtk_widget_set_has_window (GTK_WIDGET (grip), FALSE);
    
    grip->_priv = g_new0 (GdlDockItemGripPrivate, 1);
    grip->_priv->label = NULL;
    grip->_priv->handle_shown = FALSE;
    
    /* create the close button */
    gtk_widget_push_composite_child ();
    grip->_priv->close_button = gtk_button_new ();
    gtk_widget_pop_composite_child ();

    gtk_widget_set_can_focus (grip->_priv->close_button, FALSE);
    gtk_widget_set_parent (grip->_priv->close_button, GTK_WIDGET (grip));
    gtk_button_set_relief (GTK_BUTTON (grip->_priv->close_button), GTK_RELIEF_NONE);
    gtk_widget_show (grip->_priv->close_button);

    image = gdl_dock_item_button_image_new(GDL_DOCK_ITEM_BUTTON_IMAGE_CLOSE);
    gtk_container_add (GTK_CONTAINER (grip->_priv->close_button), image);
    gtk_widget_show (image);

    g_signal_connect (G_OBJECT (grip->_priv->close_button), "clicked",
                      G_CALLBACK (gdl_dock_item_grip_close_clicked), grip);

    /* create the iconify button */
    gtk_widget_push_composite_child ();
    grip->_priv->iconify_button = gtk_button_new ();
    gtk_widget_pop_composite_child ();

    gtk_widget_set_can_focus (grip->_priv->iconify_button, FALSE);
    gtk_widget_set_parent (grip->_priv->iconify_button, GTK_WIDGET (grip));
    gtk_button_set_relief (GTK_BUTTON (grip->_priv->iconify_button), GTK_RELIEF_NONE);
    gtk_widget_show (grip->_priv->iconify_button);

    image = gdl_dock_item_button_image_new(GDL_DOCK_ITEM_BUTTON_IMAGE_ICONIFY);
    gtk_container_add (GTK_CONTAINER (grip->_priv->iconify_button), image);
    gtk_widget_show (image);

    g_signal_connect (G_OBJECT (grip->_priv->iconify_button), "clicked",
                      G_CALLBACK (gdl_dock_item_grip_iconify_clicked), grip);

    /* set tooltips on the buttons */
    gtk_widget_set_tooltip_text (grip->_priv->iconify_button,
                          _("Iconify this dock"));
    gtk_widget_set_tooltip_text (grip->_priv->close_button,
                          _("Close this dock"));
}

static void
gdl_dock_item_grip_realize (GtkWidget *widget)
{
    GdlDockItemGrip *grip = GDL_DOCK_ITEM_GRIP (widget);

    GTK_WIDGET_CLASS (gdl_dock_item_grip_parent_class)->realize (widget);

    g_return_if_fail (grip->_priv != NULL);
    
    if (!grip->title_window) {
        GtkAllocation  allocation;
        GdkWindowAttr  attributes;
        GdkCursor     *cursor;

        g_return_if_fail (grip->_priv->label != NULL);

        gtk_widget_get_allocation (grip->_priv->label, &allocation);

        attributes.x           = allocation.x;
        attributes.y           = allocation.y;
        attributes.width       = allocation.width;
        attributes.height      = allocation.height;
        attributes.window_type = GDK_WINDOW_CHILD;
        attributes.wclass      = GDK_INPUT_OUTPUT;
        attributes.event_mask  = GDK_ALL_EVENTS_MASK;

        grip->title_window = gdk_window_new (gtk_widget_get_parent_window (widget),
                                             &attributes, (GDK_WA_X | GDK_WA_Y));

        gdk_window_set_user_data (grip->title_window, grip);

        /* Unref the ref from parent realize for NO_WINDOW */
        g_object_unref (gtk_widget_get_window (widget));

        /* Need to ref widget->window, because parent unrealize unrefs it */
        gtk_widget_set_window (widget, g_object_ref (grip->title_window));
        gtk_widget_set_has_window (widget, TRUE);

        /* Unset the background so as to make the colour match the parent window */
        gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, NULL);
 
        if (GDL_DOCK_ITEM_CANT_CLOSE (grip->item) &&
            GDL_DOCK_ITEM_CANT_ICONIFY (grip->item))
            cursor = NULL;
        else 
            cursor = gdk_cursor_new_for_display (gtk_widget_get_display (widget),
                                             GDK_HAND2);
        gdk_window_set_cursor (grip->title_window, cursor);
        if (cursor)
            gdk_cursor_unref (cursor);
    }
}

static void
gdl_dock_item_grip_unrealize (GtkWidget *widget)
{
    GdlDockItemGrip *grip = GDL_DOCK_ITEM_GRIP (widget);

    if (grip->title_window) {
        gtk_widget_set_has_window (widget, FALSE);
        gdk_window_set_user_data (grip->title_window, NULL);
        gdk_window_destroy (grip->title_window);
        grip->title_window = NULL;
    }

    GTK_WIDGET_CLASS (gdl_dock_item_grip_parent_class)->unrealize (widget);
}

static void
gdl_dock_item_grip_map (GtkWidget *widget)
{
    GdlDockItemGrip *grip = GDL_DOCK_ITEM_GRIP (widget);

    GTK_WIDGET_CLASS (gdl_dock_item_grip_parent_class)->map (widget);

    if (grip->title_window)
        gdk_window_show (grip->title_window);
}

static void
gdl_dock_item_grip_unmap (GtkWidget *widget)
{
    GdlDockItemGrip *grip = GDL_DOCK_ITEM_GRIP (widget);

    if (grip->title_window)
        gdk_window_hide (grip->title_window);

    GTK_WIDGET_CLASS (gdl_dock_item_grip_parent_class)->unmap (widget);
}

static void
gdl_dock_item_grip_size_request (GtkWidget      *widget,
                                 GtkRequisition *requisition)
{
    GtkRequisition   child_requisition;
    GdlDockItemGrip *grip;
    gint             layout_height = 0;
    guint            border_width;

    g_return_if_fail (GDL_IS_DOCK_ITEM_GRIP (widget));
    g_return_if_fail (requisition != NULL);

    border_width  = gtk_container_get_border_width (GTK_CONTAINER (widget));
    grip = GDL_DOCK_ITEM_GRIP (widget);
    
    requisition->width = border_width * 2/* + ALIGN_BORDER*/;
    requisition->height = border_width * 2;

    if(grip->_priv->handle_shown)
        requisition->width += DRAG_HANDLE_SIZE;

    gtk_widget_size_request (grip->_priv->close_button, &child_requisition);
    layout_height = MAX (layout_height, child_requisition.height);
    if (gtk_widget_get_visible (grip->_priv->close_button)) {
       requisition->width += child_requisition.width;
    }
    
    gtk_widget_size_request (grip->_priv->iconify_button, &child_requisition);
    layout_height = MAX (layout_height, child_requisition.height);
    if (gtk_widget_get_visible (grip->_priv->iconify_button)) {
        requisition->width += child_requisition.width;
    }
   
    gtk_widget_size_request (grip->_priv->label, &child_requisition);
    requisition->width += child_requisition.width;
    layout_height = MAX (layout_height, child_requisition.height);
    
    requisition->height += layout_height;
}

static void
gdl_dock_item_grip_size_allocate (GtkWidget     *widget,
                                  GtkAllocation *allocation)
{
    GdlDockItemGrip *grip;
    GtkRequisition   close_requisition = { 0, 0 };
    GtkRequisition   iconify_requisition = { 0, 0 };
    GtkAllocation    child_allocation;
    guint            border_width;

    g_return_if_fail (GDL_IS_DOCK_ITEM_GRIP (widget));
    g_return_if_fail (allocation != NULL);
  
    grip = GDL_DOCK_ITEM_GRIP (widget);
    border_width = gtk_container_get_border_width (GTK_CONTAINER (widget));

    GTK_WIDGET_CLASS (gdl_dock_item_grip_parent_class)->size_allocate (widget, allocation);

    gtk_widget_size_request (grip->_priv->close_button,
        &close_requisition);
    gtk_widget_size_request (grip->_priv->iconify_button,
        &iconify_requisition);
    
    /* Calculate the Minimum Width where buttons will fit */
    int min_width = close_requisition.width + iconify_requisition.width
        + border_width * 2;
    if(grip->_priv->handle_shown)
      min_width += DRAG_HANDLE_SIZE;
    const gboolean space_for_buttons = (allocation->width >= min_width);
        
    /* Set up the rolling child_allocation rectangle */
    if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
        child_allocation.x = border_width/* + ALIGN_BORDER*/;
    else
        child_allocation.x = allocation->width - border_width;
    child_allocation.y = border_width;

    /* Layout Close Button */
    if (gtk_widget_get_visible (grip->_priv->close_button)) {

        if(space_for_buttons) {
            if (gtk_widget_get_direction (widget) != GTK_TEXT_DIR_RTL)
                child_allocation.x -= close_requisition.width;
        
            child_allocation.width = close_requisition.width;
            child_allocation.height = close_requisition.height;
        } else {
            child_allocation.width = 0;
        }
        
        gtk_widget_size_allocate (grip->_priv->close_button, &child_allocation);

        if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
            child_allocation.x += close_requisition.width;
    }    

    /* Layout Iconify Button */
    if (gtk_widget_get_visible (grip->_priv->iconify_button)) {

        if(space_for_buttons) {
            if (gtk_widget_get_direction (widget) != GTK_TEXT_DIR_RTL)
                child_allocation.x -= iconify_requisition.width;

            child_allocation.width = iconify_requisition.width;
            child_allocation.height = iconify_requisition.height;
        } else {
            child_allocation.width = 0;
        }

        gtk_widget_size_allocate (grip->_priv->iconify_button, &child_allocation);

        if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
            child_allocation.x += iconify_requisition.width;
    }
    
    /* Layout the Grip Handle*/
    if (gtk_widget_get_direction (widget) != GTK_TEXT_DIR_RTL) {
        child_allocation.width = child_allocation.x;
        child_allocation.x = border_width/* + ALIGN_BORDER*/;
        
        if(grip->_priv->handle_shown) {
            child_allocation.x += DRAG_HANDLE_SIZE;
            child_allocation.width -= DRAG_HANDLE_SIZE;
        }
        
    } else {
        child_allocation.width = allocation->width -
            (child_allocation.x - allocation->x)/* - ALIGN_BORDER*/;
            
        if(grip->_priv->handle_shown)
            child_allocation.width -= DRAG_HANDLE_SIZE;
    }
    
    if(child_allocation.width < 0)
      child_allocation.width = 0;
    
    child_allocation.y = border_width;
    child_allocation.height = allocation->height - border_width * 2;
    if(grip->_priv->label) {
      gtk_widget_size_allocate (grip->_priv->label, &child_allocation);
    }
     
    if (grip->title_window) {
        gdk_window_move_resize (grip->title_window,
                                allocation->x,
                                allocation->y,
                                allocation->width,
                                allocation->height);
    }
}

static void 
gdl_dock_item_grip_add (GtkContainer *container,
                        GtkWidget    *widget)
{
    g_warning ("gtk_container_add not implemented for GdlDockItemGrip");
}

static void  
gdl_dock_item_grip_remove (GtkContainer *container,
                           GtkWidget    *widget)
{
    gdl_dock_item_grip_set_label (GDL_DOCK_ITEM_GRIP (container), NULL);
}

static void
gdl_dock_item_grip_forall (GtkContainer *container,
                           gboolean      include_internals,
                           GtkCallback   callback,
                           gpointer      callback_data)
{
    GdlDockItemGrip *grip;
    
    g_return_if_fail (GDL_IS_DOCK_ITEM_GRIP (container));
    grip = GDL_DOCK_ITEM_GRIP (container);
    
    if (grip->_priv) {
        if(grip->_priv->label) {
            (* callback) (grip->_priv->label, callback_data);
        }

        if (include_internals) {
            (* callback) (grip->_priv->close_button, callback_data);
            (* callback) (grip->_priv->iconify_button, callback_data);
        }
    }
}

static GType
gdl_dock_item_grip_child_type (GtkContainer *container)
{
    return G_TYPE_NONE;
}

static void
gdl_dock_item_grip_class_init (GdlDockItemGripClass *klass)
{
    GObjectClass *gobject_class;
    GtkObjectClass *gtk_object_class;
    GtkWidgetClass *widget_class;
    GtkContainerClass *container_class;

    gobject_class = G_OBJECT_CLASS (klass);
    gtk_object_class = GTK_OBJECT_CLASS (klass);
    widget_class = GTK_WIDGET_CLASS (klass);
    container_class = GTK_CONTAINER_CLASS (klass);

    gobject_class->set_property = gdl_dock_item_grip_set_property;

    gtk_object_class->destroy = gdl_dock_item_grip_destroy;

    widget_class->expose_event = gdl_dock_item_grip_expose;
    widget_class->realize = gdl_dock_item_grip_realize;
    widget_class->unrealize = gdl_dock_item_grip_unrealize;
    widget_class->map = gdl_dock_item_grip_map;
    widget_class->unmap = gdl_dock_item_grip_unmap;
    widget_class->size_request = gdl_dock_item_grip_size_request;
    widget_class->size_allocate = gdl_dock_item_grip_size_allocate;

    container_class->add = gdl_dock_item_grip_add;
    container_class->remove = gdl_dock_item_grip_remove;
    container_class->forall = gdl_dock_item_grip_forall;
    container_class->child_type = gdl_dock_item_grip_child_type;

    g_object_class_install_property (
        gobject_class, PROP_ITEM,
        g_param_spec_object ("item", _("Controlling dock item"),
                             _("Dockitem which 'owns' this grip"),
                             GDL_TYPE_DOCK_ITEM,
                             G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void
gdl_dock_item_grip_showhide_handle (GdlDockItemGrip *grip)
{
    gtk_widget_queue_resize (GTK_WIDGET (grip));
}

/* ----- Public interface ----- */

/**
 * gdl_dock_item_grip_new:
 * @item: The dock item that will "own" this grip widget.
 * 
 * Creates a new GDL dock item grip object.
 * Returns: The newly created dock item grip widget.
 **/
GtkWidget *
gdl_dock_item_grip_new (GdlDockItem *item)
{
    GdlDockItemGrip *grip = g_object_new (GDL_TYPE_DOCK_ITEM_GRIP, "item", item,
                                          NULL);

    return GTK_WIDGET (grip);
}

/**
 * gdl_dock_item_grip_set_label:
 * @grip: The grip that will get it's label widget set.
 * @label: The widget that will become the label.
 * 
 * Replaces the current label widget with another widget.
 **/
void
gdl_dock_item_grip_set_label (GdlDockItemGrip *grip,
                              GtkWidget *label)
{
    g_return_if_fail (grip != NULL);

    if (grip->_priv->label) {
        gtk_widget_unparent(grip->_priv->label);
        g_object_unref (grip->_priv->label);
        grip->_priv->label = NULL;
    }
    
    if (label) {
        g_object_ref (label);
        gtk_widget_set_parent (label, GTK_WIDGET (grip));
        gtk_widget_show (label);
        grip->_priv->label = label;
    }
}
/**
 * gdl_dock_item_grip_hide_handle:
 * @grip: The dock item grip to hide the handle of.
 * 
 * This function hides the dock item's grip widget handle hatching.
 **/
void 
gdl_dock_item_grip_hide_handle (GdlDockItemGrip *grip)
{
    g_return_if_fail (grip != NULL);
    if (grip->_priv->handle_shown) {
        grip->_priv->handle_shown = FALSE;
        gdl_dock_item_grip_showhide_handle (grip);
    };
}

/**
 * gdl_dock_item_grip_show_handle:
 * @grip: The dock item grip to show the handle of.
 * 
 * This function shows the dock item's grip widget handle hatching.
 **/
void
gdl_dock_item_grip_show_handle (GdlDockItemGrip *grip)
{
    g_return_if_fail (grip != NULL);
    if (!grip->_priv->handle_shown) {
        grip->_priv->handle_shown = TRUE;
        gdl_dock_item_grip_showhide_handle (grip);
    };
}

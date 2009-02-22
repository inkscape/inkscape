/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- 
 *
 * This file is part of the GNOME Devtools Libraries.
 *
 * Copyright (C) 2003 Jeroen Zwartepoorte <jeroen@xs4all.nl>
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

#include "gdl-tools.h"
#include "gdl-dock.h"
#include "gdl-dock-master.h"
#include "gdl-dock-bar.h"
#include "libgdltypebuiltins.h"

enum {
    PROP_0,
    PROP_MASTER,
    PROP_DOCKBAR_STYLE
};

/* ----- Private prototypes ----- */

static void  gdl_dock_bar_class_init      (GdlDockBarClass *klass);
static void  gdl_dock_bar_instance_init   (GdlDockBar      *dockbar);

static void  gdl_dock_bar_get_property    (GObject         *object,
                                           guint            prop_id,
                                           GValue          *value,
                                           GParamSpec      *pspec);
static void  gdl_dock_bar_set_property    (GObject         *object,
                                           guint            prop_id,
                                           const GValue    *value,
                                           GParamSpec      *pspec);

static void  gdl_dock_bar_destroy         (GtkObject       *object);

static void  gdl_dock_bar_attach          (GdlDockBar      *dockbar,
                                           GdlDockMaster   *master);
static void gdl_dock_bar_remove_item      (GdlDockBar      *dockbar,
                                           GdlDockItem     *item);

/* ----- Class variables and definitions ----- */

struct _GdlDockBarPrivate {
    GdlDockMaster   *master;
    GSList          *items;
    GtkTooltips     *tooltips;
    GtkOrientation   orientation;
    GdlDockBarStyle  dockbar_style;
};

/* ----- Private functions ----- */

GDL_CLASS_BOILERPLATE (GdlDockBar, gdl_dock_bar, GtkBox, GTK_TYPE_BOX)

static void gdl_dock_bar_size_request (GtkWidget *widget,
		                       GtkRequisition *requisition );
static void gdl_dock_bar_size_allocate (GtkWidget *widget,
		                       GtkAllocation *allocation );
static void gdl_dock_bar_size_vrequest (GtkWidget *widget,
		                       GtkRequisition *requisition );
static void gdl_dock_bar_size_vallocate (GtkWidget *widget,
		                       GtkAllocation *allocation );
static void gdl_dock_bar_size_hrequest (GtkWidget *widget,
		                       GtkRequisition *requisition );
static void gdl_dock_bar_size_hallocate (GtkWidget *widget,
		                       GtkAllocation *allocation );
static void update_dock_items (GdlDockBar *dockbar, gboolean full_update);

void
gdl_dock_bar_class_init (GdlDockBarClass *klass)
{
    GObjectClass       *g_object_class;
    GtkObjectClass     *gtk_object_class;
    GtkWidgetClass     *widget_class;
    
    g_object_class = G_OBJECT_CLASS (klass);
    gtk_object_class = GTK_OBJECT_CLASS (klass);

    g_object_class->get_property = gdl_dock_bar_get_property;
    g_object_class->set_property = gdl_dock_bar_set_property;

    gtk_object_class->destroy = gdl_dock_bar_destroy;

    g_object_class_install_property (
        g_object_class, PROP_MASTER,
        g_param_spec_object ("master", _("Master"),
                             _("GdlDockMaster object which the dockbar widget "
                               "is attached to"),
                             GDL_TYPE_DOCK_MASTER, 
                             G_PARAM_READWRITE));

    g_object_class_install_property (
        g_object_class, PROP_DOCKBAR_STYLE,
        g_param_spec_enum ("dockbar-style", _("Dockbar style"),
                           _("Dockbar style to show items on it"),
                           GDL_TYPE_DOCK_BAR_STYLE,
                           GDL_DOCK_BAR_BOTH,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    widget_class = GTK_WIDGET_CLASS (klass);
    widget_class->size_request = gdl_dock_bar_size_request;
    widget_class->size_allocate = gdl_dock_bar_size_allocate;
}

static void
gdl_dock_bar_instance_init (GdlDockBar *dockbar)
{
    dockbar->_priv = g_new0 (GdlDockBarPrivate, 1);
    dockbar->_priv->master = NULL;
    dockbar->_priv->items = NULL;
    dockbar->_priv->tooltips = gtk_tooltips_new ();
    dockbar->_priv->orientation = GTK_ORIENTATION_VERTICAL;
    dockbar->_priv->dockbar_style = GDL_DOCK_BAR_BOTH;
    g_object_ref (dockbar->_priv->tooltips);
    gtk_object_sink (GTK_OBJECT (dockbar->_priv->tooltips));
}

static void
gdl_dock_bar_get_property (GObject         *object,
                           guint            prop_id,
                           GValue          *value,
                           GParamSpec      *pspec)
{
    GdlDockBar *dockbar = GDL_DOCK_BAR (object);

    switch (prop_id) {
        case PROP_MASTER:
            g_value_set_object (value, dockbar->_priv->master);
            break;
        case PROP_DOCKBAR_STYLE:
            g_value_set_enum (value, dockbar->_priv->dockbar_style);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    };
}

static void
gdl_dock_bar_set_property (GObject         *object,
                           guint            prop_id,
                           const GValue    *value,
                           GParamSpec      *pspec)
{
    GdlDockBar *dockbar = GDL_DOCK_BAR (object);

    switch (prop_id) {
        case PROP_MASTER:
            gdl_dock_bar_attach (dockbar, g_value_get_object (value));
            break;
        case PROP_DOCKBAR_STYLE:
            dockbar->_priv->dockbar_style = g_value_get_enum (value);
            update_dock_items (dockbar, TRUE);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    };
}

static void
on_dock_item_foreach_disconnect (GdlDockItem *item, GdlDockBar *dock_bar)
{
    g_signal_handlers_disconnect_by_func (item, gdl_dock_bar_remove_item,
                                          dock_bar);
}

static void
gdl_dock_bar_destroy (GtkObject *object)
{
    GdlDockBar *dockbar = GDL_DOCK_BAR (object);

    if (dockbar->_priv) {
        GdlDockBarPrivate *priv = dockbar->_priv;

        if (priv->items) {
            g_slist_foreach (priv->items,
                             (GFunc) on_dock_item_foreach_disconnect,
                             object);
            g_slist_free (priv->items);
        }
        
        if (priv->master) {
            g_signal_handlers_disconnect_matched (priv->master,
                                                  G_SIGNAL_MATCH_DATA,
                                                  0, 0, NULL, NULL, dockbar);
            g_object_unref (priv->master);
            priv->master = NULL;
        }

        if (priv->tooltips) {
            g_object_unref (priv->tooltips);
            priv->tooltips = NULL;
        }
        
        dockbar->_priv = NULL;

        g_free (priv);
    }
    
    GDL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

static void
gdl_dock_bar_remove_item (GdlDockBar  *dockbar,
                          GdlDockItem *item)
{
    GdlDockBarPrivate *priv;
    GtkWidget *button;

    g_return_if_fail (GDL_IS_DOCK_BAR (dockbar));
    g_return_if_fail (GDL_IS_DOCK_ITEM (item));

    priv = dockbar->_priv;

    if (g_slist_index (priv->items, item) == -1) {
        g_warning ("Item has not been added to the dockbar");
        return;
    }
    
    priv->items = g_slist_remove (priv->items, item);
    
    button = g_object_get_data (G_OBJECT (item), "GdlDockBarButton");
    g_assert (button != NULL);
    gtk_container_remove (GTK_CONTAINER (dockbar), button);
    g_object_set_data (G_OBJECT (item), "GdlDockBarButton", NULL);
    g_signal_handlers_disconnect_by_func (item,
                                          G_CALLBACK (gdl_dock_bar_remove_item),
                                          dockbar);
}

static void
gdl_dock_bar_item_clicked (GtkWidget   *button,
                           GdlDockItem *item)
{
    GdlDockBar *dockbar;
    GdlDockObject *controller;

    g_return_if_fail (item != NULL);
    
    dockbar = g_object_get_data (G_OBJECT (item), "GdlDockBar");
    g_assert (dockbar != NULL);
    g_object_set_data (G_OBJECT (item), "GdlDockBar", NULL);

    controller = gdl_dock_master_get_controller (GDL_DOCK_OBJECT_GET_MASTER (item));

    GDL_DOCK_OBJECT_UNSET_FLAGS (item, GDL_DOCK_ICONIFIED);
    gdl_dock_item_show_item (item);
    gdl_dock_bar_remove_item (dockbar, item);
    gtk_widget_queue_resize (GTK_WIDGET (controller));
}

static void
gdl_dock_bar_add_item (GdlDockBar  *dockbar,
                       GdlDockItem *item)
{
    GdlDockBarPrivate *priv;
    GtkWidget *button;
    gchar *stock_id;
    gchar *name;
    GdkPixbuf *pixbuf_icon;
    GtkWidget *image, *box, *label;

    g_return_if_fail (GDL_IS_DOCK_BAR (dockbar));
    g_return_if_fail (GDL_IS_DOCK_ITEM (item));

    priv = dockbar->_priv;

    if (g_slist_index (priv->items, item) != -1) {
        g_warning ("Item has already been added to the dockbar");
        return;
    }

    priv->items = g_slist_append (priv->items, item);
    
    /* Create a button for the item. */
    button = gtk_button_new ();
    gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
    
    if (dockbar->_priv->orientation == GTK_ORIENTATION_HORIZONTAL)
        box = gtk_hbox_new (FALSE, 0);
    else
        box = gtk_vbox_new (FALSE, 0);
    
    g_object_get (item, "stock-id", &stock_id, "pixbuf-icon", &pixbuf_icon,
                  "long-name", &name, NULL);

    if (dockbar->_priv->dockbar_style == GDL_DOCK_BAR_TEXT ||
        dockbar->_priv->dockbar_style == GDL_DOCK_BAR_BOTH) {
        label = gtk_label_new (name);
        if (dockbar->_priv->orientation == GTK_ORIENTATION_VERTICAL)
            gtk_label_set_angle (GTK_LABEL (label), 90);
        gtk_box_pack_start_defaults (GTK_BOX (box), label);
    }
    
    /* FIXME: For now AUTO behaves same as BOTH */
    
    if (dockbar->_priv->dockbar_style == GDL_DOCK_BAR_ICONS ||
        dockbar->_priv->dockbar_style == GDL_DOCK_BAR_BOTH ||
        dockbar->_priv->dockbar_style == GDL_DOCK_BAR_AUTO) {
        if (stock_id) {
            image = gtk_image_new_from_stock (stock_id,
                                              GTK_ICON_SIZE_SMALL_TOOLBAR);
            g_free (stock_id);
        } else if (pixbuf_icon) {
            image = gtk_image_new_from_pixbuf (pixbuf_icon);
        } else {
            image = gtk_image_new_from_stock (GTK_STOCK_NEW,
                                              GTK_ICON_SIZE_SMALL_TOOLBAR);
        }
        gtk_box_pack_start_defaults (GTK_BOX (box), image);
    }
    
    gtk_container_add (GTK_CONTAINER (button), box);
    gtk_box_pack_start (GTK_BOX (dockbar), button, FALSE, FALSE, 0);

    gtk_tooltips_set_tip (priv->tooltips, button, name, name);
    g_free (name);

    g_object_set_data (G_OBJECT (item), "GdlDockBar", dockbar);
    g_object_set_data (G_OBJECT (item), "GdlDockBarButton", button);
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (gdl_dock_bar_item_clicked), item);

    gtk_widget_show_all (button);
    
    /* Set up destroy notify */
    g_signal_connect_swapped (item, "destroy",
                              G_CALLBACK (gdl_dock_bar_remove_item),
                              dockbar);
}

static void
build_list (GdlDockObject *object, GList **list)
{
    /* add only items, not toplevels */
    if (GDL_IS_DOCK_ITEM (object))
        *list = g_list_prepend (*list, object);
}

static void
update_dock_items (GdlDockBar *dockbar, gboolean full_update)
{
    GdlDockMaster *master;
    GList *items, *l;

    g_return_if_fail (dockbar != NULL);
    
    if (!dockbar->_priv->master)
        return;

    master = dockbar->_priv->master;
    
    /* build items list */
    items = NULL;
    gdl_dock_master_foreach (master, (GFunc) build_list, &items);
    
    if (!full_update) {
        for (l = items; l != NULL; l = l->next) {
            GdlDockItem *item = GDL_DOCK_ITEM (l->data);
            
            if (g_slist_index (dockbar->_priv->items, item) != -1 &&
                !GDL_DOCK_ITEM_ICONIFIED (item))
                gdl_dock_bar_remove_item (dockbar, item);
            else if (g_slist_index (dockbar->_priv->items, item) == -1 &&
                GDL_DOCK_ITEM_ICONIFIED (item))
                gdl_dock_bar_add_item (dockbar, item);
        }
    } else {
        for (l = items; l != NULL; l = l->next) {
            GdlDockItem *item = GDL_DOCK_ITEM (l->data);
            
            if (g_slist_index (dockbar->_priv->items, item) != -1)
                gdl_dock_bar_remove_item (dockbar, item);
            if (GDL_DOCK_ITEM_ICONIFIED (item))
                gdl_dock_bar_add_item (dockbar, item);
        }
    }
    g_list_free (items);
}

static void
gdl_dock_bar_layout_changed_cb (GdlDockMaster *master,
                                GdlDockBar    *dockbar)
{
    update_dock_items (dockbar, FALSE);
}

static void
gdl_dock_bar_attach (GdlDockBar    *dockbar,
                     GdlDockMaster *master)
{
    g_return_if_fail (dockbar != NULL);
    g_return_if_fail (master == NULL || GDL_IS_DOCK_MASTER (master));
    
    if (dockbar->_priv->master) {
        g_signal_handlers_disconnect_matched (dockbar->_priv->master,
                                              G_SIGNAL_MATCH_DATA,
                                              0, 0, NULL, NULL, dockbar);
        g_object_unref (dockbar->_priv->master);
    }
    
    dockbar->_priv->master = master;
    if (dockbar->_priv->master) {
        g_object_ref (dockbar->_priv->master);
        g_signal_connect (dockbar->_priv->master, "layout-changed",
                          G_CALLBACK (gdl_dock_bar_layout_changed_cb),
                          dockbar);
    }

    update_dock_items (dockbar, FALSE);
}

static void gdl_dock_bar_size_request (GtkWidget *widget,
		                       GtkRequisition *requisition )
{
    GdlDockBar *dockbar;

    dockbar = GDL_DOCK_BAR (widget);
    
    /* default to vertical for unknown values */
    switch (dockbar->_priv->orientation) {
	case GTK_ORIENTATION_HORIZONTAL:
		gdl_dock_bar_size_hrequest (widget, requisition);
		break;
	case GTK_ORIENTATION_VERTICAL:
	default:
		gdl_dock_bar_size_vrequest (widget, requisition);
		break;
    }
}

static void gdl_dock_bar_size_allocate (GtkWidget *widget,
		                       GtkAllocation *allocation )
{
    GdlDockBar *dockbar;

    dockbar = GDL_DOCK_BAR (widget);
    
    /* default to vertical for unknown values */
    switch (dockbar->_priv->orientation) {
	case GTK_ORIENTATION_HORIZONTAL:
		gdl_dock_bar_size_hallocate (widget, allocation);
		break;
	case GTK_ORIENTATION_VERTICAL:
	default:
		gdl_dock_bar_size_vallocate (widget, allocation);
		break;
    }
}

static void gdl_dock_bar_size_vrequest (GtkWidget *widget,
		                       GtkRequisition *requisition )
{
  GtkBox *box;
  GtkBoxChild *child;
  GtkRequisition child_requisition;
  GList *children;
  gint nvis_children;
  gint height;

  box = GTK_BOX (widget);
  requisition->width = 0;
  requisition->height = 0;
  nvis_children = 0;

  children = box->children;
  while (children)
    {
      child = children->data;
      children = children->next;

      if (GTK_WIDGET_VISIBLE (child->widget))
	{
	  gtk_widget_size_request (child->widget, &child_requisition);

	  if (box->homogeneous)
	    {
	      height = child_requisition.height + child->padding * 2;
	      requisition->height = MAX (requisition->height, height);
	    }
	  else
	    {
	      requisition->height += child_requisition.height + child->padding * 2;
	    }

	  requisition->width = MAX (requisition->width, child_requisition.width);

	  nvis_children += 1;
	}
    }

  if (nvis_children > 0)
    {
      if (box->homogeneous)
	requisition->height *= nvis_children;
      requisition->height += (nvis_children - 1) * box->spacing;
    }

  requisition->width += GTK_CONTAINER (box)->border_width * 2;
  requisition->height += GTK_CONTAINER (box)->border_width * 2;

}

static void gdl_dock_bar_size_vallocate (GtkWidget     *widget,
			GtkAllocation *allocation)
{
  GtkBox *box;
  GtkBoxChild *child;
  GList *children;
  GtkAllocation child_allocation;
  gint nvis_children;
  gint nexpand_children;
  gint child_height;
  gint height;
  gint extra;
  gint y;

  box = GTK_BOX (widget);
  widget->allocation = *allocation;

  nvis_children = 0;
  nexpand_children = 0;
  children = box->children;

  while (children)
    {
      child = children->data;
      children = children->next;

      if (GTK_WIDGET_VISIBLE (child->widget))
	{
	  nvis_children += 1;
	  if (child->expand)
	    nexpand_children += 1;
	}
    }

  if (nvis_children > 0)
    {
      if (box->homogeneous)
	{
	  height = (allocation->height -
		   GTK_CONTAINER (box)->border_width * 2 -
		   (nvis_children - 1) * box->spacing);
	  extra = height / nvis_children;
	}
      else if (nexpand_children > 0)
	{
	  height = (gint) allocation->height - (gint) widget->requisition.height;
	  extra = height / nexpand_children;
	}
      else
	{
	  height = 0;
	  extra = 0;
	}

      y = allocation->y + GTK_CONTAINER (box)->border_width;
      child_allocation.x = allocation->x + GTK_CONTAINER (box)->border_width;
      child_allocation.width = MAX (1, (gint) allocation->width - (gint) GTK_CONTAINER (box)->border_width * 2);

      children = box->children;
      while (children)
	{
	  child = children->data;
	  children = children->next;

	  if ((child->pack == GTK_PACK_START) && GTK_WIDGET_VISIBLE (child->widget))
	    {
	      if (box->homogeneous)
		{
		  if (nvis_children == 1)
		    child_height = height;
		  else
		    child_height = extra;

		  nvis_children -= 1;
		  height -= extra;
		}
	      else
		{
		  GtkRequisition child_requisition;

		  gtk_widget_get_child_requisition (child->widget, &child_requisition);
		  child_height = child_requisition.height + child->padding * 2;

		  if (child->expand)
		    {
		      if (nexpand_children == 1)
			child_height += height;
		      else
			child_height += extra;

		      nexpand_children -= 1;
		      height -= extra;
		    }
		}

	      if (child->fill)
		{
		  child_allocation.height = MAX (1, child_height - (gint)child->padding * 2);
		  child_allocation.y = y + child->padding;
		}
	      else
		{
		  GtkRequisition child_requisition;

		  gtk_widget_get_child_requisition (child->widget, &child_requisition);
		  child_allocation.height = child_requisition.height;
		  child_allocation.y = y + (child_height - child_allocation.height) / 2;
		}

	      gtk_widget_size_allocate (child->widget, &child_allocation);

	      y += child_height + box->spacing;
	    }
	}

      y = allocation->y + allocation->height - GTK_CONTAINER (box)->border_width;

      children = box->children;
      while (children)
	{
	  child = children->data;
	  children = children->next;

	  if ((child->pack == GTK_PACK_END) && GTK_WIDGET_VISIBLE (child->widget))
	    {
	      GtkRequisition child_requisition;
	      gtk_widget_get_child_requisition (child->widget, &child_requisition);

              if (box->homogeneous)
                {
                  if (nvis_children == 1)
                    child_height = height;
                  else
                    child_height = extra;

                  nvis_children -= 1;
                  height -= extra;
                }
              else
                {
		  child_height = child_requisition.height + child->padding * 2;

                  if (child->expand)
                    {
                      if (nexpand_children == 1)
                        child_height += height;
                      else
                        child_height += extra;

                      nexpand_children -= 1;
                      height -= extra;
                    }
                }

              if (child->fill)
                {
                  child_allocation.height = MAX (1, child_height - (gint)child->padding * 2);
                  child_allocation.y = y + child->padding - child_height;
                }
              else
                {
		  child_allocation.height = child_requisition.height;
                  child_allocation.y = y + (child_height - child_allocation.height) / 2 - child_height;
                }

              gtk_widget_size_allocate (child->widget, &child_allocation);

              y -= (child_height + box->spacing);
	    }
	}
    }
}

static void gdl_dock_bar_size_hrequest (GtkWidget *widget,
		                       GtkRequisition *requisition )
{
  GtkBox *box;
  GtkBoxChild *child;
  GList *children;
  gint nvis_children;
  gint width;

  box = GTK_BOX (widget);
  requisition->width = 0;
  requisition->height = 0;
  nvis_children = 0;

  children = box->children;
  while (children)
    {
      child = children->data;
      children = children->next;

      if (GTK_WIDGET_VISIBLE (child->widget))
	{
	  GtkRequisition child_requisition;

	  gtk_widget_size_request (child->widget, &child_requisition);

	  if (box->homogeneous)
	    {
	      width = child_requisition.width + child->padding * 2;
	      requisition->width = MAX (requisition->width, width);
	    }
	  else
	    {
	      requisition->width += child_requisition.width + child->padding * 2;
	    }

	  requisition->height = MAX (requisition->height, child_requisition.height);

	  nvis_children += 1;
	}
    }

  if (nvis_children > 0)
    {
      if (box->homogeneous)
	requisition->width *= nvis_children;
      requisition->width += (nvis_children - 1) * box->spacing;
    }

  requisition->width += GTK_CONTAINER (box)->border_width * 2;
  requisition->height += GTK_CONTAINER (box)->border_width * 2;
}

static void gdl_dock_bar_size_hallocate (GtkWidget     *widget,
			GtkAllocation *allocation)
{
 GtkBox *box;
  GtkBoxChild *child;
  GList *children;
  GtkAllocation child_allocation;
  gint nvis_children;
  gint nexpand_children;
  gint child_width;
  gint width;
  gint extra;
  gint x;
  GtkTextDirection direction;

  box = GTK_BOX (widget);
  widget->allocation = *allocation;

  direction = gtk_widget_get_direction (widget);
  
  nvis_children = 0;
  nexpand_children = 0;
  children = box->children;

  while (children)
    {
      child = children->data;
      children = children->next;

      if (GTK_WIDGET_VISIBLE (child->widget))
	{
	  nvis_children += 1;
	  if (child->expand)
	    nexpand_children += 1;
	}
    }

  if (nvis_children > 0)
    {
      if (box->homogeneous)
	{
	  width = (allocation->width -
		   GTK_CONTAINER (box)->border_width * 2 -
		   (nvis_children - 1) * box->spacing);
	  extra = width / nvis_children;
	}
      else if (nexpand_children > 0)
	{
	  width = (gint) allocation->width - (gint) widget->requisition.width;
	  extra = width / nexpand_children;
	}
      else
	{
	  width = 0;
	  extra = 0;
	}

      x = allocation->x + GTK_CONTAINER (box)->border_width;
      child_allocation.y = allocation->y + GTK_CONTAINER (box)->border_width;
      child_allocation.height = MAX (1, (gint) allocation->height - (gint) GTK_CONTAINER (box)->border_width * 2);

      children = box->children;
      while (children)
	{
	  child = children->data;
	  children = children->next;

	  if ((child->pack == GTK_PACK_START) && GTK_WIDGET_VISIBLE (child->widget))
	    {
	      if (box->homogeneous)
		{
		  if (nvis_children == 1)
		    child_width = width;
		  else
		    child_width = extra;

		  nvis_children -= 1;
		  width -= extra;
		}
	      else
		{
		  GtkRequisition child_requisition;

		  gtk_widget_get_child_requisition (child->widget, &child_requisition);

		  child_width = child_requisition.width + child->padding * 2;

		  if (child->expand)
		    {
		      if (nexpand_children == 1)
			child_width += width;
		      else
			child_width += extra;

		      nexpand_children -= 1;
		      width -= extra;
		    }
		}

	      if (child->fill)
		{
		  child_allocation.width = MAX (1, (gint) child_width - (gint) child->padding * 2);
		  child_allocation.x = x + child->padding;
		}
	      else
		{
		  GtkRequisition child_requisition;

		  gtk_widget_get_child_requisition (child->widget, &child_requisition);
		  child_allocation.width = child_requisition.width;
		  child_allocation.x = x + (child_width - child_allocation.width) / 2;
		}

	      if (direction == GTK_TEXT_DIR_RTL)
		child_allocation.x = allocation->x + allocation->width - (child_allocation.x - allocation->x) - child_allocation.width;

	      gtk_widget_size_allocate (child->widget, &child_allocation);

	      x += child_width + box->spacing;
	    }
	}

      x = allocation->x + allocation->width - GTK_CONTAINER (box)->border_width;

      children = box->children;
      while (children)
	{
	  child = children->data;
	  children = children->next;

	  if ((child->pack == GTK_PACK_END) && GTK_WIDGET_VISIBLE (child->widget))
	    {
	      GtkRequisition child_requisition;
	      gtk_widget_get_child_requisition (child->widget, &child_requisition);

              if (box->homogeneous)
                {
                  if (nvis_children == 1)
                    child_width = width;
                  else
                    child_width = extra;

                  nvis_children -= 1;
                  width -= extra;
                }
              else
                {
		  child_width = child_requisition.width + child->padding * 2;

                  if (child->expand)
                    {
                      if (nexpand_children == 1)
                        child_width += width;
                      else
                        child_width += extra;

                      nexpand_children -= 1;
                      width -= extra;
                    }
                }

              if (child->fill)
                {
                  child_allocation.width = MAX (1, (gint)child_width - (gint)child->padding * 2);
                  child_allocation.x = x + child->padding - child_width;
                }
              else
                {
		  child_allocation.width = child_requisition.width;
                  child_allocation.x = x + (child_width - child_allocation.width) / 2 - child_width;
                }

	      if (direction == GTK_TEXT_DIR_RTL)
		child_allocation.x = allocation->x + allocation->width - (child_allocation.x - allocation->x) - child_allocation.width;

              gtk_widget_size_allocate (child->widget, &child_allocation);

              x -= (child_width + box->spacing);
	    }
	}
    }
}

GtkWidget *
gdl_dock_bar_new (GdlDock *dock)
{
    GdlDockMaster *master = NULL;
    
    /* get the master of the given dock */
    if (dock)
        master = GDL_DOCK_OBJECT_GET_MASTER (dock);

    return g_object_new (GDL_TYPE_DOCK_BAR,
                         "master", master, NULL);
}

GtkOrientation gdl_dock_bar_get_orientation (GdlDockBar *dockbar)
{
    g_return_val_if_fail (GDL_IS_DOCK_BAR (dockbar),
                          GTK_ORIENTATION_VERTICAL);

    return dockbar->_priv->orientation;
}

void gdl_dock_bar_set_orientation (GdlDockBar *dockbar,
	                             GtkOrientation orientation)
{
    g_return_if_fail (GDL_IS_DOCK_BAR (dockbar));

    dockbar->_priv->orientation = orientation;

    gtk_widget_queue_resize (GTK_WIDGET (dockbar));
}

void gdl_dock_bar_set_style(GdlDockBar* dockbar,
			    GdlDockBarStyle style)
{
    g_object_set(G_OBJECT(dockbar), "dockbar-style", style, NULL);
}

GdlDockBarStyle gdl_dock_bar_get_style(GdlDockBar* dockbar)
{
    GdlDockBarStyle style;
    g_object_get(G_OBJECT(dockbar), "dockbar-style", &style, NULL);
    return style;
}

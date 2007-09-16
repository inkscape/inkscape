/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 8 -*- */
/**
 * gdl-dock-item-grip.c
 *
 * Based on bonobo-dock-item-grip.  Original copyright notice follows.
 *
 * Author:
 *    Michael Meeks
 *
 * Copyright (C) 2002 Sun Microsystems, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gdl-i18n.h"
#include <string.h>
#include <glib-object.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtktooltips.h>
#include <gtk/gtkimage.h>
#include "gdl-dock-item.h"
#include "gdl-dock-item-grip.h"
#include "gdl-stock.h"
#include "gdl-tools.h"

#define ALIGN_BORDER 5

enum {
    PROP_0,
    PROP_ITEM
};
 
struct _GdlDockItemGripPrivate {
    GtkWidget   *close_button;
    GtkWidget   *iconify_button;
    GtkTooltips *tooltips;

    gboolean     icon_pixbuf_valid;
    GdkPixbuf   *icon_pixbuf;

    gchar       *title;
    PangoLayout *title_layout;
};
 
GDL_CLASS_BOILERPLATE (GdlDockItemGrip, gdl_dock_item_grip,
		       GtkContainer, GTK_TYPE_CONTAINER);

/* must be called after size_allocate */
static void
gdl_dock_item_grip_get_title_area (GdlDockItemGrip *grip,
                                   GdkRectangle    *area)
{
    GtkWidget *widget = GTK_WIDGET (grip);
    gint       border = GTK_CONTAINER (grip)->border_width;
    gint       alloc_height;

    area->width = (widget->allocation.width - 2 * border - ALIGN_BORDER);
    
    pango_layout_get_pixel_size (grip->_priv->title_layout, NULL, &alloc_height);
    
    alloc_height = MAX (grip->_priv->close_button->allocation.height, alloc_height);
    alloc_height = MAX (grip->_priv->iconify_button->allocation.height, alloc_height);
    if (GTK_WIDGET_VISIBLE (grip->_priv->close_button)) {
        area->width -= grip->_priv->close_button->allocation.width;
    }
    if (GTK_WIDGET_VISIBLE (grip->_priv->iconify_button)) {
        area->width -= grip->_priv->iconify_button->allocation.width;
    }

    area->x      = widget->allocation.x + border + ALIGN_BORDER;
    area->y      = widget->allocation.y + border;
    area->height = alloc_height;

    if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
        area->x += (widget->allocation.width - 2 * border) - area->width;
}

static void
ensure_title_and_icon_pixbuf (GdlDockItemGrip *grip)
{
    gchar *stock_id;
    GdkPixbuf *pixbuf;
    
    g_return_if_fail (GDL_IS_DOCK_ITEM_GRIP (grip));
    
    /* get long name property from the dock object */
    if (!grip->_priv->title) {
        g_object_get (G_OBJECT (grip->item), "long-name", &grip->_priv->title, NULL);
        if (!grip->_priv->title)
            grip->_priv->title = g_strdup ("");
    }

    /* retrieve stock pixbuf, if any */
    if (!grip->_priv->icon_pixbuf_valid) {
        g_object_get (G_OBJECT (grip->item), "stock-id", &stock_id, NULL);
        
        if (stock_id) {
            grip->_priv->icon_pixbuf = gtk_widget_render_icon (GTK_WIDGET (grip),
                                                               stock_id,
                                                               GTK_ICON_SIZE_MENU, "");
            g_free (stock_id);
            grip->_priv->icon_pixbuf_valid = TRUE;
        }
    }

    /* retrieve pixbuf icon, if any */
    if (!grip->_priv->icon_pixbuf_valid) {
        g_object_get (G_OBJECT (grip->item), "pixbuf-icon", &pixbuf, NULL);
        
        if (pixbuf) {
            grip->_priv->icon_pixbuf = pixbuf;
            grip->_priv->icon_pixbuf_valid = TRUE;
        }
    }

    /* create layout: the actual text is reset at size_allocate */
    if (!grip->_priv->title_layout) {
        grip->_priv->title_layout = gtk_widget_create_pango_layout (GTK_WIDGET (grip),
                                                                    grip->_priv->title);
        pango_layout_set_single_paragraph_mode (grip->_priv->title_layout, TRUE);
    }
}

static gint
gdl_dock_item_grip_expose (GtkWidget      *widget,
			   GdkEventExpose *event)
{
    GdlDockItemGrip *grip;
    GdkRectangle     title_area;
    GdkRectangle     expose_area;
    GtkStyle        *bg_style;
    gint             layout_width;
    gint             layout_height;
    gint             text_x;
    gint             text_y;
    gboolean         item_or_child_has_focus;

    grip = GDL_DOCK_ITEM_GRIP (widget);
    gdl_dock_item_grip_get_title_area (grip, &title_area);

    /* draw background, highlight it if the dock item or any of its
     * descendants have focus */
    bg_style = (gdl_dock_item_or_child_has_focus (grip->item) ?
                gtk_widget_get_style (widget)->dark_gc[widget->state] :
                gtk_widget_get_style (widget)->mid_gc[widget->state]);

    gdk_draw_rectangle (GDK_DRAWABLE (widget->window), bg_style, TRUE,
                        1, 0, widget->allocation.width - 1, widget->allocation.height);

    if (grip->_priv->icon_pixbuf) {
        GdkRectangle pixbuf_rect;
        
        pixbuf_rect.width = gdk_pixbuf_get_width (grip->_priv->icon_pixbuf);
        pixbuf_rect.height = gdk_pixbuf_get_height (grip->_priv->icon_pixbuf);
        if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL) {
            pixbuf_rect.x = title_area.x + title_area.width - pixbuf_rect.width;
        } else {
            pixbuf_rect.x = title_area.x;
            title_area.x += pixbuf_rect.width + 1;
        }
        /* shrink title area by the pixbuf width plus a 1px spacing */
        title_area.width -= pixbuf_rect.width + 1;
        pixbuf_rect.y = title_area.y + (title_area.height - pixbuf_rect.height) / 2;

        if (gdk_rectangle_intersect (&event->area, &pixbuf_rect, &expose_area)) {
            GdkGC *gc;
            GtkStyle *style;

            style = gtk_widget_get_style (widget);
            gc = style->bg_gc[widget->state];
            gdk_draw_pixbuf (GDK_DRAWABLE (widget->window), gc,
                             grip->_priv->icon_pixbuf,
                             0, 0, pixbuf_rect.x, pixbuf_rect.y,
                             pixbuf_rect.width, pixbuf_rect.height,
                             GDK_RGB_DITHER_NONE, 0, 0);
	}
    }

    if (gdk_rectangle_intersect (&title_area, &event->area, &expose_area)) {
        pango_layout_get_pixel_size (grip->_priv->title_layout, &layout_width,
                                     &layout_height);

        if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
            text_x = title_area.x + title_area.width - layout_width;
        else
            text_x = title_area.x;

        text_y = title_area.y + (title_area.height - layout_height) / 2;

        gtk_paint_layout (widget->style, widget->window, widget->state, TRUE,
                          &expose_area, widget, NULL, text_x, text_y,
                          grip->_priv->title_layout);
    }

    return GTK_WIDGET_CLASS (parent_class)->expose_event (widget, event);
}  

static void
gdl_dock_item_grip_item_notify (GObject    *master,
                                GParamSpec *pspec,
                                gpointer    data)
{
    GdlDockItemGrip *grip;
    gboolean cursor;
    
    grip = GDL_DOCK_ITEM_GRIP (data);

    if (strcmp (pspec->name, "stock-id") == 0) {
        if (grip->_priv->icon_pixbuf) {
            g_object_unref (grip->_priv->icon_pixbuf);
            grip->_priv->icon_pixbuf = NULL;
        }
        grip->_priv->icon_pixbuf_valid = FALSE;
        ensure_title_and_icon_pixbuf (grip);

    } else if (strcmp (pspec->name, "long-name") == 0) {
        g_free (grip->_priv->title);
        g_object_unref (grip->_priv->title_layout);
        grip->_priv->title_layout = NULL;
        grip->_priv->title = NULL;
        ensure_title_and_icon_pixbuf (grip);
	gtk_widget_queue_draw (GTK_WIDGET (grip));
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

        if (priv->title_layout) {
            g_object_unref (priv->title_layout);
            priv->title_layout = NULL;
        }
        g_free (priv->title);
        priv->title = NULL;

        if (priv->icon_pixbuf) {
            g_object_unref (priv->icon_pixbuf);
            priv->icon_pixbuf = NULL;
        }

        if (priv->tooltips) {
            g_object_unref (priv->tooltips);
            priv->tooltips = NULL;
        }

        if (grip->item)
            g_signal_handlers_disconnect_by_func (grip->item,
                                                  gdl_dock_item_grip_item_notify,
                                                  grip);
        grip->item = NULL;

        grip->_priv = NULL;
        g_free (priv);
    }

    GDL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
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
                g_signal_connect (grip->item, "notify::long_name",
                                  G_CALLBACK (gdl_dock_item_grip_item_notify),
                                  grip);
                g_signal_connect (grip->item, "notify::stock_id",
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
    g_return_if_fail (grip->item != NULL);

    gdl_dock_item_hide_item (grip->item);
}

static void
gdl_dock_item_grip_iconify_clicked (GtkWidget       *widget,
                                    GdlDockItemGrip *grip)
{
    g_return_if_fail (grip->item != NULL);

    gdl_dock_item_iconify_item (grip->item);
    
    /* Workaround to unhighlight the iconify button. */
    GTK_BUTTON (grip->_priv->iconify_button)->in_button = FALSE;
    gtk_button_leave (GTK_BUTTON (grip->_priv->iconify_button));
}
  
static void
gdl_dock_item_grip_instance_init (GdlDockItemGrip *grip)
{
    GtkWidget *image;

    GTK_WIDGET_SET_FLAGS (grip, GTK_NO_WINDOW);
    
    grip->_priv = g_new0 (GdlDockItemGripPrivate, 1);
    grip->_priv->icon_pixbuf_valid = FALSE;
    grip->_priv->icon_pixbuf = NULL;
    grip->_priv->title_layout = NULL;

    gtk_widget_push_composite_child ();
    grip->_priv->close_button = gtk_button_new ();
    gtk_widget_pop_composite_child ();
    
    GTK_WIDGET_UNSET_FLAGS (grip->_priv->close_button, GTK_CAN_FOCUS);
    gtk_widget_set_parent (grip->_priv->close_button, GTK_WIDGET (grip));
    gtk_button_set_relief (GTK_BUTTON (grip->_priv->close_button), GTK_RELIEF_NONE);
    gtk_widget_show (grip->_priv->close_button);

    image = gtk_image_new_from_stock (GDL_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
    gtk_container_add (GTK_CONTAINER (grip->_priv->close_button), image);
    gtk_widget_show (image);

    g_signal_connect (G_OBJECT (grip->_priv->close_button), "clicked",
                      G_CALLBACK (gdl_dock_item_grip_close_clicked), grip);

    gtk_widget_push_composite_child ();
    grip->_priv->iconify_button = gtk_button_new ();
    gtk_widget_pop_composite_child ();
    
    GTK_WIDGET_UNSET_FLAGS (grip->_priv->iconify_button, GTK_CAN_FOCUS);
    gtk_widget_set_parent (grip->_priv->iconify_button, GTK_WIDGET (grip));
    gtk_button_set_relief (GTK_BUTTON (grip->_priv->iconify_button), GTK_RELIEF_NONE);
    gtk_widget_show (grip->_priv->iconify_button);

    image = gtk_image_new_from_stock (GDL_STOCK_MENU_RIGHT, GTK_ICON_SIZE_MENU);
    gtk_container_add (GTK_CONTAINER (grip->_priv->iconify_button), image);
    gtk_widget_show (image);

    g_signal_connect (G_OBJECT (grip->_priv->iconify_button), "clicked",
                      G_CALLBACK (gdl_dock_item_grip_iconify_clicked), grip);

    grip->_priv->tooltips = gtk_tooltips_new ();
    g_object_ref (grip->_priv->tooltips);
    gtk_object_sink (GTK_OBJECT (grip->_priv->tooltips));
    gtk_tooltips_set_tip (grip->_priv->tooltips, grip->_priv->iconify_button,
                          _("Iconify"), _("Iconify this dock"));
    gtk_tooltips_set_tip (grip->_priv->tooltips, grip->_priv->close_button,
                          _("Close"), _("Close this dock"));
}

static void
gdl_dock_item_grip_realize (GtkWidget *widget)
{
    GdlDockItemGrip *grip = GDL_DOCK_ITEM_GRIP (widget);

    GTK_WIDGET_CLASS (parent_class)->realize (widget);

    if (!grip->title_window) {
        GdkWindowAttr  attributes;
        GdkRectangle   area;
        GdkCursor     *cursor;

        ensure_title_and_icon_pixbuf (grip);
        gdl_dock_item_grip_get_title_area (grip, &area);

        attributes.x                 = area.x;
        attributes.y                 = area.y;
        attributes.width             = area.width;
        attributes.height            = area.height;
        attributes.window_type       = GDK_WINDOW_TEMP;
        attributes.wclass            = GDK_INPUT_ONLY;
        attributes.override_redirect = TRUE;
        attributes.event_mask        = (GDK_BUTTON_PRESS_MASK   |
                                        GDK_BUTTON_RELEASE_MASK |
                                        GDK_BUTTON_MOTION_MASK  |
                                        gtk_widget_get_events (widget));

        grip->title_window = gdk_window_new (gtk_widget_get_parent_window (widget),
                                             &attributes,
                                             (GDK_WA_X |
                                              GDK_WA_Y |
                                              GDK_WA_NOREDIR));

        gdk_window_set_user_data (grip->title_window, widget);
 
        if (GDL_DOCK_ITEM_CANT_CLOSE (grip->item))
	    cursor = NULL;
	else if (GDL_DOCK_ITEM_CANT_ICONIFY (grip->item))
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
        gdk_window_set_user_data (grip->title_window, NULL);
        gdk_window_destroy (grip->title_window);
        grip->title_window = NULL;
    }

    GTK_WIDGET_CLASS (parent_class)->unrealize (widget);
}

static void
gdl_dock_item_grip_map (GtkWidget *widget)
{
    GdlDockItemGrip *grip = GDL_DOCK_ITEM_GRIP (widget);

    GTK_WIDGET_CLASS (parent_class)->map (widget);

    if (grip->title_window)
        gdk_window_show (grip->title_window);
}

static void
gdl_dock_item_grip_unmap (GtkWidget *widget)
{
    GdlDockItemGrip *grip = GDL_DOCK_ITEM_GRIP (widget);

    if (grip->title_window)
        gdk_window_hide (grip->title_window);

    GTK_WIDGET_CLASS (parent_class)->unmap (widget);
}

static void
gdl_dock_item_grip_size_request (GtkWidget      *widget,
                                 GtkRequisition *requisition)
{
    GtkRequisition   child_requisition;
    GtkContainer    *container;
    GdlDockItemGrip *grip;
    gint             layout_height;

    g_return_if_fail (GDL_IS_DOCK_ITEM_GRIP (widget));
    g_return_if_fail (requisition != NULL);

    container = GTK_CONTAINER (widget);
    grip = GDL_DOCK_ITEM_GRIP (widget);
    
    requisition->width = container->border_width * 2 + ALIGN_BORDER;
    requisition->height = container->border_width * 2;

    ensure_title_and_icon_pixbuf (grip);
    pango_layout_get_pixel_size (grip->_priv->title_layout, NULL, &layout_height);

    gtk_widget_size_request (grip->_priv->close_button, &child_requisition);

    requisition->width += child_requisition.width;
    layout_height = MAX (layout_height, child_requisition.height);
    
    gtk_widget_size_request (grip->_priv->iconify_button, &child_requisition);

    requisition->width += child_requisition.width;
    layout_height = MAX (layout_height, child_requisition.height);
    
    requisition->height += layout_height;

    if (grip->_priv->icon_pixbuf) {
        requisition->width += gdk_pixbuf_get_width (grip->_priv->icon_pixbuf) + 1;
    }
}

#define ELLIPSIS "..."

static void
ellipsize_layout (PangoLayout *layout, gint width)
{
    PangoLayoutLine *line;
    PangoLayout *ell;
    gint h, w, ell_w, x;
    GString *text;
    
    if (width <= 0) {
        pango_layout_set_text (layout, "", -1);
        return;
    }
    
    pango_layout_get_pixel_size (layout, &w, &h);
    if (w <= width) return;
    
    /* calculate ellipsis width */
    ell = pango_layout_copy (layout);
    pango_layout_set_text (ell, ELLIPSIS, -1);
    pango_layout_get_pixel_size (ell, &ell_w, NULL);
    g_object_unref (ell);

    if (width < ell_w) {
        /* not even ellipsis fits, so hide the text */
        pango_layout_set_text (layout, "", -1);
        return;
    }

    /* shrink total available width by the width of the ellipsis */
    width -= ell_w;
    line = pango_layout_get_line (layout, 0);
    text = g_string_new (pango_layout_get_text (layout));
    if (pango_layout_line_x_to_index (line, width * PANGO_SCALE, &x, NULL)) {
        g_string_set_size (text, x);
        g_string_append (text, ELLIPSIS);
        pango_layout_set_text (layout, text->str, -1);
    }
    g_string_free (text, TRUE);
}

static void
gdl_dock_item_grip_size_allocate (GtkWidget     *widget,
                                  GtkAllocation *allocation)
{
    GdlDockItemGrip *grip;
    GtkContainer    *container;
    GtkRequisition   button_requisition = { 0, };
    GtkAllocation    child_allocation;

    g_return_if_fail (GDL_IS_DOCK_ITEM_GRIP (widget));
    g_return_if_fail (allocation != NULL);
  
    grip = GDL_DOCK_ITEM_GRIP (widget);
    container = GTK_CONTAINER (widget);

    GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);

    if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
        child_allocation.x = allocation->x + container->border_width + ALIGN_BORDER;
    else
        child_allocation.x = allocation->x + allocation->width - container->border_width;
    child_allocation.y = allocation->y + container->border_width;

    gtk_widget_size_request (grip->_priv->close_button, &button_requisition);

    if (gtk_widget_get_direction (widget) != GTK_TEXT_DIR_RTL)
        child_allocation.x -= button_requisition.width;

    child_allocation.width = button_requisition.width;
    child_allocation.height = button_requisition.height;

    gtk_widget_size_allocate (grip->_priv->close_button, &child_allocation);

    if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
        child_allocation.x += button_requisition.width;
    

    gtk_widget_size_request (grip->_priv->iconify_button, &button_requisition);

    if (gtk_widget_get_direction (widget) != GTK_TEXT_DIR_RTL)
        child_allocation.x -= button_requisition.width;

    child_allocation.width = button_requisition.width;
    child_allocation.height = button_requisition.height;

    gtk_widget_size_allocate (grip->_priv->iconify_button, &child_allocation);

    if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
        child_allocation.x += button_requisition.width;

    
    if (grip->title_window) {
        GdkRectangle area;
        
        /* set layout text */
        ensure_title_and_icon_pixbuf (grip);
        pango_layout_set_text (grip->_priv->title_layout, grip->_priv->title, -1);

        gdl_dock_item_grip_get_title_area (grip, &area);

        gdk_window_move_resize (grip->title_window,
                                area.x, area.y, area.width, area.height);

        if (grip->_priv->icon_pixbuf)
            area.width -= gdk_pixbuf_get_width (grip->_priv->icon_pixbuf) + 1;
            
        /* ellipsize title if it doesn't fit the title area */
        ellipsize_layout (grip->_priv->title_layout, area.width);
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
    g_warning ("gtk_container_remove not implemented for GdlDockItemGrip");
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

    if (include_internals) {
        (* callback) (grip->_priv->close_button, callback_data);
        (* callback) (grip->_priv->iconify_button, callback_data);
    }
}

static GtkType
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

    parent_class = g_type_class_peek_parent (klass);
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

    /* initialize stock images */
    gdl_stock_init ();
}

GtkWidget *
gdl_dock_item_grip_new (GdlDockItem *item)
{
    GdlDockItemGrip *grip = g_object_new (GDL_TYPE_DOCK_ITEM_GRIP, "item", item,
                                          NULL);

    return GTK_WIDGET (grip);
}

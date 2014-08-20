/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- 
 *
 * gdl-dock-item-button-image.h
 *
 * Author: Joel Holdsworth <joel@airwebreathe.org.uk>
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
 
#ifndef _GDL_DOCK_ITEM_BUTTON_IMAGE_H_
#define _GDL_DOCK_ITEM_BUTTON_IMAGE_H_

#include <gtk/gtk.h>

G_BEGIN_DECLS

/* Standard Macros */
#define GDL_TYPE_DOCK_ITEM_BUTTON_IMAGE            \
    (gdl_dock_item_button_image_get_type())
#define GDL_DOCK_ITEM_BUTTON_IMAGE(obj)            \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDL_TYPE_DOCK_ITEM_BUTTON_IMAGE, GdlDockItemButtonImage))
#define GDL_DOCK_ITEM_BUTTON_IMAGE_CLASS(klass)    \
    (G_TYPE_CHECK_CLASS_CAST ((klass), GDL_TYPE_DOCK_ITEM_BUTTON_IMAGE, GdlDockItemButtonImageClass))
#define GDL_IS_DOCK_ITEM_BUTTON_IMAGE(obj)         \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDL_TYPE_DOCK_ITEM_BUTTON_IMAGE))
#define GDL_IS_DOCK_ITEM_BUTTON_IMAGE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), GDL_TYPE_DOCK_ITEM_BUTTON_IMAGE))
#define GDL_DOCK_ITEM_BUTTON_IMAGE_GET_CLASS(obj)  \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), GDL_TYPE_DOCK_ITEM_BUTTON_IMAGE, GdlDockItemButtonImageClass))

/* Data Types & Structures */
typedef enum {
    GDL_DOCK_ITEM_BUTTON_IMAGE_CLOSE,
    GDL_DOCK_ITEM_BUTTON_IMAGE_ICONIFY
} GdlDockItemButtonImageType;

typedef struct _GdlDockItemButtonImage GdlDockItemButtonImage;
typedef struct _GdlDockItemButtonImageClass GdlDockItemButtonImageClass;

struct _GdlDockItemButtonImage {
    GtkWidget parent;
    
    GdlDockItemButtonImageType image_type;
};

struct _GdlDockItemButtonImageClass {
    GtkWidgetClass parent_class;
};

/* Data Public Functions */
GType      gdl_dock_item_button_image_get_type (void);
GtkWidget *gdl_dock_item_button_image_new (
    GdlDockItemButtonImageType image_type);

G_END_DECLS

#endif /* _GDL_DOCK_ITEM_BUTTON_IMAGE_H_ */

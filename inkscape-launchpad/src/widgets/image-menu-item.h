/* GTK - The GIMP Toolkit
 * Copyright (C) Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.
 * Forked for , icons in menus are important to us.
 */

#ifndef __IMAGE_MENU_ITEM_H__
#define __IMAGE_MENU_ITEM_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_PARAM_READABLE G_PARAM_READABLE|G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB
#define GTK_PARAM_WRITABLE G_PARAM_WRITABLE|G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB
#define GTK_PARAM_READWRITE G_PARAM_READWRITE|G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB

#define TYPE_IMAGE_MENU_ITEM            (image_menu_item_get_type ())
#define IMAGE_MENU_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_IMAGE_MENU_ITEM, ImageMenuItem))
#define IMAGE_MENU_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_IMAGE_MENU_ITEM, ImageMenuItemClass))
#define IS_IMAGE_MENU_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_IMAGE_MENU_ITEM))
#define IS_IMAGE_MENU_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_IMAGE_MENU_ITEM))
#define IMAGE_MENU_ITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_IMAGE_MENU_ITEM, ImageMenuItemClass))

typedef struct _ImageMenuItem              ImageMenuItem;
typedef struct _ImageMenuItemPrivate       ImageMenuItemPrivate;
typedef struct _ImageMenuItemClass         ImageMenuItemClass;

struct _ImageMenuItem
{
  GtkMenuItem menu_item;

  /*< private >*/
  ImageMenuItemPrivate *priv;
};

struct _ImageMenuItemClass
{
  GtkMenuItemClass parent_class;

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};

GType	   image_menu_item_get_type          (void) G_GNUC_CONST;
GtkWidget* image_menu_item_new               (void);
GtkWidget* image_menu_item_new_with_label    (const gchar      *label);
GtkWidget* image_menu_item_new_with_mnemonic (const gchar      *label);
GtkWidget* image_menu_item_new_from_stock    (const gchar      *stock_id,
                                                  GtkAccelGroup    *accel_group);
void       image_menu_item_set_image         (ImageMenuItem *image_menu_item,
                                                  GtkWidget        *image);
GtkWidget* image_menu_item_get_image         (ImageMenuItem *image_menu_item);
void       image_menu_item_set_use_stock     (ImageMenuItem *image_menu_item,
						  gboolean          use_stock);
gboolean   image_menu_item_get_use_stock     (ImageMenuItem *image_menu_item);
void       image_menu_item_set_accel_group   (ImageMenuItem *image_menu_item, 
						  GtkAccelGroup    *accel_group);

G_END_DECLS

#endif /* __IMAGE_MENU_ITEM_H__ */

/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * gdl-dock-item.h
 *
 * Author: Gustavo Giráldez <gustavo.giraldez@gmx.net>
 *
 * Based on GnomeDockItem/BonoboDockItem.  Original copyright notice follows.
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

#ifndef __GDL_DOCK_ITEM_H__
#define __GDL_DOCK_ITEM_H__

#include "libgdl/gdl-dock-object.h"

G_BEGIN_DECLS

/* standard macros */
#define GDL_TYPE_DOCK_ITEM            (gdl_dock_item_get_type ())
#define GDL_DOCK_ITEM(obj)            (GTK_CHECK_CAST ((obj), GDL_TYPE_DOCK_ITEM, GdlDockItem))
#define GDL_DOCK_ITEM_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GDL_TYPE_DOCK_ITEM, GdlDockItemClass))
#define GDL_IS_DOCK_ITEM(obj)         (GTK_CHECK_TYPE ((obj), GDL_TYPE_DOCK_ITEM))
#define GDL_IS_DOCK_ITEM_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GDL_TYPE_DOCK_ITEM))
#define GDL_DOCK_ITEM_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_DOCK_ITEM, GdlDockItemClass))

/* data types & structures */
typedef enum {
    GDL_DOCK_ITEM_BEH_NORMAL           = 0,
    GDL_DOCK_ITEM_BEH_NEVER_FLOATING   = 1 << 0,
    GDL_DOCK_ITEM_BEH_NEVER_VERTICAL   = 1 << 1,
    GDL_DOCK_ITEM_BEH_NEVER_HORIZONTAL = 1 << 2,
    GDL_DOCK_ITEM_BEH_LOCKED           = 1 << 3,
    GDL_DOCK_ITEM_BEH_CANT_DOCK_TOP    = 1 << 4,
    GDL_DOCK_ITEM_BEH_CANT_DOCK_BOTTOM = 1 << 5,
    GDL_DOCK_ITEM_BEH_CANT_DOCK_LEFT   = 1 << 6,
    GDL_DOCK_ITEM_BEH_CANT_DOCK_RIGHT  = 1 << 7,
    GDL_DOCK_ITEM_BEH_CANT_DOCK_CENTER = 1 << 8,
    GDL_DOCK_ITEM_BEH_CANT_CLOSE       = 1 << 9,
    GDL_DOCK_ITEM_BEH_CANT_ICONIFY     = 1 << 10,
    GDL_DOCK_ITEM_BEH_NO_GRIP          = 1 << 11
} GdlDockItemBehavior;

typedef enum {
    GDL_DOCK_IN_DRAG             = 1 << GDL_DOCK_OBJECT_FLAGS_SHIFT,
    GDL_DOCK_IN_PREDRAG          = 1 << (GDL_DOCK_OBJECT_FLAGS_SHIFT + 1),
    GDL_DOCK_ICONIFIED           = 1 << (GDL_DOCK_OBJECT_FLAGS_SHIFT + 2),
    /* for general use: indicates the user has started an action on
       the dock item */
    GDL_DOCK_USER_ACTION         = 1 << (GDL_DOCK_OBJECT_FLAGS_SHIFT + 3)
} GdlDockItemFlags;

typedef struct _GdlDockItem        GdlDockItem;
typedef struct _GdlDockItemClass   GdlDockItemClass;
typedef struct _GdlDockItemPrivate GdlDockItemPrivate;

struct _GdlDockItem {
    GdlDockObject        object;

    GtkWidget           *child;
    GdlDockItemBehavior  behavior;
    GtkOrientation       orientation;

    guint                resize : 1;

    gint                 dragoff_x, dragoff_y;    /* these need to be
                                                     accesible from
                                                     outside */
    GdlDockItemPrivate  *_priv;
};

struct _GdlDockItemClass {
    GdlDockObjectClass  parent_class;

    gboolean            has_grip;
    
    /* virtuals */
    void     (* dock_drag_begin)  (GdlDockItem      *item);
    void     (* dock_drag_motion) (GdlDockItem      *item,
                                   gint              x,
                                   gint              y);
    void     (* dock_drag_end)    (GdlDockItem      *item,
                                   gboolean          cancelled);
    void     (* move_focus_child) (GdlDockItem      *item,
                                   GtkDirectionType  direction);
                                   
    void     (* set_orientation)  (GdlDockItem      *item,
                                   GtkOrientation    orientation);
};

/* additional macros */
#define GDL_DOCK_ITEM_FLAGS(item)     (GDL_DOCK_OBJECT (item)->flags)
#define GDL_DOCK_ITEM_IN_DRAG(item) \
    ((GDL_DOCK_ITEM_FLAGS (item) & GDL_DOCK_IN_DRAG) != 0)
#define GDL_DOCK_ITEM_IN_PREDRAG(item) \
    ((GDL_DOCK_ITEM_FLAGS (item) & GDL_DOCK_IN_PREDRAG) != 0)
#define GDL_DOCK_ITEM_ICONIFIED(item) \
    ((GDL_DOCK_ITEM_FLAGS (item) & GDL_DOCK_ICONIFIED) != 0)
#define GDL_DOCK_ITEM_USER_ACTION(item) \
    ((GDL_DOCK_ITEM_FLAGS (item) & GDL_DOCK_USER_ACTION) != 0)
#define GDL_DOCK_ITEM_NOT_LOCKED(item) !((item)->behavior & GDL_DOCK_ITEM_BEH_LOCKED)
#define GDL_DOCK_ITEM_NO_GRIP(item) ((item)->behavior & GDL_DOCK_ITEM_BEH_NO_GRIP)

#define GDL_DOCK_ITEM_SET_FLAGS(item,flag) \
    G_STMT_START { (GDL_DOCK_ITEM_FLAGS (item) |= (flag)); } G_STMT_END
#define GDL_DOCK_ITEM_UNSET_FLAGS(item,flag) \
    G_STMT_START { (GDL_DOCK_ITEM_FLAGS (item) &= ~(flag)); } G_STMT_END

#define GDL_DOCK_ITEM_HAS_GRIP(item) ((GDL_DOCK_ITEM_GET_CLASS (item)->has_grip)&& \
		! GDL_DOCK_ITEM_NO_GRIP (item))

#define GDL_DOCK_ITEM_CANT_CLOSE(item) \
    ((((item)->behavior & GDL_DOCK_ITEM_BEH_CANT_CLOSE) != 0)|| \
     ! GDL_DOCK_ITEM_NOT_LOCKED(item))

#define GDL_DOCK_ITEM_CANT_ICONIFY(item) \
    ((((item)->behavior & GDL_DOCK_ITEM_BEH_CANT_ICONIFY) != 0)|| \
     ! GDL_DOCK_ITEM_NOT_LOCKED(item))

/* public interface */
 
GtkWidget     *gdl_dock_item_new               (const gchar         *name,
                                                const gchar         *long_name,
                                                GdlDockItemBehavior  behavior);
GtkWidget     *gdl_dock_item_new_with_stock    (const gchar         *name,
                                                const gchar         *long_name,
                                                const gchar         *stock_id,
                                                GdlDockItemBehavior  behavior);

GtkWidget     *gdl_dock_item_new_with_pixbuf_icon (const gchar         *name,
                                                   const gchar         *long_name,
                                                   const GdkPixbuf     *pixbuf_icon,
                                                   GdlDockItemBehavior  behavior);

GType          gdl_dock_item_get_type          (void);

void           gdl_dock_item_dock_to           (GdlDockItem      *item,
                                                GdlDockItem      *target,
                                                GdlDockPlacement  position,
                                                gint              docking_param);

void           gdl_dock_item_set_orientation   (GdlDockItem    *item,
                                                GtkOrientation  orientation);

GtkWidget     *gdl_dock_item_get_tablabel      (GdlDockItem *item);
void           gdl_dock_item_set_tablabel      (GdlDockItem *item,
                                                GtkWidget   *tablabel);
void           gdl_dock_item_hide_grip         (GdlDockItem *item);
void           gdl_dock_item_show_grip         (GdlDockItem *item);

/* bind and unbind items to a dock */
void           gdl_dock_item_bind              (GdlDockItem *item,
                                                GtkWidget   *dock);

void           gdl_dock_item_unbind            (GdlDockItem *item);

void           gdl_dock_item_hide_item         (GdlDockItem *item);

void           gdl_dock_item_iconify_item      (GdlDockItem *item);

void           gdl_dock_item_show_item         (GdlDockItem *item);

void           gdl_dock_item_lock              (GdlDockItem *item);

void           gdl_dock_item_unlock            (GdlDockItem *item);

void        gdl_dock_item_set_default_position (GdlDockItem      *item,
                                                GdlDockObject    *reference);

void        gdl_dock_item_preferred_size       (GdlDockItem      *item,
                                                GtkRequisition   *req);

gboolean    gdl_dock_item_or_child_has_focus  (GdlDockItem      *item);

G_END_DECLS

#endif

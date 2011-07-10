/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 8 -*- */
/*
 * gdl-dock-item-grip.h
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

#ifndef _GDL_DOCK_ITEM_GRIP_H_
#define _GDL_DOCK_ITEM_GRIP_H_

#include <gtk/gtk.h>
#include "libgdl/gdl-dock-item.h"

G_BEGIN_DECLS

#define GDL_TYPE_DOCK_ITEM_GRIP            (gdl_dock_item_grip_get_type())
#define GDL_DOCK_ITEM_GRIP(obj)            \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDL_TYPE_DOCK_ITEM_GRIP, GdlDockItemGrip))
#define GDL_DOCK_ITEM_GRIP_CLASS(klass)    \
    (G_TYPE_CHECK_CLASS_CAST ((klass), GDL_TYPE_DOCK_ITEM_GRIP, GdlDockItemGripClass))
#define GDL_IS_DOCK_ITEM_GRIP(obj)         \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDL_TYPE_DOCK_ITEM_GRIP))
#define GDL_IS_DOCK_ITEM_GRIP_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), GDL_TYPE_DOCK_ITEM_GRIP))
#define GDL_DOCK_ITEM_GRIP_GET_CLASS(obj)  \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), GDL_TYPE_DOCK_ITEM_GRIP, GdlDockItemGripClass))

typedef struct _GdlDockItemGrip        GdlDockItemGrip;
typedef struct _GdlDockItemGripClass   GdlDockItemGripClass;
typedef struct _GdlDockItemGripPrivate GdlDockItemGripPrivate;

struct _GdlDockItemGrip {
    GtkContainer parent;
	
    GdlDockItem *item;
    
    GdkWindow *title_window;
    
    GdlDockItemGripPrivate *_priv;
};

struct _GdlDockItemGripClass {
    GtkContainerClass parent_class;
};

GType      gdl_dock_item_grip_get_type (void);
GtkWidget *gdl_dock_item_grip_new      (GdlDockItem *item);
void       gdl_dock_item_grip_set_label (GdlDockItemGrip *grip,
                                          GtkWidget *label);
void       gdl_dock_item_grip_hide_handle (GdlDockItemGrip *grip);
void       gdl_dock_item_grip_show_handle (GdlDockItemGrip *grip);

G_END_DECLS

#endif /* _GDL_DOCK_ITEM_GRIP_H_ */

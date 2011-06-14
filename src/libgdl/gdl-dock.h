/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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

#ifndef __GDL_DOCK_H__
#define __GDL_DOCK_H__

#include <gtk/gtk.h>
#include "libgdl/gdl-dock-object.h"
#include "libgdl/gdl-dock-item.h"
#include "libgdl/gdl-dock-placeholder.h"

G_BEGIN_DECLS

/* standard macros */
#define GDL_TYPE_DOCK            (gdl_dock_get_type ())
#define GDL_DOCK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDL_TYPE_DOCK, GdlDock))
#define GDL_DOCK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GDL_TYPE_DOCK, GdlDockClass))
#define GDL_IS_DOCK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDL_TYPE_DOCK))
#define GDL_IS_DOCK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GDL_TYPE_DOCK))
#define GDL_DOCK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_DOCK, GdlDockClass))

/* data types & structures */
typedef struct _GdlDock        GdlDock;
typedef struct _GdlDockClass   GdlDockClass;
typedef struct _GdlDockPrivate GdlDockPrivate;

struct _GdlDock {
    GdlDockObject    object;

    GdlDockObject   *root;

    GdlDockPrivate  *_priv;
};

struct _GdlDockClass {
    GdlDockObjectClass parent_class;

    void  (* layout_changed)  (GdlDock *dock);    /* proxy signal for the master */
};

/* additional macros */
#define GDL_DOCK_IS_CONTROLLER(dock)  \
    (gdl_dock_master_get_controller (GDL_DOCK_OBJECT_GET_MASTER (dock)) == \
     GDL_DOCK_OBJECT (dock))

/* public interface */
 
GtkWidget     *gdl_dock_new               (void);

GtkWidget     *gdl_dock_new_from          (GdlDock          *original,
                                           gboolean          floating);

GType          gdl_dock_get_type          (void);

void           gdl_dock_add_item          (GdlDock          *dock,
                                           GdlDockItem      *item,
                                           GdlDockPlacement  place);

void           gdl_dock_add_floating_item (GdlDock        *dock,
                                           GdlDockItem    *item,
                                           gint            x,
                                           gint            y,
                                           gint            width,
                                           gint            height);

GdlDockItem   *gdl_dock_get_item_by_name  (GdlDock     *dock,
                                           const gchar *name);

GdlDockPlaceholder *gdl_dock_get_placeholder_by_name (GdlDock     *dock,
                                                      const gchar *name);

GList         *gdl_dock_get_named_items   (GdlDock    *dock);

GdlDock       *gdl_dock_object_get_toplevel (GdlDockObject *object);

void           gdl_dock_xor_rect            (GdlDock       *dock,
                                             GdkRectangle  *rect);

G_END_DECLS

#endif

/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- 
 *
 * gdl-dock-master.h - Object which manages a dock ring
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

#ifndef __GDL_DOCK_MASTER_H__
#define __GDL_DOCK_MASTER_H__

#include <glib-object.h>
#include <gtk/gtk.h>
#include "libgdl/gdl-dock-object.h"


G_BEGIN_DECLS

/* standard macros */
#define GDL_TYPE_DOCK_MASTER             (gdl_dock_master_get_type ())
#define GDL_DOCK_MASTER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDL_TYPE_DOCK_MASTER, GdlDockMaster))
#define GDL_DOCK_MASTER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GDL_TYPE_DOCK_MASTER, GdlDockMasterClass))
#define GDL_IS_DOCK_MASTER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDL_TYPE_DOCK_MASTER))
#define GDL_IS_DOCK_MASTER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GDL_TYPE_DOCK_MASTER))
#define GDL_DOCK_MASTER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_DOCK_MASTER, GdlDockMasterClass))

/* data types & structures */
typedef struct _GdlDockMaster        GdlDockMaster;
typedef struct _GdlDockMasterClass   GdlDockMasterClass;
typedef struct _GdlDockMasterPrivate GdlDockMasterPrivate;

typedef enum {
    GDL_SWITCHER_STYLE_TEXT,
    GDL_SWITCHER_STYLE_ICON,
    GDL_SWITCHER_STYLE_BOTH,
    GDL_SWITCHER_STYLE_TOOLBAR,
    GDL_SWITCHER_STYLE_TABS,
    GDL_SWITCHER_STYLE_NONE
} GdlSwitcherStyle;

struct _GdlDockMaster {
    GObject               object;

    GHashTable           *dock_objects;
    GList                *toplevel_docks;
    GdlDockObject        *controller;      /* GUI root object */
    
    gint                  dock_number;     /* for toplevel dock numbering */
    
    GdlDockMasterPrivate *_priv;
};

struct _GdlDockMasterClass {
    GObjectClass parent_class;

    void (* layout_changed) (GdlDockMaster *master);
};

/* additional macros */

#define GDL_DOCK_OBJECT_GET_MASTER(object) \
    (GDL_DOCK_OBJECT (object)->master ? \
        GDL_DOCK_MASTER (GDL_DOCK_OBJECT (object)->master) : NULL)

/* public interface */
 
GType          gdl_dock_master_get_type         (void);

void           gdl_dock_master_add              (GdlDockMaster *master,
                                                 GdlDockObject *object);
void           gdl_dock_master_remove           (GdlDockMaster *master,
                                                 GdlDockObject *object);
void           gdl_dock_master_foreach          (GdlDockMaster *master,
                                                 GFunc          function,
                                                 gpointer       user_data);

void           gdl_dock_master_foreach_toplevel (GdlDockMaster *master,
                                                 gboolean       include_controller,
                                                 GFunc          function,
                                                 gpointer       user_data);

GdlDockObject *gdl_dock_master_get_object       (GdlDockMaster *master,
                                                 const gchar   *nick_name);

GdlDockObject *gdl_dock_master_get_controller   (GdlDockMaster *master);
void           gdl_dock_master_set_controller   (GdlDockMaster *master,
                                                 GdlDockObject *new_controller);

G_END_DECLS

#endif /* __GDL_DOCK_MASTER_H__ */

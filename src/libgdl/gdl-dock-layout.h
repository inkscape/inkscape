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


#ifndef __GDL_DOCK_LAYOUT_H__
#define __GDL_DOCK_LAYOUT_H__

#include <glib-object.h>
#include "libgdl/gdl-dock-master.h"
#include "libgdl/gdl-dock.h"

G_BEGIN_DECLS

/* standard macros */
#define	GDL_TYPE_DOCK_LAYOUT		  (gdl_dock_layout_get_type ())
#define GDL_DOCK_LAYOUT(object)		  (GTK_CHECK_CAST ((object), GDL_TYPE_DOCK_LAYOUT, GdlDockLayout))
#define GDL_DOCK_LAYOUT_CLASS(klass)	  (GTK_CHECK_CLASS_CAST ((klass), GDL_TYPE_DOCK_LAYOUT, GdlDockLayoutClass))
#define GDL_IS_DOCK_LAYOUT(object)	  (GTK_CHECK_TYPE ((object), GDL_TYPE_DOCK_LAYOUT))
#define GDL_IS_DOCK_LAYOUT_CLASS(klass)	  (GTK_CHECK_CLASS_TYPE ((klass), GDL_TYPE_DOCK_LAYOUT))
#define	GDL_DOCK_LAYOUT_GET_CLASS(object) (GTK_CHECK_GET_CLASS ((object), GDL_TYPE_DOCK_LAYOUT, GdlDockLayoutClass))

/* data types & structures */
typedef struct _GdlDockLayout GdlDockLayout;
typedef struct _GdlDockLayoutClass GdlDockLayoutClass;
typedef struct _GdlDockLayoutPrivate GdlDockLayoutPrivate;

struct _GdlDockLayout {
    GObject               g_object;

    gboolean              dirty;
    GdlDockMaster        *master;

    GdlDockLayoutPrivate *_priv;
};

struct _GdlDockLayoutClass {
    GObjectClass  g_object_class;
};


/* public interface */
 
GType            gdl_dock_layout_get_type       (void);

GdlDockLayout   *gdl_dock_layout_new            (GdlDock       *dock);

void             gdl_dock_layout_attach         (GdlDockLayout *layout,
                                                 GdlDockMaster *master);

gboolean         gdl_dock_layout_load_layout    (GdlDockLayout *layout,
                                                 const gchar   *name);

void             gdl_dock_layout_save_layout    (GdlDockLayout *layout,
                                                 const gchar   *name);

void             gdl_dock_layout_delete_layout  (GdlDockLayout *layout,
                                                 const gchar   *name);

GList           *gdl_dock_layout_get_layouts    (GdlDockLayout *layout,
                                                 gboolean       include_default);

void             gdl_dock_layout_run_manager    (GdlDockLayout *layout);

gboolean         gdl_dock_layout_load_from_file (GdlDockLayout *layout,
                                                 const gchar   *filename);

gboolean         gdl_dock_layout_save_to_file   (GdlDockLayout *layout,
                                                 const gchar   *filename);

gboolean         gdl_dock_layout_is_dirty       (GdlDockLayout *layout);

GtkWidget       *gdl_dock_layout_get_ui         (GdlDockLayout *layout);
GtkWidget       *gdl_dock_layout_get_items_ui   (GdlDockLayout *layout);
GtkWidget       *gdl_dock_layout_get_layouts_ui (GdlDockLayout *layout);

G_END_DECLS

#endif



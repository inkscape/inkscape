/*  -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * 
 * This file is part of the GNOME Devtools Libraries.
 * 
 * Copyright (C) 2001 Dave Camp <dave@ximian.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 */

#ifndef GDL_DATA_MODEL_H
#define GDL_DATA_MODEL_H

#include <glib.h>
#include <glib-object.h>

/* Using GtkTreePath to save time  */
#include <gtk/gtktreemodel.h>
#include <gtk/gtkcellrenderer.h>

G_BEGIN_DECLS

#define GDL_TYPE_DATA_MODEL            (gdl_data_model_get_type ())
#define GDL_DATA_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDL_TYPE_DATA_MODEL, GdlDataModel))
#define GDL_IS_DATA_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDL_TYPE_DATA_MODEL))
#define GDL_DATA_MODEL_GET_IFACE(obj)  ((GdlDataModelIface *)g_type_interface_peek (((GTypeInstance *)GDL_DATA_MODEL (obj))->g_class, GDL_TYPE_DATA_MODEL))

typedef struct _GdlDataModel      GdlDataModel;
typedef struct _GdlDataIter       GdlDataIter;
typedef struct _GdlDataModelIface GdlDataModelIface;

struct _GdlDataIter {
	int stamp;

	gpointer data1;
	gpointer data2;
	gpointer data3;
};

struct _GdlDataModelIface {
	GTypeInterface g_iface;
	
	/* Signals */
	void (*path_changed) (GdlDataModel *dm, GtkTreePath *path);
	void (*path_inserted) (GdlDataModel *dm, GtkTreePath *path);
	void (*path_deleted) (GdlDataModel *dm, GtkTreePath *path);
	
	/* Virtual Table */
	gboolean (*get_iter) (GdlDataModel *dm, GdlDataIter *iter, 
			      GtkTreePath *path);
	GtkTreePath* (*get_path) (GdlDataModel *dm, GdlDataIter *iter);

	void (*get_name) (GdlDataModel *dm, GdlDataIter *iter, 
			  char **name);
	void (*get_value) (GdlDataModel *dm, GdlDataIter *iter, 
			   GValue *value);
	void (*get_renderer) (GdlDataModel *dm, GdlDataIter *iter,
			      GtkCellRenderer **renderer, char **field,
			      gboolean *is_editable);
	gboolean (*iter_next) (GdlDataModel *dm, GdlDataIter *iter);
	gboolean (*iter_children) (GdlDataModel *dm, GdlDataIter *iter,
				   GdlDataIter *parent);
	gboolean (*iter_has_child) (GdlDataModel *dm, GdlDataIter *iter);
};

GType        gdl_data_model_get_type       (void);
gboolean     gdl_data_model_get_iter       (GdlDataModel     *dm,
					    GdlDataIter      *iter,
					    GtkTreePath      *path);
GtkTreePath *gdl_data_model_get_path       (GdlDataModel     *dm,
					    GdlDataIter      *iter);
void         gdl_data_model_get_name       (GdlDataModel     *dm,
					    GdlDataIter      *iter,
					    char            **name);
void         gdl_data_model_get_value      (GdlDataModel     *dm,
					    GdlDataIter      *iter,
					    GValue           *value);
void         gdl_data_model_get_renderer   (GdlDataModel     *dm,
					    GdlDataIter      *iter,
					    GtkCellRenderer **renderer,
					    char            **field,
					    gboolean         *is_editable);
gboolean     gdl_data_model_iter_next      (GdlDataModel     *dm,
					    GdlDataIter      *iter);
gboolean     gdl_data_model_iter_children  (GdlDataModel     *dm,
					    GdlDataIter      *iter,
					    GdlDataIter      *children);
gboolean     gdl_data_model_iter_has_child (GdlDataModel     *dm,
					    GdlDataIter      *iter);

G_END_DECLS

#endif

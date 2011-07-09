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

#ifndef GDL_DATA_ROW_H
#define GDL_DATA_ROW_H

#include <glib.h>

#include <glib-object.h>
#include <gtk/gtktreemodel.h>
#include <gtk/gtkcellrenderer.h>
#include <gdl/gdl-data-view.h>

G_BEGIN_DECLS

#define GDL_TYPE_DATA_ROW            (gdl_data_row_get_type ())
#define GDL_DATA_ROW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDL_TYPE_DATA_ROW, GdlDataRow))
#define GDL_DATA_ROW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GDL_TYPE_DATA_ROW, GdlDataRowClass))
#define GDL_IS_DATA_ROW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDL_TYPE_DATA_ROW))
#define GDL_IS_DATA_ROW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GDL_TYPE_DATA_ROW))

typedef struct _GdlDataRow           GdlDataRow;
typedef struct _GdlDataRowClass      GdlDataRowClass;
typedef struct _GdlDataRowPrivate    GdlDataRowPrivate;

struct _GdlDataRow {
	GObject parent;

	GdlDataRowPrivate *priv;
};

struct _GdlDataRowClass {
	GObjectClass parent_class;
};

GType       gdl_data_row_get_type      (void);
GdlDataRow *gdl_data_row_new           (GdlDataView           *view,
					GtkTreePath           *path);
void        gdl_data_row_get_size      (GdlDataRow            *row,
					int                   *text_w,
					int                   *cell_w,
					int                   *total_width,
					int                   *height);
void        gdl_data_row_set_show_name (GdlDataRow            *row,
					gboolean               show_name);
void        gdl_data_row_layout        (GdlDataRow            *row,
					GdkRectangle          *alloc);
void        gdl_data_row_render        (GdlDataRow            *row,
					GdkDrawable           *drawable,
					GdkRectangle          *expose_area,
					GtkCellRendererState   flags);
GdlDataRow *gdl_data_row_at            (GdlDataRow            *row,
					int                    x,
					int                    y);
gboolean    gdl_data_row_event         (GdlDataRow            *row,
					GdkEvent              *event,
					GtkCellEditable      **editable_widget);
void        gdl_data_row_get_cell_area (GdlDataRow            *row,
					GdkRectangle          *rect);
void        gdl_data_row_set_split     (GdlDataRow            *row,
					int                    split);
void        gdl_data_row_set_selected  (GdlDataRow            *row,
					gboolean               selected);
void        gdl_data_row_set_focused   (GdlDataRow            *row,
					gboolean               focused);
const char *gdl_data_row_get_title     (GdlDataRow            *row);


G_END_DECLS

#endif

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gdl-i18n.h"
#include "gdl-tools.h"
#include "gdl-data-row.h"
#include "gdl-data-model.h"

#include <gtk/gtktreemodel.h>
#include <gtk/gtkcellrenderer.h>

struct _GdlDataRowPrivate {
	GdlDataModel *model;
	GtkTreePath *path;
	GdlDataView *view;
	
	char *name;

	/* area_r
	 *  +- title_r
	 *  |   +- name_r
	 *  |   +- sep_r
	 *  +- data_r
	 *      +- expand_r
	 *      +- cell_r
	 */
	 
	GdkRectangle area_r;
	
	GdkRectangle title_r;
	GdkRectangle name_r;
	GdkRectangle sep_r;
	
	GdkRectangle data_r;
	GdkRectangle expand_r;
	GdkRectangle cell_r;
	
	gboolean multi;
	GtkCellRenderer *cell;
	GList *subrows;

	char *renderer_field;

	gboolean expanded;
	gboolean focused;
	gboolean editable;
	
	int split;
	int child_split;
	gboolean selected;
};

GDL_CLASS_BOILERPLATE (GdlDataRow, gdl_data_row, GObject, G_TYPE_OBJECT);


static void
expand (GdlDataRow *row)
{
	GdlDataIter iter;
	gboolean valid;

	if (gdl_data_model_get_iter (row->priv->model, &iter, row->priv->path)) {
		valid = gdl_data_model_iter_children (row->priv->model,
						      &iter, &iter);
		while (valid) {
			GdlDataRow *new_row;
			GtkTreePath *path;
			
			path = gdl_data_model_get_path (row->priv->model,
							&iter);
			new_row = gdl_data_row_new (row->priv->view,
						    path);
			row->priv->subrows = 
				g_list_prepend (row->priv->subrows, new_row);
			gtk_tree_path_free (path);

			valid = gdl_data_model_iter_next (row->priv->model,
							  &iter);
		}
		row->priv->subrows = g_list_reverse (row->priv->subrows);

		row->priv->expanded = TRUE;
		gdl_data_view_layout (GDL_DATA_VIEW (row->priv->view));
		gtk_widget_queue_draw (GTK_WIDGET (row->priv->view));
	}
}

static void
contract (GdlDataRow *row)
{
	GList *l;
	for (l = row->priv->subrows; l != NULL; l = l->next) {
		g_object_unref (G_OBJECT (l->data));
	}
	g_list_free (row->priv->subrows);
	row->priv->subrows = NULL;

	row->priv->expanded = FALSE;
	gdl_data_view_layout (GDL_DATA_VIEW (row->priv->view));
	gtk_widget_queue_draw (GTK_WIDGET (row->priv->view));
}

static void
load_path (GdlDataRow *row) 
{
	GdlDataIter iter;

	/* Make sure the path has been unloaded */
	g_return_if_fail (row->priv->name == NULL);
	g_return_if_fail (row->priv->cell == NULL);
	
	if (gdl_data_model_get_iter (row->priv->model, 
				     &iter, row->priv->path)) {
		GValue val = { 0, };
		char *str;
		
		gdl_data_model_get_name (row->priv->model, &iter, 
					 &str);
		row->priv->name = g_strdup (str);
		
		if (gdl_data_model_iter_has_child (row->priv->model, &iter)) {
			row->priv->multi = TRUE;
		}
		
		gdl_data_model_get_renderer (row->priv->model, &iter, 
					     &row->priv->cell,
					     &str,
					     &row->priv->editable);
		g_object_ref (GTK_OBJECT (row->priv->cell));
		gtk_object_sink (GTK_OBJECT (row->priv->cell));
		
		row->priv->renderer_field = g_strdup (str);
		gdl_data_model_get_value (row->priv->model, &iter, &val);
		
		g_object_set_property (G_OBJECT (row->priv->cell), 
				       row->priv->renderer_field,
				       &val);
		g_value_unset (&val);
	}
}

static void
unload_path (GdlDataRow *row)
{
	if (row->priv->renderer_field) {
		g_free (row->priv->renderer_field);
		row->priv->renderer_field = NULL;
	}
	
	if (row->priv->name) {
		g_free (row->priv->name);
		row->priv->name = NULL;
	}
	
	if (row->priv->cell) {
		g_object_unref (row->priv->cell);
		row->priv->cell = NULL;
	}

	if (row->priv->subrows) {
		GList *l;
		for (l = row->priv->subrows; l != NULL; l = l->next) {
			g_object_unref (G_OBJECT (l->data));
		}
		g_list_free (row->priv->subrows);
		row->priv->subrows = NULL;
	}
}

static void
gdl_data_row_instance_init (GdlDataRow *row)
{
	row->priv = g_new0 (GdlDataRowPrivate, 1);
}

static void
gdl_data_row_finalize (GObject *object)
{
	GdlDataRow *row = GDL_DATA_ROW (object);
	if (row->priv) {
		unload_path (row);
		
		if (row->priv->path) {
			gtk_tree_path_free (row->priv->path);
			row->priv->path = NULL;
		}

		
		g_object_unref (row->priv->model);
		
		g_free (row->priv);
		row->priv = NULL;
	}
}

static void
gdl_data_row_class_init (GdlDataRowClass *klass)
{
	GObjectClass *gobject_class = (GObjectClass*) klass;
	gobject_class->finalize = gdl_data_row_finalize;

	parent_class = g_type_class_peek_parent (klass);
}

GdlDataRow *
gdl_data_row_new (GdlDataView *view,
		  GtkTreePath *path)
{
	GdlDataRow *row;
	
	row = GDL_DATA_ROW (g_object_new (gdl_data_row_get_type (),
					  NULL));
	
	row->priv->view = view;
	row->priv->model = g_object_ref (view->model);
	row->priv->path = gtk_tree_path_copy (path);
	load_path (row);

	return row;
}


#define PAD 3

static void
layout_row (GdlDataRow *row,
	    int x, int y, int width, int height)
{
	PangoLayout *layout;
	
	/* sizes */

	layout = gtk_widget_create_pango_layout (GTK_WIDGET (row->priv->view),
						 row->priv->name);
	pango_layout_get_pixel_size (layout, 
				     &row->priv->name_r.width, 
				     &row->priv->name_r.height);
	g_object_unref (layout);
	
	layout = gtk_widget_create_pango_layout (GTK_WIDGET (row->priv->view), "=");
	pango_layout_get_pixel_size (layout, 
				     &row->priv->sep_r.width, 
				     &row->priv->sep_r.height);
	g_object_unref (layout);

	row->priv->title_r.width = 
		MAX (row->priv->name_r.width + row->priv->sep_r.width + PAD,
		     row->priv->split);
	row->priv->title_r.height = 
		MAX (row->priv->name_r.height, row->priv->sep_r.width);

	if (row->priv->cell) {
		gtk_cell_renderer_get_size (row->priv->cell,
					    GTK_WIDGET (row->priv->view),
					    NULL, NULL, NULL,
					    &row->priv->cell_r.width,
					    &row->priv->cell_r.height);
	} else {
		row->priv->cell_r.width = row->priv->cell_r.height = 0;
	}
	
	row->priv->data_r.width = row->priv->cell_r.width;
	row->priv->data_r.height = row->priv->cell_r.height;

	if (row->priv->multi) {
		row->priv->expand_r.width = 10;
		row->priv->expand_r.height = 10;

		row->priv->data_r.width += row->priv->expand_r.width;
		row->priv->data_r.height = MAX (row->priv->expand_r.height,
						row->priv->data_r.height);

		if (row->priv->expanded) {
			GList *l;
			int name_w = 0, data_w = 0;
			for (l = row->priv->subrows; l != NULL; l = l->next) {
				int w1, w2, h;
				gdl_data_row_get_size (GDL_DATA_ROW (l->data),
						       &w1, &w2, NULL, &h);
				name_w = MAX (name_w, w1);
				data_w = MAX (data_w, w2);
				row->priv->data_r.height += h;
			}
			row->priv->child_split = name_w;
			row->priv->data_r.width = 
				MAX (name_w + data_w + 3 * PAD,
				     row->priv->data_r.width);
			row->priv->data_r.height += 2 * PAD;
		}
	}

	row->priv->area_r.width = MAX (width,
				       row->priv->data_r.width + row->priv->title_r.width + PAD);
	
	row->priv->area_r.height = MAX (height,
					(MAX (row->priv->data_r.height,
					      row->priv->title_r.height)));

	/* Positions */

	row->priv->area_r.x = x;
	row->priv->area_r.y = y;
	
	row->priv->title_r.x = x;
	row->priv->title_r.y = y + ((row->priv->area_r.height) / 2) - (row->priv->title_r.height / 2);

	row->priv->name_r.x = x;
	row->priv->name_r.y = y + ((row->priv->area_r.height) / 2) - (row->priv->name_r.height / 2);
	
	row->priv->sep_r.x = row->priv->title_r.x + row->priv->title_r.width - row->priv->sep_r.width;
	row->priv->sep_r.y = y + ((row->priv->area_r.height) / 2) - (row->priv->sep_r.height / 2);
	
	row->priv->data_r.x = row->priv->title_r.x + row->priv->title_r.width + PAD;
	row->priv->data_r.y = y;
	
	/* Readjust the data area size to fit */
	row->priv->data_r.width = row->priv->area_r.width - (row->priv->title_r.width + PAD);
	row->priv->data_r.height = row->priv->area_r.height;

	if (row->priv->multi) {
		row->priv->expand_r.x = row->priv->data_r.x;
		row->priv->expand_r.y = row->priv->data_r.y + ((row->priv->cell_r.height) / 2) - (row->priv->expand_r.height / 2);

		row->priv->cell_r.y = row->priv->data_r.y;
		row->priv->cell_r.height = MAX (row->priv->expand_r.height, row->priv->cell_r.height);
		row->priv->cell_r.width = (row->priv->data_r.width - row->priv->expand_r.width);

		row->priv->cell_r.x = row->priv->expand_r.x + row->priv->expand_r.width;
	} else {
		row->priv->cell_r = row->priv->data_r;
	}
}

void
gdl_data_row_get_size (GdlDataRow *row, int *sep_width, 
		       int *cell_width, int *total_width, int *height)
{
	layout_row (row, 0, 0, 0, 0);

	if (sep_width) {
		*sep_width = row->priv->name_r.width + row->priv->sep_r.width + PAD;
	}
	
	if (cell_width) *cell_width = row->priv->data_r.width;
	if (total_width) *total_width = row->priv->area_r.width;
	if (height) *height = row->priv->area_r.height;
}

void 
gdl_data_row_set_show_name (GdlDataRow *row, gboolean show_name)
{
}

void
gdl_data_row_layout (GdlDataRow *row, GdkRectangle *alloc)
{
	layout_row (row, alloc->x, alloc->y, alloc->width, alloc->height);
	
	if (row->priv->multi && row->priv->expanded) {
		GList *l;
		GdkRectangle sub;
		sub.y = row->priv->expand_r.y + row->priv->expand_r.width + PAD;
		sub.x = row->priv->data_r.x + PAD;
		sub.width = row->priv->data_r.width - 2 * PAD;
		sub.height = row->priv->data_r.height - 2 * PAD;

		for (l = row->priv->subrows; l != NULL; l = l->next) {
			gdl_data_row_get_size (GDL_DATA_ROW (l->data), 
					       NULL, NULL, NULL, &sub.height);
			gdl_data_row_set_split (GDL_DATA_ROW (l->data),
						row->priv->child_split);
			gdl_data_row_layout (GDL_DATA_ROW (l->data), 
					     &sub);
			sub.y += sub.height;
		}
	}
}

#if 0
#define DRAWR(r) 		{ gdk_draw_rectangle (drawable, GTK_WIDGET (row->priv->view)->style->text_gc[GTK_STATE_NORMAL],FALSE,row->priv->r.x,row->priv->r.y,row->priv->r.width, row->priv->r.height); }
#else
#define DRAWR(r)
#endif


void
gdl_data_row_render (GdlDataRow *row, GdkDrawable *drawable,
		     GdkRectangle *expose_area,
		     GtkCellRendererState flags)
{
	PangoLayout *layout;

	guint state = GTK_STATE_NORMAL;

	if (row->priv->selected) {
		if (flags & GTK_CELL_RENDERER_SELECTED)
			state = GTK_STATE_SELECTED;
		else 
			state = GTK_STATE_ACTIVE;
		gtk_paint_flat_box (GTK_WIDGET (row->priv->view)->style,
				    drawable, state,
				    GTK_SHADOW_NONE, expose_area, 
				    GTK_WIDGET (row->priv->view), "cell_even",
				    row->priv->area_r.x, row->priv->area_r.y,
				    row->priv->area_r.width + 1, 
				    row->priv->area_r.height + 1);

	}
	
	layout = gtk_widget_create_pango_layout (GTK_WIDGET (row->priv->view),
						 row->priv->name);
	gdk_draw_layout (drawable, 
			 GTK_WIDGET (row->priv->view)->style->text_gc[state],
			 row->priv->name_r.x, row->priv->name_r.y, layout);
	g_object_unref (layout);
	DRAWR(name_r);
	
	layout = gtk_widget_create_pango_layout (GTK_WIDGET (row->priv->view), "=");
	gdk_draw_layout (drawable, 
			 GTK_WIDGET (row->priv->view)->style->text_gc[state],
			 row->priv->sep_r.x, row->priv->sep_r.y, layout);
	g_object_unref (layout);
	DRAWR(sep_r);
	DRAWR(title_r);

	if (row->priv->cell) {
		if (row->priv->focused) {
			gtk_paint_focus (GTK_WIDGET (row->priv->view)->style,
					 drawable, 
					 GTK_WIDGET_STATE (GTK_WIDGET (row->priv->view)),
					 NULL, GTK_WIDGET (row->priv->view),
					 "treeview", 
					 row->priv->cell_r.x - 1,
					 row->priv->cell_r.y - 1,
					 row->priv->cell_r.width + 2,
					 row->priv->cell_r.height + 2);
		}
					 
		gtk_cell_renderer_render (row->priv->cell,
					  drawable, GTK_WIDGET (row->priv->view), 
					  &row->priv->area_r,
					  &row->priv->cell_r, 
					  expose_area, 
					  row->priv->selected ? GTK_CELL_RENDERER_SELECTED : 0);
		DRAWR(cell_r);
	}
	if (row->priv->multi) {
		gtk_paint_expander (GTK_WIDGET (row->priv->view)->style,
				    drawable,
				    GTK_WIDGET_STATE (GTK_WIDGET (row->priv->view)),
				    expose_area,
				    GTK_WIDGET (row->priv->view),
				    "gdldataview",
				    row->priv->expand_r.x + row->priv->expand_r.width / 2,
				    row->priv->expand_r.y + row->priv->expand_r.height / 2,
				    row->priv->expanded ? GTK_EXPANDER_EXPANDED : GTK_EXPANDER_COLLAPSED);
		DRAWR(expand_r);
		
		if (row->priv->expanded) {
			GList *l;
			
			for (l = row->priv->subrows; l != NULL; l = l->next) {
				gdl_data_row_render (GDL_DATA_ROW (l->data), 
						     drawable,
						     expose_area, flags);
			}
		}
		gdk_draw_rectangle (drawable, 
				    GTK_WIDGET (row->priv->view)->style->text_gc[GTK_STATE_NORMAL],
				    FALSE,
				    row->priv->data_r.x, 
				    row->priv->data_r.y,
				    row->priv->data_r.width, 
				    row->priv->data_r.height);
	}
	DRAWR(data_r);
}


GdlDataRow *
gdl_data_row_at (GdlDataRow *row, int x, int y)
{
	if (!GDL_POINT_IN (x, y, &row->priv->area_r)) {
		return NULL;
	}
	
	if (row->priv->multi && row->priv->expanded) {
		GList *l;
		for (l = row->priv->subrows; l != NULL; l = l->next) {
			GdlDataRow *ret = gdl_data_row_at (GDL_DATA_ROW (l->data), x, y);
			if (ret) 
				return ret;
		}
	}
	
	return row;
}

static gboolean
button_press_event (GdlDataRow *row, GdkEventButton *event, 
		    GtkCellEditable **editable_widget)
{
	if (editable_widget)
		*editable_widget = NULL;

	if (GDL_POINT_IN (event->x, event->y, &row->priv->expand_r)) {
		if (row->priv->expanded)
			contract (row);
		else 
			expand (row);
	}
	
	if (GDL_POINT_IN (event->x, event->y, &row->priv->cell_r)
	    && row->priv->editable) {
		g_return_val_if_fail (editable_widget, FALSE);
		*editable_widget = gtk_cell_renderer_start_editing 
			(row->priv->cell,
			 (GdkEvent*)event,
			 GTK_WIDGET (row->priv->view),
			 "1:2:3",
			 &row->priv->area_r,
			 &row->priv->cell_r,
			 GTK_CELL_RENDERER_SELECTED);
	}
	
	return FALSE;

}


gboolean 
gdl_data_row_event (GdlDataRow *row, GdkEvent *event, 
		    GtkCellEditable **editable_widget)
{
	switch (((GdkEventAny *)event)->type) {
		case GDK_BUTTON_PRESS:
			return button_press_event (row, 
						   (GdkEventButton *)event, 
						   editable_widget);
		default:
			break;
	}
	return FALSE;
}

void
gdl_data_row_get_cell_area (GdlDataRow           *row,
			    GdkRectangle         *rect)
{
	*rect = row->priv->cell_r;
}

void
gdl_data_row_set_split (GdlDataRow *row, int split)
{
	row->priv->split = split;
}

void
gdl_data_row_set_selected (GdlDataRow *row, gboolean selected)
{
	row->priv->selected = selected;

	/* FIXME: invalidate here */
	gtk_widget_queue_draw (GTK_WIDGET (row->priv->view));
}

void
gdl_data_row_set_focused (GdlDataRow *row, gboolean focused)
{
	row->priv->focused = focused;
	
	/* FIXME: invalidate here */
	gtk_widget_queue_draw (GTK_WIDGET (row->priv->view));
}

const char *
gdl_data_row_get_title (GdlDataRow *row)
{
	return row->priv->name;
}

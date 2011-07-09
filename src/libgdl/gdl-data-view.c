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
#include "gdl-data-view.h"
#include "gdl-data-frame.h"

#include "tree-expand.xpm"
#include "tree-contract.xpm"

struct _GdlDataViewPrivate {
	GList *frames;
	GList *rows;
	GList *widgets;
	
	GdlDataFrame *selected_frame;
	GdlDataRow *selected_row;

	GtkCellEditable *editable;
    
	GdkPixbuf *close_pixbuf;
	GdkPixbuf *expand_pixbuf;
	GdkPixbuf *contract_pixbuf;
};

typedef struct {
	GtkWidget *widget;
	int x, y, height, width;
} ChildWidget;

static void gdl_data_view_instance_init (GdlDataView *dv);
static void gdl_data_view_class_init (GdlDataViewClass *klass);

GDL_CLASS_BOILERPLATE (GdlDataView, gdl_data_view, GtkLayout, GTK_TYPE_LAYOUT);


#define GRID_SPACING 15

static void
paint_grid (GtkWidget *widget, GdkDrawable *drawable,
	    int offset_x, int offset_y, int width, int height)
{
	GdlDataView *dv;
	int x;
	int y;
	
	g_return_if_fail (GDL_IS_DATA_VIEW (widget));
	dv = GDL_DATA_VIEW (widget);
	
	x = offset_x + ((GRID_SPACING - (offset_x % GRID_SPACING)) % GRID_SPACING);

	/* Draw grid points */
	for (; x  < width; x += GRID_SPACING) {
		y = offset_y + ((GRID_SPACING - (offset_y % GRID_SPACING)) % GRID_SPACING);
		
		for (; y < height; y += GRID_SPACING) {
			gdk_draw_point (drawable,
					widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
					x, y);
		}
	}	
}

static void
expose_frames (GdlDataView *view, GdkEventExpose *event)
{
	GList *l;
	
	for (l = view->priv->frames; l != NULL; l = l->next) {
		GdkRectangle intersect;
		GdlDataFrame *frame = GDL_DATA_FRAME (l->data);
		if (gdk_rectangle_intersect (&frame->area, 
					     &event->area, 
					     &intersect)) {
			gdl_data_frame_draw (GDL_DATA_FRAME (l->data),
					     GTK_LAYOUT(view)->bin_window,
					     &intersect);
		}
	}	
}

static void
expose_widgets (GdlDataView *view, GdkEventExpose *event)
{
	GList *l;
	
	for (l = view->priv->widgets; l != NULL; l = l->next) {
		ChildWidget *child = l->data;
		gtk_container_propagate_expose (GTK_CONTAINER (view),
						child->widget, event);
	}
}

static gboolean
gdl_data_view_expose (GtkWidget *widget,
		      GdkEventExpose *event)
{
	if (GTK_WIDGET_DRAWABLE (widget)) {
		if (event->window == GTK_LAYOUT (widget)->bin_window) {
			paint_grid (widget, GTK_LAYOUT (widget)->bin_window,
				    event->area.x, event->area.y,
				    event->area.width, event->area.height);
			expose_frames (GDL_DATA_VIEW (widget), event);
			expose_widgets (GDL_DATA_VIEW (widget), event);
			return TRUE;
		} else {
			GTK_WIDGET_CLASS (parent_class)->expose_event (widget, event);
		}
	}

	return FALSE;
}

static GdlDataFrame *
frame_at (GdlDataView *dv, int x, int y)
{
	GList *l;
	for (l = dv->priv->frames; l != NULL; l = l->next) {
		GdlDataFrame *frame = l->data;
		if (x >= frame->area.x && x <= frame->area.x + frame->area.width 
		    && y >= frame->area.y && y <= frame->area.y + frame->area.height) {
			return frame;
		}
	}
	return NULL;
}

static GdlDataRow *
row_at (GdlDataView *view, int x, int y)
{
	GList *l;
	GdlDataRow *ret = NULL;
	for (l = view->priv->rows; l != NULL; l = l->next) {
		GdlDataRow *row = l->data;
		ret = gdl_data_row_at (row, x, y);
		if (ret) break;
	}
	return ret;
}

static void 
gdl_data_view_put (GdlDataView *view, GtkWidget *widget, 
		   int x, int y, int width, int height) 
{
	ChildWidget *child = g_new0 (ChildWidget, 1);
	
	child->widget = widget;
	child->x = x;
	child->y = y;
	child->width = width;
	child->height = height;
	
	view->priv->widgets = g_list_append (view->priv->widgets, child);

	if (GTK_WIDGET_REALIZED (view)) {
		gtk_widget_set_parent_window (child->widget, 
					      GTK_LAYOUT (view)->bin_window);
	}
	
	gtk_widget_set_parent (child->widget, GTK_WIDGET (view));
}

static void
stop_editing (GdlDataView *dv)
{
	if (dv->priv->editable) {
		gtk_cell_editable_editing_done (dv->priv->editable);
		gtk_cell_editable_remove_widget (dv->priv->editable);
	}
}

static void
remove_widget_cb (GtkCellEditable *cell_editable, GdlDataView *view)
{
	if (view->priv->editable) {
		view->priv->editable = NULL;
		gdl_data_row_set_focused (view->priv->selected_row, FALSE);
		gtk_widget_grab_focus (GTK_WIDGET (view));
		gtk_container_remove (GTK_CONTAINER (view), 
				      GTK_WIDGET (cell_editable));
	}
}

static gboolean
button_press_event_cb (GdlDataView *dv, GdkEventButton *event, gpointer data)
{
	GdlDataFrame *frame;
	GdlDataRow *row;
	gboolean ret = FALSE;

	stop_editing (dv);

	if (event->type == GDK_BUTTON_PRESS) {
		frame = frame_at (dv, event->x, event->y);
		if (frame) {
			if (dv->priv->selected_frame) {
				gdl_data_frame_set_selected (dv->priv->selected_frame, FALSE);
			}
			gdl_data_frame_set_selected (frame, TRUE);
			dv->priv->selected_frame = frame;
		} 
		
		row = row_at (dv, event->x, event->y);
		if (row) {
			GtkCellEditable *editable;
			
			if (dv->priv->selected_row) {
				gdl_data_row_set_selected (dv->priv->selected_row, 
							   FALSE);
			}
			dv->priv->selected_row = row;
			gdl_data_row_set_selected (row, TRUE);
			ret = gdl_data_row_event (row, (GdkEvent*)event, 
						  &editable);
			if (editable) {
				GdkRectangle area;
				dv->priv->editable = editable;
				gtk_cell_editable_start_editing (editable, 
								 (GdkEvent*)event);
				
				gdl_data_row_get_cell_area (row, &area);
				gdl_data_view_put (dv,
						   GTK_WIDGET (editable), 
						   area.x,
						   area.y,
						   area.width,
						   area.height);
				
				gtk_widget_grab_focus (GTK_WIDGET (editable));
				dv->priv->editable = editable;
				gdl_data_row_set_focused (row, TRUE);
				
				g_signal_connect
					(G_OBJECT (editable), 
					 "remove_widget",
					 G_CALLBACK (remove_widget_cb), dv);
			}
		}
	}

	return ret;
}

static void
gdl_data_view_instance_init (GdlDataView *dv)
{
	GTK_WIDGET_SET_FLAGS (dv, GTK_CAN_FOCUS);
	dv->priv = g_new0 (GdlDataViewPrivate, 1);

	g_signal_connect (G_OBJECT (dv), "button_press_event",
			  G_CALLBACK (button_press_event_cb), 
			  NULL);

	dv->priv->close_pixbuf = gtk_widget_render_icon (GTK_WIDGET (dv),
							 "gtk-close",
							 GTK_ICON_SIZE_MENU,
							 "gdl-data-view-close");
	
	dv->priv->expand_pixbuf = 
		gdk_pixbuf_new_from_xpm_data ((const char **)tree_expand_xpm);
	
	dv->priv->contract_pixbuf = 
		gdk_pixbuf_new_from_xpm_data ((const char **)tree_contract_xpm);
}

static void
gdl_data_view_realize (GtkWidget *widget)
{
	GList *l;
	GdlDataView *view = GDL_DATA_VIEW (widget);
	
	GDL_CALL_PARENT (GTK_WIDGET_CLASS, realize, (widget));

	for (l = view->priv->widgets; l != NULL; l = l->next) {
		ChildWidget *child = l->data;
		gtk_widget_set_parent_window (child->widget, 
					      GTK_LAYOUT (view)->bin_window);
	}
}

static void
gdl_data_view_size_request (GtkWidget *widget, GtkRequisition *req)
{
	GList *l;
	
	req->width = req->height = 0;
	
	for (l = GDL_DATA_VIEW (widget)->priv->widgets; l != NULL; l = l->next) {
		GtkRequisition child_req;
		ChildWidget *child = l->data;
		
		gtk_widget_size_request (child->widget, &child_req);
	}
}


static void
gdl_data_view_size_allocate (GtkWidget *widget, GtkAllocation *alloc)
{
	GdlDataView *view = GDL_DATA_VIEW (widget);
	GList *l;
;
	for (l = view->priv->widgets; l != NULL; l = l->next) {
		ChildWidget *child = l->data;
		GtkAllocation child_alloc;

		child_alloc.x = child->x;
		child_alloc.y = child->y;
		child_alloc.width = child->width;
		child_alloc.height = child->height;
		
		gtk_widget_size_allocate (child->widget, &child_alloc);
	}
	GDL_CALL_PARENT (GTK_WIDGET_CLASS, size_allocate, (widget, alloc));
}

static void
gdl_data_view_forall (GtkContainer *container, gboolean include_internals,
		      GtkCallback callback, gpointer callback_data)
{
	GdlDataView *view = GDL_DATA_VIEW (container);
	GList *l;

	for (l = view->priv->widgets; l != NULL; l = l->next) {
		ChildWidget *child = l->data;
		(*callback) (child->widget, callback_data);
	}
}

static void
gdl_data_view_remove (GtkContainer *container, GtkWidget *widget)
{
	GList *l;
	GdlDataView *view = GDL_DATA_VIEW (container);
	
	for (l = view->priv->widgets; l != NULL; l = l->next) {
		ChildWidget *child = l->data;
		if (child->widget == widget) {
			gtk_widget_unparent (widget);
			view->priv->widgets = 
				g_list_remove_link (view->priv->widgets, l);
			g_list_free_1 (l);
			g_free (child);
			return;
		}
	}
}

static void
gdl_data_view_destroy (GtkObject *obj)
{
	GdlDataView *dv = GDL_DATA_VIEW (obj);
	
	stop_editing (dv);

	if (dv->priv) {
		GList *l;
		for (l = dv->priv->frames; l != NULL; l = l->next) {
			g_object_unref (G_OBJECT (l->data));
		}
		g_list_free (dv->priv->frames);
		
		g_object_unref (dv->priv->close_pixbuf);
		g_object_unref (dv->priv->expand_pixbuf);
		g_object_unref (dv->priv->contract_pixbuf);

		g_free (dv->priv);
		dv->priv = NULL;
	}
	GDL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (obj));
}

static void
gdl_data_view_class_init (GdlDataViewClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *)klass;
	GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;
	GtkContainerClass *container_class = (GtkContainerClass *)klass;

	parent_class = gtk_type_class (GTK_TYPE_LAYOUT);
	
	container_class->forall = gdl_data_view_forall;
	container_class->remove = gdl_data_view_remove;

	widget_class->expose_event = gdl_data_view_expose;
	widget_class->realize = gdl_data_view_realize;
	/* FIXME: unrealize */
	widget_class->size_request = gdl_data_view_size_request;
	widget_class->size_allocate = gdl_data_view_size_allocate;
	object_class->destroy = gdl_data_view_destroy;

	gtk_widget_class_install_style_property (widget_class,
						 g_param_spec_int ("expander-size",
								   _("Expander Size"),
								   _("Size of the expander arrow."),
								   0, 
								   G_MAXINT,
								   10,
								   G_PARAM_READABLE));
}

GtkWidget *
gdl_data_view_new (void)
{
	GdlDataView *dv;
	dv = g_object_new (gdl_data_view_get_type (), NULL);
	return GTK_WIDGET (dv);
}

void
gdl_data_view_set_model (GdlDataView *dv, GdlDataModel *model)
{
	GtkTreePath *path;
	GdlDataIter iter;
	gboolean iter_valid;
	int x = 5;

	dv->model = model;

	path = gtk_tree_path_new_from_string ("0");

	iter_valid = gdl_data_model_get_iter (model, &iter, path);
	gtk_tree_path_free (path);
	
	while (iter_valid) {
		GdlDataFrame *frame;
		GdlDataRow *row;
		
		path = gdl_data_model_get_path (model, &iter);

		row = gdl_data_row_new (dv, path);
		frame = gdl_data_frame_new (dv, row);
		gdl_data_frame_set_position (frame, x, 5);

		dv->priv->frames = g_list_append (dv->priv->frames,
						  frame);
		dv->priv->rows = g_list_append (dv->priv->rows, row);

		gtk_tree_path_free (path);
		
		x += 150;

		iter_valid = gdl_data_model_iter_next (model, &iter);
	}
}

void 
gdl_data_view_layout (GdlDataView *view)
{
	GList *l;
	for (l = view->priv->frames; l != NULL; l = l->next) {
		gdl_data_frame_layout (GDL_DATA_FRAME (l->data));
	}
}

GdkPixbuf *
gdl_data_view_get_close_pixbuf (GdlDataView *view)
{
	return view->priv->close_pixbuf;
}

void
gdl_data_view_set_close_pixbuf (GdlDataView *view, GdkPixbuf *pixbuf)
{
	if (view->priv->close_pixbuf) {
		g_object_unref (view->priv->close_pixbuf);
	}

	view->priv->close_pixbuf = g_object_ref (pixbuf);
}

GdkPixbuf *
gdl_data_view_get_expand_pixbuf (GdlDataView *view)
{
	return view->priv->expand_pixbuf;
}

void
gdl_data_view_set_expand_pixbuf (GdlDataView *view, GdkPixbuf *pixbuf)
{
	if (view->priv->expand_pixbuf) {
		g_object_unref (view->priv->expand_pixbuf);
	}

	view->priv->expand_pixbuf = g_object_ref (pixbuf);
}

GdkPixbuf *
gdl_data_view_get_contract_pixbuf (GdlDataView *view)
{
	return view->priv->contract_pixbuf;
}

void
gdl_data_view_set_contract_pixbuf (GdlDataView *view, GdkPixbuf *pixbuf)
{
	if (view->priv->contract_pixbuf) {
		g_object_unref (view->priv->contract_pixbuf);
	}

	view->priv->contract_pixbuf = g_object_ref (pixbuf);
}

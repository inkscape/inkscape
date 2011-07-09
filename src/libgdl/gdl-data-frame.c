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
#include <string.h>

#include "gdl-data-view.h"
#include "gdl-data-frame.h"
#include "gdl-data-model.h"
#include "gdl-data-row.h"

struct _GdlDataFramePrivate {
	GdkRectangle shadow_r;
	GdkRectangle frame_r;
	GdkRectangle titlebar_r;
	GdkRectangle title_r;
	GdkRectangle close_r;
	GdkRectangle row_r;
	
	int shadow_offset;
	int titlebar_height;
	char *title;
	
	GdlDataRow *row;
	
	PangoLayout *layout;

	gboolean selected;
};

static void gdl_data_frame_class_init (GdlDataFrameClass *klass);
static void gdl_data_frame_instance_init (GdlDataFrame *obj);
static void gdl_data_frame_finalize (GObject *object);

GDL_CLASS_BOILERPLATE (GdlDataFrame, gdl_data_frame, GObject, G_TYPE_OBJECT);

#define PAD 2
#define BORDER 1

#define CENTERY(r1, r2) { r1.y = ((r2.y + (r2.height / 2)) - (r1.height / 2)); }

void
gdl_data_frame_layout (GdlDataFrame *frame)
{
	GdkPixbuf *close_pixbuf;
	/* Sizes */
	if (frame->priv->row) {
		gdl_data_row_get_size (frame->priv->row,
				       NULL, NULL,
				       &frame->priv->row_r.width,
				       &frame->priv->row_r.height);
	} else {
		frame->priv->row_r.height = frame->priv->row_r.width = 0;
	}	

	if (frame->priv->layout) {
		pango_layout_get_pixel_size (frame->priv->layout, 
					     &frame->priv->title_r.width,
					     &frame->priv->title_r.height);
	} else {
		frame->priv->title_r.width = frame->priv->title_r.height = 0;
	}

	close_pixbuf = gdl_data_view_get_close_pixbuf (frame->view);
	if (close_pixbuf) {
		frame->priv->close_r.width = 
			gdk_pixbuf_get_width (close_pixbuf);
		frame->priv->close_r.height = 
			gdk_pixbuf_get_width (close_pixbuf);
	} else {
		frame->priv->close_r.width = frame->priv->close_r.height = 0;
	}
	
	frame->priv->titlebar_r.height = MAX (frame->priv->titlebar_height, 
					      frame->priv->title_r.height);
	frame->priv->titlebar_r.height = MAX (frame->priv->titlebar_r.height,
					      frame->priv->close_r.height);

	frame->priv->frame_r.width = 2 * BORDER + 3 * PAD + frame->priv->title_r.width + frame->priv->close_r.width;
	frame->priv->frame_r.width = MAX (frame->priv->frame_r.width,
					  frame->priv->row_r.width + 2 * BORDER + 2 * PAD);
	frame->priv->frame_r.height = frame->priv->row_r.height + frame->priv->titlebar_r.height + 2 * PAD + 2 * BORDER;
	frame->priv->titlebar_r.width = frame->priv->frame_r.width - BORDER;
	frame->priv->shadow_r.width = frame->priv->frame_r.width;
	frame->priv->shadow_r.height = frame->priv->frame_r.height;

	/* Locations */
	frame->priv->frame_r.x = frame->area.x;
	frame->priv->frame_r.y = frame->area.y;
	
	frame->priv->shadow_r.x = frame->priv->frame_r.x + frame->priv->shadow_offset;
	frame->priv->shadow_r.y = frame->priv->frame_r.y + frame->priv->shadow_offset;
	frame->priv->titlebar_r.x = frame->priv->frame_r.x + BORDER;
	frame->priv->titlebar_r.y = frame->priv->frame_r.y + BORDER;
	frame->priv->title_r.x = frame->priv->frame_r.x + BORDER + PAD;
	CENTERY (frame->priv->title_r, frame->priv->titlebar_r);
	frame->priv->close_r.x = (frame->priv->frame_r.x + frame->priv->frame_r.width) - (frame->priv->close_r.width + BORDER + PAD);
	CENTERY (frame->priv->close_r, frame->priv->titlebar_r);

	if (frame->priv->row) {
		frame->priv->row_r.x = frame->priv->frame_r.x + BORDER + PAD;
		frame->priv->row_r.y = frame->priv->titlebar_r.y + frame->priv->titlebar_r.height + PAD;
		gdl_data_row_layout (frame->priv->row, &frame->priv->row_r);
	} else {
		frame->priv->row_r.x = frame->priv->row_r.y = 0;
	}

	frame->area.width = frame->priv->frame_r.width + frame->priv->shadow_offset;
	frame->area.height = frame->priv->frame_r.height + frame->priv->shadow_offset;
}

#if 0 /* not used */
static void
change_layout (GdlDataFrame *frame)
{
	char *text = frame->priv->title ? frame->priv->title : "?";
	pango_layout_set_text (frame->priv->layout, text, strlen (text));
}
#endif

#define EXPLODE(r) (r).x, (r).y, (r).width, (r).height

void
gdl_data_frame_draw (GdlDataFrame *frame, GdkDrawable *drawable,
		     GdkRectangle *expose_area)
{
	GdkRectangle inter;
	guint8 state = 
		frame->priv->selected ? GTK_STATE_SELECTED : GTK_STATE_NORMAL;
	
	gdk_draw_rectangle (drawable, 
			    GTK_WIDGET (frame->view)->style->dark_gc[state], 
			    TRUE,
			    EXPLODE (frame->priv->shadow_r));
	gdk_draw_rectangle (drawable, 
			    GTK_WIDGET (frame->view)->style->base_gc[GTK_STATE_NORMAL], 
			    TRUE,
			    EXPLODE (frame->priv->frame_r));
	gdk_draw_rectangle (drawable, 
			    GTK_WIDGET (frame->view)->style->black_gc,
			    FALSE, 
			    EXPLODE (frame->priv->frame_r));
	gdk_draw_rectangle (drawable, 
			    GTK_WIDGET (frame->view)->style->bg_gc[state], 
			    TRUE,
			    EXPLODE (frame->priv->titlebar_r));
	gdk_draw_layout (drawable, 
			 GTK_WIDGET (frame->view)->style->fg_gc[state], 
			 frame->priv->title_r.x, frame->priv->title_r.y,
			 frame->priv->layout);

	if (gdk_rectangle_intersect (expose_area, &frame->priv->close_r, &inter)) {
		GdkPixbuf *pixbuf = gdl_data_view_get_close_pixbuf (frame->view);
		gdk_draw_pixbuf (drawable, NULL, pixbuf,
				 0, 0, 
				 inter.x - frame->priv->close_r.x,
				 inter.y - frame->priv->close_r.y,
				 gdk_pixbuf_get_width (pixbuf),
				 gdk_pixbuf_get_height (pixbuf),
				 GDK_RGB_DITHER_NORMAL, 0, 0);
	}
	
	if (frame->priv->row) {
		if (gdk_rectangle_intersect (expose_area, &frame->priv->row_r,
					     &inter)) {
			gdl_data_row_render (frame->priv->row, drawable,
					     &inter,
					     frame->priv->selected ? GTK_CELL_RENDERER_SELECTED : 0);
		}
	}
}

void
gdl_data_frame_class_init (GdlDataFrameClass *klass)
{
	GObjectClass *gobject_class = (GObjectClass *)klass;
	
	parent_class = g_type_class_peek_parent (klass);	

	gobject_class->finalize = gdl_data_frame_finalize;
}

void
gdl_data_frame_instance_init (GdlDataFrame *frame)
{
	frame->priv = g_new0 (GdlDataFramePrivate, 1);
	frame->area.x = frame->area.y = 0;
	frame->priv->shadow_offset = 3;
	frame->priv->titlebar_height = 20;

	frame->area.height = frame->area.width = 100;
}

void
gdl_data_frame_finalize (GObject *object)
{
	GdlDataFrame *frame = GDL_DATA_FRAME (object);

	if (frame->priv) {
		g_free (frame->priv->title);
		g_object_unref (frame->priv->layout);
		g_object_unref (frame->priv->row);
		
		g_free (frame->priv);
		frame->priv = NULL;
	}
	GDL_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

void
gdl_data_frame_set_selected (GdlDataFrame *frame, 
			     gboolean val)
{
	frame->priv->selected = val;

	gdk_window_invalidate_rect (GTK_LAYOUT (frame->view)->bin_window,
				    &frame->priv->frame_r,
				    TRUE);
}

gboolean 
gdl_data_frame_button_press (GdlDataFrame *frame,
			     GdkEventButton *event)
{
	return FALSE;
}

void
gdl_data_frame_set_position (GdlDataFrame *frame,
			     int x,
			     int y)
{
	frame->area.x = x;
	frame->area.y = y;

	gdl_data_frame_layout (frame);
}

static void
setup_layout (GdlDataFrame *frame)
{
	PangoFontDescription *font_desc = 
		pango_font_description_copy (GTK_WIDGET (frame->view)->style->font_desc);
	
	pango_font_description_set_weight (font_desc,
					   PANGO_WEIGHT_BOLD);
	
	frame->priv->layout = gtk_widget_create_pango_layout (GTK_WIDGET (frame->view),
							      frame->priv->title ? frame->priv->title : "?");
	pango_layout_set_font_description (frame->priv->layout,
					   font_desc);
	pango_font_description_free (font_desc);
}


GdlDataFrame *
gdl_data_frame_new (GdlDataView *view,
		    GdlDataRow *row)
{
	GdlDataFrame *frame;
	frame = GDL_DATA_FRAME (g_object_new (GDL_TYPE_DATA_FRAME, NULL));
	
	frame->view = view;

	frame->priv->row = row;
	frame->priv->title = g_strdup (gdl_data_row_get_title (row));

	setup_layout (frame);

	gdl_data_frame_layout (frame);
	
	return frame;
}

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

#ifndef GDL_DATA_FRAME_H
#define GDL_DATA_FRAME_H

#include <glib-object.h>
#include <gdl/gdl-data-row.h>

G_BEGIN_DECLS

#define GDL_TYPE_DATA_FRAME            (gdl_data_frame_get_type ())
#define GDL_DATA_FRAME(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDL_TYPE_DATA_FRAME, GdlDataFrame))
#define GDL_DATA_FRAME_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GDL_DATA_VIEW_FRAM, GdlDataFrame))
#define GDL_IS_DATA_FRAME(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDL_TYPE_DATA_FRAME))
#define GDL_IS_DATA_FRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GDL_TYPE_DATA_FRAME))
#define GDL_DATA_FRAME_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GDL_TYPE_DATA_FRAME, GdlDataFrameClass))

typedef struct _GdlDataFrame        GdlDataFrame;
typedef struct _GdlDataFramePrivate GdlDataFramePrivate;
typedef struct _GdlDataFrameClass   GdlDataFrameClass;

struct _GdlDataFrame {
	GObject parent;
	
	GdlDataView *view;
	GdkRectangle area;
	
	GdlDataFramePrivate *priv;
};

struct _GdlDataFrameClass {
	GObjectClass parent_class;
};

GType         gdl_data_frame_get_type     (void);
GdlDataFrame *gdl_data_frame_new          (GdlDataView    *view,
					   GdlDataRow     *row);
void          gdl_data_frame_layout       (GdlDataFrame   *frame);
void          gdl_data_frame_draw         (GdlDataFrame   *item,
					   GdkDrawable    *drawable,
					   GdkRectangle   *expose_area);
void          gdl_data_frame_set_selected (GdlDataFrame   *frame,
					   gboolean        val);
gboolean      gdl_data_frame_button_press (GdlDataFrame   *frame,
					   GdkEventButton *event);
void          gdl_data_frame_set_position (GdlDataFrame   *frame,
					   int             x,
					   int             y);

G_END_DECLS

#endif

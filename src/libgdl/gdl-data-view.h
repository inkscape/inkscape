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

#ifndef GDL_DATA_VIEW_H
#define GDL_DATA_VIEW_H

#include <gdl/gdl-data-model.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GDL_TYPE_DATA_VIEW              (gdl_data_view_get_type ())
#define GDL_DATA_VIEW(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDL_TYPE_DATA_VIEW, GdlDataView))
#define GDL_DATA_VIEW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GDL_TYPE_DATA_VIEW, GdlDataViewClass))
#define GDL_IS_DATA_VIEW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDL_TYPE_DATA_VIEW))
#define GDL_IS_DATA_VIEW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GDL_TYPE_DATA_VIEW))
#define GDL_DATA_VIEW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GDL_TYPE_DATA_VIEW, GdlDataViewClass))

typedef struct _GdlDataView        GdlDataView;
typedef struct _GdlDataViewClass   GdlDataViewClass;
typedef struct _GdlDataViewPrivate GdlDataViewPrivate;

#define GDL_POINT_IN(x1,y1,r) ((x1) >= (r)->x && x1 < (r)->x + (r)->width && (y1) >= (r)->y && y1 < (r)->y + (r)->height)

struct _GdlDataView {
	GtkLayout layout;
	
	GdlDataModel *model;
	
	GdlDataViewPrivate *priv;
};

struct _GdlDataViewClass {
	GtkLayoutClass parent_class;
};

GType    gdl_data_view_get_type            (void);
GtkWidget *gdl_data_view_new                 (void);
void       gdl_data_view_set_model           (GdlDataView  *view,
					      GdlDataModel *model);
void       gdl_data_view_layout              (GdlDataView  *view);
GdkPixbuf *gdl_data_view_get_close_pixbuf    (GdlDataView  *view);
void       gdl_data_view_set_close_pixbuf    (GdlDataView  *view,
					      GdkPixbuf    *pixbuf);
GdkPixbuf *gdl_data_view_get_expand_pixbuf   (GdlDataView  *view);
void       gdl_data_view_set_expand_pixbuf   (GdlDataView  *view,
					      GdkPixbuf    *pixbuf);
GdkPixbuf *gdl_data_view_get_contract_pixbuf (GdlDataView  *view);
void       gdl_data_view_set_contract_pixbuf (GdlDataView  *view,
					      GdkPixbuf    *pixbuf);

#endif

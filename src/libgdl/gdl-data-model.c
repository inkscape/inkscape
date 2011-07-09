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

#include "gdl-data-model.h"

gboolean
gdl_data_model_get_iter (GdlDataModel *dm,
			 GdlDataIter *iter,
			 GtkTreePath *path)
{
	g_return_val_if_fail (dm != NULL, FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);
	g_return_val_if_fail (path != NULL, FALSE);
	g_return_val_if_fail (GDL_DATA_MODEL_GET_IFACE (dm)->get_iter != NULL,
			      FALSE);

	return (*GDL_DATA_MODEL_GET_IFACE (dm)->get_iter) (dm, iter, path);
}

GtkTreePath *
gdl_data_model_get_path (GdlDataModel *dm,
			 GdlDataIter *iter)
{
	g_return_val_if_fail (dm != NULL, NULL);
	g_return_val_if_fail (iter != NULL, NULL);
	g_return_val_if_fail (GDL_DATA_MODEL_GET_IFACE (dm)->get_path != NULL,
			      NULL);

	return (*GDL_DATA_MODEL_GET_IFACE (dm)->get_path) (dm, iter);
}

void
gdl_data_model_get_name (GdlDataModel *dm, 
			 GdlDataIter *iter,
			 char **name)
{
	g_return_if_fail (dm != NULL);
	g_return_if_fail (iter != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (GDL_DATA_MODEL_GET_IFACE (dm)->get_name != NULL);
	
	(*GDL_DATA_MODEL_GET_IFACE (dm)->get_name) (dm, iter, name);	
}

void
gdl_data_model_get_value (GdlDataModel *dm,
			  GdlDataIter *iter,
			  GValue *value)
{
	g_return_if_fail (dm != NULL);
	g_return_if_fail (iter != NULL);
	g_return_if_fail (value != NULL);
	g_return_if_fail (GDL_DATA_MODEL_GET_IFACE (dm)->get_value != NULL);
	
	(*GDL_DATA_MODEL_GET_IFACE (dm)->get_value) (dm, iter, value);
}

void
gdl_data_model_get_renderer (GdlDataModel *dm,
			     GdlDataIter *iter,
			     GtkCellRenderer **renderer,
			     char **field,
			     gboolean *is_editable)
{
	g_return_if_fail (dm != NULL);
	g_return_if_fail (iter != NULL);
	g_return_if_fail (renderer != NULL);
	g_return_if_fail (GDL_DATA_MODEL_GET_IFACE (dm)->get_renderer != NULL);
	
	(*GDL_DATA_MODEL_GET_IFACE (dm)->get_renderer) (dm, iter, 
							renderer, field,
							is_editable);
}

gboolean
gdl_data_model_iter_next (GdlDataModel *dm,
			  GdlDataIter *iter)
{
	g_return_val_if_fail (dm != NULL, FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);
	g_return_val_if_fail (GDL_DATA_MODEL_GET_IFACE (dm)->iter_next != NULL, FALSE);
	
	return (*GDL_DATA_MODEL_GET_IFACE (dm)->iter_next) (dm, iter);
}

gboolean
gdl_data_model_iter_children (GdlDataModel *dm,
			      GdlDataIter *iter,
			      GdlDataIter *parent)
{
	g_return_val_if_fail (dm != NULL, FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);
	g_return_val_if_fail (GDL_DATA_MODEL_GET_IFACE (dm)->iter_children != NULL, FALSE);
	
	return (*GDL_DATA_MODEL_GET_IFACE (dm)->iter_children) (dm, iter, parent);
}

gboolean
gdl_data_model_iter_has_child (GdlDataModel *dm,
			       GdlDataIter *iter)
{
	g_return_val_if_fail (dm != NULL, FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);
	g_return_val_if_fail (GDL_DATA_MODEL_GET_IFACE (dm)->iter_has_child != NULL, FALSE);
	
	return (*GDL_DATA_MODEL_GET_IFACE (dm)->iter_has_child) (dm, iter);
}

static void
gdl_data_model_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;
	
	if (!initialized) {
	}
}

GType 
gdl_data_model_get_type (void)
{
	static GType type = 0;
	
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GdlDataModelIface),
			gdl_data_model_base_init,
			NULL, NULL, NULL, NULL, 0, 0, NULL 
		};
		
		type = g_type_register_static (G_TYPE_INTERFACE, 
					       "GdlDataModel",
					       &info, 0);
		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}
	
	return type;
}

/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- 
 * gdl-stock.c
 * 
 * Copyright (C) 2003 Jeroen Zwartepoorte
 *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <gtk/gtkiconfactory.h>
#include "gdl-stock.h"
#include "gdl-stock-icons.h"

static GtkIconFactory *gdl_stock_factory = NULL;

static struct {
	const gchar *stock_id;
	const guint8 *icon_data;
	const guint data_size;
}
gdl_icons[] = 
{
	{ GDL_STOCK_CLOSE, stock_close_icon, sizeof (stock_close_icon) },
	{ GDL_STOCK_MENU_LEFT, stock_menu_left_icon, sizeof (stock_menu_left_icon) },
	{ GDL_STOCK_MENU_RIGHT, stock_menu_right_icon, sizeof (stock_menu_right_icon) }
};

static void
icon_set_from_data (GtkIconSet       *set,
		    const guint8     *icon_data,
		    const guint       data_size,
		    GtkIconSize       size,
		    gboolean          fallback)
{
	GtkIconSource *source;
	GdkPixbuf     *pixbuf;
	GError        *err = NULL;

	source = gtk_icon_source_new ();

	gtk_icon_source_set_size (source, size);
	gtk_icon_source_set_size_wildcarded (source, FALSE);
	
	pixbuf = gdk_pixbuf_new_from_inline (data_size, icon_data, FALSE, &err);
	if (err) {
	    g_warning ("%s",err->message);
	    g_error_free (err);
	    err = NULL;
	    g_object_unref (source);
	    return;
	}
	
	gtk_icon_source_set_pixbuf (source, pixbuf);
	
	g_object_unref (pixbuf);
	
	gtk_icon_set_add_source (set, source);
	
	if (fallback) {
		gtk_icon_source_set_size_wildcarded (source, TRUE);
		gtk_icon_set_add_source (set, source);
	}
	
	gtk_icon_source_free (source);
}

static void
add_icon (GtkIconFactory *factory,
	  const gchar    *stock_id,
	  const guint8   *icon_data, 
	  const guint     data_size)
{
	GtkIconSet *set;
	gboolean    fallback = FALSE;

	set = gtk_icon_factory_lookup (factory, stock_id);

	if (!set) {
		set = gtk_icon_set_new ();
		gtk_icon_factory_add (factory, stock_id, set);
		gtk_icon_set_unref (set);

		fallback = TRUE;
	}
	
	icon_set_from_data (set, icon_data, data_size, GTK_ICON_SIZE_MENU, fallback);
}

void
gdl_stock_init (void)
{
	static gboolean initialized = FALSE;
	gint i;

	if (initialized)
		return;

	gdl_stock_factory = gtk_icon_factory_new ();

	for (i = 0; i < G_N_ELEMENTS (gdl_icons); i++) {
		add_icon (gdl_stock_factory,
			  gdl_icons[i].stock_id,
			  gdl_icons[i].icon_data, 
			  gdl_icons[i].data_size);
	}

	gtk_icon_factory_add_default (gdl_stock_factory);

	initialized = TRUE;
}

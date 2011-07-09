/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gdl-icons.c
 *
 * Copyright (C) 2000-2001 Dave Camp
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
 *
 * Authors: Dave Camp, Jeroen Zwartepoorte
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gdl-i18n.h"
#include "gdl-tools.h"
#include <string.h>
#include <libgnomeui/gnome-icon-lookup.h>
#include <libgnomevfs/gnome-vfs-ops.h>
#include "gdl-icons.h"

enum {
	PROP_BOGUS,
	PROP_ICON_SIZE,
};

#define GDL_ICONS_GET_PRIVATE(obj)  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GDL_TYPE_ICONS, GdlIconsPrivate))

typedef struct _GdlIconsPrivate GdlIconsPrivate;

struct _GdlIconsPrivate {
	int icon_size;

	GtkIconTheme *icon_theme;
	GHashTable *icons;
};

GDL_CLASS_BOILERPLATE (GdlIcons, gdl_icons, GObject, G_TYPE_OBJECT);

static void
gdl_icons_get_property (GObject *object,
			guint prop_id,
			GValue *value,
			GParamSpec *pspec)
{
	GdlIconsPrivate *priv = GDL_ICONS_GET_PRIVATE (object);

	switch (prop_id) {
		case PROP_ICON_SIZE:
			g_value_set_int (value, priv->icon_size);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gdl_icons_set_property (GObject *object,
			guint prop_id,
			const GValue *value,
			GParamSpec *pspec)
{
	GdlIconsPrivate *priv = GDL_ICONS_GET_PRIVATE (object);

	switch (prop_id) {
		case PROP_ICON_SIZE:
			priv->icon_size = g_value_get_int (value);
			g_hash_table_destroy (priv->icons);
			priv->icons = g_hash_table_new_full (g_str_hash, g_str_equal,
							     (GDestroyNotify) g_free,
							     (GDestroyNotify) gdk_pixbuf_unref);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
theme_changed_cb (GtkIconTheme *theme,
		  gpointer user_data)
{
	GdlIconsPrivate *priv = GDL_ICONS_GET_PRIVATE (user_data);

	g_hash_table_destroy (priv->icons);
	priv->icons = g_hash_table_new_full (g_str_hash, g_str_equal,
					     (GDestroyNotify) g_free,
					     (GDestroyNotify) gdk_pixbuf_unref);
}

static void
gdl_icons_dispose (GObject *object)
{
	GdlIconsPrivate *priv = GDL_ICONS_GET_PRIVATE (object);

	if (priv->icon_theme) {
		/* Don't do that - look a GTK+ docs */
		/* g_object_unref (priv->icon_theme); */
		priv->icon_theme = NULL;
	}
	
	if (priv->icons) {
		g_hash_table_destroy (priv->icons);
		priv->icons = NULL;
	}
}

static void
gdl_icons_class_init (GdlIconsClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->dispose = gdl_icons_dispose;
	object_class->get_property = gdl_icons_get_property;
	object_class->set_property = gdl_icons_set_property;

	g_object_class_install_property (object_class, PROP_ICON_SIZE,
					 g_param_spec_int ("icon-size", 
							   _("Icon size"),
							   _("Icon size"),
							   12, 256, 24,
							   G_PARAM_READWRITE));

	g_type_class_add_private (object_class, sizeof (GdlIconsPrivate));
}

static void
gdl_icons_instance_init (GdlIcons *icons)
{
	GdlIconsPrivate *priv = GDL_ICONS_GET_PRIVATE (icons);

	priv->icon_theme = gtk_icon_theme_get_default ();
	/* gtk_icon_theme_get_default() does not ref the returned object */
	/* but API docs state the you should NOT ref it */
	/* g_object_ref (priv->icon_theme);*/
	g_signal_connect_object (G_OBJECT (priv->icon_theme), "changed",
				 G_CALLBACK (theme_changed_cb), icons, 0);
	priv->icons = g_hash_table_new_full (g_str_hash, g_str_equal,
					     (GDestroyNotify) g_free,
					     (GDestroyNotify) gdk_pixbuf_unref);
}

GdlIcons *
gdl_icons_new (int icon_size)
{
	return GDL_ICONS (g_object_new (GDL_TYPE_ICONS,
					"icon-size", icon_size,
					NULL));
}

GdkPixbuf *
gdl_icons_get_folder_icon (GdlIcons *icons)
{
	g_return_val_if_fail (icons != NULL, NULL);
	g_return_val_if_fail (GDL_IS_ICONS (icons), NULL);

	return gdl_icons_get_mime_icon (icons, "application/directory-normal");
}

GdkPixbuf *
gdl_icons_get_uri_icon (GdlIcons *icons,
			const char *uri)
{
	GnomeVFSFileInfo *info;
	GdkPixbuf *pixbuf;

	g_return_val_if_fail (icons != NULL, NULL);
	g_return_val_if_fail (GDL_IS_ICONS (icons), NULL);
	g_return_val_if_fail (uri != NULL, NULL);

	info = gnome_vfs_file_info_new ();
	gnome_vfs_get_file_info (uri, info,
				 GNOME_VFS_FILE_INFO_FOLLOW_LINKS |
				 GNOME_VFS_FILE_INFO_GET_MIME_TYPE |
				 GNOME_VFS_FILE_INFO_FORCE_FAST_MIME_TYPE);
	if (info->mime_type)
		pixbuf = gdl_icons_get_mime_icon (icons, info->mime_type);
	else
		pixbuf = gdl_icons_get_mime_icon (icons, "gnome-fs-regular");
	gnome_vfs_file_info_unref (info);

	return pixbuf;
}

GdkPixbuf *
gdl_icons_get_mime_icon (GdlIcons *icons,
			 const char *mime_type)
{
	GdkPixbuf *pixbuf;
	char *icon_name;

	g_return_val_if_fail (icons != NULL, NULL);
	g_return_val_if_fail (GDL_IS_ICONS (icons), NULL);
	g_return_val_if_fail (mime_type != NULL, NULL);

	GdlIconsPrivate *priv = GDL_ICONS_GET_PRIVATE (icons);

	pixbuf = g_hash_table_lookup (priv->icons, mime_type);
	if (pixbuf != NULL) {
		g_object_ref (G_OBJECT (pixbuf));
		return pixbuf;
	}

	if (!strcmp (mime_type, "application/directory-normal")) {
		icon_name = g_strdup ("gnome-fs-directory");
	} else {
		icon_name = gnome_icon_lookup (priv->icon_theme,
					       NULL,
					       NULL,
					       NULL,
					       NULL,
					       mime_type,
					       GNOME_ICON_LOOKUP_FLAGS_NONE,
					       NULL);
	}

	if (!icon_name) {
		/* Return regular icon if one doesn't exist for mime type. */
		if (!strcmp (mime_type, "gnome-fs-regular"))
			return NULL;
		else
			return gdl_icons_get_mime_icon (icons, "gnome-fs-regular");
	} else {
		if (!gtk_icon_theme_has_icon (priv->icon_theme, icon_name)) {
			g_free (icon_name);
			if (!strcmp (mime_type, "gnome-fs-regular"))
				return NULL;
			else
				return gdl_icons_get_mime_icon (icons, "gnome-fs-regular");
		} else {
			pixbuf = gtk_icon_theme_load_icon (priv->icon_theme,
							   icon_name,
							   priv->icon_size,
							   0, /* lookup flags */
							   NULL);
			g_free (icon_name);

			if (pixbuf == NULL) {
				if (!strcmp (mime_type, "gnome-fs-regular"))
					return NULL;
				else
					return gdl_icons_get_mime_icon (icons,
									"gnome-fs-regular");
			}
		}
	}

	g_hash_table_insert (priv->icons, g_strdup (mime_type), pixbuf);
	g_object_ref (pixbuf);

	return pixbuf;
}

/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gdl-icons.h
 *
 * Copyright (C) 2000-2001 JP Rosevear
 *               2000 Dave Camp
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors: JP Rosevear, Dave Camp, Jeroen Zwartepoorte
 */

#ifndef _GDL_ICONS_H_
#define _GDL_ICONS_H_

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

#define GDL_TYPE_ICONS			(gdl_icons_get_type ())
#define GDL_ICONS(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GDL_TYPE_ICONS, GdlIcons))
#define GDL_ICONS_CLASS(obj)		(G_TYPE_CHECK_CLASS_CAST ((klass), GDL_TYPE_ICONS, GdlIconsClass))
#define GDL_IS_ICONS(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDL_TYPE_ICONS))
#define GDL_IS_ICONS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((obj), GDL_TYPE_ICONS))

typedef struct _GdlIcons		GdlIcons;
typedef struct _GdlIconsClass		GdlIconsClass;

struct _GdlIcons {
	GObject parent;
};

struct _GdlIconsClass {
	GObjectClass parent_class;
};

GType gdl_icons_get_type             (void);
GdlIcons *gdl_icons_new              (int         icon_size);

GdkPixbuf *gdl_icons_get_folder_icon (GdlIcons   *icons);
GdkPixbuf *gdl_icons_get_uri_icon    (GdlIcons   *icons,
				      const char *uri);
GdkPixbuf *gdl_icons_get_mime_icon   (GdlIcons   *icons,
				      const char *mime_type);

G_END_DECLS

#endif /* _GDL_ICONS_H_ */

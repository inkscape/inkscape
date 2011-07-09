/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- 
 * gdl-combo-button.h
 * 
 * Copyright (C) 2003 Jeroen Zwartepoorte
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

#ifndef _GDL_COMBO_BUTTON_H_
#define _GDL_COMBO_BUTTON_H_

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkmenu.h>

G_BEGIN_DECLS

#define GDL_TYPE_COMBO_BUTTON		 (gdl_combo_button_get_type ())
#define GDL_COMBO_BUTTON(obj)		 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDL_TYPE_COMBO_BUTTON, GdlComboButton))
#define GDL_COMBO_BUTTON_CLASS(klass)	 (G_TYPE_CHECK_CLASS_CAST ((klass), GDL_TYPE_COMBO_BUTTON, GdlComboButtonClass))
#define GDL_IS_COMBO_BUTTON(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDL_TYPE_COMBO_BUTTON))
#define GDL_IS_COMBO_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), GDL_TYPE_COMBO_BUTTON))

typedef struct _GdlComboButton        GdlComboButton;
typedef struct _GdlComboButtonPrivate GdlComboButtonPrivate;
typedef struct _GdlComboButtonClass   GdlComboButtonClass;

struct _GdlComboButton {
	GtkHBox parent;

	GdlComboButtonPrivate *priv;
};

struct _GdlComboButtonClass {
	GtkHBoxClass parent_class;

	/* Signals. */
	void (* activate_default) (GdlComboButton *combo_button);
};

GType      gdl_combo_button_get_type       (void);
GtkWidget *gdl_combo_button_new            (void);

void       gdl_combo_button_set_icon   (GdlComboButton *combo_button,
					GdkPixbuf      *pixbuf);
void       gdl_combo_button_set_label  (GdlComboButton *combo_button,
					const gchar    *label);
void       gdl_combo_button_set_menu   (GdlComboButton *combo_button,
					GtkMenu        *menu);

G_END_DECLS

#endif /* _GDL_COMBO_BUTTON_H_ */

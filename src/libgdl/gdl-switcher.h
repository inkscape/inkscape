/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 8 -*- */
/* gdl-switcher.h
 *
 * Copyright (C) 2003  Ettore Perazzoli
 *               2007  Naba Kumar
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
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
 * Authors: Ettore Perazzoli <ettore@ximian.com>
 *          Naba Kumar  <naba@gnome.org>
 */

#ifndef _GDL_SWITCHER_H_
#define _GDL_SWITCHER_H_

#include <gtk/gtknotebook.h>

G_BEGIN_DECLS

#define GDL_TYPE_SWITCHER            (gdl_switcher_get_type ())
#define GDL_SWITCHER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDL_TYPE_SWITCHER, GdlSwitcher))
#define GDL_SWITCHER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GDL_TYPE_SWITCHER, GdlSwitcherClass))
#define GDL_IS_SWITCHER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDL_TYPE_SWITCHER))
#define GDL_IS_SWITCHER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), GDL_TYPE_SWITCHER))

typedef struct _GdlSwitcher        GdlSwitcher;
typedef struct _GdlSwitcherPrivate GdlSwitcherPrivate;
typedef struct _GdlSwitcherClass   GdlSwitcherClass;

typedef enum {
    GDL_SWITCHER_STYLE_TEXT,
    GDL_SWITCHER_STYLE_ICON,
    GDL_SWITCHER_STYLE_BOTH,
    GDL_SWITCHER_STYLE_TOOLBAR,
    GDL_SWITCHER_STYLE_TABS
} GdlSwitcherStyle;

struct _GdlSwitcher {
    GtkNotebook parent;

    GdlSwitcherPrivate *priv;
};

struct _GdlSwitcherClass {
    GtkNotebookClass parent_class;
};

GType      gdl_switcher_get_type     (void);
GtkWidget *gdl_switcher_new          (void);

gint       gdl_switcher_insert_page  (GdlSwitcher *switcher,
                                      GtkWidget   *page,
                                      GtkWidget   *tab_widget,
                                      const gchar *label,
                                      const gchar *tooltips,
                                      const gchar *stock_id,
                                      const GdkPixbuf *pixbuf_icon,
                                      gint position);
G_END_DECLS

#endif /* _GDL_SWITCHER_H_ */

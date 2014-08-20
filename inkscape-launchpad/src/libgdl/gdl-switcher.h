/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 8 -*- */
/* gdl-switcher.h
 *
 * Copyright (C) 2003  Ettore Perazzoli
 *               2007  Naba Kumar
 *
* This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *
 * Authors: Ettore Perazzoli <ettore@ximian.com>
 *          Naba Kumar  <naba@gnome.org>
 */

#ifndef _GDL_SWITCHER_H_
#define _GDL_SWITCHER_H_

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GDL_TYPE_SWITCHER            (gdl_switcher_get_type ())
#define GDL_SWITCHER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDL_TYPE_SWITCHER, GdlSwitcher))
#define GDL_SWITCHER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GDL_TYPE_SWITCHER, GdlSwitcherClass))
#define GDL_IS_SWITCHER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDL_TYPE_SWITCHER))
#define GDL_IS_SWITCHER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), GDL_TYPE_SWITCHER))

typedef struct _GdlSwitcher        GdlSwitcher;
typedef struct _GdlSwitcherPrivate GdlSwitcherPrivate;
typedef struct _GdlSwitcherClass   GdlSwitcherClass;

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
                                      GdkPixbuf *pixbuf_icon,
                                      gint position);
G_END_DECLS

#endif /* _GDL_SWITCHER_H_ */

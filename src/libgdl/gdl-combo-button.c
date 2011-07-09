/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- 
 * gdl-combo-button.c
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
#include "gdl-tools.h"
#include "gdl-combo-button.h"

struct _GdlComboButtonPrivate {
	GtkWidget *default_button;
	GtkWidget *image;
	GtkWidget *label;
	GtkWidget *menu_button;
	GtkWidget *menu;
	gboolean   menu_popped_up;
};

GDL_CLASS_BOILERPLATE (GdlComboButton, gdl_combo_button, GtkHBox, GTK_TYPE_HBOX);

static void
default_button_clicked_cb (GtkButton *button,
			   gpointer user_data)
{
	GdlComboButton *combo;
	GdlComboButtonPrivate *priv;

	combo = GDL_COMBO_BUTTON (user_data);
	priv = combo->priv;

	if (!priv->menu_popped_up)
		g_signal_emit_by_name (G_OBJECT (combo),
				       "activate-default", NULL);
}

static gboolean
default_button_press_event_cb (GtkWidget *widget,
			       GdkEventButton *event,
			       gpointer user_data)
{
	GdlComboButton *combo_button;
	GdlComboButtonPrivate *priv;

	combo_button = GDL_COMBO_BUTTON (user_data);
	priv = combo_button->priv;

	if (event->type == GDK_BUTTON_PRESS && event->button == 1) {
		GTK_BUTTON (priv->menu_button)->button_down = TRUE;
		gtk_button_pressed (GTK_BUTTON (priv->menu_button));
	}

	return FALSE;
}

static gboolean
default_button_release_event_cb (GtkWidget *widget,
				 GdkEventButton *event,
				 gpointer user_data)
{
	GdlComboButton *combo_button;
	GdlComboButtonPrivate *priv;

	combo_button = GDL_COMBO_BUTTON (user_data);
	priv = combo_button->priv;

	if (event->button == 1) {
		gtk_button_released (GTK_BUTTON (priv->menu_button));
	}

	return FALSE;
}

static gboolean
button_enter_notify_cb (GtkWidget *widget,
			GdkEventCrossing *event,
			gpointer user_data)
{
	GdlComboButton *combo_button;
	GdlComboButtonPrivate *priv;

	combo_button = GDL_COMBO_BUTTON (user_data);
	priv = combo_button->priv;

	if (event->detail != GDK_NOTIFY_INFERIOR) {
		GTK_BUTTON (priv->default_button)->in_button = TRUE;
		GTK_BUTTON (priv->menu_button)->in_button = TRUE;
		gtk_button_enter (GTK_BUTTON (priv->default_button));
		gtk_button_enter (GTK_BUTTON (priv->menu_button));
	}

	return TRUE;
}

static gboolean
button_leave_notify_cb (GtkWidget *widget,
			GdkEventCrossing *event,
			gpointer user_data)
{
	GdlComboButton *combo_button;
	GdlComboButtonPrivate *priv;

	combo_button = GDL_COMBO_BUTTON (user_data);
	priv = combo_button->priv;

	if (priv->menu_popped_up)
		return TRUE;

	if (event->detail != GDK_NOTIFY_INFERIOR) {
		GTK_BUTTON (priv->default_button)->in_button = FALSE;
		GTK_BUTTON (priv->menu_button)->in_button = FALSE;
		gtk_button_leave (GTK_BUTTON (priv->default_button));
		gtk_button_leave (GTK_BUTTON (priv->menu_button));
	}

	return TRUE;
}

static void
menu_position_func (GtkMenu *menu,
		    gint *x_return,
		    gint *y_return,
		    gboolean *push_in,
		    gpointer user_data)
{
	GdlComboButton *combo_button;
	GdlComboButtonPrivate *priv;
	GtkAllocation *allocation;

	combo_button = GDL_COMBO_BUTTON (user_data);
	priv = combo_button->priv;
	allocation = &(priv->default_button->allocation);

	gdk_window_get_origin (priv->default_button->window, x_return, y_return);

	*x_return += allocation->x; 
	*y_return += allocation->height;
}

static gboolean
menu_button_press_event_cb (GtkWidget *widget,
			    GdkEventButton *event,
			    gpointer user_data)
{
	GdlComboButton *combo_button;
	GdlComboButtonPrivate *priv;

	combo_button = GDL_COMBO_BUTTON (user_data);
	priv = combo_button->priv;

	if (event->type == GDK_BUTTON_PRESS && 
	    (event->button == 1 || event->button == 3)) {
		GTK_BUTTON (priv->menu_button)->button_down = TRUE;

		gtk_button_pressed (GTK_BUTTON (priv->menu_button));

		priv->menu_popped_up = TRUE;
		gtk_menu_popup (GTK_MENU (priv->menu), NULL, NULL,
				menu_position_func, combo_button,
				event->button, event->time);
	}

	return TRUE;
}

static void
menu_deactivate_cb (GtkMenuShell *menu_shell,
		    gpointer user_data)
{
	GdlComboButton *combo_button;
	GdlComboButtonPrivate *priv;

	combo_button = GDL_COMBO_BUTTON (user_data);
	priv = combo_button->priv;

	priv->menu_popped_up = FALSE;

	GTK_BUTTON (priv->menu_button)->button_down = FALSE;
	GTK_BUTTON (priv->menu_button)->in_button = FALSE;
	GTK_BUTTON (priv->default_button)->in_button = FALSE;
	gtk_button_leave (GTK_BUTTON (priv->menu_button));
	gtk_button_leave (GTK_BUTTON (priv->default_button));
	gtk_button_clicked (GTK_BUTTON (priv->menu_button));
}

static void
menu_detacher (GtkWidget *widget,
	       GtkMenu *menu)
{
	GdlComboButton *combo_button;

	combo_button = GDL_COMBO_BUTTON (widget);

	g_signal_handlers_disconnect_by_func (G_OBJECT (menu),
					      menu_deactivate_cb,
					      combo_button);
	combo_button->priv->menu = NULL;
}

static void
gdl_combo_button_destroy (GtkObject *object)
{
	GdlComboButton *combo_button;
	GdlComboButtonPrivate *priv;

	combo_button = GDL_COMBO_BUTTON (object);
	priv = combo_button->priv;

	if (priv) {
		g_free (priv);
		combo_button->priv = NULL;
	}
	
	(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gdl_combo_button_class_init (GdlComboButtonClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = GTK_OBJECT_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);

	object_class->destroy = gdl_combo_button_destroy;

	g_signal_new ("activate-default",
		      G_TYPE_FROM_CLASS (klass),
		      G_SIGNAL_RUN_FIRST,
		      G_STRUCT_OFFSET (GdlComboButtonClass, activate_default),
		      NULL, NULL,
		      g_cclosure_marshal_VOID__VOID,
		      G_TYPE_NONE, 0);
}

static void
gdl_combo_button_instance_init (GdlComboButton *combo_button)
{
	GdlComboButtonPrivate *priv;
	GtkWidget *hbox, *align, *arrow;

	priv = g_new (GdlComboButtonPrivate, 1);
	combo_button->priv = priv;

	priv->menu = NULL;
	priv->menu_popped_up = FALSE;

	priv->default_button = gtk_button_new ();
	gtk_button_set_relief (GTK_BUTTON (priv->default_button), GTK_RELIEF_NONE);

	/* Following code copied from gtk_button_construct_child. */
	priv->label = gtk_label_new ("");
	gtk_label_set_use_underline (GTK_LABEL (priv->label), TRUE);
	gtk_label_set_mnemonic_widget (GTK_LABEL (priv->label),
				       priv->default_button);
      
	priv->image = gtk_image_new ();
	hbox = gtk_hbox_new (FALSE, 2);

	align = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
      
	gtk_box_pack_start (GTK_BOX (hbox), priv->image, FALSE, FALSE, 0);
	gtk_box_pack_end (GTK_BOX (hbox), priv->label, FALSE, FALSE, 0);
      
	gtk_container_add (GTK_CONTAINER (priv->default_button), align);
	gtk_container_add (GTK_CONTAINER (align), hbox);
	/* End copied block. */

	gtk_box_pack_start (GTK_BOX (combo_button), priv->default_button,
			    FALSE, FALSE, 0);
	gtk_widget_show_all (priv->default_button);

	priv->menu_button = gtk_button_new ();
	gtk_button_set_relief (GTK_BUTTON (priv->menu_button), GTK_RELIEF_NONE);
	arrow = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (priv->menu_button), arrow);
	gtk_box_pack_start (GTK_BOX (combo_button), priv->menu_button, FALSE,
			    FALSE, 0);
	gtk_widget_show_all (priv->menu_button);

	/* Default button. */
	g_signal_connect (G_OBJECT (priv->default_button), "clicked",
			  G_CALLBACK (default_button_clicked_cb), combo_button);
	g_signal_connect (G_OBJECT (priv->default_button), "button_press_event",
			  G_CALLBACK (default_button_press_event_cb), combo_button);
	g_signal_connect (G_OBJECT (priv->default_button), "button_release_event",
			  G_CALLBACK (default_button_release_event_cb), combo_button);
	g_signal_connect (G_OBJECT (priv->default_button), "enter_notify_event",
			  G_CALLBACK (button_enter_notify_cb), combo_button);
	g_signal_connect (G_OBJECT (priv->default_button), "leave_notify_event",
			  G_CALLBACK (button_leave_notify_cb), combo_button);

	/* Menu button. */
	g_signal_connect (G_OBJECT (priv->menu_button), "button_press_event",
			  G_CALLBACK (menu_button_press_event_cb), combo_button);
	g_signal_connect (G_OBJECT (priv->menu_button), "enter_notify_event",
			  G_CALLBACK (button_enter_notify_cb), combo_button);
	g_signal_connect (G_OBJECT (priv->menu_button), "leave_notify_event",
			  G_CALLBACK (button_leave_notify_cb), combo_button);
}

GtkWidget *
gdl_combo_button_new (void)
{
	GtkWidget *combo_button;
	
	combo_button = GTK_WIDGET (g_object_new (GDL_TYPE_COMBO_BUTTON, NULL));

	return combo_button;
}

void
gdl_combo_button_set_icon (GdlComboButton *combo_button,
			   GdkPixbuf *pixbuf)
{
	GdlComboButtonPrivate *priv;

	g_return_if_fail (GDL_IS_COMBO_BUTTON (combo_button));
	g_return_if_fail (GDK_IS_PIXBUF (pixbuf));

	priv = combo_button->priv;

	gtk_image_set_from_pixbuf (GTK_IMAGE (priv->image), pixbuf);
}

void
gdl_combo_button_set_label (GdlComboButton *combo_button,
			    const gchar *label)
{
	GdlComboButtonPrivate *priv;

	g_return_if_fail (GDL_IS_COMBO_BUTTON (combo_button));
	g_return_if_fail (label != NULL);

	priv = combo_button->priv;

	gtk_label_set_text (GTK_LABEL (priv->label), label);
}

void
gdl_combo_button_set_menu (GdlComboButton *combo_button,
			   GtkMenu *menu)
{
	GdlComboButtonPrivate *priv;

	g_return_if_fail (GDL_IS_COMBO_BUTTON (combo_button));
	g_return_if_fail (GTK_IS_MENU (menu));

	priv = combo_button->priv;

	if (priv->menu != NULL)
		gtk_menu_detach (GTK_MENU (priv->menu));

	priv->menu = GTK_WIDGET (menu);
	if (menu == NULL)
		return;

	gtk_menu_attach_to_widget (menu, GTK_WIDGET (combo_button), menu_detacher);

	g_signal_connect (G_OBJECT (menu), "deactivate",
			  G_CALLBACK (menu_deactivate_cb), combo_button);
}

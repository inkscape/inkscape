/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* test-combo-button.c
 *
 * Copyright (C) 2003 Jeroen Zwartepoorte
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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include "gdl-combo-button.h"

static void
combo_button_activate_default_cb (GdlComboButton *combo,
				  gpointer data)
{
	g_message ("combo_button_activate_default_cb");
}

int
main (int argc, char **argv)
{
	GtkWidget *window, *hbox, *combo, *menu, *menuitem;
	GdkPixbuf *icon;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect (G_OBJECT (window), "delete_event", 
			  G_CALLBACK (gtk_main_quit), NULL);
	gtk_window_set_title (GTK_WINDOW (window), "Combo button test");
	gtk_window_set_resizable (GTK_WINDOW (window), FALSE);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), hbox);

	combo = gtk_button_new_from_stock (GTK_STOCK_OPEN);
	gtk_button_set_relief (GTK_BUTTON (combo), GTK_RELIEF_NONE);
	gtk_box_pack_start (GTK_BOX (hbox), combo, FALSE, FALSE, 0);	

	menu = gtk_menu_new ();
	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_OPEN, NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_SAVE, NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	gtk_widget_show_all (menu);
	
	combo = gdl_combo_button_new ();
	gdl_combo_button_set_label (GDL_COMBO_BUTTON (combo), "Run");
	gdl_combo_button_set_menu (GDL_COMBO_BUTTON (combo), GTK_MENU (menu));
	icon = gtk_widget_render_icon (combo, GTK_STOCK_EXECUTE,
				       GTK_ICON_SIZE_LARGE_TOOLBAR, NULL);
	gdl_combo_button_set_icon (GDL_COMBO_BUTTON (combo), icon);
	gtk_box_pack_start (GTK_BOX (hbox), combo, FALSE, FALSE, 0);

	g_signal_connect (combo, "activate_default",
			  G_CALLBACK (combo_button_activate_default_cb), NULL);

	combo = gtk_button_new_from_stock (GTK_STOCK_SAVE);
	gtk_button_set_relief (GTK_BUTTON (combo), GTK_RELIEF_NONE);
	gtk_box_pack_start (GTK_BOX (hbox), combo, FALSE, FALSE, 0);	

	menu = gtk_menu_new ();
	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_OPEN, NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_SAVE, NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	gtk_widget_show_all (menu);
	
	combo = gdl_combo_button_new ();
	gdl_combo_button_set_label (GDL_COMBO_BUTTON (combo), "Open");
	gdl_combo_button_set_menu (GDL_COMBO_BUTTON (combo), GTK_MENU (menu));
	icon = gtk_widget_render_icon (combo, GTK_STOCK_OPEN,
				       GTK_ICON_SIZE_LARGE_TOOLBAR, NULL);
	gdl_combo_button_set_icon (GDL_COMBO_BUTTON (combo), icon);
	gtk_widget_set_sensitive (combo, FALSE);
	gtk_box_pack_start (GTK_BOX (hbox), combo, FALSE, FALSE, 0);

	g_signal_connect (combo, "activate_default",
			  G_CALLBACK (combo_button_activate_default_cb), NULL);

	menu = gtk_menu_new ();
	combo = gdl_combo_button_new ();
	gdl_combo_button_set_label (GDL_COMBO_BUTTON (combo), "Open");
	gdl_combo_button_set_menu (GDL_COMBO_BUTTON (combo), GTK_MENU (menu));
	icon = gtk_widget_render_icon (combo, GTK_STOCK_OPEN,
				       GTK_ICON_SIZE_LARGE_TOOLBAR, NULL);
	gdl_combo_button_set_icon (GDL_COMBO_BUTTON (combo), icon);
	gtk_box_pack_start (GTK_BOX (hbox), combo, FALSE, FALSE, 0);

	gtk_widget_show_all (window);
	
	gtk_main ();

	return 0;
}

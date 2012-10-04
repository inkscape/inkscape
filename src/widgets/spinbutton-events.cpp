/*
 * Common callbacks for spinbuttons
 *
 * Authors:
 *   bulia byak <bulia@users.sourceforge.net>
 *
 * Copyright (C) 2003 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "../event-context.h"

#include "sp-widget.h"
#include "widget-sizes.h"
#include "spinbutton-events.h"

gboolean
spinbutton_focus_in (GtkWidget *w, GdkEventKey */*event*/, gpointer /*data*/)
{
	gdouble *ini;

	ini = (gdouble *) g_object_get_data(G_OBJECT (w), "ini");
	if (ini) g_free (ini); // free the old value if any

	// retrieve the value
	ini = g_new (gdouble, 1);
	*ini = gtk_spin_button_get_value (GTK_SPIN_BUTTON(w));

	// remember it
	g_object_set_data (G_OBJECT (w), "ini", ini);

	return FALSE; // I didn't consume the event
}

void
spinbutton_undo (GtkWidget *w)
{
	gdouble *ini = (gdouble *) g_object_get_data(G_OBJECT (w), "ini");
	if (ini) {
	    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), *ini);
	}
}

void
spinbutton_defocus (GtkWidget *container)
{
	// defocus spinbuttons by moving focus to the canvas, unless "stay" is on
	gboolean stay = GPOINTER_TO_INT(g_object_get_data(G_OBJECT (container), "stay"));
	if (stay) {
		g_object_set_data (G_OBJECT (container), "stay", GINT_TO_POINTER (FALSE));
	} else {
		GtkWidget *canvas = (GtkWidget *) g_object_get_data(G_OBJECT (container), "dtw");
		if (canvas) {
			gtk_widget_grab_focus (GTK_WIDGET(canvas));
		}
	}
}

gboolean
spinbutton_keypress (GtkWidget *w, GdkEventKey *event, gpointer data)
{
    SPWidget *spw = (SPWidget *) data;
    gdouble v;
    gdouble step;
    gdouble page;

	switch (get_group0_keyval (event)) {
	case GDK_KEY_Escape: // defocus
		spinbutton_undo (w);
		spinbutton_defocus(GTK_WIDGET(spw));
		return TRUE; // I consumed the event
		break;
	case GDK_KEY_Return: // defocus
	case GDK_KEY_KP_Enter:
		spinbutton_defocus (GTK_WIDGET(spw));
		return TRUE; // I consumed the event
		break;
	case GDK_KEY_Tab:
	case GDK_KEY_ISO_Left_Tab:
		// set the flag meaning "do not leave toolbar when changing value"
		g_object_set_data (G_OBJECT (spw), "stay", GINT_TO_POINTER(TRUE));
		return FALSE; // I didn't consume the event
		break;

	// The following keys are processed manually because GTK implements them in strange ways
	// (increments start with double step value and seem to grow as you press the key continuously)

	case GDK_KEY_Up:
	case GDK_KEY_KP_Up:
		g_object_set_data (G_OBJECT (spw), "stay", GINT_TO_POINTER(TRUE));
		v = gtk_spin_button_get_value(GTK_SPIN_BUTTON (w));
	    gtk_spin_button_get_increments(GTK_SPIN_BUTTON (w), &step, &page);
		v += step;
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), v);
		return TRUE; // I consumed the event
		break;
	case GDK_KEY_Down:
	case GDK_KEY_KP_Down:
		g_object_set_data (G_OBJECT (spw), "stay", GINT_TO_POINTER(TRUE));
		v = gtk_spin_button_get_value(GTK_SPIN_BUTTON (w));
	    gtk_spin_button_get_increments(GTK_SPIN_BUTTON (w), &step, &page);
		v -= step;
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), v);
		return TRUE; // I consumed the event
		break;
	case GDK_KEY_Page_Up:
	case GDK_KEY_KP_Page_Up:
		g_object_set_data (G_OBJECT (spw), "stay", GINT_TO_POINTER(TRUE));
		v = gtk_spin_button_get_value(GTK_SPIN_BUTTON (w));
	    gtk_spin_button_get_increments(GTK_SPIN_BUTTON (w), &step, &page);
		v += page;
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), v);
		return TRUE; // I consumed the event
		break;
	case GDK_KEY_Page_Down:
	case GDK_KEY_KP_Page_Down:
		g_object_set_data (G_OBJECT (spw), "stay", GINT_TO_POINTER(TRUE));
		v = gtk_spin_button_get_value(GTK_SPIN_BUTTON (w));
	    gtk_spin_button_get_increments(GTK_SPIN_BUTTON (w), &step, &page);
		v -= page;
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), v);
		return TRUE; // I consumed the event
		break;
	case GDK_KEY_z:
	case GDK_KEY_Z:
		g_object_set_data (G_OBJECT (spw), "stay", GINT_TO_POINTER(TRUE));
		if (event->state & GDK_CONTROL_MASK) {
			spinbutton_undo (w);
			return TRUE; // I consumed the event
		}
		break;
	default:
		return FALSE;
		break;
	}
	return FALSE; // I didn't consume the event
}

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


gboolean
spinbutton_focus_in (GtkWidget *w, GdkEventKey */*event*/, gpointer /*data*/)
{
	gdouble *ini;

	ini = (gdouble *) gtk_object_get_data (GTK_OBJECT (w), "ini");
	if (ini) g_free (ini); // free the old value if any

	// retrieve the value
	ini = g_new (gdouble, 1);
	*ini = gtk_spin_button_get_value (GTK_SPIN_BUTTON(w));

	// remember it
	gtk_object_set_data (GTK_OBJECT (w), "ini", ini);

	return FALSE; // I didn't consume the event
}

void
spinbutton_undo (GtkWidget *w)
{
	gdouble *ini = (gdouble *) gtk_object_get_data (GTK_OBJECT (w), "ini");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), *ini);
}

void
spinbutton_defocus (GtkObject *container)
{
	// defocus spinbuttons by moving focus to the canvas, unless "stay" is on
	gboolean stay = GPOINTER_TO_INT(gtk_object_get_data (GTK_OBJECT (container), "stay"));
	if (stay) {
		gtk_object_set_data (GTK_OBJECT (container), "stay", GINT_TO_POINTER (FALSE));
	} else {
		GtkWidget *canvas = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (container), "dtw");
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

	switch (get_group0_keyval (event)) {
	case GDK_Escape: // defocus
		spinbutton_undo (w);
		spinbutton_defocus (GTK_OBJECT (spw));
		return TRUE; // I consumed the event
		break;
	case GDK_Return: // defocus
	case GDK_KP_Enter:
		spinbutton_defocus (GTK_OBJECT (spw));
		return TRUE; // I consumed the event
		break;
	case GDK_Tab:
	case GDK_ISO_Left_Tab:
		// set the flag meaning "do not leave toolbar when changing value"
		gtk_object_set_data (GTK_OBJECT (spw), "stay", GINT_TO_POINTER(TRUE));
		return FALSE; // I didn't consume the event
		break;

	// The following keys are processed manually because GTK implements them in strange ways
	// (increments start with double step value and seem to grow as you press the key continuously)

	case GDK_Up:
	case GDK_KP_Up:
		gtk_object_set_data (GTK_OBJECT (spw), "stay", GINT_TO_POINTER(TRUE));
		v = gtk_spin_button_get_value(GTK_SPIN_BUTTON (w));
		v += SPIN_STEP;
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), v);
		return TRUE; // I consumed the event
		break;
	case GDK_Down:
	case GDK_KP_Down:
		gtk_object_set_data (GTK_OBJECT (spw), "stay", GINT_TO_POINTER(TRUE));
		v = gtk_spin_button_get_value(GTK_SPIN_BUTTON (w));
		v -= SPIN_STEP;
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), v);
		return TRUE; // I consumed the event
		break;
	case GDK_Page_Up:
	case GDK_KP_Page_Up:
		gtk_object_set_data (GTK_OBJECT (spw), "stay", GINT_TO_POINTER(TRUE));
		v = gtk_spin_button_get_value(GTK_SPIN_BUTTON (w));
		v += SPIN_PAGE_STEP;
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), v);
		return TRUE; // I consumed the event
		break;
	case GDK_Page_Down:
	case GDK_KP_Page_Down:
		gtk_object_set_data (GTK_OBJECT (spw), "stay", GINT_TO_POINTER(TRUE));
		v = gtk_spin_button_get_value(GTK_SPIN_BUTTON (w));
		v -= SPIN_PAGE_STEP;
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), v);
		return TRUE; // I consumed the event
		break;
	case GDK_z:
	case GDK_Z:
		gtk_object_set_data (GTK_OBJECT (spw), "stay", GINT_TO_POINTER(TRUE));
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

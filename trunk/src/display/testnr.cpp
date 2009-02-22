#include <gtk/gtk.h>
#include "sp-arena.h"

int
main (int argc, char ** argv)
{
	GtkWidget * w, * c;

	gtk_init (&argc, &argv);

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	c = sp_arena_new ();
	gtk_widget_show (c);

	gtk_container_add (GTK_CONTAINER (w), c);

	gtk_widget_show (w);

	gtk_main ();

	return 0;
}


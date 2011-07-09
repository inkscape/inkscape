#include <config.h>
#include <gtk/gtk.h>
#include <libgnome/libgnome.h>

#include "gdl-data-view.h"
#include "gdl-data-model-test.h"

int
main (int argc, char *argv[])
{
	GtkWidget *win;
	GtkWidget *view;
	GdlDataModel *model;
	GtkWidget *vbox;

	gtk_init (&argc, &argv);

	win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (win), 500, 200);

	vbox = gtk_vbox_new (FALSE, 5);

	view = gdl_data_view_new ();

	gtk_layout_set_hadjustment (GTK_LAYOUT (view), NULL);
	gtk_layout_set_vadjustment (GTK_LAYOUT (view), NULL);

	
	model = GDL_DATA_MODEL (gdl_data_model_test_new ());
	gdl_data_view_set_model (GDL_DATA_VIEW (view), 
				 model);

	gtk_box_pack_start (GTK_BOX (vbox), view, TRUE, TRUE, 0);
	
	gtk_container_add (GTK_CONTAINER (win), vbox);

	gtk_widget_show_all (win);
	gtk_widget_grab_focus (GTK_WIDGET (view));

	gtk_main ();

	return 0;
}

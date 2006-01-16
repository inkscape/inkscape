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

#include <glib/gtypes.h>
#include <gtk/gtkstyle.h>      /* GtkWidget */
#include <gtk/gtktypeutils.h>  /* GtkObject */

gboolean spinbutton_focus_in (GtkWidget *w, GdkEventKey *event, gpointer data);
void spinbutton_undo (GtkWidget *w);
gboolean spinbutton_keypress (GtkWidget *w, GdkEventKey *event, gpointer data);
void spinbutton_defocus (GtkObject *container);

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

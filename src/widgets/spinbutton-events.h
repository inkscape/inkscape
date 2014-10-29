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

#include <glib.h>

typedef struct _GdkEventKey GdkEventKey;
typedef struct _GtkWidget   GtkWidget;

gboolean spinbutton_focus_in (GtkWidget *w, GdkEventKey *event, gpointer data);
void spinbutton_undo (GtkWidget *w);
gboolean spinbutton_keypress (GtkWidget *w, GdkEventKey *event, gpointer data);
void spinbutton_defocus (GtkWidget *container);

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

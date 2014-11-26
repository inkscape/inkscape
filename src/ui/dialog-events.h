/** @file
 * @brief  Event handler for dialog windows
 */
/* Authors:
 *   bulia byak <bulia@dr.com>
 *
 * Copyright (C) 2003-2014 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_DIALOG_EVENTS_H
#define SEEN_DIALOG_EVENTS_H

 
/*
 * event callback can only accept one argument, but we need two,
 * hence this struct.
 * each dialog has a local static copy:
 *   win is the dialog window 
 *   stop is the transientize semaphore: when 0, retransientizing this dialog 
 *   is allowed
 */

namespace Gtk {
class Window;
class Entry;
}

class SPDesktop;
 
typedef struct { 
    GtkWidget *win;
    guint stop;
} win_data;


gboolean sp_dialog_event_handler ( GtkWindow *win, 
                                   GdkEvent *event, 
                                   gpointer data );

void sp_dialog_defocus_cpp         (Gtk::Window *win);
void sp_dialog_defocus_callback_cpp(Gtk::Entry *e);
void sp_dialog_defocus_on_enter_cpp(Gtk::Entry *e);

void sp_dialog_defocus           ( GtkWindow *win );
void sp_dialog_defocus_callback  ( GtkWindow *win, gpointer data );
void sp_dialog_defocus_on_enter  ( GtkWidget *w );
void sp_transientize             ( GtkWidget *win );

void on_transientize             ( SPDesktop *desktop, 
                                   win_data *wd );

void sp_transientize_callback    ( SPDesktop *desktop, 
                                   win_data *wd );

void on_dialog_hide (GtkWidget *w);
void on_dialog_unhide (GtkWidget *w);

//gboolean sp_dialog_hide (GObject *object, gpointer data);
//gboolean sp_dialog_unhide (GObject *object, gpointer data);

#endif

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

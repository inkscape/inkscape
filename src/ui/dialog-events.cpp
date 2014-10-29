/**
 * @file
 * Event handler for dialog windows.
 */
/* Authors:
 *   bulia byak <bulia@dr.com>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *
 * Copyright (C) 2003-2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if GLIBMM_DISABLE_DEPRECATED && HAVE_GLIBMM_THREADS_H
#include <glibmm/threads.h>
#endif

#include <gtkmm/entry.h>
#include <gtkmm/window.h>
#include <gdk/gdkkeysyms.h>
#include "macros.h"
#include <gtk/gtk.h>
#include "desktop.h"
#include "inkscape-private.h"
#include "preferences.h"
#include "ui/tools/tool-base.h"

#include "ui/dialog-events.h"


/**
 * Remove focus from window to whoever it is transient for.
 */
void sp_dialog_defocus_cpp(Gtk::Window *win)
{
    //find out the document window we're transient for
    Gtk::Window *w = win->get_transient_for();

    //switch to it
    if (w) {
        w->present();
    }
}

void
sp_dialog_defocus (GtkWindow *win)
{
    GtkWindow *w;
    //find out the document window we're transient for
    w = gtk_window_get_transient_for(GTK_WINDOW(win));
    //switch to it

    if (w) {
        gtk_window_present (w);
    }
}


/**
 * Callback to defocus a widget's parent dialog.
 */
void sp_dialog_defocus_callback_cpp(Gtk::Entry *e)
{
    sp_dialog_defocus_cpp(dynamic_cast<Gtk::Window *>(e->get_toplevel()));
}

void
sp_dialog_defocus_callback (GtkWindow * /*win*/, gpointer data)
{
    sp_dialog_defocus( GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(data))) );
}



void
sp_dialog_defocus_on_enter_cpp (Gtk::Entry *e)
{
    e->signal_activate().connect(sigc::bind(sigc::ptr_fun(&sp_dialog_defocus_callback_cpp), e));
}

void
sp_dialog_defocus_on_enter (GtkWidget *w)
{
    g_signal_connect ( G_OBJECT (w), "activate",
                       G_CALLBACK (sp_dialog_defocus_callback), w );
}



gboolean
sp_dialog_event_handler (GtkWindow *win, GdkEvent *event, gpointer data)
{

// if the focus is inside the Text and Font textview, do nothing
    GObject *dlg = G_OBJECT(data);
    if (g_object_get_data (dlg, "eatkeys")) {
        return FALSE;
    }

    gboolean ret = FALSE;

    switch (event->type) {

        case GDK_KEY_PRESS:

            switch (Inkscape::UI::Tools::get_group0_keyval (&event->key)) {
                case GDK_KEY_Escape:
                    sp_dialog_defocus (win);
                    ret = TRUE;
                    break;
                case GDK_KEY_F4:
                case GDK_KEY_w:
                case GDK_KEY_W:
                    // close dialog
                    if (MOD__CTRL_ONLY(event)) {

                        /* this code sends a delete_event to the dialog,
                         * instead of just destroying it, so that the
                         * dialog can do some housekeeping, such as remember
                         * its position.
                         */
                        GdkEventAny event;
                        GtkWidget *widget = GTK_WIDGET(win);
                        event.type = GDK_DELETE;
                        event.window = gtk_widget_get_window (widget);
                        event.send_event = TRUE;
                        g_object_ref (G_OBJECT (event.window));
                        gtk_main_do_event(reinterpret_cast<GdkEvent*>(&event));
                        g_object_unref (G_OBJECT (event.window));

                        ret = TRUE;
                    }
                    break;
                default: // pass keypress to the canvas
                    break;
            }
    default:
        ;
    }

    return ret;

}



/**
 * Make the argument dialog transient to the currently active document
 * window.
 */
void sp_transientize(GtkWidget *dialog)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
#ifndef WIN32  // FIXME: Temporary Win32 special code to enable transient dialogs
    // _set_skip_taskbar_hint makes transient dialogs NON-transient! When dialogs
    // are made transient (_set_transient_for), they are already removed from
    // the taskbar in Win32.
    if (prefs->getBool( "/options/dialogsskiptaskbar/value")) {
        gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
    }
#endif

    gint transient_policy = prefs->getIntLimited("/options/transientpolicy/value", 1, 0, 2);

#ifdef WIN32 // Win32 special code to enable transient dialogs
    transient_policy = 2;
#endif

    if (transient_policy) {

    // if there's an active document window, attach dialog to it as a transient:

        if ( SP_ACTIVE_DESKTOP )
        {
            SP_ACTIVE_DESKTOP->setWindowTransient (dialog, transient_policy);
        }
    }
} // end of sp_transientize()

void on_transientize (SPDesktop *desktop, win_data *wd )
{
    sp_transientize_callback (0, desktop, wd);
}

void
sp_transientize_callback ( InkscapeApplication * /*inkscape*/,
                           SPDesktop *desktop, win_data *wd )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gint transient_policy = prefs->getIntLimited( "/options/transientpolicy/value", 1, 0, 2);

#ifdef WIN32 // Win32 special code to enable transient dialogs
    transient_policy = 1;
#endif

    if (!transient_policy)
        return;

    if (wd->win)
    {
        desktop->setWindowTransient (wd->win, transient_policy);
    }
}

void on_dialog_hide (GtkWidget *w)
{
    if (w)
        gtk_widget_hide (w);
}

void on_dialog_unhide (GtkWidget *w)
{
    if (w)
        gtk_widget_show (w);
}

gboolean
sp_dialog_hide(GObject * /*object*/, gpointer data)
{
    GtkWidget *dlg = GTK_WIDGET(data);

    if (dlg)
        gtk_widget_hide (dlg);

    return TRUE;
}



gboolean
sp_dialog_unhide(GObject * /*object*/, gpointer data)
{
    GtkWidget *dlg = GTK_WIDGET(data);

    if (dlg)
        gtk_widget_show (dlg);

    return TRUE;
}


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

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

#include <gtkmm/entry.h>
#include <gtkmm/window.h>
#include <gdk/gdkkeysyms.h>
#include "macros.h"
#include <gtk/gtk.h>
#include "desktop.h"
#include "inkscape-private.h"
#include "preferences.h"
#include "event-context.h"

#include "dialog-events.h"

#if !GTK_CHECK_VERSION(2,22,0)
#define GDK_KEY_VoidSymbol 0xffffff
#define GDK_KEY_Up 0xff52
#define GDK_KEY_KP_Up 0xff97
#define GDK_KEY_Down 0xff54
#define GDK_KEY_KP_Down 0xff99
#define GDK_KEY_Left 0xff51
#define GDK_KEY_KP_Left 0xff96
#define GDK_KEY_Right 0xff53
#define GDK_KEY_KP_Right 0xff98
#define GDK_KEY_Page_Up 0xff55
#define GDK_KEY_KP_Page_Up 0xff9a
#define GDK_KEY_Page_Down 0xff56
#define GDK_KEY_KP_Page_Down 0xff9b
#define GDK_KEY_Home 0xff50
#define GDK_KEY_KP_Home 0xff95
#define GDK_KEY_End 0xff57
#define GDK_KEY_KP_End 0xff9c
#define GDK_KEY_a 0x061
#define GDK_KEY_A 0x041
#define GDK_KEY_b 0x062
#define GDK_KEY_B 0x042
#define GDK_KEY_d 0x064
#define GDK_KEY_D 0x044
#define GDK_KEY_g 0x067
#define GDK_KEY_G 0x047
#define GDK_KEY_i 0x069
#define GDK_KEY_I 0x049
#define GDK_KEY_j 0x06a
#define GDK_KEY_J 0x04a
#define GDK_KEY_k 0x06b
#define GDK_KEY_K 0x04b
#define GDK_KEY_l 0x06c
#define GDK_KEY_L 0x04c
#define GDK_KEY_q 0x071
#define GDK_KEY_Q 0x051
#define GDK_KEY_r 0x072
#define GDK_KEY_R 0x052
#define GDK_KEY_s 0x073
#define GDK_KEY_S 0x053
#define GDK_KEY_u 0x075
#define GDK_KEY_U 0x055
#define GDK_KEY_w 0x077
#define GDK_KEY_W 0x057
#define GDK_KEY_x 0x078
#define GDK_KEY_X 0x058
#define GDK_KEY_z 0x07a
#define GDK_KEY_Z 0x05a
#define GDK_KEY_Escape 0xff1b
#define GDK_KEY_Control_L 0xffe3
#define GDK_KEY_Control_R 0xffe4
#define GDK_KEY_Alt_L 0xffe9
#define GDK_KEY_Alt_R 0xffea
#define GDK_KEY_Shift_L 0xffe1
#define GDK_KEY_Shift_R 0xffe2
#define GDK_KEY_Meta_L 0xffe7
#define GDK_KEY_Meta_R 0xffe8
#define GDK_KEY_KP_0 0xffb0
#define GDK_KEY_KP_1 0xffb1
#define GDK_KEY_KP_2 0xffb2
#define GDK_KEY_KP_3 0xffb3
#define GDK_KEY_KP_4 0xffb4
#define GDK_KEY_KP_5 0xffb5
#define GDK_KEY_KP_6 0xffb6
#define GDK_KEY_KP_7 0xffb7
#define GDK_KEY_KP_8 0xffb8
#define GDK_KEY_KP_9 0xffb9
#define GDK_KEY_F1 0xffbe
#define GDK_KEY_F2 0xffbf
#define GDK_KEY_F3 0xffc0
#define GDK_KEY_F4 0xffc1
#define GDK_KEY_F5 0xffc2
#define GDK_KEY_F6 0xffc3
#define GDK_KEY_F7 0xffc4
#define GDK_KEY_F8 0xffc5
#define GDK_KEY_F9 0xffc6
#define GDK_KEY_F10 0xffc7
#define GDK_KEY_F11 0xffc8
#define GDK_KEY_Insert 0xff63
#define GDK_KEY_KP_Insert 0xff9e
#define GDK_KEY_Delete 0xffff
#define GDK_KEY_KP_Delete 0xff9f
#define GDK_KEY_BackSpace 0xff08
#define GDK_KEY_Return 0xff0d
#define GDK_KEY_KP_Enter 0xff8d
#define GDK_KEY_space 0x020
#define GDK_KEY_KP_Space 0xff80
#define GDK_KEY_Tab 0xff09
#define GDK_KEY_ISO_Left_Tab 0xfe20
#define GDK_KEY_bracketleft 0x05b
#define GDK_KEY_bracketright 0x05d
#define GDK_KEY_less 0x03c
#define GDK_KEY_greater 0x03e
#define GDK_KEY_comma 0x02c
#define GDK_KEY_period 0x02e
#define GDK_KEY_KP_Add 0xffab
#define GDK_KEY_KP_Subtract 0xffad
#endif


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
    w = gtk_window_get_transient_for ((GtkWindow *) win);
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
sp_dialog_defocus_callback (GtkWindow */*win*/, gpointer data)
{
    sp_dialog_defocus ((GtkWindow *)
        gtk_widget_get_toplevel ((GtkWidget *) data));
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
    GObject *dlg = (GObject *) data;
    if (g_object_get_data (dlg, "eatkeys")) {
        return FALSE;
    }

    gboolean ret = FALSE;

    switch (event->type) {

        case GDK_KEY_PRESS:

            switch (get_group0_keyval (&event->key)) {
                case GDK_KEY_Escape:
                    sp_dialog_defocus (win);
                    ret = TRUE;
                    break;
                case GDK_KEY_F4:
                case GDK_KEY_w:
                case GDK_KEY_W:
                    // close dialog
                    if (MOD__CTRL_ONLY) {

                        /* this code sends a delete_event to the dialog,
                         * instead of just destroying it, so that the
                         * dialog can do some housekeeping, such as remember
                         * its position.
                         */
                        GdkEventAny event;
                        GtkWidget *widget = (GtkWidget *) win;
                        event.type = GDK_DELETE;
                        event.window = gtk_widget_get_window (widget);
                        event.send_event = TRUE;
                        g_object_ref (G_OBJECT (event.window));
                        gtk_main_do_event ((GdkEvent*)&event);
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
sp_transientize_callback ( Inkscape::Application * /*inkscape*/,
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
sp_dialog_hide (GtkObject */*object*/, gpointer data)
{
    GtkWidget *dlg = (GtkWidget *) data;

    if (dlg)
        gtk_widget_hide (dlg);

    return TRUE;
}



gboolean
sp_dialog_unhide (GtkObject */*object*/, gpointer data)
{
    GtkWidget *dlg = (GtkWidget *) data;

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

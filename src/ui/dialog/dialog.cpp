/**
 * \brief Base class for dialogs in Inkscape.  This class provides certain
 *        common behaviors and styles wanted of all dialogs in the application.
 *
 * Author:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   buliabyak@gmail.com
 *
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/stock.h>
#include <gtk/gtk.h>

#include "application/application.h"
#include "application/editor.h"
#include "inkscape.h"
#include "event-context.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "dialog-manager.h"
#include "dialogs/dialog-events.h"
#include "shortcuts.h"
#include "prefs-utils.h"
#include "interface.h"
#include "verbs.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

#ifndef WIN32
static gboolean
sp_retransientize_again (gpointer dlgPtr)
{
    Dialog *dlg = (Dialog *)dlgPtr;
    dlg->retransientize_suppress = false;
    return FALSE; // so that it is only called once
}
#endif

static void
sp_retransientize (Inkscape::Application *inkscape, SPDesktop *desktop, gpointer dlgPtr)
{
    Dialog *dlg = (Dialog *)dlgPtr;
    dlg->onDesktopActivated (desktop);
}

static void
sp_dialog_shutdown (GtkObject *object, gpointer dlgPtr)
{
    Dialog *dlg = (Dialog *)dlgPtr;
    dlg->onShutdown();
}

void 
Dialog::present()
{
    Gtk::Dialog::present();
}

void
Dialog::save_geometry()
{
    int y, x, w, h;

    get_position(x, y);
    get_size(w, h);

//    g_print ("write %d %d %d %d\n", x, y, w, h);

    if (x<0) x=0;
    if (y<0) y=0;

    prefs_set_int_attribute (_prefs_path, "x", x);
    prefs_set_int_attribute (_prefs_path, "y", y);
    prefs_set_int_attribute (_prefs_path, "w", w);
    prefs_set_int_attribute (_prefs_path, "h", h);
}

void
Dialog::read_geometry()
{
    _user_hidden = false;

    int x = prefs_get_int_attribute (_prefs_path, "x", -1000);
    int y = prefs_get_int_attribute (_prefs_path, "y", -1000);
    int w = prefs_get_int_attribute (_prefs_path, "w", 0);
    int h = prefs_get_int_attribute (_prefs_path, "h", 0);

//    g_print ("read %d %d %d %d\n", x, y, w, h);

        if (x<0) x=0;
        if (y<0) y=0;

    // If there are stored height and width values for the dialog,
    // resize the window to match; otherwise we leave it at its default
    if (w != 0 && h != 0) {
        resize (w, h);
    }

    // If there are stored values for where the dialog should be
    // located, then restore the dialog to that position.
    if (x != -1000 && y != -1000) {
        move(x, y);
    } else {
        // ...otherwise just put it in the middle of the screen
        set_position(Gtk::WIN_POS_CENTER);
    }
}

void hideCallback(GtkObject *object, gpointer dlgPtr)
{
    g_return_if_fail( dlgPtr != NULL );

    Dialog *dlg = (Dialog *)dlgPtr;
    dlg->onHideF12();
}

void unhideCallback(GtkObject *object, gpointer dlgPtr)
{
    g_return_if_fail( dlgPtr != NULL );

    Dialog *dlg = (Dialog *)dlgPtr;
    dlg->onShowF12();
}

//=====================================================================

/**
 * UI::Dialog::Dialog is a base class for all dialogs in Inkscape.  The
 * purpose of this class is to provide a unified place for ensuring
 * style and behavior.  Specifically, this class provides functionality
 * for saving and restoring the size and position of dialogs (through
 * the user's preferences file).
 *
 * It also provides some general purpose signal handlers for things like
 * showing and hiding all dialogs.
 */
Dialog::Dialog(const char *prefs_path, int verb_num, const char *apply_label)
{
    hide();
    set_has_separator(false);

    _prefs_path = prefs_path;

    if (prefs_get_int_attribute ("dialogs", "showclose", 0) || apply_label) {
        // TODO: make the order of buttons obey the global preference
        if (apply_label) {
            add_button(Glib::ustring(apply_label), Gtk::RESPONSE_APPLY);
            set_default_response(Gtk::RESPONSE_APPLY);
        }
       add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
    }

    GtkWidget *dlg = GTK_WIDGET(gobj());

    if (verb_num)
    {
        gchar title[500];
        sp_ui_dialog_title_string (Inkscape::Verb::get(verb_num), title);
        set_title(title);
    }

    sp_transientize(dlg);
    retransientize_suppress = false;

    gtk_signal_connect( GTK_OBJECT (dlg), "event", GTK_SIGNAL_FUNC(sp_dialog_event_handler), dlg );

    _hiddenF12 = false;
    if (Inkscape::NSApplication::Application::getNewGui())
    {
        _desktop_activated_connection = Inkscape::NSApplication::Editor::connectDesktopActivated (sigc::mem_fun (*this, &Dialog::onDesktopActivated));
        _dialogs_hidden_connection = Inkscape::NSApplication::Editor::connectDialogsHidden (sigc::mem_fun (*this, &Dialog::onHideF12));
        _dialogs_unhidden_connection = Inkscape::NSApplication::Editor::connectDialogsUnhidden (sigc::mem_fun (*this, &Dialog::onShowF12));
        _shutdown_connection = Inkscape::NSApplication::Editor::connectShutdown (sigc::mem_fun (*this, &Dialog::onShutdown));
    }
    else
    {
        g_signal_connect   (G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_retransientize), (void *)this);
        g_signal_connect( G_OBJECT(INKSCAPE), "dialogs_hide", G_CALLBACK(hideCallback), (void *)this );
        g_signal_connect( G_OBJECT(INKSCAPE), "dialogs_unhide", G_CALLBACK(unhideCallback), (void *)this );
        g_signal_connect   (G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (sp_dialog_shutdown), (void *)this);
    }

    g_signal_connect_after( gobj(), "key_press_event", (GCallback)windowKeyPress, NULL );

    read_geometry();
    present();
}

Dialog::Dialog(BaseObjectType *gobj)
    : Gtk::Dialog(gobj)
{
}

Dialog::~Dialog()
{
    if (Inkscape::NSApplication::Application::getNewGui())
    {
        _desktop_activated_connection.disconnect();
        _dialogs_hidden_connection.disconnect();
        _dialogs_unhidden_connection.disconnect();
        _shutdown_connection.disconnect();
    }
    
    save_geometry();
}


bool Dialog::windowKeyPress( GtkWidget *widget, GdkEventKey *event )
{
    unsigned int shortcut = 0;
    shortcut = get_group0_keyval (event) |
        ( event->state & GDK_SHIFT_MASK ?
          SP_SHORTCUT_SHIFT_MASK : 0 ) |
        ( event->state & GDK_CONTROL_MASK ?
          SP_SHORTCUT_CONTROL_MASK : 0 ) |
        ( event->state & GDK_MOD1_MASK ?
          SP_SHORTCUT_ALT_MASK : 0 );
    return sp_shortcut_invoke( shortcut, SP_ACTIVE_DESKTOP );
}

//---------------------------------------------------------------------

void
Dialog::on_response(int response_id)
{
    switch (response_id) {
        case Gtk::RESPONSE_APPLY: {
            _apply();
            break;
        }
        case Gtk::RESPONSE_CLOSE: {
            _close();
            break;
        }
    }
}

bool
Dialog::on_delete_event (GdkEventAny *event)
{
    save_geometry();
    _user_hidden = true;

    return false;
}

void
Dialog::onHideF12()
{
    _hiddenF12 = true;
    save_geometry();
    hide();
}

void
Dialog::onShowF12()
{
    if (_user_hidden)
        return;

    if (_hiddenF12) {
        show();
        read_geometry();
    }

    _hiddenF12 = false;
}

void 
Dialog::onShutdown()
{
    save_geometry();
    _user_hidden = true;
}

void
Dialog::onDesktopActivated (SPDesktop *desktop)
{
    gint transient_policy = prefs_get_int_attribute_limited ( "options.transientpolicy", "value", 1, 0, 2);

    if (!transient_policy)
        return;

#ifndef WIN32
    GtkWindow *dialog_win = GTK_WINDOW(gobj());

    if (retransientize_suppress) {
         /* if retransientizing of this dialog is still forbidden after
          * previous call warning turned off because it was confusingly fired
          * when loading many files from command line
          */

         // g_warning("Retranzientize aborted! You're switching windows too fast!");
        return;
    }

    if (dialog_win)
    {
        retransientize_suppress = true; // disallow other attempts to retranzientize this dialog

        desktop->setWindowTransient (dialog_win);

        /*
         * This enables "aggressive" transientization,
         * i.e. dialogs always emerging on top when you switch documents. Note
         * however that this breaks "click to raise" policy of a window
         * manager because the switched-to document will be raised at once
         * (so that its transients also could raise)
         */
        if (transient_policy == 2 && !_hiddenF12 && !_user_hidden) {
            // without this, a transient window not always emerges on top
            gtk_window_present (dialog_win);
        }
    }

    // we're done, allow next retransientizing not sooner than after 120 msec
    gtk_timeout_add (120, (GtkFunction) sp_retransientize_again, (gpointer) this);
#endif
}


Inkscape::Selection*
Dialog::_getSelection()
{
    return sp_desktop_selection(SP_ACTIVE_DESKTOP);
}

void
Dialog::_apply()
{
    g_warning("Apply button clicked for dialog [Dialog::_apply()]");
}

void
Dialog::_close()
{
    GtkWidget *dlg = GTK_WIDGET(gobj());

                        /* this code sends a delete_event to the dialog,
                         * instead of just destroying it, so that the
                         * dialog can do some housekeeping, such as remember
                         * its position.
                         */
                        GdkEventAny event;
                        event.type = GDK_DELETE;
                        event.window = dlg->window;
                        event.send_event = TRUE;
                        g_object_ref (G_OBJECT (event.window));
                        gtk_main_do_event ((GdkEvent*)&event);
                        g_object_unref (G_OBJECT (event.window));
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

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

/**
 * @file
 * Base class for dialogs in Inkscape - implementation.
 */
/* Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   buliabyak@gmail.com
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2004--2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "dialog-manager.h"
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include <gdk/gdkkeysyms.h>

#include "inkscape.h"
#include "ui/tools/tool-base.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "shortcuts.h"
#include "preferences.h"
#include "ui/interface.h"
#include "verbs.h"
#include "ui/tool/event-utils.h"

#include <gtk/gtk.h>

#define MIN_ONSCREEN_DISTANCE 50


namespace Inkscape {
namespace UI {
namespace Dialog {

void sp_retransientize(InkscapeApplication */*inkscape*/, SPDesktop *desktop, gpointer dlgPtr)
{
    Dialog *dlg = static_cast<Dialog *>(dlgPtr);
    dlg->onDesktopActivated (desktop);
}

gboolean sp_retransientize_again(gpointer dlgPtr)
{
    Dialog *dlg = static_cast<Dialog *>(dlgPtr);
    dlg->retransientize_suppress = false;
    return FALSE; // so that it is only called once
}

void sp_dialog_shutdown(GObject * /*object*/, gpointer dlgPtr)
{
    Dialog *dlg = static_cast<Dialog *>(dlgPtr);
    dlg->onShutdown();
}


static void hideCallback(GObject * /*object*/, gpointer dlgPtr)
{
    g_return_if_fail( dlgPtr != NULL );

    Dialog *dlg = static_cast<Dialog *>(dlgPtr);
    dlg->onHideF12();
}

static void unhideCallback(GObject * /*object*/, gpointer dlgPtr)
{
    g_return_if_fail( dlgPtr != NULL );

    Dialog *dlg = static_cast<Dialog *>(dlgPtr);
    dlg->onShowF12();
}


//=====================================================================

Dialog::Dialog(Behavior::BehaviorFactory behavior_factory, const char *prefs_path, int verb_num,
               Glib::ustring const &apply_label)
    : _user_hidden(false), 
      _hiddenF12(false),
      retransientize_suppress(false),
      _prefs_path(prefs_path),
      _verb_num(verb_num),
      _title(),
      _apply_label(apply_label),
      _desktop(NULL),
      _is_active_desktop(true),
      _behavior(0)
{
    gchar title[500];

    if (verb_num) {
        sp_ui_dialog_title_string (Inkscape::Verb::get(verb_num), title);
    }

    _title = title;
    _behavior = behavior_factory(*this);
    _desktop = SP_ACTIVE_DESKTOP;

    g_signal_connect(G_OBJECT(INKSCAPE), "activate_desktop", G_CALLBACK(sp_retransientize), (void *)this);
    g_signal_connect(G_OBJECT(INKSCAPE), "dialogs_hide", G_CALLBACK(hideCallback), (void *)this);
    g_signal_connect(G_OBJECT(INKSCAPE), "dialogs_unhide", G_CALLBACK(unhideCallback), (void *)this);
    g_signal_connect(G_OBJECT(INKSCAPE), "shut_down", G_CALLBACK(sp_dialog_shutdown), (void *)this);

    Glib::wrap(gobj())->signal_event().connect(sigc::mem_fun(*this, &Dialog::_onEvent));
    Glib::wrap(gobj())->signal_key_press_event().connect(sigc::mem_fun(*this, &Dialog::_onKeyPress));

    read_geometry();
}

Dialog::~Dialog()
{
    save_geometry();
    delete _behavior;
    _behavior = 0;
}


//---------------------------------------------------------------------


void Dialog::onDesktopActivated(SPDesktop *desktop)
{
    _is_active_desktop = (desktop == _desktop);
    _behavior->onDesktopActivated(desktop);
}

void Dialog::onShutdown()
{
    save_geometry();
    //_user_hidden = true;
    _behavior->onShutdown();
}

void Dialog::onHideF12()
{
    _hiddenF12 = true;
    _behavior->onHideF12();
}

void Dialog::onShowF12()
{
    if (_user_hidden)
        return;

    if (_hiddenF12) {
        _behavior->onShowF12();
    }

    _hiddenF12 = false;
}


inline Dialog::operator Gtk::Widget &()                          { return *_behavior; }
inline GtkWidget *Dialog::gobj()                                 { return _behavior->gobj(); }
inline void Dialog::present()                                    { _behavior->present(); }
inline Gtk::Box *Dialog::get_vbox()                             {  return _behavior->get_vbox(); }
inline void Dialog::hide()                                       { _behavior->hide(); }
inline void Dialog::show()                                       { _behavior->show(); }
inline void Dialog::show_all_children()                          { _behavior->show_all_children(); }
inline void Dialog::set_size_request(int width, int height)      { _behavior->set_size_request(width, height); }
inline void Dialog::size_request(Gtk::Requisition &requisition)  { _behavior->size_request(requisition); }
inline void Dialog::get_position(int &x, int &y)                 { _behavior->get_position(x, y); }
inline void Dialog::get_size(int &width, int &height)            { _behavior->get_size(width, height); }
inline void Dialog::resize(int width, int height)                { _behavior->resize(width, height); }
inline void Dialog::move(int x, int y)                           { _behavior->move(x, y); }
inline void Dialog::set_position(Gtk::WindowPosition position)   { _behavior->set_position(position); }
inline void Dialog::set_title(Glib::ustring title)               { _behavior->set_title(title); }
inline void Dialog::set_sensitive(bool sensitive)                { _behavior->set_sensitive(sensitive); }

Glib::SignalProxy0<void> Dialog::signal_show() { return _behavior->signal_show(); }
Glib::SignalProxy0<void> Dialog::signal_hide() { return _behavior->signal_hide(); }

void Dialog::read_geometry()
{
    _user_hidden = false;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int x = prefs->getInt(_prefs_path + "/x", -1000);
    int y = prefs->getInt(_prefs_path + "/y", -1000);
    int w = prefs->getInt(_prefs_path + "/w", 0);
    int h = prefs->getInt(_prefs_path + "/h", 0);

    // g_print ("read %d %d %d %d\n", x, y, w, h);

    // If there are stored height and width values for the dialog,
    // resize the window to match; otherwise we leave it at its default
    if (w != 0 && h != 0) {
        resize(w, h);
    }

    // If there are stored values for where the dialog should be
    // located, then restore the dialog to that position.
    // also check if (x,y) is actually onscreen with the current screen dimensions
    if ( (x >= 0) && (y >= 0) && (x < (gdk_screen_width()-MIN_ONSCREEN_DISTANCE)) && (y < (gdk_screen_height()-MIN_ONSCREEN_DISTANCE)) ) {
        move(x, y);
    } else {
        // ...otherwise just put it in the middle of the screen
        set_position(Gtk::WIN_POS_CENTER);
    }

}


void Dialog::save_geometry()
{
    int y, x, w, h;

    get_position(x, y);
    get_size(w, h);

    // g_print ("write %d %d %d %d\n", x, y, w, h);

    if (x<0) x=0;
    if (y<0) y=0;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt(_prefs_path + "/x", x);
    prefs->setInt(_prefs_path + "/y", y);
    prefs->setInt(_prefs_path + "/w", w);
    prefs->setInt(_prefs_path + "/h", h);

}

void
Dialog::save_status(int visible, int state, int placement)
{
   // Only save dialog status for dialogs on the "last document"
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop != NULL || !_is_active_desktop ) {
        return;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs) {
        prefs->setInt(_prefs_path + "/visible", visible);
        prefs->setInt(_prefs_path + "/state", state);
        prefs->setInt(_prefs_path + "/placement", placement);
    }
}


void Dialog::_handleResponse(int response_id)
{
    switch (response_id) {
        case Gtk::RESPONSE_CLOSE: {
            _close();
            break;
        }
    }
}

bool Dialog::_onDeleteEvent(GdkEventAny */*event*/)
{
    save_geometry();
    _user_hidden = true;

    return false;
}

bool Dialog::_onEvent(GdkEvent *event)
{
    bool ret = false;

    switch (event->type) {
        case GDK_KEY_PRESS: {
            switch (Inkscape::UI::Tools::get_group0_keyval (&event->key)) {
                case GDK_KEY_Escape: {
                    _defocus();
                    ret = true;
                    break;
                }
                case GDK_KEY_F4:
                case GDK_KEY_w:
                case GDK_KEY_W: {
                    if (Inkscape::UI::held_only_control(event->key)) {
                        _close();
                        ret = true;
                    }
                    break;
                }
                default: { // pass keypress to the canvas
                    break;
                }
            }
        }
        default:
            ;
    }

    return ret;
}

bool Dialog::_onKeyPress(GdkEventKey *event)
{
    unsigned int shortcut;
    shortcut = Inkscape::UI::Tools::get_group0_keyval(event) |
        ( event->state & GDK_SHIFT_MASK ?
          SP_SHORTCUT_SHIFT_MASK : 0 ) |
        ( event->state & GDK_CONTROL_MASK ?
          SP_SHORTCUT_CONTROL_MASK : 0 ) |
        ( event->state & GDK_MOD1_MASK ?
          SP_SHORTCUT_ALT_MASK : 0 );
    return sp_shortcut_invoke(shortcut, SP_ACTIVE_DESKTOP);
}

void Dialog::_apply()
{
    g_warning("Apply button clicked for dialog [Dialog::_apply()]");
}

void Dialog::_close()
{
    _behavior->hide();
    _onDeleteEvent(NULL);
}

void Dialog::_defocus()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    if (desktop) {
        Gtk::Widget *canvas = Glib::wrap(GTK_WIDGET(desktop->canvas));

        // make sure the canvas window is present before giving it focus
        Gtk::Window *toplevel_window = dynamic_cast<Gtk::Window *>(canvas->get_toplevel());
        if (toplevel_window)
            toplevel_window->present();

        canvas->grab_focus();
    }
}

Inkscape::Selection*
Dialog::_getSelection()
{
    return sp_desktop_selection(SP_ACTIVE_DESKTOP);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

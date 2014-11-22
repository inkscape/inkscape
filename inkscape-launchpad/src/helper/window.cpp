/**
 * Generic window implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <glibmm.h>
#include <gtkmm/window.h>

#include "desktop.h"
#include "inkscape.h"
#include "shortcuts.h"
#include "ui/tools/tool-base.h"
#include "window.h"

static bool on_window_key_press(GdkEventKey* event)
{
    unsigned shortcut = 0;
    // FIXME why?
    shortcut = Inkscape::UI::Tools::get_group0_keyval (event) |
	           ( event->state & GDK_SHIFT_MASK ?
	             SP_SHORTCUT_SHIFT_MASK : 0 ) |
	           ( event->state & GDK_CONTROL_MASK ?
	             SP_SHORTCUT_CONTROL_MASK : 0 ) |
	           ( event->state & GDK_MOD1_MASK ?
	             SP_SHORTCUT_ALT_MASK : 0 );
    return sp_shortcut_invoke (shortcut, SP_ACTIVE_DESKTOP);
}

Gtk::Window * Inkscape::UI::window_new (const gchar *title, unsigned int resizeable)
{
    Gtk::Window *window = new Gtk::Window(Gtk::WINDOW_TOPLEVEL);
    window->set_title (title);
    window->set_resizable (resizeable);
    window->signal_key_press_event().connect(sigc::ptr_fun(&on_window_key_press));

    return window;
}

static gboolean sp_window_key_press(GtkWidget *, GdkEventKey *event)
{
    return on_window_key_press(event);
}

GtkWidget * sp_window_new (const gchar *title, unsigned int resizeable)
{
    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title ((GtkWindow *) window, title);
    gtk_window_set_resizable ((GtkWindow *) window, resizeable);
    g_signal_connect_after ((GObject *) window, "key_press_event", (GCallback) sp_window_key_press, NULL);

    return window;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :

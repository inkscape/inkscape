/** @file
 * @brief A floating dialog implementation.
 */
/* Author:
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */


#ifndef INKSCAPE_UI_DIALOG_FLOATING_BEHAVIOR_H
#define INKSCAPE_UI_DIALOG_FLOATING_BEHAVIOR_H

#include <glibmm/property.h>
#include "behavior.h"

namespace Gtk {
class Dialog;
}

namespace Inkscape {
namespace UI {
namespace Dialog {
namespace Behavior {

class FloatingBehavior : public Behavior {

public:
    static Behavior *create(Dialog &dialog);

    ~FloatingBehavior();

    /** Gtk::Dialog methods */
    operator Gtk::Widget &();
    GtkWidget *gobj();
    void present();
    Gtk::Box *get_vbox();
    void show();
    void hide();
    void show_all_children();
    void resize(int width, int height);
    void move(int x, int y);
    void set_position(Gtk::WindowPosition);
    void set_size_request(int width, int height);
    void size_request(Gtk::Requisition &requisition);
    void get_position(int &x, int &y);
    void get_size(int& width, int &height);
    void set_title(Glib::ustring title);
    void set_sensitive(bool sensitive);

    /** Gtk::Dialog signal proxies */
    Glib::SignalProxy0<void> signal_show();
    Glib::SignalProxy0<void> signal_hide();
    Glib::SignalProxy1<bool, GdkEventAny *> signal_delete_event();

    /** Custom signal handlers */
    void onHideF12();
    void onShowF12();
    void onDesktopActivated(SPDesktop *desktop);
    void onShutdown();

private:
    FloatingBehavior(Dialog& dialog);

    Gtk::Dialog *_d;   //< the actual dialog

    void _focus_event (void);
    bool _trans_timer (void);

    Glib::PropertyProxy_ReadOnly<bool> _dialog_active;  //< Variable proxy to track whether the dialog is the active window
    int _steps;          //< Number of steps for the timer to animate the transparent dialog
    float _trans_focus;  //< The percentage opacity when the dialog is focused
    float _trans_blur;   //< The percentage opactiy when the dialog is not focused
    int _trans_time;     //< The amount of time (in ms) for the dialog to change it's transparency
};

} // namespace Behavior
} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_FLOATING_BEHAVIOR_H

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

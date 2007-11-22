/**
 * \brief A floating dialog implementation.
 *
 * Author:
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */


#ifndef INKSCAPE_UI_DIALOG_FLOATING_BEHAVIOR_H
#define INKSCAPE_UI_DIALOG_FLOATING_BEHAVIOR_H

#include <gtkmm/dialog.h>
#include "behavior.h"

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
    Gtk::VBox *get_vbox();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

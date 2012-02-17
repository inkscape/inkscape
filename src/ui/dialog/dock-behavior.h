/** @file
 * @brief A dockable dialog implementation.
 */
/* Author:
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */


#ifndef INKSCAPE_UI_DIALOG_DOCK_BEHAVIOR_H
#define INKSCAPE_UI_DIALOG_DOCK_BEHAVIOR_H

#include "ui/widget/dock-item.h"
#include "behavior.h"

namespace Gtk {
	class Paned;
}

namespace Inkscape {
namespace UI {
namespace Dialog {
namespace Behavior {

class DockBehavior : public Behavior {

public:
    static Behavior *create(Dialog& dialog);

    ~DockBehavior();

    /** Gtk::Dialog methods */
    operator Gtk::Widget&();
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
    void size_request(Gtk::Requisition& requisition);
    void get_position(int& x, int& y);
    void get_size(int& width, int& height);
    void set_title(Glib::ustring title);
    void set_sensitive(bool sensitive);

    /** Gtk::Dialog signal proxies */
    Glib::SignalProxy0<void> signal_show();
    Glib::SignalProxy0<void> signal_hide();
    Glib::SignalProxy1<bool, GdkEventAny *> signal_delete_event();
    Glib::SignalProxy0<void> signal_drag_begin();
    Glib::SignalProxy1<void, bool> signal_drag_end();

    /** Custom signal handlers */
    void onHideF12();
    void onShowF12();
    void onDesktopActivated(SPDesktop *desktop);
    void onShutdown();

private:
    Widget::DockItem _dock_item;

    DockBehavior(Dialog& dialog);

    /** Internal helpers */
    Gtk::Paned *_getPaned();              //< gives the parent pane, if the dock item has one
    void _requestHeight(int height);      //< tries to resize the dock item to the requested hieght

    /** Internal signal handlers */
    void _onHide();
    void _onShow();
    bool _onDeleteEvent(GdkEventAny *event);
    void _onStateChanged(Widget::DockItem::State prev_state, Widget::DockItem::State new_state);
    bool _onKeyPress(GdkEventKey *event);

    sigc::connection _signal_hide_connection;
    sigc::connection _signal_key_press_event_connection;

};

} // namespace Behavior
} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_DOCK_BEHAVIOR_H

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

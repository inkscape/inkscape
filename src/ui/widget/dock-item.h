/*
 * Author:
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */


#ifndef INKSCAPE_UI_WIGET_DOCK_ITEM_H
#define INKSCAPE_UI_WIGET_DOCK_ITEM_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/window.h>

#if WITH_EXT_GDL
#include <gdl/gdl.h>
#else
#include "libgdl/gdl.h"
#endif

namespace Gtk {
	class HButtonBox;
}

namespace Inkscape {
namespace UI {
namespace Widget {

class Dock;

/**
 * A custom wrapper around gdl-dock-item.
 */
class DockItem {

public:

    enum State { UNATTACHED,                // item not bound to the dock (a temporary state)
                 FLOATING_STATE,            // item not in its dock (but can be docked in other,
                                            // e.g. floating, docks)
                 DOCKED_STATE,              // item in its assigned dock
                 ICONIFIED_DOCKED_STATE,    // item iconified in its assigned dock from dock
                 ICONIFIED_FLOATING_STATE}; // item iconified in its assigned dock from float

    enum Placement { 
        NONE     = GDL_DOCK_NONE,
        TOP      = GDL_DOCK_TOP,
        BOTTOM   = GDL_DOCK_BOTTOM,
        RIGHT    = GDL_DOCK_RIGHT,
        LEFT     = GDL_DOCK_LEFT,
        CENTER   = GDL_DOCK_CENTER,
        FLOATING = GDL_DOCK_FLOATING
    };

    DockItem(Dock& dock, const Glib::ustring& name, const Glib::ustring& long_name, 
             const Glib::ustring& icon_name, State state, Placement placement);

    ~DockItem();

    Gtk::Widget& getWidget();
    GtkWidget *gobj();

    Gtk::VBox *get_vbox();

    void get_position(int& x, int& y);
    void get_size(int& width, int& height);

    void resize(int width, int height);
    void move(int x, int y);
    void set_position(Gtk::WindowPosition);
    void set_size_request(int width, int height);
    void size_request(Gtk::Requisition& requisition);
    void set_title(Glib::ustring title);

    bool isAttached() const;
    bool isFloating() const;
    bool isIconified() const;
    State getState() const;
    State getPrevState() const;
    Placement getPlacement() const;

    Gtk::Window *getWindow();   //< gives the parent window, if the dock item has one (i.e. it's floating)

    void hide();
    void show();
    void iconify();
    void show_all();

    void present();

    void grab_focus();

    Glib::SignalProxy0<void> signal_show();
    Glib::SignalProxy0<void> signal_hide();
    Glib::SignalProxy1<bool, GdkEventAny *> signal_delete_event();
    Glib::SignalProxy0<void> signal_drag_begin();
    Glib::SignalProxy1<void, bool> signal_drag_end();
    Glib::SignalProxy0<void> signal_realize();

    sigc::signal<void, State, State> signal_state_changed();

private:
    Dock &_dock;              //< parent dock

    State _prev_state;        //< last known state

    int _prev_position;

    Gtk::Window *_window;     //< reference to floating window, if any 
    int _x, _y;               //< last known position of window, if floating

    bool _grab_focus_on_realize;   //< if the dock item should grab focus on the next realize

    GtkWidget *_gdl_dock_item;
    Glib::RefPtr<Gdk::Pixbuf> _icon_pixbuf;

    /** Interface widgets, will be packed like 
     * gdl_dock_item -> _frame -> _dock_item_box -> (_dock_item_action_area) 
     */
    Gtk::Frame _frame;
    Gtk::VBox _dock_item_box;
    Gtk::HButtonBox *_dock_item_action_area;

    /** Internal signal handlers */
    void _onHide();
    void _onHideWindow();
    void _onShow();
    void _onDragBegin();
    void _onDragEnd(bool cancelled);
    void _onRealize();

    bool _onKeyPress(GdkEventKey *event);
    void _onStateChanged(State prev_state, State new_state);
    bool _onDeleteEvent(GdkEventAny *event);

    sigc::connection _signal_key_press_event_connection;

    /** GdlDockItem signal proxy structures */
    static const Glib::SignalProxyInfo _signal_show_proxy;
    static const Glib::SignalProxyInfo _signal_hide_proxy;
    static const Glib::SignalProxyInfo _signal_delete_event_proxy;

    static const Glib::SignalProxyInfo _signal_drag_begin_proxy;
    static const Glib::SignalProxyInfo _signal_drag_end_proxy;
    static const Glib::SignalProxyInfo _signal_realize_proxy;

    static gboolean _signal_delete_event_callback(GtkWidget *self, GdkEventAny *event, void *data);
    static void _signal_drag_end_callback(GtkWidget* self, gboolean p0, void* data);

    sigc::signal<void, State, State> _signal_state_changed;

    DockItem();
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIGET_DOCK_ITEM_H

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

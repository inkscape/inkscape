/** @file
 * @brief A desktop dock pane to dock dialogs, a custom wrapper around gdl-dock.
 */
/* Author:
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_DOCK_H
#define INKSCAPE_UI_WIDGET_DOCK_H

#include <gtkmm/box.h>
#include <list>
#include "ui/widget/dock-item.h"

struct _GdlDock;
typedef _GdlDock GdlDock;
struct _GdlDockBar;
typedef _GdlDockBar GdlDockBar;

namespace Gtk {
class Paned;
class ScrolledWindow;
}

namespace Inkscape {
namespace UI {
namespace Widget {

class Dock {

public:

    Dock(Gtk::Orientation orientation=Gtk::ORIENTATION_VERTICAL);
    ~Dock();

    void addItem(DockItem& item, DockItem::Placement placement);

    Gtk::Widget& getWidget();     //< return the top widget
    Gtk::Paned *getParentPaned();
    Gtk::Paned *getPaned();

    GtkWidget* getGdlWidget();    //< return the top gdl widget

    bool isEmpty() const;         //< true iff none of the dock's items are in a docked state
    bool hasIconifiedItems() const;

    Glib::SignalProxy0<void> signal_layout_changed();

    void hide();
    void show();

    /** Toggle size of dock between the previous dimensions and the ones sent as parameters */
    void toggleDockable(int width=0, int height=0);

    /** Scrolls the scrolled window container to make the provided dock item visible, if needed */
    void scrollToItem(DockItem& item);

protected:

    std::list<const DockItem *> _dock_items;   //< added dock items

    /** Interface widgets, will be packed like 
     * _scrolled_window -> (_dock_box -> (_paned -> (_dock -> _filler) | _dock_bar))
     */
    Gtk::Box            *_dock_box;
    Gtk::Paned          *_paned;
    GtkWidget           *_gdl_dock;
    GdlDockBar          *_gdl_dock_bar;
    Gtk::VBox            _filler;
    Gtk::ScrolledWindow *_scrolled_window;

    /** Internal signal handlers */
    void _onLayoutChanged();
    void _onPanedButtonEvent(GdkEventButton *event);

    static gboolean _on_paned_button_event(GtkWidget *widget, GdkEventButton *event, 
                                           gpointer user_data);

    /** GdlDock signal proxy structures */
    static const Glib::SignalProxyInfo _signal_layout_changed_proxy;

    /** Standard widths */
    static const int _default_empty_width;
    static const int _default_dock_bar_width;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif //INKSCAPE_UI_DIALOG_BEHAVIOUR_H

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


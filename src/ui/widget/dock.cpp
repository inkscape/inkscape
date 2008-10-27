/** @file
 * @brief A desktop dock pane to dock dialogs.
 */
/* Author:
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include "inkscape.h"
#include "preferences.h"
#include "desktop.h"

#include "dock.h"

#include <gtkmm/adjustment.h>

namespace Inkscape {
namespace UI {
namespace Widget {

namespace {

void hideCallback(GtkObject */*object*/, gpointer dock_ptr)
{
    g_return_if_fail( dock_ptr != NULL );

    Dock *dock = (Dock *)dock_ptr;
    dock->hide();
}

void unhideCallback(GtkObject */*object*/, gpointer dock_ptr)
{
    g_return_if_fail( dock_ptr != NULL );

    Dock *dock = (Dock *)dock_ptr;
    dock->show();
}

}

const int Dock::_default_empty_width = 0;
const int Dock::_default_dock_bar_width = 36;


Dock::Dock(Gtk::Orientation orientation)
    : _gdl_dock (GDL_DOCK (gdl_dock_new())),
      _gdl_dock_bar (GDL_DOCK_BAR (gdl_dock_bar_new(GDL_DOCK(_gdl_dock)))),
      _scrolled_window (Gtk::manage(new Gtk::ScrolledWindow))
{
    gdl_dock_bar_set_orientation(_gdl_dock_bar, static_cast<GtkOrientation>(orientation));

    switch (orientation) {
        case Gtk::ORIENTATION_VERTICAL:
            _dock_box = Gtk::manage(new Gtk::HBox());
            _paned = Gtk::manage(new Gtk::VPaned());
            break;
        case Gtk::ORIENTATION_HORIZONTAL:
            _dock_box = Gtk::manage(new Gtk::VBox());
            _paned = Gtk::manage(new Gtk::HPaned());
    }

    _scrolled_window->add(*_dock_box);
    _scrolled_window->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

    _paned->pack1(*Glib::wrap(GTK_WIDGET(_gdl_dock)), false, false);
    _paned->pack2(_filler, true, false);

    _dock_box->pack_start(*_paned, Gtk::PACK_EXPAND_WIDGET);
    _dock_box->pack_end(*Gtk::manage(Glib::wrap(GTK_WIDGET(_gdl_dock_bar))), Gtk::PACK_SHRINK);
    _dock_box->get_parent()->set_resize_mode(Gtk::RESIZE_PARENT);

    _scrolled_window->set_size_request(0);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    GdlSwitcherStyle gdl_switcher_style =
        static_cast<GdlSwitcherStyle>(prefs->getIntLimited("/options/dock/switcherstyle",
                                                                      GDL_SWITCHER_STYLE_BOTH, 0, 4));

    g_object_set (GDL_DOCK_OBJECT(_gdl_dock)->master,
                  "switcher-style", gdl_switcher_style,
                  "expand-direction", GDL_DOCK_EXPANSION_DIRECTION_DOWN,
                  NULL);

    GdlDockBarStyle gdl_dock_bar_style =
        static_cast<GdlDockBarStyle>(prefs->getIntLimited("/options/dock/dockbarstyle",
                                                                     GDL_DOCK_BAR_BOTH, 0, 3));

    gdl_dock_bar_set_style(_gdl_dock_bar, gdl_dock_bar_style);

    g_signal_connect(G_OBJECT(INKSCAPE), "dialogs_hide", G_CALLBACK(hideCallback), (void *)this);
    g_signal_connect(G_OBJECT(INKSCAPE), "dialogs_unhide", G_CALLBACK(unhideCallback), (void *)this);

    g_signal_connect(_paned->gobj(), "button-press-event", G_CALLBACK(_on_paned_button_event), (void *)this);
    g_signal_connect(_paned->gobj(), "button-release-event", G_CALLBACK(_on_paned_button_event), (void *)this);

    signal_layout_changed().connect(sigc::mem_fun(*this, &Inkscape::UI::Widget::Dock::_onLayoutChanged));
}

Dock::~Dock()
{
    g_free(_gdl_dock);
    g_free(_gdl_dock_bar);
}

void
Dock::addItem(DockItem& item, DockItem::Placement placement)
{
    _dock_items.push_back(&item);
    gdl_dock_add_item(_gdl_dock, GDL_DOCK_ITEM(item.gobj()), (GdlDockPlacement)placement);

    // FIXME: This is a hack to prevent the dock from expanding the main window, this can't be done
    // initially as the paned doesn't exist.
    if (Gtk::Paned *paned = getParentPaned()) {
        paned->set_resize_mode(Gtk::RESIZE_QUEUE);
    }
}

Gtk::Widget&
Dock::getWidget()
{
     return *_scrolled_window;
}

Gtk::Paned *
Dock::getParentPaned()
{
    g_return_val_if_fail(_dock_box, 0);
    Gtk::Container *parent = getWidget().get_parent();
    return (parent != 0 ? dynamic_cast<Gtk::Paned *>(parent) : 0);
}


Gtk::Paned *
Dock::getPaned()
{
    return _paned;
}

GtkWidget *
Dock::getGdlWidget()
{
    return GTK_WIDGET(_gdl_dock);
}

bool
Dock::isEmpty() const
{
    std::list<const DockItem *>::const_iterator
        i = _dock_items.begin(),
        e = _dock_items.end();

    for (; i != e; ++i) {
        if ((*i)->getState() == DockItem::DOCKED_STATE) {
            return false;
        }
    }

    return true;
}

bool
Dock::hasIconifiedItems() const
{
    std::list<const DockItem *>::const_iterator
        i = _dock_items.begin(),
        e = _dock_items.end();

    for (; i != e; ++i) {
        if ((*i)->isIconified()) {
            return true;
        }
    }

    return false;
}

void
Dock::hide()
{
    getWidget().hide();
}

void
Dock::show()
{
    getWidget().show();
}

void
Dock::toggleDockable(int width, int height)
{
    static int prev_horizontal_position, prev_vertical_position;

    Gtk::Paned *parent_paned = getParentPaned();

    if (width > 0 && height > 0) {
        prev_horizontal_position = parent_paned->get_position();
        prev_vertical_position = _paned->get_position();

        if (getWidget().get_width() < width)
            parent_paned->set_position(parent_paned->get_width() - width);

        if (_paned->get_position() < height)
            _paned->set_position(height);

    } else {
        parent_paned->set_position(prev_horizontal_position);
        _paned->set_position(prev_vertical_position);
    }

}

void
Dock::scrollToItem(DockItem& item)
{
    int item_x, item_y;
    item.getWidget().translate_coordinates(getWidget(), 0, 0, item_x, item_y);

    int dock_height = getWidget().get_height(), item_height = item.getWidget().get_height();
    double vadjustment = _scrolled_window->get_vadjustment()->get_value();

    if (item_y < 0)
        _scrolled_window->get_vadjustment()->set_value(vadjustment + item_y);
    else if (item_y + item_height > dock_height)
        _scrolled_window->get_vadjustment()->set_value(
            vadjustment + ((item_y + item_height) - dock_height));
}

Glib::SignalProxy0<void>
Dock::signal_layout_changed()
{
    return Glib::SignalProxy0<void>(Glib::wrap(GTK_WIDGET(_gdl_dock)),
                                    &_signal_layout_changed_proxy);
}

void
Dock::_onLayoutChanged()
{
    if (isEmpty()) {
        if (hasIconifiedItems()) {
            _scrolled_window->set_size_request(_default_dock_bar_width);
        } else {
            _scrolled_window->set_size_request(_default_empty_width);
        }

        getParentPaned()->set_position(INT_MAX);
    } else {
        // unset any forced size requests
        _paned->get_child1()->set_size_request(-1, -1);
        _scrolled_window->set_size_request(-1);
    }
}

void
Dock::_onPanedButtonEvent(GdkEventButton *event)
{
    if (event->button == 1 && event->type == GDK_BUTTON_PRESS)
        /* unset size request when starting a drag */
        _paned->get_child1()->set_size_request(-1, -1);
}

gboolean
Dock::_on_paned_button_event(GtkWidget */*widget*/, GdkEventButton *event, gpointer user_data)
{
    if (Dock *dock = static_cast<Dock *>(user_data))
        dock->_onPanedButtonEvent(event);

    return FALSE;
}

const Glib::SignalProxyInfo
Dock::_signal_layout_changed_proxy =
{
    "layout-changed",
    (GCallback) &Glib::SignalProxyNormal::slot0_void_callback,
    (GCallback) &Glib::SignalProxyNormal::slot0_void_callback
};


} // namespace Widget
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99

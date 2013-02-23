/*
 * Author:
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include "ui/widget/dock.h"

#include "dock-item.h"
#include "desktop.h"
#include "inkscape.h"
#include "preferences.h"
#include "ui/icon-names.h"
#include "widgets/icon.h"

#include <gtkmm/icontheme.h>
#include <gtkmm/stockitem.h>
#include <glibmm/exceptionhandler.h>

namespace Inkscape {
namespace UI {
namespace Widget {

DockItem::DockItem(Dock& dock, const Glib::ustring& name, const Glib::ustring& long_name,
                   const Glib::ustring& icon_name, State state, Placement placement) :
    _dock(dock),
    _prev_state(state),
    _prev_position(0),
    _window(0),
    _x(0),
    _y(0),
    _grab_focus_on_realize(false),
    _gdl_dock_item(0),
    _dock_item_action_area(0)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    GdlDockItemBehavior gdl_dock_behavior =
        (prefs->getBool("/options/dock/cancenterdock", true) ?
            GDL_DOCK_ITEM_BEH_NORMAL :
            GDL_DOCK_ITEM_BEH_CANT_DOCK_CENTER);


    if (!icon_name.empty()) {
        Glib::RefPtr<Gtk::IconTheme> iconTheme = Gtk::IconTheme::get_default();

        if (!iconTheme->has_icon(icon_name)) {
            Inkscape::queueIconPrerender( INKSCAPE_ICON(icon_name.data()), Inkscape::ICON_SIZE_MENU );
        }
        // Icon might be in the icon theme, or might be a stock item. Check the proper source:
        if ( iconTheme->has_icon(icon_name) ) {
            int width = 0;
            int height = 0;
            Gtk::IconSize::lookup(Gtk::ICON_SIZE_MENU, width, height);
            _icon_pixbuf = iconTheme->load_icon(icon_name, width);
        } else {
            Gtk::StockItem item;
            Gtk::StockID stockId(icon_name);
            if ( Gtk::StockItem::lookup(stockId, item) ) {
#if WITH_GTKMM_3_0
                _icon_pixbuf = _dock.getWidget().render_icon_pixbuf( stockId, Gtk::ICON_SIZE_MENU );
#else
                _icon_pixbuf = _dock.getWidget().render_icon( stockId, Gtk::ICON_SIZE_MENU );
#endif
            }
        }
    }

    if ( _icon_pixbuf ) {
        _gdl_dock_item = gdl_dock_item_new_with_pixbuf_icon( name.c_str(), long_name.c_str(),
                                                             _icon_pixbuf->gobj(), gdl_dock_behavior );
    } else {
        _gdl_dock_item = gdl_dock_item_new(name.c_str(), long_name.c_str(), gdl_dock_behavior);
    }

    _frame.set_shadow_type(Gtk::SHADOW_IN);
    gtk_container_add (GTK_CONTAINER (_gdl_dock_item), GTK_WIDGET (_frame.gobj()));
    _frame.add(_dock_item_box);
    _dock_item_box.set_border_width(3);

    signal_drag_begin().connect(sigc::mem_fun(*this, &Inkscape::UI::Widget::DockItem::_onDragBegin));
    signal_drag_end().connect(sigc::mem_fun(*this, &Inkscape::UI::Widget::DockItem::_onDragEnd));
    signal_hide().connect(sigc::mem_fun(*this, &Inkscape::UI::Widget::DockItem::_onHide), false);
    signal_show().connect(sigc::mem_fun(*this, &Inkscape::UI::Widget::DockItem::_onShow), false);
    signal_state_changed().connect(sigc::mem_fun(*this, &Inkscape::UI::Widget::DockItem::_onStateChanged));
    signal_delete_event().connect(sigc::mem_fun(*this, &Inkscape::UI::Widget::DockItem::_onDeleteEvent));
    signal_realize().connect(sigc::mem_fun(*this, &Inkscape::UI::Widget::DockItem::_onRealize));

    _dock.addItem(*this, ( _prev_state == FLOATING_STATE || _prev_state == ICONIFIED_FLOATING_STATE ) ? FLOATING : placement);

    if (_prev_state == ICONIFIED_FLOATING_STATE || _prev_state == ICONIFIED_DOCKED_STATE) {
        iconify();
    }

    show_all();

}

DockItem::~DockItem()
{
    g_free(_gdl_dock_item);
}

Gtk::Widget&
DockItem::getWidget()
{
    return *Glib::wrap(GTK_WIDGET(_gdl_dock_item));
}

GtkWidget *
DockItem::gobj()
{
    return _gdl_dock_item;
}

Gtk::VBox *
DockItem::get_vbox()
{
    return &_dock_item_box;
}


void
DockItem::get_position(int& x, int& y)
{
    if (getWindow()) {
        getWindow()->get_position(x, y);
    } else {
        x = _x;
        y = _y;
    }
}

void
DockItem::get_size(int& width, int& height)
{
    if (getWindow()) {
        getWindow()->get_size(width, height);
    } else {
        width = get_vbox()->get_width();
        height = get_vbox()->get_height();
    }
}


void
DockItem::resize(int width, int height)
{
    if (_window)
        _window->resize(width, height);
}


void
DockItem::move(int x, int y)
{
    if (_window)
        _window->move(x, y);
}

void
DockItem::set_position(Gtk::WindowPosition position)
{
    if (_window)
        _window->set_position(position);
}

void
DockItem::set_size_request(int width, int height)
{
    getWidget().set_size_request(width, height);
}

void DockItem::size_request(Gtk::Requisition& requisition)
{
#if WITH_GTKMM_3_0
    Gtk::Requisition req_natural;
    getWidget().get_preferred_size(req_natural, requisition);
#else
    requisition = getWidget().size_request();
#endif
}

void
DockItem::set_title(Glib::ustring title)
{
    g_object_set (_gdl_dock_item,
                  "long-name", title.c_str(),
                  NULL);

    gdl_dock_item_set_tablabel(GDL_DOCK_ITEM(_gdl_dock_item),
                               gtk_label_new (title.c_str()));
}

bool
DockItem::isAttached() const
{
    return GDL_DOCK_OBJECT_ATTACHED (_gdl_dock_item);
}


bool
DockItem::isFloating() const
{
    return (GTK_WIDGET(gdl_dock_object_get_toplevel(GDL_DOCK_OBJECT (_gdl_dock_item))) !=
            _dock.getGdlWidget());
}

bool
DockItem::isIconified() const
{
    return GDL_DOCK_ITEM_ICONIFIED (_gdl_dock_item);
}

DockItem::State
DockItem::getState() const
{
    if (isIconified() && _prev_state == FLOATING_STATE) {
        return ICONIFIED_FLOATING_STATE;
    } else if (isIconified()) {
        return ICONIFIED_DOCKED_STATE;
    } else if (isFloating() && isAttached()) {
        return FLOATING_STATE;
    } else if (isAttached()) {
        return DOCKED_STATE;
    }

    return UNATTACHED;
}

DockItem::State
DockItem::getPrevState() const
{
    return _prev_state;
}

DockItem::Placement
DockItem::getPlacement() const
{
    GdlDockPlacement placement = (GdlDockPlacement)TOP;
    GdlDockObject *parent = gdl_dock_object_get_parent_object (GDL_DOCK_OBJECT(_gdl_dock_item));
    if (parent) {
        gdl_dock_object_child_placement(parent, GDL_DOCK_OBJECT(_gdl_dock_item), &placement);
    }

    return (Placement)placement;
}

void
DockItem::hide()
{
    gdl_dock_item_hide_item (GDL_DOCK_ITEM(_gdl_dock_item));
}

void
DockItem::show()
{
    gdl_dock_item_show_item (GDL_DOCK_ITEM(_gdl_dock_item));
}

void
DockItem::iconify()
{
    gdl_dock_item_iconify_item (GDL_DOCK_ITEM(_gdl_dock_item));
}

void
DockItem::show_all()
{
    gtk_widget_show_all(_gdl_dock_item);
}

void
DockItem::present()
{

    if (!isIconified() && !isAttached()) {
        show();
    }
    // tabbed
    else if (getPlacement() == CENTER) {
        int i = gtk_notebook_page_num(GTK_NOTEBOOK(gtk_widget_get_parent(_gdl_dock_item)),
                                       GTK_WIDGET (_gdl_dock_item));
        if (i >= 0)
            gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_widget_get_parent(_gdl_dock_item)), i);
    }

    // always grab focus, even if we're already present
    grab_focus();

    if (!isFloating() && getWidget().get_realized())
        _dock.scrollToItem(*this);
}


void
DockItem::grab_focus()
{
    if (gtk_widget_get_realized (_gdl_dock_item)) {

        // make sure the window we're in is present
        Gtk::Widget *toplevel = getWidget().get_toplevel();
        if (Gtk::Window *window = dynamic_cast<Gtk::Window *>(toplevel)) {
            window->present();
        }

        gtk_widget_grab_focus (_gdl_dock_item);

    } else {
        _grab_focus_on_realize = true;
    }
}


/* Signal wrappers */

Glib::SignalProxy0<void>
DockItem::signal_show()
{
    return Glib::SignalProxy0<void>(Glib::wrap(GTK_WIDGET(_gdl_dock_item)),
                                    &_signal_show_proxy);
}

Glib::SignalProxy0<void>
DockItem::signal_hide()
{
    return Glib::SignalProxy0<void>(Glib::wrap(GTK_WIDGET(_gdl_dock_item)),
                                    &_signal_hide_proxy);
}

Glib::SignalProxy1<bool, GdkEventAny *>
DockItem::signal_delete_event()
{
    return Glib::SignalProxy1<bool, GdkEventAny *>(Glib::wrap(GTK_WIDGET(_gdl_dock_item)),
                                                  &_signal_delete_event_proxy);
}

Glib::SignalProxy0<void>
DockItem::signal_drag_begin()
{
    return Glib::SignalProxy0<void>(Glib::wrap(GTK_WIDGET(_gdl_dock_item)),
                                    &_signal_drag_begin_proxy);
}

Glib::SignalProxy1<void, bool>
DockItem::signal_drag_end()
{
    return Glib::SignalProxy1<void, bool>(Glib::wrap(GTK_WIDGET(_gdl_dock_item)),
                                          &_signal_drag_end_proxy);
}

Glib::SignalProxy0<void>
DockItem::signal_realize()
{
    return Glib::SignalProxy0<void>(Glib::wrap(GTK_WIDGET(_gdl_dock_item)),
                                    &_signal_realize_proxy);
}

sigc::signal<void, DockItem::State, DockItem::State>
DockItem::signal_state_changed()
{
    return _signal_state_changed;
}

void
DockItem::_onHideWindow()
{
    if (_window)
        _window->get_position(_x, _y);
}

void
DockItem::_onHide()
{
    if (_prev_state == ICONIFIED_DOCKED_STATE)
        _prev_state = DOCKED_STATE;
    else if (_prev_state == ICONIFIED_FLOATING_STATE)
        _prev_state = FLOATING_STATE;

    _signal_state_changed.emit(UNATTACHED, getState());
}

void
DockItem::_onShow()
{
    _signal_state_changed.emit(UNATTACHED, getState());
}

void
DockItem::_onDragBegin()
{
    _prev_state = getState();
    if (_prev_state == FLOATING_STATE)
        _dock.toggleDockable(getWidget().get_width(), getWidget().get_height());
}

void
DockItem::_onDragEnd(bool)
{
    State state = getState();

    if (state != _prev_state)
        _signal_state_changed.emit(_prev_state, state);

    if (state == FLOATING_STATE) {
        if (_prev_state == FLOATING_STATE)
            _dock.toggleDockable();
    }

    _prev_state = state;
}

void
DockItem::_onRealize()
{
    if (_grab_focus_on_realize) {
        _grab_focus_on_realize = false;
        grab_focus();
    }
}

bool
DockItem::_onKeyPress(GdkEventKey *event)
{
    gboolean return_value;
    g_signal_emit_by_name (_gdl_dock_item, "key_press_event", event, &return_value);
    return return_value;
}

void
DockItem::_onStateChanged(State /*prev_state*/, State new_state)
{
    _window = getWindow();

    if (new_state == FLOATING_STATE && _window) {
        _window->signal_hide().connect(sigc::mem_fun(*this, &Inkscape::UI::Widget::DockItem::_onHideWindow));
        _signal_key_press_event_connection =
            _window->signal_key_press_event().connect(sigc::mem_fun(*this, &Inkscape::UI::Widget::DockItem::_onKeyPress));
    }
}


bool
DockItem::_onDeleteEvent(GdkEventAny */*event*/)
{
    hide();
    return false;
}


Gtk::Window *
DockItem::getWindow()
{
    g_return_val_if_fail(_gdl_dock_item, 0);
    Gtk::Container *parent = getWidget().get_parent();
    parent = (parent ? parent->get_parent() : 0);
    return (parent ? dynamic_cast<Gtk::Window *>(parent) : 0);
}

const Glib::SignalProxyInfo
DockItem::_signal_show_proxy =
{
    "show",
    (GCallback) &Glib::SignalProxyNormal::slot0_void_callback,
    (GCallback) &Glib::SignalProxyNormal::slot0_void_callback
};

const Glib::SignalProxyInfo
DockItem::_signal_hide_proxy =
{
    "hide",
    (GCallback) &Glib::SignalProxyNormal::slot0_void_callback,
    (GCallback) &Glib::SignalProxyNormal::slot0_void_callback
};


const Glib::SignalProxyInfo
DockItem::_signal_delete_event_proxy =
{
    "delete_event",
    (GCallback) &_signal_delete_event_callback,
    (GCallback) &_signal_delete_event_callback
};


const Glib::SignalProxyInfo
DockItem::_signal_drag_begin_proxy =
{
    "dock-drag-begin",
    (GCallback) &Glib::SignalProxyNormal::slot0_void_callback,
    (GCallback) &Glib::SignalProxyNormal::slot0_void_callback
};


const Glib::SignalProxyInfo
DockItem::_signal_drag_end_proxy =
{
    "dock_drag_end",
    (GCallback) &_signal_drag_end_callback,
    (GCallback) &_signal_drag_end_callback
};


const Glib::SignalProxyInfo
DockItem::_signal_realize_proxy =
{
    "realize",
    (GCallback) &Glib::SignalProxyNormal::slot0_void_callback,
    (GCallback) &Glib::SignalProxyNormal::slot0_void_callback
};


gboolean
DockItem::_signal_delete_event_callback(GtkWidget *self, GdkEventAny *event, void *data)
{
    using namespace Gtk;
    typedef sigc::slot<bool, GdkEventAny *> SlotType;

    if (Glib::ObjectBase::_get_current_wrapper((GObject *) self)) {
        try {
            if(sigc::slot_base *const slot = Glib::SignalProxyNormal::data_to_slot(data))
                return static_cast<int>( (*static_cast<SlotType*>(slot))(event) );
        } catch(...) {
            Glib::exception_handlers_invoke();
        }
    }

    typedef gboolean RType;
    return RType();
}

void
DockItem::_signal_drag_end_callback(GtkWidget *self, gboolean cancelled, void *data)
{
    using namespace Gtk;
    typedef sigc::slot<void, bool> SlotType;

    if (Glib::ObjectBase::_get_current_wrapper((GObject *) self)) {
        try {
            if(sigc::slot_base *const slot = Glib::SignalProxyNormal::data_to_slot(data))
                (*static_cast<SlotType *>(slot))(cancelled);
        } catch(...) {
            Glib::exception_handlers_invoke();
        }
    }
}


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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

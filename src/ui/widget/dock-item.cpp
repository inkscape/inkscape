/**
 * \brief A custom Inkscape wrapper around gdl_dock_item
 *
 * Author:
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include "dock-item.h"
#include "desktop.h"
#include "inkscape.h"
#include "prefs-utils.h"
#include "ui/widget/dock.h"
#include "widgets/icon.h"

#include <gtk/gtk.h>

#include <gtkmm/invisible.h>
#include <gtkmm/stock.h>

namespace Inkscape {
namespace UI {
namespace Widget {

DockItem::DockItem(Dock& dock, const Glib::ustring& name, const Glib::ustring& long_name,
                   const Glib::ustring& icon_name, State state) :
    _dock (dock),
    _prev_state (state),
    _window (NULL),
    _dock_item_action_area (NULL)
{
    /* Add a "signal_response" signal to the GdlDockItem, make sure it is
     * only done once for the class.
     */
    static guint response_signal = 0;

    if (response_signal == 0) {
        response_signal = g_signal_new ("signal_response",
                                        GDL_TYPE_DOCK_ITEM,
                                        G_SIGNAL_RUN_FIRST,
                                        0,
                                        NULL, NULL,
                                        g_cclosure_marshal_VOID__INT,
                                        G_TYPE_NONE, 1, G_TYPE_INT);
    }


    GdlDockItemBehavior gdl_dock_behavior = 
        (prefs_get_int_attribute_limited ("options.dock", "cancenterdock", 1, 0, 1) == 0 ?
         GDL_DOCK_ITEM_BEH_CANT_DOCK_CENTER
         : GDL_DOCK_ITEM_BEH_NORMAL);

    if (!icon_name.empty()) {
        Gtk::Widget *icon = sp_icon_get_icon(icon_name, Inkscape::ICON_SIZE_MENU);
        if (icon) {
            // check icon type (inkscape, gtk, none) 
            if ( SP_IS_ICON(icon->gobj()) ) { 
                SPIcon* sp_icon = SP_ICON(icon->gobj());
                sp_icon_fetch_pixbuf(sp_icon);
                _icon_pixbuf = Glib::wrap(sp_icon->pb, true);
            } else if ( GTK_IS_IMAGE(icon->gobj()) ) {
                _icon_pixbuf = Gtk::Invisible().render_icon(Gtk::StockID(icon_name),
                                                            Gtk::ICON_SIZE_MENU);
            }
            delete icon;

            _gdl_dock_item = 
                gdl_dock_item_new_with_pixbuf_icon(name.c_str(), long_name.c_str(), 
                                                   _icon_pixbuf->gobj(), gdl_dock_behavior);
        }
    } else {
        _gdl_dock_item = 
            gdl_dock_item_new(name.c_str(), long_name.c_str(), gdl_dock_behavior);
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

    _dock.addItem(*this, (_prev_state == FLOATING_STATE ? FLOATING : TOP));

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
    if (_window) {
        _window->get_size(width, height);
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

void 
DockItem::size_request(Gtk::Requisition& requisition)
{ 
    getWidget().size_request(requisition);
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
    gboolean floating = FALSE;
    if (GDL_IS_DOCK (gdl_dock_object_get_parent_object (GDL_DOCK_OBJECT (_gdl_dock_item)))) {
        GdlDock* dock = GDL_DOCK (gdl_dock_object_get_parent_object (GDL_DOCK_OBJECT (_gdl_dock_item)));
        g_object_get (dock,
                      "floating", &floating,
                      NULL);
    }
    return floating;
}

bool
DockItem::isIconified() const
{
    return GDL_DOCK_ITEM_ICONIFIED (_gdl_dock_item);
}

DockItem::State
DockItem::getState() const
{
    return (isAttached() ? (isFloating() ? FLOATING_STATE : DOCKED_STATE) : UNATTACHED);
}

DockItem::State
DockItem::getPrevState() const
{
    return _prev_state;
}

DockItem::Placement
DockItem::getPlacement() const
{
    GdlDockPlacement placement = (GdlDockPlacement)NONE;
    gdl_dock_object_child_placement(gdl_dock_object_get_parent_object (GDL_DOCK_OBJECT(_gdl_dock_item)),
                                    GDL_DOCK_OBJECT(_gdl_dock_item),
                                    &placement);
    return (Placement)placement;
}


void
DockItem::addButton(Gtk::Button* button, int response_id)
{
    // Create a button box for the response buttons if it's the first button to be added
    if (!_dock_item_action_area) {
        _dock_item_action_area = new Gtk::HButtonBox(Gtk::BUTTONBOX_END, 6);
        _dock_item_box.pack_end(*_dock_item_action_area, Gtk::PACK_SHRINK, 0);
        _dock_item_action_area->set_border_width(6);
    }

    _dock_item_action_area->pack_start(*button);
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
DockItem::show_all()
{
    gtk_widget_show_all(_gdl_dock_item);
}

void
DockItem::present()
{
    // iconified or unattached
    if (isIconified() || !isAttached()) {
        show();
    }        

    // tabbed
    else if (getPlacement() == CENTER) {
        int i = gtk_notebook_page_num (GTK_NOTEBOOK (_gdl_dock_item->parent),
                                       GTK_WIDGET (_gdl_dock_item));
        if (i >= 0)
            gtk_notebook_set_current_page (GTK_NOTEBOOK (_gdl_dock_item->parent), i);
        return;
    }

    // always grab focus, even if we're already present
    grab_focus();
}


void
DockItem::grab_focus()
{
    if (GTK_WIDGET_REALIZED (_gdl_dock_item)) {

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

Glib::SignalProxy1<void, int>
DockItem::signal_response()
{
    return Glib::SignalProxy1<void, int>(Glib::wrap(GTK_WIDGET(_gdl_dock_item)), 
                                         &_signal_response_proxy);
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
DockItem::_onStateChanged(State prev_state, State new_state)
{
    _window = getWindow();

    if (new_state == FLOATING_STATE) {
        _window->signal_hide().connect(sigc::mem_fun(*this, &Inkscape::UI::Widget::DockItem::_onHideWindow));
        _signal_key_press_event_connection = 
            _window->signal_key_press_event().connect(sigc::mem_fun(*this, &Inkscape::UI::Widget::DockItem::_onKeyPress));
    }
}


bool
DockItem::_onDeleteEvent(GdkEventAny *event)
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
DockItem::_signal_response_proxy = 
{
    "signal_response",
    (GCallback) &_signal_response_callback,
    (GCallback) &_signal_response_callback
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
DockItem::_signal_response_callback(GtkWidget *self, gint response_id, void *data)
{
    using namespace Gtk;
    typedef sigc::slot<void, int> SlotType;

    if (Glib::ObjectBase::_get_current_wrapper((GObject *) self)) {
        try {
            if(sigc::slot_base *const slot = Glib::SignalProxyNormal::data_to_slot(data))
                (*static_cast<SlotType *>(slot))(response_id);
        } catch(...) {
            Glib::exception_handlers_invoke();
        }
    }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

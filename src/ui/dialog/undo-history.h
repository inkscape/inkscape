/** @file
 * @brief Undo History dialog
 */
/* Author:
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2006 Authors
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_UNDO_HISTORY_H
#define INKSCAPE_UI_DIALOG_UNDO_HISTORY_H

#include <glibmm/refptr.h>
#include <gtkmm/cellrendererpixbuf.h>
#include <gtkmm/image.h>
#include <gtkmm/invisible.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeselection.h>

#include <functional>
#include <sstream>

#include "desktop.h"
#include "event-log.h"

#include "ui/widget/panel.h"
#include "widgets/icon.h"

namespace Inkscape {
namespace UI {
namespace Dialog {


/* Custom cell renderers */

class CellRendererSPIcon : public Gtk::CellRendererPixbuf {
public:

    CellRendererSPIcon() :
        Glib::ObjectBase(typeid(CellRendererPixbuf)),
        Gtk::CellRendererPixbuf(),
        _property_icon(*this, "icon", Glib::RefPtr<Gdk::Pixbuf>(0)),
        _property_event_type(*this, "event_type", 0)
    { }
    
    Glib::PropertyProxy<unsigned int> 
    property_event_type() { return _property_event_type.get_proxy(); }

protected:

    virtual void
    render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window,
                 Gtk::Widget& widget,
                 const Gdk::Rectangle& background_area,
                 const Gdk::Rectangle& cell_area,
                 const Gdk::Rectangle& expose_area,
                 Gtk::CellRendererState flags);
private:

    Glib::Property<Glib::RefPtr<Gdk::Pixbuf> > _property_icon;
    Glib::Property<unsigned int> _property_event_type;
    std::map<const unsigned int, Glib::RefPtr<Gdk::Pixbuf> > _icon_cache;
};


class CellRendererInt : public Gtk::CellRendererText {
public:

    struct Filter : std::unary_function<int, bool> {
        virtual ~Filter() {}
        virtual bool operator() (const int&) const =0;
    };

    CellRendererInt(const Filter& filter=no_filter) :
        Glib::ObjectBase(typeid(CellRendererText)),
        Gtk::CellRendererText(),
        _property_number(*this, "number", 0),
        _filter (filter)
    { }


    Glib::PropertyProxy<int> 
    property_number() { return _property_number.get_proxy(); }

    static const Filter& no_filter;

 protected:

    virtual void 
    render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window,
                 Gtk::Widget& widget,
                 const Gdk::Rectangle& background_area,
                 const Gdk::Rectangle& cell_area,
                 const Gdk::Rectangle& expose_area,
                 Gtk::CellRendererState flags);

private:

    Glib::Property<int> _property_number;
    const Filter& _filter;

    struct NoFilter : Filter { bool operator() (const int& /*x*/) const { return true; } };
};

/**
 * \brief Dialog for presenting document change history
 *
 * This dialog allows the user to undo and redo multiple events in a more convenient way
 * than repateaded ctrl-z, ctrl-shift-z.
 */
class UndoHistory : public Widget::Panel {
public:
    virtual ~UndoHistory();

    static UndoHistory &getInstance();
    void setDesktop(SPDesktop* desktop);

    sigc::connection _document_replaced_connection;

protected:

    Document *_document;
    EventLog *_event_log;

    const EventLog::EventModelColumns *_columns;

    Gtk::ScrolledWindow _scrolled_window;    

    Glib::RefPtr<Gtk::TreeModel> _event_list_store;
    Gtk::TreeView _event_list_view;
    Glib::RefPtr<Gtk::TreeSelection> _event_list_selection;

    EventLog::CallbackMap _callback_connections;

    void _onListSelectionChange();
    void _onExpandEvent(const Gtk::TreeModel::iterator &iter, const Gtk::TreeModel::Path &path);
    void _onCollapseEvent(const Gtk::TreeModel::iterator &iter, const Gtk::TreeModel::Path &path);

private:
    UndoHistory();
  
    // no default constructor, noncopyable, nonassignable
    UndoHistory(UndoHistory const &d);
    UndoHistory operator=(UndoHistory const &d);

    struct GreaterThan : CellRendererInt::Filter {
        GreaterThan(int _i) : i (_i) {}
        bool operator() (const int& x) const { return x > i; }
        int i;
    };

    static const CellRendererInt::Filter& greater_than_1;
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //INKSCAPE_UI_DIALOG_UNDO_HISTORY_H

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

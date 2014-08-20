/** @file
 * @brief Undo History dialog
 */
/* Author:
 *   Gustav Broberg <broberg@kth.se>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2014 Authors
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_UNDO_HISTORY_H
#define INKSCAPE_UI_DIALOG_UNDO_HISTORY_H

#include "ui/widget/panel.h"
#include <gtkmm/cellrendererpixbuf.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeselection.h>
#include <glibmm/property.h>

#include <functional>
#include <sstream>

#include "event-log.h"

#include "widgets/icon.h"
#include "ui/dialog/desktop-tracker.h"

class SPDesktop;

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
#if WITH_GTKMM_3_0
    virtual void render_vfunc(const Cairo::RefPtr<Cairo::Context>& cr,
                              Gtk::Widget& widget,
                              const Gdk::Rectangle& background_area,
                              const Gdk::Rectangle& cell_area,
                              Gtk::CellRendererState flags);
#else
    virtual void render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window,
                              Gtk::Widget& widget,
                              const Gdk::Rectangle& background_area,
                              const Gdk::Rectangle& cell_area,
                              const Gdk::Rectangle& expose_area,
                              Gtk::CellRendererState flags);
#endif
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
#if WITH_GTKMM_3_0
    virtual void render_vfunc(const Cairo::RefPtr<Cairo::Context>& cr,
                              Gtk::Widget& widget,
                              const Gdk::Rectangle& background_area,
                              const Gdk::Rectangle& cell_area,
                              Gtk::CellRendererState flags);
#else
    virtual void render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window,
                              Gtk::Widget& widget,
                              const Gdk::Rectangle& background_area,
                              const Gdk::Rectangle& cell_area,
                              const Gdk::Rectangle& expose_area,
                              Gtk::CellRendererState flags);
#endif

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

    SPDesktop *_desktop;
    SPDocument *_document;
    EventLog *_event_log;


    const EventLog::EventModelColumns *_columns;

    Gtk::ScrolledWindow _scrolled_window;    

    Glib::RefPtr<Gtk::TreeModel> _event_list_store;
    Gtk::TreeView _event_list_view;
    Glib::RefPtr<Gtk::TreeSelection> _event_list_selection;

    DesktopTracker _deskTrack;
    sigc::connection _desktopChangeConn;

    EventLog::CallbackMap _callback_connections;

    static void *_handleEventLogDestroyCB(void *data);

    void _connectDocument(SPDesktop* desktop, SPDocument *document);
    void _connectEventLog();
    void _handleDocumentReplaced(SPDesktop* desktop, SPDocument *document);
    void *_handleEventLogDestroy();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

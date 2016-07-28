/**
 * @file
 * Undo History dialog - implementation.
 */
/* Author:
 *   Gustav Broberg <broberg@kth.se>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2014 Authors
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "undo-history.h"
#include <glibmm/i18n.h>
#include <stddef.h>
#include <sigc++/sigc++.h>

#include "document.h"
#include "document-undo.h"
#include "inkscape.h"
#include "verbs.h"

#include "util/signal-blocker.h"

#include "desktop.h"
#include <gtkmm/invisible.h>

namespace Inkscape {
namespace UI {
namespace Dialog {

/* Rendering functions for custom cell renderers */
#if WITH_GTKMM_3_0
void CellRendererSPIcon::render_vfunc(const Cairo::RefPtr<Cairo::Context>& cr,
                                      Gtk::Widget& widget,
                                      const Gdk::Rectangle& background_area,
                                      const Gdk::Rectangle& cell_area,
                                      Gtk::CellRendererState flags)
#else
void CellRendererSPIcon::render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window,
                                      Gtk::Widget& widget,
                                      const Gdk::Rectangle& background_area,
                                      const Gdk::Rectangle& cell_area,
                                      const Gdk::Rectangle& expose_area,
                                      Gtk::CellRendererState flags)
#endif
{
    // if this event type doesn't have an icon...
    if ( !Inkscape::Verb::get(_property_event_type)->get_image() ) return;

    // if the icon isn't cached, render it to a pixbuf
    if ( !_icon_cache[_property_event_type] ) {

        Glib::ustring image = Inkscape::Verb::get(_property_event_type)->get_image();
        Gtk::Widget* icon = sp_icon_get_icon(image, Inkscape::ICON_SIZE_MENU);

        if (icon) {

            // check icon type (inkscape, gtk, none)
            if ( SP_IS_ICON(icon->gobj()) ) {
                SPIcon* sp_icon = SP_ICON(icon->gobj());
                sp_icon_fetch_pixbuf(sp_icon);
                _property_icon = Glib::wrap(sp_icon->pb, true);
            } else if ( GTK_IS_IMAGE(icon->gobj()) ) {
#if WITH_GTKMM_3_0
                _property_icon = Gtk::Invisible().render_icon_pixbuf(Gtk::StockID(image),
                                                                     Gtk::ICON_SIZE_MENU);
#else
                _property_icon = Gtk::Invisible().render_icon(Gtk::StockID(image),
                                                              Gtk::ICON_SIZE_MENU);
#endif
            } else {
                delete icon;
                return;
            }

            delete icon;
            property_pixbuf() = _icon_cache[_property_event_type] = _property_icon.get_value();
        }

    } else {
        property_pixbuf() = _icon_cache[_property_event_type];
    }

#if WITH_GTKMM_3_0
    Gtk::CellRendererPixbuf::render_vfunc(cr, widget, background_area,
                                          cell_area, flags);
#else
    Gtk::CellRendererPixbuf::render_vfunc(window, widget, background_area,
                                          cell_area, expose_area, flags);
#endif
}


#if WITH_GTKMM_3_0
void CellRendererInt::render_vfunc(const Cairo::RefPtr<Cairo::Context>& cr,
                                   Gtk::Widget& widget,
                                   const Gdk::Rectangle& background_area,
                                   const Gdk::Rectangle& cell_area,
                                   Gtk::CellRendererState flags)
#else
void CellRendererInt::render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window,
                                   Gtk::Widget& widget,
                                   const Gdk::Rectangle& background_area,
                                   const Gdk::Rectangle& cell_area,
                                   const Gdk::Rectangle& expose_area,
                                   Gtk::CellRendererState flags)
#endif
{
    if( _filter(_property_number) ) {
        std::ostringstream s;
        s << _property_number << std::flush;
        property_text() = s.str();
#if WITH_GTKMM_3_0
        Gtk::CellRendererText::render_vfunc(cr, widget, background_area,
                                            cell_area, flags);
#else
        Gtk::CellRendererText::render_vfunc(window, widget, background_area,
                                            cell_area, expose_area, flags);
#endif
    }
}

const CellRendererInt::Filter& CellRendererInt::no_filter = CellRendererInt::NoFilter();

UndoHistory& UndoHistory::getInstance()
{
    return *new UndoHistory();
}

UndoHistory::UndoHistory()
    : UI::Widget::Panel ("", "/dialogs/undo-history", SP_VERB_DIALOG_UNDO_HISTORY),
      _document_replaced_connection(),
      _desktop(getDesktop()),
      _document(_desktop ? _desktop->doc() : NULL),
      _event_log(_desktop ? _desktop->event_log : NULL),
      _columns(_event_log ? &_event_log->getColumns() : NULL),
      _scrolled_window(),
      _event_list_store(),
      _event_list_selection(_event_list_view.get_selection()),
      _deskTrack(),
      _desktopChangeConn(),
      _callback_connections()
{
    if ( !_document || !_event_log || !_columns ) return;

    set_size_request(-1, 95);

    _getContents()->pack_start(_scrolled_window);
    _scrolled_window.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

    // connect with the EventLog
    _connectEventLog();

    _event_list_view.set_rules_hint(false);
    _event_list_view.set_enable_search(false);
    _event_list_view.set_headers_visible(false);

    CellRendererSPIcon* icon_renderer = Gtk::manage(new CellRendererSPIcon());
    icon_renderer->property_xpad() = 2;
    icon_renderer->property_width() = 24;
    int cols_count = _event_list_view.append_column("Icon", *icon_renderer);

    Gtk::TreeView::Column* icon_column = _event_list_view.get_column(cols_count-1);
    icon_column->add_attribute(icon_renderer->property_event_type(), _columns->type);

    CellRendererInt* children_renderer = Gtk::manage(new CellRendererInt(greater_than_1));
    children_renderer->property_weight() = 600; // =Pango::WEIGHT_SEMIBOLD (not defined in old versions of pangomm)
    children_renderer->property_xalign() = 1.0;
    children_renderer->property_xpad() = 2;
    children_renderer->property_width() = 24;

    cols_count = _event_list_view.append_column("Children", *children_renderer);
    Gtk::TreeView::Column* children_column = _event_list_view.get_column(cols_count-1);
    children_column->add_attribute(children_renderer->property_number(), _columns->child_count);

    Gtk::CellRendererText* description_renderer = Gtk::manage(new Gtk::CellRendererText());
    description_renderer->property_ellipsize() = Pango::ELLIPSIZE_END;

    cols_count = _event_list_view.append_column("Description", *description_renderer);
    Gtk::TreeView::Column* description_column = _event_list_view.get_column(cols_count-1);
    description_column->add_attribute(description_renderer->property_text(), _columns->description);
    description_column->set_resizable();
    description_column->set_sizing(Gtk::TREE_VIEW_COLUMN_AUTOSIZE);
    description_column->set_min_width (150);

    _event_list_view.set_expander_column( *_event_list_view.get_column(cols_count-1) );

    _scrolled_window.add(_event_list_view);

    // connect EventLog callbacks
    _callback_connections[EventLog::CALLB_SELECTION_CHANGE] =
        _event_list_selection->signal_changed().connect(sigc::mem_fun(*this, &Inkscape::UI::Dialog::UndoHistory::_onListSelectionChange));

    _callback_connections[EventLog::CALLB_EXPAND] =
        _event_list_view.signal_row_expanded().connect(sigc::mem_fun(*this, &Inkscape::UI::Dialog::UndoHistory::_onExpandEvent));

    _callback_connections[EventLog::CALLB_COLLAPSE] =
        _event_list_view.signal_row_collapsed().connect(sigc::mem_fun(*this, &Inkscape::UI::Dialog::UndoHistory::_onCollapseEvent));

    _desktopChangeConn = _deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &UndoHistory::setDesktop) );
    _deskTrack.connect(GTK_WIDGET(gobj()));

    // connect to be informed of document changes
    signalDocumentReplaced().connect(sigc::mem_fun(*this, &UndoHistory::_handleDocumentReplaced));

    show_all_children();

    // scroll to the selected row
    _event_list_view.set_cursor(_event_list_store->get_path(_event_log->getCurrEvent()));
}

UndoHistory::~UndoHistory()
{
    _desktopChangeConn.disconnect();
}


void UndoHistory::setDesktop(SPDesktop* desktop)
{
    Panel::setDesktop(desktop);

    EventLog *newEventLog = desktop ? desktop->event_log : NULL;
    if ((_desktop == desktop) && (_event_log == newEventLog)) {
        // same desktop set
    }
    else
    {
        _connectDocument(desktop, desktop ? desktop->doc() : NULL);
    }
}

void UndoHistory::_connectDocument(SPDesktop* desktop, SPDocument * /*document*/)
{
    // disconnect from prior
    if (_event_log) {
        _event_log->removeDialogConnection(&_event_list_view, &_callback_connections);
    }

    SignalBlocker blocker(&_callback_connections[EventLog::CALLB_SELECTION_CHANGE]);

    _event_list_view.unset_model();

    // connect to new EventLog/Desktop
    _desktop = desktop;
    _event_log = desktop ? desktop->event_log : NULL;
    _document = desktop ? desktop->doc() : NULL;
    _connectEventLog();
}

void UndoHistory::_connectEventLog()
{
    if (_event_log) {
        _event_log->add_destroy_notify_callback(this, &_handleEventLogDestroyCB);
        _event_list_store = _event_log->getEventListStore();

        _event_list_view.set_model(_event_list_store);

        _event_log->addDialogConnection(&_event_list_view, &_callback_connections);
        _event_list_view.scroll_to_row(_event_list_store->get_path(_event_list_selection->get_selected()));        
    }
}

void UndoHistory::_handleDocumentReplaced(SPDesktop* desktop, SPDocument *document)
{
    if ((desktop != _desktop) || (document != _document)) {
        _connectDocument(desktop, document);
    }
}

void *UndoHistory::_handleEventLogDestroyCB(void *data)
{
    void *result = NULL;
    if (data) {
        UndoHistory *self = reinterpret_cast<UndoHistory*>(data);
        result = self->_handleEventLogDestroy();
    }
    return result;
}

// called *after* _event_log has been destroyed.
void *UndoHistory::_handleEventLogDestroy()
{
    if (_event_log) {
        SignalBlocker blocker(&_callback_connections[EventLog::CALLB_SELECTION_CHANGE]);

        _event_list_view.unset_model();
        _event_list_store.reset();
        _event_log = NULL;
    }

    return NULL;
}

void
UndoHistory::_onListSelectionChange()
{

    EventLog::const_iterator selected = _event_list_selection->get_selected();

    /* If no event is selected in the view, find the right one and select it. This happens whenever
     * a branch we're currently in is collapsed.
     */
    if (!selected) {

        EventLog::iterator curr_event = _event_log->getCurrEvent();

        if (curr_event->parent()) {

            EventLog::iterator curr_event_parent = curr_event->parent();
            EventLog::iterator last = curr_event_parent->children().end();

            _event_log->blockNotifications();
            for ( --last ; curr_event != last ; ++curr_event ) {
                DocumentUndo::redo(_document);
            }
            _event_log->blockNotifications(false);

            _event_log->setCurrEvent(curr_event);
            _event_list_selection->select(curr_event_parent);

        } else {  // this should not happen
            _event_list_selection->select(curr_event);
        }

    } else {

        EventLog::const_iterator last_selected = _event_log->getCurrEvent();

        /* Selecting a collapsed parent event is equal to selecting the last child
         * of that parent's branch.
         */

        if ( !selected->children().empty() &&
             !_event_list_view.row_expanded(_event_list_store->get_path(selected)) )
        {
            selected = selected->children().end();
            --selected;
        }

        // An event before the current one has been selected. Undo to the selected event.
        if ( _event_list_store->get_path(selected) <
             _event_list_store->get_path(last_selected) )
        {
            _event_log->blockNotifications();

            while ( selected != last_selected ) {

                DocumentUndo::undo(_document);

                if ( last_selected->parent() &&
                     last_selected == last_selected->parent()->children().begin() )
                {
                    last_selected = last_selected->parent();
                    _event_log->setCurrEventParent((EventLog::iterator)NULL);
                } else {
                    --last_selected;
                    if ( !last_selected->children().empty() ) {
                        _event_log->setCurrEventParent(last_selected);
                        last_selected = last_selected->children().end();
                        --last_selected;
                    }
                }
            }
            _event_log->blockNotifications(false);
            _event_log->updateUndoVerbs();

        } else { // An event after the current one has been selected. Redo to the selected event.

            _event_log->blockNotifications();

            while ( selected != last_selected ) {

                DocumentUndo::redo(_document);

                if ( !last_selected->children().empty() ) {
                    _event_log->setCurrEventParent(last_selected);
                    last_selected = last_selected->children().begin();
                } else {
                    ++last_selected;
                    if ( last_selected->parent() &&
                         last_selected == last_selected->parent()->children().end() )
                    {
                        last_selected = last_selected->parent();
                        ++last_selected;
                        _event_log->setCurrEventParent((EventLog::iterator)NULL);
                    }
                }
            }
            _event_log->blockNotifications(false);

        }

        _event_log->setCurrEvent(selected);
        _event_log->updateUndoVerbs();
    }

}

void
UndoHistory::_onExpandEvent(const Gtk::TreeModel::iterator &iter, const Gtk::TreeModel::Path &/*path*/)
{
    if ( iter == _event_list_selection->get_selected() ) {
        _event_list_selection->select(_event_log->getCurrEvent());
    }
}

void
UndoHistory::_onCollapseEvent(const Gtk::TreeModel::iterator &iter, const Gtk::TreeModel::Path &/*path*/)
{
    // Collapsing a branch we're currently in is equal to stepping to the last event in that branch
    if ( iter == _event_log->getCurrEvent() ) {
        EventLog::const_iterator curr_event_parent = _event_log->getCurrEvent();
        EventLog::const_iterator curr_event = curr_event_parent->children().begin();
        EventLog::const_iterator last = curr_event_parent->children().end();

        _event_log->blockNotifications();
        DocumentUndo::redo(_document);

        for ( --last ; curr_event != last ; ++curr_event ) {
            DocumentUndo::redo(_document);
        }
        _event_log->blockNotifications(false);

        _event_log->setCurrEvent(curr_event);
        _event_log->setCurrEventParent(curr_event_parent);
        _event_list_selection->select(curr_event_parent);
    }
}

const CellRendererInt::Filter& UndoHistory::greater_than_1 = UndoHistory::GreaterThan(1);

} // namespace Dialog
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

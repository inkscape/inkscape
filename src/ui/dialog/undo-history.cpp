/** @file
 * @brief Undo History dialog - implementation
 */
/* Author:
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2006 Authors
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/i18n.h>
#include <gtk/gtkimage.h>
#include <sigc++/sigc++.h>


#include "document.h"
#include "inkscape.h"
#include "ui/icons.h"
#include "verbs.h"
#include "desktop-handles.h"

#include "undo-history.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

/* Rendering functions for custom cell renderers */

void
CellRendererSPIcon::render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window,
                                 Gtk::Widget& widget,
                                 const Gdk::Rectangle& background_area,
                                 const Gdk::Rectangle& cell_area,
                                 const Gdk::Rectangle& expose_area,
                                 Gtk::CellRendererState flags)
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
                _property_icon = Gtk::Invisible().render_icon(Gtk::StockID(image),
                                                              Gtk::ICON_SIZE_MENU);
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

    Gtk::CellRendererPixbuf::render_vfunc(window, widget, background_area,
                                          cell_area, expose_area, flags);
}


void
CellRendererInt::render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window,
                              Gtk::Widget& widget,
                              const Gdk::Rectangle& background_area,
                              const Gdk::Rectangle& cell_area,
                              const Gdk::Rectangle& expose_area,
                              Gtk::CellRendererState flags)
{
    if( _filter(_property_number) ) {
        std::ostringstream s;
        s << _property_number << std::flush;
        property_text() = s.str();
        Gtk::CellRendererText::render_vfunc(window, widget, background_area,
                                            cell_area, expose_area, flags);
    }
}

const CellRendererInt::Filter& CellRendererInt::no_filter = CellRendererInt::NoFilter();

UndoHistory& UndoHistory::getInstance()
{
    return *new UndoHistory();
}

void
UndoHistory::setDesktop(SPDesktop* desktop)
{
    Panel::setDesktop(desktop);

    if (!desktop) return;

    _document = sp_desktop_document(desktop);

    _event_log = desktop->event_log;

    _callback_connections[EventLog::CALLB_SELECTION_CHANGE].block();

    _event_list_store = _event_log->getEventListStore();
    _event_list_view.set_model(_event_list_store);
    _event_list_selection = _event_list_view.get_selection();

    _event_log->connectWithDialog(&_event_list_view, &_callback_connections);
    _event_list_view.scroll_to_row(_event_list_store->get_path(_event_list_selection->get_selected()));

    _callback_connections[EventLog::CALLB_SELECTION_CHANGE].block(false);
}

UndoHistory::UndoHistory()
    : UI::Widget::Panel ("", "/dialogs/undo-history", SP_VERB_DIALOG_UNDO_HISTORY),
      _document (sp_desktop_document(getDesktop())),
      _event_log (getDesktop() ? getDesktop()->event_log : NULL),
      _columns (_event_log ? &_event_log->getColumns() : NULL),
      _event_list_selection (_event_list_view.get_selection())
{
    if ( !_document || !_event_log || !_columns ) return;

    set_size_request(300, 95);

    _getContents()->pack_start(_scrolled_window);
    _scrolled_window.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

    _event_list_store = _event_log->getEventListStore();

    _event_list_view.set_model(_event_list_store);
    _event_list_view.set_rules_hint(false);
    _event_list_view.set_enable_search(false);
    _event_list_view.set_headers_visible(false);

    CellRendererSPIcon* icon_renderer = Gtk::manage(new CellRendererSPIcon());
    icon_renderer->property_xpad() = 8;
    icon_renderer->property_width() = 36;
    int cols_count = _event_list_view.append_column("Icon", *icon_renderer);

    Gtk::TreeView::Column* icon_column = _event_list_view.get_column(cols_count-1);
    icon_column->add_attribute(icon_renderer->property_event_type(), _columns->type);

    Gtk::CellRendererText* description_renderer = Gtk::manage(new Gtk::CellRendererText());

    cols_count = _event_list_view.append_column("Description", *description_renderer);
    Gtk::TreeView::Column* description_column = _event_list_view.get_column(cols_count-1);
    description_column->add_attribute(description_renderer->property_text(), _columns->description);
    description_column->set_resizable();

    _event_list_view.set_expander_column( *_event_list_view.get_column(cols_count-1) );

    CellRendererInt* children_renderer = Gtk::manage(new CellRendererInt(greater_than_1));
    children_renderer->property_weight() = 600; // =Pango::WEIGHT_SEMIBOLD (not defined in old versions of pangomm)
    children_renderer->property_xalign() = 1.0;
    children_renderer->property_xpad() = 20;

    cols_count = _event_list_view.append_column("Children", *children_renderer);
    Gtk::TreeView::Column* children_column = _event_list_view.get_column(cols_count-1);
    children_column->add_attribute(children_renderer->property_number(), _columns->child_count);

    _scrolled_window.add(_event_list_view);

    // connect EventLog callbacks
    _callback_connections[EventLog::CALLB_SELECTION_CHANGE] =
        _event_list_selection->signal_changed().connect(sigc::mem_fun(*this, &Inkscape::UI::Dialog::UndoHistory::_onListSelectionChange));

    _callback_connections[EventLog::CALLB_EXPAND] =
        _event_list_view.signal_row_expanded().connect(sigc::mem_fun(*this, &Inkscape::UI::Dialog::UndoHistory::_onExpandEvent));

    _callback_connections[EventLog::CALLB_COLLAPSE] =
        _event_list_view.signal_row_collapsed().connect(sigc::mem_fun(*this, &Inkscape::UI::Dialog::UndoHistory::_onCollapseEvent));

    // connect with the EventLog
    _event_log->connectWithDialog(&_event_list_view, &_callback_connections);

    show_all_children();

    // scroll to the selected row
    _event_list_view.set_cursor(_event_list_store->get_path(_event_log->getCurrEvent()));
}

UndoHistory::~UndoHistory()
{
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
                sp_document_redo(_document);
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

                sp_document_undo(_document);

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

                sp_document_redo(_document);

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
        sp_document_redo(_document);

        for ( --last ; curr_event != last ; ++curr_event ) {
            sp_document_redo(_document);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

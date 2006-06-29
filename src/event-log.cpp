/*
 * Author:
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (c) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "desktop.h"

#include "event-log.h"

namespace Inkscape {

EventLog::EventLog() :
    UndoStackObserver(),
    _connected (false),
    _event_list_store (Gtk::TreeStore::create(_columns)),
    _event_list_selection (NULL),
    _event_list_view (NULL),
    _curr_event_parent (NULL),
    _notifications_blocked (false),
    _callback_connections (NULL)
{
    // add initial pseudo event
    Gtk::TreeRow curr_row = *(_event_list_store->append());
    _curr_event = _last_event = curr_row;
    
    curr_row[_columns.description] = _("[Unchanged]");
    curr_row[_columns.type] = SP_VERB_FILE_NEW;
}

EventLog::~EventLog() { }

void
EventLog::notifyUndoEvent(Event* log) 
{
    if ( !_notifications_blocked ) {
      
        // if we're on the first child event...
        if ( _curr_event->parent() &&
             _curr_event == _curr_event->parent()->children().begin() )
	{
            // ...back up to the parent
            _curr_event = _curr_event->parent();
            _curr_event_parent = (iterator)NULL;

	} else {

            // if we're about to leave a branch, collapse it
            if ( !_curr_event->children().empty() && _connected ) {
                (*_callback_connections)[CALLB_COLLAPSE].block();
                _event_list_view->collapse_row(_event_list_store->get_path(_curr_event));
                (*_callback_connections)[CALLB_COLLAPSE].block(false);
            }

            --_curr_event;

            // if we're entering a branch, move to the end of it
            if (!_curr_event->children().empty()) {
                _curr_event_parent = _curr_event;
                _curr_event = _curr_event->children().end();
                --_curr_event;
            }
	}

        // update the view
        if (_connected) {
            (*_callback_connections)[CALLB_SELECTION_CHANGE].block();
            (*_callback_connections)[CALLB_EXPAND].block();

            Gtk::TreePath curr_path = _event_list_store->get_path(_curr_event);
            _event_list_view->expand_to_path(curr_path);
            _event_list_selection->select(curr_path);
            _event_list_view->scroll_to_row(curr_path);  

            (*_callback_connections)[CALLB_EXPAND].block(false);
            (*_callback_connections)[CALLB_SELECTION_CHANGE].block(false);
        }

    }
}

void
EventLog::notifyRedoEvent(Event* log)
{
    if ( !_notifications_blocked ) {

        // if we're on a parent event...
        if ( !_curr_event->children().empty() ) {

            // ...move to its first child
            _curr_event_parent = _curr_event;
            _curr_event = _curr_event->children().begin();

        } else {
	
            ++_curr_event;

            // if we are about to leave a branch...
            if ( _curr_event->parent() &&
                 _curr_event == _curr_event->parent()->children().end() )
            {

                // ...collapse it
                if (_connected) {
                    (*_callback_connections)[CALLB_SELECTION_CHANGE].block();
                    (*_callback_connections)[CALLB_COLLAPSE].block();
                    _event_list_view->collapse_row(_event_list_store->get_path(_curr_event->parent()));
                    (*_callback_connections)[CALLB_COLLAPSE].block(false);
                    (*_callback_connections)[CALLB_SELECTION_CHANGE].block(false);
                }

                // ...and move to the next event at parent level
                _curr_event = _curr_event->parent();
                _curr_event_parent = (iterator)NULL;

                ++_curr_event;
            }
        }

        // update the view
        if (_connected) {
            Gtk::TreePath curr_path = _event_list_store->get_path(_curr_event);

            (*_callback_connections)[CALLB_SELECTION_CHANGE].block();
            (*_callback_connections)[CALLB_EXPAND].block();

            _event_list_view->expand_to_path(curr_path);
            _event_list_selection->select(curr_path);
            _event_list_view->scroll_to_row(curr_path);  

            (*_callback_connections)[CALLB_EXPAND].block(false);
            (*_callback_connections)[CALLB_SELECTION_CHANGE].block(false);
        }

    }
}

void 
EventLog::notifyUndoCommitEvent(Event* log)
{
    // If we're not at the last event in list then erase the previously undone events 
    if ( _last_event != _curr_event ) {

        _last_event = _curr_event;

        if ( !_last_event->children().empty() ) {
            _last_event = _last_event->children().begin();
        } else {
            ++_last_event;
        }

        while ( _last_event != _event_list_store->children().end() ) {

            if (_last_event->parent()) {
                while ( _last_event != _last_event->parent()->children().end() ) {
                    _last_event = _event_list_store->erase(_last_event);
                }
                _last_event = _last_event->parent();

                (*_last_event)[_columns.child_count] = _last_event->children().size() + 1;

                ++_last_event;
            } else {
                _last_event = _event_list_store->erase(_last_event);
            }

        }
    }

    const unsigned int event_type = log->type;

    Gtk::TreeRow curr_row;

    // if the new event is of the same type as the previous then create a new branch
    if ( event_type == (*_curr_event)[_columns.type] ) {
        if ( !_curr_event_parent ) {
            _curr_event_parent = _curr_event;
        }
        curr_row = *(_event_list_store->append(_curr_event_parent->children()));
        (*_curr_event_parent)[_columns.child_count] = _curr_event_parent->children().size() + 1;
    } else {
        curr_row = *(_event_list_store->append());
        curr_row[_columns.child_count] = 1;

        _curr_event = _last_event = curr_row;

        // collapse if we're leaving a branch
        if (_curr_event_parent && _connected) {
            (*_callback_connections)[CALLB_COLLAPSE].block();
            _event_list_view->collapse_row(_event_list_store->get_path(_curr_event_parent));
            (*_callback_connections)[CALLB_COLLAPSE].block(false);
        }

        _curr_event_parent = (iterator)(NULL);
    }      

    _curr_event = _last_event = curr_row;

    curr_row[_columns.type] = event_type;
    curr_row[_columns.description] = log->description;

    // update the view
    if (_connected) {
        Gtk::TreePath curr_path = _event_list_store->get_path(_curr_event);

        (*_callback_connections)[CALLB_SELECTION_CHANGE].block();
        (*_callback_connections)[CALLB_EXPAND].block();

        _event_list_view->expand_to_path(curr_path);
        _event_list_selection->select(curr_path);
        _event_list_view->scroll_to_row(curr_path);  

        (*_callback_connections)[CALLB_EXPAND].block(false);
        (*_callback_connections)[CALLB_SELECTION_CHANGE].block(false);
    }

}

void 
EventLog::connectWithDialog(Gtk::TreeView *event_list_view, CallbackMap *callback_connections)
{
    _event_list_view = event_list_view;
    _event_list_selection = event_list_view->get_selection();
    _event_list_selection->set_mode(Gtk::SELECTION_SINGLE);

    _callback_connections = callback_connections;

    (*_callback_connections)[CALLB_SELECTION_CHANGE].block();
    (*_callback_connections)[CALLB_EXPAND].block();

    _event_list_view->expand_to_path(_event_list_store->get_path(_curr_event));
    _event_list_selection->select(_curr_event);

    (*_callback_connections)[CALLB_EXPAND].block(false);
    (*_callback_connections)[CALLB_SELECTION_CHANGE].block(false);

    _connected = true;
}

}
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

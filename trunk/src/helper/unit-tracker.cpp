/*
 * Inkscape::UnitTracker - Simple mediator to synchronize changes to a set
 *   of possible units
 *
 * Authors:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2007 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtkliststore.h>

#include "unit-tracker.h"
#include "ege-select-one-action.h"

namespace Inkscape {

enum {
    COLUMN_STRING,
    COLUMN_SPUNIT,
    N_COLUMNS
};

UnitTracker::UnitTracker( guint bases ) :
    _active(0),
    _isUpdating(false),
    _activeUnit(0),
    _store(0),
    _unitList(0),
    _actionList(0),
    _adjList(0),
    _priorValues()
{
    _store = gtk_list_store_new( N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER );
    setBase( bases );
}

UnitTracker::~UnitTracker()
{
    if ( _unitList ) {
        sp_unit_free_list( _unitList );
    }

    // Unhook weak references to GtkActions
    while ( _actionList ) {
        g_signal_handlers_disconnect_by_func( G_OBJECT(_actionList->data), (gpointer)_unitChangedCB, this );
        g_object_weak_unref( G_OBJECT(_actionList->data), _actionFinalizedCB, this );
        _actionList = g_slist_delete_link( _actionList, _actionList );
    }

    // Unhook wek references to GtkAdjustments
    while ( _adjList ) {
        g_object_weak_unref( G_OBJECT(_adjList->data), _adjustmentFinalizedCB, this );
        _adjList = g_slist_delete_link( _adjList, _adjList );
    }
}

void UnitTracker::setBase( guint bases )
{
    GtkTreeIter iter;
    _unitList = sp_unit_get_list( bases );
    for ( GSList* cur = _unitList; cur; cur = g_slist_next(cur) ) {
        SPUnit* unit = static_cast<SPUnit*>(cur->data);
        gtk_list_store_append( _store, &iter );
        gtk_list_store_set( _store, &iter, COLUMN_STRING, unit->abbr, COLUMN_SPUNIT, unit, -1 );
    }
    gint count = gtk_tree_model_iter_n_children( GTK_TREE_MODEL(_store), 0 );
    if ( (count > 0) && (_active > count) ) {
        _setActive( count - 1 );
    } else {
        _setActive( _active );
    }
}

void UnitTracker::addUnit( SPUnitId id, gint index )
{
    GtkTreeIter iter;
    const SPUnit* percentUnit = &sp_unit_get_by_id( id );
    gtk_list_store_insert( _store, &iter, index );
    gtk_list_store_set( _store, &iter, COLUMN_STRING, percentUnit->abbr, COLUMN_SPUNIT, percentUnit, -1 );
}

bool UnitTracker::isUpdating() const
{
    return _isUpdating;
}

SPUnit const* UnitTracker::getActiveUnit() const
{
    return _activeUnit;
}

void UnitTracker::setActiveUnit( SPUnit const *unit )
{
    if ( unit ) {
        GtkTreeIter iter;
        int index = 0;
        gboolean found = gtk_tree_model_get_iter_first( GTK_TREE_MODEL(_store), &iter );
        while ( found ) {
            SPUnit* storedUnit = 0;
            gtk_tree_model_get( GTK_TREE_MODEL(_store), &iter, COLUMN_SPUNIT, &storedUnit, -1 );
            if ( storedUnit && (storedUnit->unit_id == unit->unit_id) ) {
                _setActive(index);
                break;
            }

            found = gtk_tree_model_iter_next( GTK_TREE_MODEL(_store), &iter );
            index++;
        }
    }
}

void UnitTracker::addAdjustment( GtkAdjustment* adj )
{
    if ( !g_slist_find( _adjList, adj ) ) {
        g_object_weak_ref( G_OBJECT(adj), _adjustmentFinalizedCB, this );
        _adjList = g_slist_append( _adjList, adj );
    }
}

void UnitTracker::setFullVal( GtkAdjustment* adj, gdouble val )
{
    _priorValues[adj] = val;
}

GtkAction* UnitTracker::createAction( gchar const* name, gchar const* label, gchar const* tooltip )
{
    EgeSelectOneAction* act1 = ege_select_one_action_new( name, label, tooltip, NULL, GTK_TREE_MODEL(_store) );
    ege_select_one_action_set_label_column( act1, COLUMN_STRING );
    if ( _active ) {
        ege_select_one_action_set_active( act1, _active );
    }

    ege_select_one_action_set_appearance( act1, "minimal" );
    g_object_weak_ref( G_OBJECT(act1), _actionFinalizedCB, this );
    g_signal_connect( G_OBJECT(act1), "changed", G_CALLBACK( _unitChangedCB ), this );
    _actionList = g_slist_append( _actionList, act1 );

    return GTK_ACTION(act1);
}

void UnitTracker::_unitChangedCB( GtkAction* action, gpointer data )
{
    if ( action && data ) {
        EgeSelectOneAction* act = EGE_SELECT_ONE_ACTION(action);
        gint active = ege_select_one_action_get_active( act );
        UnitTracker* self = reinterpret_cast<UnitTracker*>(data);
        self->_setActive(active);
    }
}

void UnitTracker::_actionFinalizedCB( gpointer data, GObject *where_the_object_was )
{
    if ( data && where_the_object_was ) {
        UnitTracker* self = reinterpret_cast<UnitTracker*>(data);
        self->_actionFinalized( where_the_object_was );
    }
}

void UnitTracker::_adjustmentFinalizedCB( gpointer data, GObject *where_the_object_was )
{
    if ( data && where_the_object_was ) {
        UnitTracker* self = reinterpret_cast<UnitTracker*>(data);
        self->_adjustmentFinalized( where_the_object_was );
    }
}

void UnitTracker::_actionFinalized( GObject *where_the_object_was )
{
    GSList* target = g_slist_find( _actionList, where_the_object_was );
    if ( target ) {
        _actionList = g_slist_remove( _actionList, where_the_object_was );
    } else {
        g_warning("Received a finalization callback for unknown object %p", where_the_object_was );
    }
}

void UnitTracker::_adjustmentFinalized( GObject *where_the_object_was )
{
    GSList* target = g_slist_find( _adjList, where_the_object_was );
    if ( target ) {
        _adjList = g_slist_remove( _adjList, where_the_object_was );
    } else {
        g_warning("Received a finalization callback for unknown object %p", where_the_object_was );
    }
}

void UnitTracker::_setActive( gint active )
{
    if ( active != _active || (_activeUnit == 0) ) {
        gint oldActive = _active;

        GtkTreeIter iter;
        gboolean found = gtk_tree_model_iter_nth_child( GTK_TREE_MODEL(_store), &iter, NULL, oldActive );
        if ( found ) {
            SPUnit* unit = 0;
            gtk_tree_model_get( GTK_TREE_MODEL(_store), &iter, COLUMN_SPUNIT, &unit, -1 );

            found = gtk_tree_model_iter_nth_child( GTK_TREE_MODEL(_store), &iter, NULL, active );
            if ( found ) {
                SPUnit* newUnit = 0;
                gtk_tree_model_get( GTK_TREE_MODEL(_store), &iter, COLUMN_SPUNIT, &newUnit, -1 );
                _activeUnit = newUnit;

                if ( _adjList ) {
                    _fixupAdjustments( unit, newUnit );
                }

            } else {
                g_warning("Did not find new unit");
            }
        } else {
            g_warning("Did not find old unit");
        }

        _active = active;

        for ( GSList* cur = _actionList; cur; cur = g_slist_next(cur) ) {
            if ( IS_EGE_SELECT_ONE_ACTION( cur->data ) ) {
                EgeSelectOneAction* act = EGE_SELECT_ONE_ACTION( cur->data );
                ege_select_one_action_set_active( act, active );
            }
        }
    }
}

void UnitTracker::_fixupAdjustments( SPUnit const* oldUnit, SPUnit const *newUnit )
{
    _isUpdating = true;
    for ( GSList* cur = _adjList; cur; cur = g_slist_next(cur) ) {
        GtkAdjustment* adj = GTK_ADJUSTMENT(cur->data);
        gdouble oldVal = gtk_adjustment_get_value(adj);
        gdouble val = oldVal;

        if ((oldUnit->base == SP_UNIT_ABSOLUTE || oldUnit->base == SP_UNIT_DEVICE)
            && (newUnit->base == SP_UNIT_DIMENSIONLESS))
        {
            val = 1.0 / newUnit->unittobase;
            _priorValues[adj] = sp_units_get_pixels( oldVal, *oldUnit );
        } else if ((oldUnit->base == SP_UNIT_DIMENSIONLESS)
                   && (newUnit->base == SP_UNIT_ABSOLUTE || newUnit->base == SP_UNIT_DEVICE)) {
            if ( _priorValues.find(adj) != _priorValues.end() ) {
                val = sp_pixels_get_units( _priorValues[adj], *newUnit );
            }
        } else {
            val = sp_convert_distance_full( oldVal, *oldUnit, *newUnit );
        }

        gtk_adjustment_set_value( adj, val );
    }
    _isUpdating = false;
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

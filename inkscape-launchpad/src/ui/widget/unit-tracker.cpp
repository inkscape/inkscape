/*
 * Inkscape::UI::Widget::UnitTracker
 * Simple mediator to synchronize changes to unit menus
 *
 * Authors:
 *   Jon A. Cruz <jon@joncruz.org>
 *   Matthew Petroff <matthew@mpetroff.net>
 *
 * Copyright (C) 2007 Jon A. Cruz
 * Copyright (C) 2013 Matthew Petroff
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "style-internal.h"
#include "unit-tracker.h"
#include "widgets/ege-select-one-action.h"

#define COLUMN_STRING 0

using Inkscape::Util::UnitTable;
using Inkscape::Util::unit_table;

namespace Inkscape {
namespace UI {
namespace Widget {

UnitTracker::UnitTracker(UnitType unit_type) :
    _active(0),
    _isUpdating(false),
    _activeUnit(NULL),
    _activeUnitInitialized(false),
    _store(0),
    _unitList(0),
    _actionList(0),
    _adjList(0),
    _priorValues()
{
    _store = gtk_list_store_new(1, G_TYPE_STRING);
    
    GtkTreeIter iter;
    UnitTable::UnitMap m = unit_table.units(unit_type);
    

    for (UnitTable::UnitMap::iterator m_iter = m.begin(); m_iter != m.end(); ++m_iter) {
        Glib::ustring text = m_iter->first;
        gtk_list_store_append(_store, &iter);
        gtk_list_store_set(_store, &iter, COLUMN_STRING, text.c_str(), -1);
    }
    gint count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(_store), 0);
    if ((count > 0) && (_active > count)) {
        _setActive(--count);
    } else {
        _setActive(_active);
    }
}

UnitTracker::~UnitTracker()
{
    // Unhook weak references to GtkActions
    while (_actionList) {
        g_signal_handlers_disconnect_by_func(G_OBJECT(_actionList->data), (gpointer) _unitChangedCB, this);
        g_object_weak_unref(G_OBJECT(_actionList->data), _actionFinalizedCB, this);
        _actionList = g_slist_delete_link(_actionList, _actionList);
    }

    // Unhook weak references to GtkAdjustments
    while (_adjList) {
        g_object_weak_unref(G_OBJECT(_adjList->data), _adjustmentFinalizedCB, this);
        _adjList = g_slist_delete_link(_adjList, _adjList);
    }
}

bool UnitTracker::isUpdating() const
{
    return _isUpdating;
}

Inkscape::Util::Unit const * UnitTracker::getActiveUnit() const
{
    return _activeUnit;
}

void UnitTracker::setActiveUnit(Inkscape::Util::Unit const *unit)
{
    if (unit) {
        GtkTreeIter iter;
        int index = 0;
        gboolean found = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(_store), &iter);
        while (found) {
            gchar *storedUnit = 0;
            gtk_tree_model_get(GTK_TREE_MODEL(_store), &iter, COLUMN_STRING, &storedUnit, -1);
            if (storedUnit && (!unit->abbr.compare(storedUnit))) {
                _setActive(index);
                break;
            }
            
            found = gtk_tree_model_iter_next(GTK_TREE_MODEL(_store), &iter);
            index++;
        }
    }
}

void UnitTracker::setActiveUnitByAbbr(gchar const *abbr)
{
    Inkscape::Util::Unit const *u = unit_table.getUnit(abbr);
    setActiveUnit(u);
}

void UnitTracker::addAdjustment(GtkAdjustment *adj)
{
    if (!g_slist_find(_adjList, adj)) {
        g_object_weak_ref(G_OBJECT(adj), _adjustmentFinalizedCB, this);
        _adjList = g_slist_append(_adjList, adj);
    }
}

void UnitTracker::addUnit(Inkscape::Util::Unit const *u)
{
    GtkTreeIter iter;
    gtk_list_store_append(_store, &iter);
    gtk_list_store_set(_store, &iter, COLUMN_STRING, u ? u->abbr.c_str() : "NULL", -1);
}

void UnitTracker::prependUnit(Inkscape::Util::Unit const *u)
{
    GtkTreeIter iter;
    gtk_list_store_prepend(_store, &iter);
    gtk_list_store_set(_store, &iter, COLUMN_STRING, u ? u->abbr.c_str() : "NULL", -1);
    /* Re-shuffle our default selection here (_active gets out of sync) */
    setActiveUnit(_activeUnit);
}

void UnitTracker::setFullVal(GtkAdjustment *adj, gdouble val)
{
    _priorValues[adj] = val;
}

GtkAction *UnitTracker::createAction(gchar const *name, gchar const *label, gchar const *tooltip)
{
    EgeSelectOneAction *act1 = ege_select_one_action_new(name, label, tooltip, NULL, GTK_TREE_MODEL(_store));
    ege_select_one_action_set_label_column(act1, COLUMN_STRING);
    if (_active) {
        ege_select_one_action_set_active(act1, _active);
    }

    ege_select_one_action_set_appearance(act1, "minimal");
    g_object_weak_ref(G_OBJECT(act1), _actionFinalizedCB, this);
    g_signal_connect(G_OBJECT(act1), "changed", G_CALLBACK(_unitChangedCB), this);
    _actionList = g_slist_append(_actionList, act1);

    return GTK_ACTION(act1);
}

void UnitTracker::_unitChangedCB(GtkAction *action, gpointer data)
{
    if (action && data) {
        EgeSelectOneAction *act = EGE_SELECT_ONE_ACTION(action);
        gint active = ege_select_one_action_get_active(act);
        UnitTracker *self = reinterpret_cast<UnitTracker *>(data);
        self->_setActive(active);
    }
}

void UnitTracker::_actionFinalizedCB(gpointer data, GObject *where_the_object_was)
{
    if (data && where_the_object_was) {
        UnitTracker *self = reinterpret_cast<UnitTracker *>(data);
        self->_actionFinalized(where_the_object_was);
    }
}

void UnitTracker::_adjustmentFinalizedCB(gpointer data, GObject *where_the_object_was)
{
    if (data && where_the_object_was) {
        UnitTracker *self = reinterpret_cast<UnitTracker *>(data);
        self->_adjustmentFinalized(where_the_object_was);
    }
}

void UnitTracker::_actionFinalized(GObject *where_the_object_was)
{
    GSList *target = g_slist_find(_actionList, where_the_object_was);
    if (target) {
        _actionList = g_slist_remove(_actionList, where_the_object_was);
    } else {
        g_warning("Received a finalization callback for unknown object %p", where_the_object_was);
    }
}

void UnitTracker::_adjustmentFinalized(GObject *where_the_object_was)
{
    GSList *target = g_slist_find(_adjList, where_the_object_was);
    if (target) {
        _adjList = g_slist_remove(_adjList, where_the_object_was);
    } else {
        g_warning("Received a finalization callback for unknown object %p", where_the_object_was);
    }
}

void UnitTracker::_setActive(gint active)
{
    if ( active != _active || !_activeUnitInitialized ) {
        gint oldActive = _active;

        GtkTreeIter iter;
        gboolean found = gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(_store), &iter, NULL, oldActive);
        if (found) {
            gchar *abbr;
            gtk_tree_model_get(GTK_TREE_MODEL(_store), &iter, COLUMN_STRING, &abbr, -1);
            Inkscape::Util::Unit const *unit = unit_table.getUnit(abbr);

            found = gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(_store), &iter, NULL, active);
            if (found) {
                gchar *newAbbr;
                gtk_tree_model_get(GTK_TREE_MODEL(_store), &iter, COLUMN_STRING, &newAbbr, -1);
                Inkscape::Util::Unit const *newUnit = unit_table.getUnit(newAbbr);
                _activeUnit = newUnit;

                if (_adjList) {
                    _fixupAdjustments(unit, newUnit);
                }

            } else {
                g_warning("Did not find new unit");
            }
        } else {
            g_warning("Did not find old unit");
        }

        _active = active;

        for ( GSList *cur = _actionList ; cur ; cur = g_slist_next(cur) ) {
            if (IS_EGE_SELECT_ONE_ACTION(cur->data)) {
                EgeSelectOneAction *act = EGE_SELECT_ONE_ACTION(cur->data);
                ege_select_one_action_set_active(act, active);
            }
        }
        
        _activeUnitInitialized = true;
    }
}

void UnitTracker::_fixupAdjustments(Inkscape::Util::Unit const *oldUnit, Inkscape::Util::Unit const *newUnit)
{
    _isUpdating = true;
    for ( GSList *cur = _adjList ; cur ; cur = g_slist_next(cur) ) {
        GtkAdjustment *adj = GTK_ADJUSTMENT(cur->data);
        gdouble oldVal = gtk_adjustment_get_value(adj);
        gdouble val = oldVal;

        if ( (oldUnit->type != Inkscape::Util::UNIT_TYPE_DIMENSIONLESS)
            && (newUnit->type == Inkscape::Util::UNIT_TYPE_DIMENSIONLESS) )
        {
            val = newUnit->factor * 100;
            _priorValues[adj] = Inkscape::Util::Quantity::convert(oldVal, oldUnit, "px");
        } else if ( (oldUnit->type == Inkscape::Util::UNIT_TYPE_DIMENSIONLESS)
            && (newUnit->type != Inkscape::Util::UNIT_TYPE_DIMENSIONLESS) )
        {
            if (_priorValues.find(adj) != _priorValues.end()) {
                val = Inkscape::Util::Quantity::convert(_priorValues[adj], "px", newUnit);
            }
        } else {
            val = Inkscape::Util::Quantity::convert(oldVal, oldUnit, newUnit);
        }

        gtk_adjustment_set_value(adj, val);
    }
    _isUpdating = false;
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape

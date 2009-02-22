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

#ifndef SEEN_INKSCAPE_UNIT_TRACKER_H
#define SEEN_INKSCAPE_UNIT_TRACKER_H

#include <map>

#include <gtk/gtkaction.h>
#include <gtk/gtkliststore.h>

#include "helper/units.h"

namespace Inkscape {

class UnitTracker
{
public:
    UnitTracker( guint bases = (SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE) );
    virtual ~UnitTracker();

    void setBase( guint bases ); // SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE
    void addUnit( SPUnitId id, gint index );

    bool isUpdating() const;

    void setActiveUnit( SPUnit const *unit );
    SPUnit const* getActiveUnit() const;

    void addAdjustment( GtkAdjustment* adj );
    void setFullVal( GtkAdjustment* adj, gdouble val );

    GtkAction* createAction( gchar const* name, gchar const* label, gchar const* tooltip );

private:
    static void _unitChangedCB( GtkAction* action, gpointer data );
    static void _actionFinalizedCB( gpointer data, GObject *where_the_object_was );
    static void _adjustmentFinalizedCB( gpointer data, GObject *where_the_object_was );
    void _setActive( gint index );
    void _fixupAdjustments( SPUnit const* oldUnit, SPUnit const *newUnit );
    void _actionFinalized( GObject *where_the_object_was );
    void _adjustmentFinalized( GObject *where_the_object_was );

    gint _active;
    bool _isUpdating;
    SPUnit* _activeUnit;
    GtkListStore* _store;
    GSList* _unitList;
    GSList* _actionList;
    GSList* _adjList;
    std::map <GtkAdjustment*, gdouble> _priorValues;
};

}

#endif // SEEN_INKSCAPE_UNIT_TRACKER_H

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

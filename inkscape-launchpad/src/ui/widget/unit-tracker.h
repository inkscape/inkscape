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

#ifndef INKSCAPE_UI_WIDGET_UNIT_TRACKER_H
#define INKSCAPE_UI_WIDGET_UNIT_TRACKER_H

#include <map>
#include "util/units.h"

using Inkscape::Util::Unit;
using Inkscape::Util::UnitType;

typedef struct _GObject       GObject;
typedef struct _GtkAction     GtkAction;
typedef struct _GtkAdjustment GtkAdjustment;
typedef struct _GtkListStore  GtkListStore;

namespace Inkscape {
namespace UI {
namespace Widget {

class UnitTracker {
public:
    UnitTracker(UnitType unit_type);
    virtual ~UnitTracker();

    bool isUpdating() const;

    void setActiveUnit(Inkscape::Util::Unit const *unit);
    void setActiveUnitByAbbr(gchar const *abbr);
    Inkscape::Util::Unit const * getActiveUnit() const;

    void addUnit(Inkscape::Util::Unit const *u);
    void addAdjustment(GtkAdjustment *adj);
    void prependUnit(Inkscape::Util::Unit const *u);
    void setFullVal(GtkAdjustment *adj, gdouble val);

    GtkAction *createAction(gchar const *name, gchar const *label, gchar const *tooltip);

protected:
    UnitType _type;

private:
    static void _unitChangedCB(GtkAction *action, gpointer data);
    static void _actionFinalizedCB(gpointer data, GObject *where_the_object_was);
    static void _adjustmentFinalizedCB(gpointer data, GObject *where_the_object_was);
    void _setActive(gint index);
    void _fixupAdjustments(Inkscape::Util::Unit const *oldUnit, Inkscape::Util::Unit const *newUnit);
    void _actionFinalized(GObject *where_the_object_was);
    void _adjustmentFinalized(GObject *where_the_object_was);

    gint _active;
    bool _isUpdating;
    Inkscape::Util::Unit const *_activeUnit;
    bool _activeUnitInitialized;
    GtkListStore *_store;
    GSList *_unitList;
    GSList *_actionList;
    GSList *_adjList;
    std::map <GtkAdjustment *, gdouble> _priorValues;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_UNIT_TRACKER_H

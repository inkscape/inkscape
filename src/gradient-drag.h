#ifndef SEEN_GRADIENT_DRAG_H
#define SEEN_GRADIENT_DRAG_H

/*
 * On-canvas gradient dragging
 *
 * Authors:
 *   bulia byak <bulia@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 2012 Authors
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <stddef.h>
#include <sigc++/sigc++.h>
#include <vector>
#include <glib.h>
#include <glibmm/ustring.h>

#include <2geom/point.h>

#include "sp-gradient.h" // TODO refactor enums to external .h file
#include "sp-mesh-array.h"

class SPKnot;

class SPDesktop;
class SPCSSAttr;
class SPLinearGradient;
class SPMeshGradient;
class SPItem;
class SPObject;
class SPRadialGradient;
class SPStop;

namespace Inkscape {
class Selection;
} // namespace Inkscape

/**
This class represents a single draggable point of a gradient. It remembers the item
which has the gradient, whether it's fill or stroke, the point type (from the
GrPointType enum), and the point number (needed if more than 2 stops are present).
*/
struct GrDraggable {
    GrDraggable(SPItem *item, GrPointType point_type, guint point_i, Inkscape::PaintTarget fill_or_stroke);
    virtual ~GrDraggable();

    SPItem *item;
    GrPointType point_type;
    gint point_i;  // the stop number of this point ( = 0 POINT_LG_BEGIN and POINT_RG_CENTER)
    Inkscape::PaintTarget fill_or_stroke;

    SPObject *getServer();

    bool mayMerge(GrDraggable *da2);

    inline int equals(GrDraggable *other) {
        return ((item == other->item) && (point_type == other->point_type) && (point_i == other->point_i) && (fill_or_stroke == other->fill_or_stroke));
    }
};

class GrDrag;

/**
This class holds together a visible on-canvas knot and a list of draggables that need to
be moved when the knot moves. Normally there's one draggable in the list, but there may
be more when draggers are snapped together.
*/
struct GrDragger {
    GrDragger(GrDrag *parent, Geom::Point p, GrDraggable *draggable);
    virtual ~GrDragger();

    GrDrag *parent;

    SPKnot *knot;

    // position of the knot, desktop coords
    Geom::Point point;
    // position of the knot before it began to drag; updated when released
    Geom::Point point_original;

    GSList *draggables;

    void addDraggable(GrDraggable *draggable);

    void updateKnotShape();
    void updateTip();

    void select();
    void deselect();
    bool isSelected();

    /* Given one GrDraggable, these all update other draggables belonging to same GrDragger */
    void moveThisToDraggable(SPItem *item, GrPointType point_type, gint point_i, Inkscape::PaintTarget fill_or_stroke, bool write_repr);
    void moveOtherToDraggable(SPItem *item, GrPointType point_type, gint point_i, Inkscape::PaintTarget fill_or_stroke, bool write_repr);
    void updateMidstopDependencies(GrDraggable *draggable, bool write_repr);
    void updateDependencies(bool write_repr);

    /* Update handles/tensors when mesh corner moved */
    void updateHandles( Geom::Point pc_old,  MeshNodeOperation op );

    bool mayMerge(GrDragger *other);
    bool mayMerge(GrDraggable *da2);

    bool isA(GrPointType point_type);
    bool isA(SPItem *item, GrPointType point_type, Inkscape::PaintTarget fill_or_stroke);
    bool isA(SPItem *item, GrPointType point_type, gint point_i, Inkscape::PaintTarget fill_or_stroke);

    void fireDraggables(bool write_repr, bool scale_radial = false, bool merging_focus = false);

private:
    sigc::connection _moved_connection;
    sigc::connection _clicked_connection;
    sigc::connection _doubleclicked_connection;
    sigc::connection _grabbed_connection;
    sigc::connection _ungrabbed_connection;
};

/**
This is the root class of the gradient dragging machinery. It holds lists of GrDraggers
and of lines (simple canvas items). It also remembers one of the draggers as selected.
*/
class GrDrag {
public: // FIXME: make more of this private!

    GrDrag(SPDesktop *desktop);
    virtual ~GrDrag();

    bool isNonEmpty() {return (draggers != NULL);}
    bool hasSelection() {return (selected != NULL);}
    guint numSelected() {return (selected? g_list_length(selected) : 0);}
    guint numDraggers() {return (draggers? g_list_length(draggers) : 0);}

    guint singleSelectedDraggerNumDraggables() {
        return (selected? g_slist_length(( static_cast<GrDragger *>(selected->data))->draggables) : 0);
    }

    guint singleSelectedDraggerSingleDraggableType() {
        return (selected? (static_cast<GrDraggable*>((static_cast<GrDragger*>(selected->data))->draggables->data))->point_type : 0);}

    // especially the selection must be private, fix gradient-context to remove direct access to it
    GList *selected; // list of GrDragger*
    void setSelected(GrDragger *dragger, bool add_to_selection = false, bool override = true);
    void setDeselected(GrDragger *dragger);
    void deselectAll();
    void selectAll();
    void selectByCoords(std::vector<Geom::Point> coords);
    void selectByStop(SPStop *stop,  bool add_to_selection = true, bool override = true);
    void selectRect(Geom::Rect const &r);

    bool dropColor(SPItem *item, gchar const *c, Geom::Point p);

    SPStop *addStopNearPoint(SPItem *item, Geom::Point mouse_p, double tolerance);

    void deleteSelected(bool just_one = false);

    guint32 getColor();

    bool keep_selection;

    GrDragger *getDraggerFor(SPItem *item, GrPointType point_type, gint point_i, Inkscape::PaintTarget fill_or_stroke);

    void grabKnot(GrDragger *dragger, gint x, gint y, guint32 etime);
    void grabKnot(SPItem *item, GrPointType point_type, gint point_i, Inkscape::PaintTarget fill_or_stroke, gint x, gint y, guint32 etime);

    bool local_change;

    SPDesktop *desktop;

    // lists of edges of selection bboxes, to snap draggers to
    std::vector<double> hor_levels;
    std::vector<double> vert_levels;

    GList *draggers;
    GSList *lines;

    void updateDraggers();
    void updateLines();
    void updateLevels();

    bool mouseOver();

    void selected_move_nowrite(double x, double y, bool scale_radial);
    void selected_move(double x, double y, bool write_repr = true, bool scale_radial = false);
    void selected_move_screen(double x, double y);

    GrDragger *select_next();
    GrDragger *select_prev();

    void selected_reverse_vector();

private:
    void deselect_all();

    void addLine( SPItem *item, Geom::Point p1, Geom::Point p2, Inkscape::PaintTarget fill_or_stroke);
    void addCurve(SPItem *item, Geom::Point p0, Geom::Point p1, Geom::Point p2, Geom::Point p3, Inkscape::PaintTarget fill_or_stroke);

    void addDragger(GrDraggable *draggable);

    void addDraggersRadial(SPRadialGradient *rg, SPItem *item, Inkscape::PaintTarget fill_or_stroke);
    void addDraggersLinear(SPLinearGradient *lg, SPItem *item, Inkscape::PaintTarget fill_or_stroke);
    void addDraggersMesh(  SPMeshGradient   *mg, SPItem *item, Inkscape::PaintTarget fill_or_stroke);

    bool styleSet( const SPCSSAttr *css );

    Glib::ustring makeStopSafeColor( gchar const *str, bool &isNull );

    Inkscape::Selection *selection;
    sigc::connection sel_changed_connection;
    sigc::connection sel_modified_connection;

    sigc::connection style_set_connection;
    sigc::connection style_query_connection;
};

#endif // SEEN_GRADIENT_DRAG_H
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

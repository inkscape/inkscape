#ifndef __GRADIENT_DRAG_H__
#define __GRADIENT_DRAG_H__

/*
 * On-canvas gradient dragging 
 *
 * Authors:
 *   bulia byak <bulia@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gslist.h>
#include <sigc++/sigc++.h>
#include <vector>

#include <forward.h>
#include <libnr/nr-forward.h>
#include <2geom/point.h>
#include <knot-enums.h>

struct SPItem;
struct SPKnot;

/**
This class represents a single draggable point of a gradient. It remembers the item
which has the gradient, whether it's fill or stroke, the point type (from the
GrPointType enum), and the point number (needed if more than 2 stops are present).
*/
struct GrDraggable {
    GrDraggable(SPItem *item, guint point_type, guint point_i, bool fill_or_stroke);
    virtual ~GrDraggable();

    SPItem *item;
    gint point_type;
    gint point_i;  // the stop number of this point ( = 0 POINT_LG_BEGIN and POINT_RG_CENTER)
    bool fill_or_stroke;

    SPObject *getServer();

    bool mayMerge (GrDraggable *da2);

    inline int equals (GrDraggable *other) {
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
    GrDragger (GrDrag *parent, Geom::Point p, GrDraggable *draggable);
    virtual ~GrDragger();

    GrDrag *parent;

    SPKnot *knot;

    // position of the knot, desktop coords
    Geom::Point point;
    // position of the knot before it began to drag; updated when released
    Geom::Point point_original;

    /** Connection to \a knot's "moved" signal, for blocking it (unused?). */
    guint handler_id;

    GSList *draggables;

    void addDraggable(GrDraggable *draggable);

    void updateKnotShape();
    void updateTip();
    
    void select();
    void deselect();
    bool isSelected();

    void moveThisToDraggable (SPItem *item, gint point_type, gint point_i, bool fill_or_stroke, bool write_repr);
    void moveOtherToDraggable (SPItem *item, gint point_type, gint point_i, bool fill_or_stroke, bool write_repr);
    void updateMidstopDependencies (GrDraggable *draggable, bool write_repr);
    void updateDependencies (bool write_repr);

    bool mayMerge (GrDragger *other);
    bool mayMerge (GrDraggable *da2);

    bool isA (gint point_type);
    bool isA (SPItem *item, gint point_type, bool fill_or_stroke);
    bool isA (SPItem *item, gint point_type, gint point_i, bool fill_or_stroke);

    void fireDraggables (bool write_repr, bool scale_radial = false, bool merging_focus = false);
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
    guint singleSelectedDraggerNumDraggables() {return (selected? g_slist_length(((GrDragger *) selected->data)->draggables) : 0);}
    guint singleSelectedDraggerSingleDraggableType() {return (selected? ((GrDraggable *) ((GrDragger *) selected->data)->draggables->data)->point_type : 0);}

    // especially the selection must be private, fix gradient-context to remove direct access to it
    GList *selected; // list of GrDragger*
    void setSelected (GrDragger *dragger, bool add_to_selection = false, bool override = true);
    void setDeselected (GrDragger *dragger);
    void deselectAll();
    void selectAll();
    void selectByCoords(std::vector<Geom::Point> coords);
    void selectRect(Geom::Rect const &r);

    bool dropColor(SPItem *item, gchar const *c, Geom::Point p);

    SPStop *addStopNearPoint (SPItem *item, Geom::Point mouse_p, double tolerance);

    void deleteSelected (bool just_one = false);

    guint32 getColor();
    
    bool keep_selection;    
    
    GrDragger *getDraggerFor (SPItem *item, gint point_type, gint point_i, bool fill_or_stroke);

    void grabKnot (GrDragger *dragger, gint x, gint y, guint32 etime);
    void grabKnot (SPItem *item, gint point_type, gint point_i, bool fill_or_stroke, gint x, gint y, guint32 etime);

    bool local_change;

    SPDesktop *desktop;

    // lists of edges of selection bboxes, to snap draggers to
    std::vector<double> hor_levels;
    std::vector<double> vert_levels;

    GList *draggers;
    GSList *lines;

    void updateDraggers ();
    void updateLines ();
    void updateLevels ();

    void selected_move_nowrite (double x, double y, bool scale_radial);
    void selected_move (double x, double y, bool write_repr = true, bool scale_radial = false);
    void selected_move_screen (double x, double y);

    GrDragger *select_next ();
    GrDragger *select_prev ();

    void selected_reverse_vector ();

private: 
    void deselect_all();

    void addLine (SPItem *item, Geom::Point p1, Geom::Point p2, guint32 rgba);

    void addDragger (GrDraggable *draggable);

    void addDraggersRadial (SPRadialGradient *rg, SPItem *item, bool fill_or_stroke);
    void addDraggersLinear (SPLinearGradient *lg, SPItem *item, bool fill_or_stroke);

    Inkscape::Selection *selection;
    sigc::connection sel_changed_connection;
    sigc::connection sel_modified_connection;

    sigc::connection style_set_connection;
    sigc::connection style_query_connection;
};

#endif

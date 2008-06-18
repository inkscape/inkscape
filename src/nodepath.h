#ifndef __SP_NODEPATH_H__
#define __SP_NODEPATH_H__

/** \file
 * Path handler in node edit mode
 */

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

//#include "knot.h"
//#include "sp-path.h"
//#include "desktop-handles.h"
#include "libnr/nr-path-code.h"
#include "livarot/Path.h"
#include <glibmm/ustring.h>
#include <gdk/gdkevents.h>
#include <list>

class SPObject;
class SPDesktop;
class SPPath;
class SPKnot;
class LivePathEffectObject;

namespace Inkscape {
namespace XML {
class Node;
}
}


/**
 * Radial objects are represented by an angle and a distance from
 * 0,0.  0,0 is represented by a == big_num.
 */
class Radial{
 public:
/**  Radius */
	double r;
/**  Amplitude */
	double a;
	Radial() {}
	//	Radial(NR::Point const &p); // Convert a point to radial coordinates
	Radial(Radial &p) : r(p.r),a(p.a) {}
	//	operator NR::Point() const;

/**
 * Construct Radial from NR::Point.
 */
Radial(NR::Point const &p)
{
	r = NR::L2(p);
	if (r > 0) {
		a = NR::atan2 (p);
	} else {
		a = HUGE_VAL; //undefined
	}
}

/**
 * Cast Radial to cartesian NR::Point.
 */
operator NR::Point() const
{
	if (a == HUGE_VAL) {
		return NR::Point(0,0);
	} else {
		return r*NR::Point(cos(a), sin(a));
	}
}

};

class ShapeEditor;

namespace Inkscape {
namespace NodePath {

/**
 * This is a node on a subpath
 */
class Path;

/**
 * This is a subdivision of a NodePath
 */
class SubPath;

class NodeSide;

/**
 * This is a node (point) along a subpath
 */
class Node;


/**
 *  This is the lowest list item, a simple list of nodes.
 */
class SubPath {
 public:
/**  The parent of this subpath */
	Path * nodepath;
/**  Is this path closed (no endpoints) or not?*/
	gboolean closed;
/**  The nodes in this subpath. */
	GList * nodes;
/**  The first node of the subpath (does not imply open/closed)*/
	Node * first;
/**  The last node of the subpath */
	Node * last;
};



/**
 *  What kind of node is this?  This is the value for the node->type
 *  field.  NodeType indicates the degree of continuity required for
 *  the node.  I think that the corresponding integer indicates which
 *  derivate is connected. (Thus 2 means that the node is continuous
 *  to the second derivative, i.e. has matching endpoints and tangents)
 */
typedef enum {
/**  A normal node */
	NODE_NONE,
/**  This node non-continuously joins two segments.*/
	NODE_CUSP,
/**  This node continuously joins two segments. */
	NODE_SMOOTH,
/**  This node is symmetric. */
	NODE_SYMM
} NodeType;



/**
 * A NodeSide is a datarecord which may be on either side (n or p) of a node,
 * which describes the segment going to the next node.
 */
class NodeSide{
 public:
/**  Pointer to the next node, */
	Node * other;
/**  Position */
	NR::Point pos;
/**  Origin (while dragging) in radial notation */
	Radial origin_radial;
/**  Origin (while dragging) in x/y notation */
	NR::Point origin;
/**  Knots are Inkscape's way of providing draggable points.  This
 *  Knot is the point on the curve representing the control point in a
 *  bezier curve.*/
	SPKnot * knot;
/**  What kind of rendering? */
	SPCanvasItem * line;
};

/**
 * A node along a NodePath
 */
class Node {
 public:
/**  The parent subpath of this node */
	SubPath * subpath;
/**  Type is selected from NodeType.*/
	guint type : 4;
/**  Code refers to which ArtCode is used to represent the segment
 *  (which segment?).*/
	guint code : 4;
/**  Boolean.  Am I currently selected or not? */
	guint selected : 1;
/**  */
	NR::Point pos;
/**  */
	NR::Point origin;
/**  Knots are Inkscape's way of providing draggable points.  This
 *  Knot is the point on the curve representing the endpoint.*/
	SPKnot * knot;
/**  The NodeSide in the 'next' direction */
	NodeSide n;
/**  The NodeSide in the 'previous' direction */
	NodeSide p;

	/** The pointer to the nodeside which we are dragging out with Shift */
	NodeSide *dragging_out;
  
  /** Boolean.  Am I being dragged? */
  guint is_dragging : 1;
};

/**
 *  This is a collection of subpaths which contain nodes
 *
 * In the following data model.   Nodepaths are made up of subpaths which
 * are comprised of nodes.
 *
 * Nodes are linked thus:
 * \verbatim
           n              other
    node -----> nodeside ------> node            \endverbatim
 */
class Path {
 public:
/**  Pointer to the current desktop, for reporting purposes */
	SPDesktop * desktop;
/**  The parent path of this nodepath */
	SPObject * object;
/**  The parent livepatheffect of this nodepath, if applicable */
    SPItem * item;
/**  The context which created this nodepath.  Important if this nodepath is deleted */
	ShapeEditor *shape_editor;
/**  The subpaths which comprise this NodePath */
	GList * subpaths;
/**  A list of nodes which are currently selected */
	GList * selected;
/**  Transforms (userspace <---> virtual space?   someone please describe )
	 njh: I'd be guessing that these are item <-> desktop transforms.*/
	NR::Matrix i2d, d2i;
/**  The DOM node which describes this NodePath */
    Inkscape::XML::Node *repr;
    gchar *repr_key;
    gchar *repr_nodetypes_key;
	//STL compliant method to get the selected nodes
	void selection(std::list<Node *> &l);

	guint numSelected() {return (selected? g_list_length(selected) : 0);}
	NR::Point& singleSelectedCoords() {return (((Node *) selected->data)->pos);}

      /// livarot library is used for "point on path" and "nearest position on path", so we need to maintain its path representation as well
	::Path *livarot_path;
    
    /// draw a "sketch" of the path by using these variables
    SPCanvasItem *helper_path;
    SPCurve *curve;
    bool show_helperpath;
    guint32 helperpath_rgba;
    gdouble helperpath_width;

      /// true if we changed repr, to tell this change from an external one such as from undo, simplify, or another desktop
	unsigned int local_change;

	/// true if we're showing selected nodes' handles
	bool show_handles;

    /// true if the path cannot contain curves, just straight lines
    bool straight_path;

	/// active_node points to the node that is currently mouseovered (= NULL if
	/// there isn't any); we also consider the node mouseovered if it is covered
	/// by one of its handles and the latter is mouseovered
	static Node *active_node;
};

}  // namespace NodePath
}  // namespace Inkscape

enum {
  SCULPT_PROFILE_LINEAR,
  SCULPT_PROFILE_BELL,
  SCULPT_PROFILE_ELLIPTIC
};

// Do function documentation in nodepath.cpp
Inkscape::NodePath::Path * sp_nodepath_new (SPDesktop * desktop, SPObject *object, bool show_handles, const char * repr_key = NULL, SPItem *item = NULL);
void sp_nodepath_destroy (Inkscape::NodePath::Path * nodepath);
void sp_nodepath_ensure_livarot_path(Inkscape::NodePath::Path *np);
void sp_nodepath_deselect (Inkscape::NodePath::Path *nodepath);
void sp_nodepath_select_all (Inkscape::NodePath::Path *nodepath, bool invert);
void sp_nodepath_select_all_from_subpath(Inkscape::NodePath::Path *nodepath, bool invert);
void sp_nodepath_select_next (Inkscape::NodePath::Path *nodepath);
void sp_nodepath_select_prev (Inkscape::NodePath::Path *nodepath);
void sp_nodepath_select_rect (Inkscape::NodePath::Path * nodepath, NR::Rect const &b, gboolean incremental);
GList *save_nodepath_selection (Inkscape::NodePath::Path *nodepath);
void restore_nodepath_selection (Inkscape::NodePath::Path *nodepath, GList *r);
gboolean nodepath_repr_d_changed (Inkscape::NodePath::Path * np, const char *newd);
gboolean nodepath_repr_typestr_changed (Inkscape::NodePath::Path * np, const char *newtypestr);
gboolean node_key (GdkEvent * event);
void sp_nodepath_update_repr(Inkscape::NodePath::Path *np, const gchar *annotation);
void sp_nodepath_update_statusbar (Inkscape::NodePath::Path *nodepath);
void sp_nodepath_selected_align(Inkscape::NodePath::Path *nodepath, NR::Dim2 axis);
void sp_nodepath_selected_distribute(Inkscape::NodePath::Path *nodepath, NR::Dim2 axis);
void sp_nodepath_select_segment_near_point(Inkscape::NodePath::Path *nodepath, NR::Point p, bool toggle);
void sp_nodepath_add_node_near_point(Inkscape::NodePath::Path *nodepath, NR::Point p);
void sp_nodepath_curve_drag(int node, double t, NR::Point delta);
Inkscape::NodePath::Node * sp_nodepath_get_node_by_index(int index);
/* possibly private functions */

void sp_node_selected_add_node (Inkscape::NodePath::Path *nodepath);
void sp_node_selected_break (Inkscape::NodePath::Path *nodepath);
void sp_node_selected_duplicate (Inkscape::NodePath::Path *nodepath);
void sp_node_selected_join (Inkscape::NodePath::Path *nodepath);
void sp_node_selected_join_segment (Inkscape::NodePath::Path *nodepath);
void sp_node_delete_preserve (GList *nodes_to_delete);
void sp_node_selected_delete (Inkscape::NodePath::Path *nodepath);
void sp_node_selected_delete_segment (Inkscape::NodePath::Path *nodepath);
void sp_node_selected_set_type (Inkscape::NodePath::Path *nodepath, Inkscape::NodePath::NodeType type);
void sp_node_selected_set_line_type (Inkscape::NodePath::Path *nodepath, NRPathcode code);
void sp_node_selected_move (Inkscape::NodePath::Path *nodepath, gdouble dx, gdouble dy);
void sp_node_selected_move_screen (Inkscape::NodePath::Path *nodepath, gdouble dx, gdouble dy);
void sp_node_selected_move_absolute (Inkscape::NodePath::Path *nodepath, NR::Coord val, NR::Dim2 axis);
NR::Rect sp_node_selected_bbox (Inkscape::NodePath::Path *nodepath);
NR::Maybe<NR::Coord> sp_node_selected_common_coord (Inkscape::NodePath::Path *nodepath, NR::Dim2 axis);

void sp_nodepath_show_handles(Inkscape::NodePath::Path *nodepath, bool show);
SPCanvasItem *sp_nodepath_generate_helperpath(SPDesktop *desktop, SPCurve *curve, const SPItem *item);
SPCanvasItem *sp_nodepath_generate_helperpath(SPDesktop *desktop, SPPath *path);
void sp_nodepath_show_helperpath(Inkscape::NodePath::Path *nodepath, bool show);
void sp_nodepath_make_straight_path(Inkscape::NodePath::Path *np);

void sp_nodepath_selected_nodes_rotate (Inkscape::NodePath::Path * nodepath, gdouble angle, int which, bool screen);

void sp_nodepath_selected_nodes_scale (Inkscape::NodePath::Path * nodepath, gdouble grow, int which);
void sp_nodepath_selected_nodes_scale_screen (Inkscape::NodePath::Path * nodepath, gdouble grow, int which);

void sp_nodepath_flip (Inkscape::NodePath::Path *nodepath, NR::Dim2 axis, NR::Maybe<NR::Point> center);

#endif

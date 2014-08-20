#ifndef SEEN_SP_MESH_ARRAY_H
#define SEEN_SP_MESH_ARRAY_H
/*
 * Authors:
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyrigt  (C) 2012 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/**
   A group of classes and functions for manipulating mesh gradients.

   A mesh is made up of an array of patches. Each patch has four sides and four corners. The sides can
   be shared between two patches and the corners between up to four.

   The order of the points for each side always goes from left to right or top to bottom.
   For sides 2 and 3 the points must be reversed when used (as in calls to cairo functions). 

   Two patches: (C=corner, S=side, H=handle, T=tensor)

                         C0   H1  H2 C1 C0 H1  H2  C1
                          + ---------- + ---------- +
                          |     S0     |     S0     |
                       H1 |  T0    T1  |H1 T0   T1  | H1
                          |S3        S1|S3        S1|
                       H2 |  T3    T2  |H2 T3   T2  | H2
                          |     S2     |     S2     |
                          + ---------- + ---------- +
                         C3   H1  H2 C2 C3 H1  H2   C2

   The mesh is stored internally as an array of nodes that includes the tensor nodes.

   Note: This code uses tensor points which are not part of the SVG2 plan at the moment.
   Including tensor points was motivated by a desire to experiment with their usefulness
   in smoothing color transitions. There doesn't seem to be much advantage for that
   purpose. However including them internally allows for storing all the points in
   an array which simplifies things like inserting new rows or columns.
*/

#include <gdk/gdk.h>
#include <glibmm/ustring.h>
#include <2geom/point.h>
#include "color.h"

// For color picking
#include "sp-item.h"

enum NodeType {
  MG_NODE_TYPE_UNKNOWN,
  MG_NODE_TYPE_CORNER,
  MG_NODE_TYPE_HANDLE,
  MG_NODE_TYPE_TENSOR
};

// Is a node along an edge?
enum NodeEdge {
  MG_NODE_EDGE_NONE,
  MG_NODE_EDGE_TOP = 1,
  MG_NODE_EDGE_LEFT = 2,
  MG_NODE_EDGE_BOTTOM = 4,
  MG_NODE_EDGE_RIGHT = 8
};

enum MeshCornerOperation {
  MG_CORNER_SIDE_TOGGLE,
  MG_CORNER_SIDE_ARC,
  MG_CORNER_TENSOR_TOGGLE,
  MG_CORNER_COLOR_SMOOTH,
  MG_CORNER_COLOR_PICK
};

enum MeshNodeOperation {
  MG_NODE_NO_SCALE,
  MG_NODE_SCALE,
  MG_NODE_SCALE_HANDLE
};


class SPMeshNode {
public:
  SPMeshNode() {
    node_type = MG_NODE_TYPE_UNKNOWN;
    node_edge = MG_NODE_EDGE_NONE;
    set = false;
    draggable = -1;
    path_type = 'u';
    opacity = 0.0;
  }
  NodeType node_type;
  guint     node_edge;
  bool set;
  Geom::Point p;
  guint draggable;  // index of on-screen node
  gchar path_type;
  SPColor color;
  gdouble opacity;
};


// I for Internal to distinguish it from the Object class
// This is a convenience class...
class SPMeshPatchI {

private:
  std::vector<std::vector< SPMeshNode* > > *nodes;
  int row;
  int col;

public:
  SPMeshPatchI( std::vector<std::vector< SPMeshNode* > > *n, int r, int c );
  Geom::Point getPoint( guint side, guint point );
  std::vector< Geom::Point > getPointsForSide( guint i );
  void        setPoint( guint side, guint point, Geom::Point p, bool set = true );
  gchar getPathType( guint i );
  void  setPathType( guint, gchar t );
  Geom::Point getTensorPoint( guint i );
  void        setTensorPoint( guint i, Geom::Point p );
  bool tensorIsSet();
  bool tensorIsSet( guint i );
  Geom::Point coonsTensorPoint( guint i );
  void    updateNodes();
  SPColor getColor( guint i );
  void    setColor( guint i, SPColor c );
  gdouble getOpacity( guint i );
  void    setOpacity( guint i, gdouble o );
};

class SPMeshGradient;

// An array of mesh nodes.
class SPMeshNodeArray {

// Should be private
public:
  SPMeshGradient *mg;
  std::vector< std::vector< SPMeshNode* > > nodes;

public:
  // Draggables to nodes
  bool drag_valid;
  std::vector< SPMeshNode* > corners;
  std::vector< SPMeshNode* > handles;
  std::vector< SPMeshNode* > tensors;

public:

  friend class SPMeshPatchI;

  SPMeshNodeArray() { built = false; mg = NULL; drag_valid = false; };
  SPMeshNodeArray( SPMeshGradient *mg );
  ~SPMeshNodeArray() { clear(); };
  bool built;

  void read( SPMeshGradient *mg );
  void write( SPMeshGradient *mg );
  void create( SPMeshGradient *mg, SPItem *item, Geom::OptRect bbox );
  void clear();
  void print();

  // Get size of patch
  guint patch_rows();
  guint patch_columns();

  SPMeshNode * node( guint i, guint j ) { return nodes[i][j]; }

  // Operations on corners
  bool adjacent_corners( guint i, guint j, SPMeshNode* n[4] );
  guint side_toggle( std::vector< guint > );
  guint side_arc( std::vector< guint > );
  guint tensor_toggle( std::vector< guint > );
  guint color_smooth( std::vector< guint > );
  guint color_pick( std::vector< guint >, SPItem* );

  // Update other nodes in response to a node move.
  void update_handles( guint corner, std::vector< guint > selected_corners, Geom::Point old_p, MeshNodeOperation op );

  void split_row( guint i, guint n );
  void split_column( guint j, guint n );
  void split_row( guint i, double coord );
  void split_column( guint j, double coord );
};

#endif /* !SEEN_SP_MESH_ARRAY_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  c-basic-offset:2
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

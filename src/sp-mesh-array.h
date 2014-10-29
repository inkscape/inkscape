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
  unsigned int     node_edge;
  bool set;
  Geom::Point p;
  unsigned int draggable;  // index of on-screen node
  char path_type;
  SPColor color;
  double opacity;
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
  Geom::Point getPoint( unsigned int side, unsigned int point );
  std::vector< Geom::Point > getPointsForSide( unsigned int i );
  void        setPoint( unsigned int side, unsigned int point, Geom::Point p, bool set = true );
  char getPathType( unsigned int i );
  void  setPathType( unsigned int, char t );
  Geom::Point getTensorPoint( unsigned int i );
  void        setTensorPoint( unsigned int i, Geom::Point p );
  bool tensorIsSet();
  bool tensorIsSet( unsigned int i );
  Geom::Point coonsTensorPoint( unsigned int i );
  void    updateNodes();
  SPColor getColor( unsigned int i );
  void    setColor( unsigned int i, SPColor c );
  double  getOpacity( unsigned int i );
  void    setOpacity( unsigned int i, double o );
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
  unsigned int patch_rows();
  unsigned int patch_columns();

  SPMeshNode * node( unsigned int i, unsigned int j ) { return nodes[i][j]; }

  // Operations on corners
  bool adjacent_corners( unsigned int i, unsigned int j, SPMeshNode* n[4] );
  unsigned int side_toggle( std::vector< unsigned int > );
  unsigned int side_arc( std::vector< unsigned int > );
  unsigned int tensor_toggle( std::vector< unsigned int > );
  unsigned int color_smooth( std::vector< unsigned int > );
  unsigned int color_pick( std::vector< unsigned int >, SPItem* );

  // Update other nodes in response to a node move.
  void update_handles( unsigned int corner, std::vector< unsigned int > selected_corners, Geom::Point old_p, MeshNodeOperation op );

  void split_row( unsigned int i, unsigned int n );
  void split_column( unsigned int j, unsigned int n );
  void split_row( unsigned int i, double coord );
  void split_column( unsigned int j, double coord );
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

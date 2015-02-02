/** \file
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

/*
 * Authors:
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright  (C) 2012, 2015 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm.h>

// For color picking
#include "display/drawing.h"
#include "display/drawing-context.h"
#include "display/cairo-utils.h"
#include "document.h"
#include "sp-root.h"

#include "sp-mesh-array.h"
#include "sp-mesh-gradient.h"
#include "sp-mesh-row.h"
#include "sp-mesh-patch.h"
#include "sp-stop.h"

// For new mesh creation
#include "preferences.h"
#include "sp-ellipse.h"
#include "sp-star.h"

// For writing color/opacity to style
#include "svg/css-ostringstream.h"

// For default color
#include "style.h"
#include "svg/svg-color.h"


// Includes bezier-curve.h, ray.h, crossing.h
#include "2geom/line.h"

#include "xml/repr.h"
#include <cmath>
#include <algorithm>

enum { ROW, COL };

SPMeshPatchI::SPMeshPatchI( std::vector<std::vector< SPMeshNode* > > * n, int r, int c ) {

    nodes = n;
    row = r*3; // Convert from patch array to node array
    col = c*3;

    guint i = 0;
    if( row != 0 ) i = 1;
    for( ; i < 4; ++i ) {
        if( nodes->size() < row+i+1 ) {
            std::vector< SPMeshNode* > row;
            nodes->push_back( row );
        }

        guint j = 0;
        if( col != 0 ) j = 1;
        for( ; j < 4; ++j ) {
            if( (*nodes)[row+i].size() < col+j+1 ){
                SPMeshNode* node = new SPMeshNode;
                // Ensure all nodes know their type.
                node->node_type = MG_NODE_TYPE_HANDLE;
                if( (i == 0 || i == 3) && (j == 0 || j == 3 ) ) node->node_type = MG_NODE_TYPE_CORNER;
                if( (i == 1 || i == 2) && (j == 1 || j == 2 ) ) node->node_type = MG_NODE_TYPE_TENSOR;
                (*nodes)[row+i].push_back( node );
            }
        }
    }
}

/**
   Returns point for side in proper order for patch
*/
Geom::Point SPMeshPatchI::getPoint( guint s, guint pt ) {

    assert( s < 4 );
    assert( pt < 4 );

    Geom::Point p;
    switch ( s ) {
        case 0:
            p = (*nodes)[ row      ][ col+pt   ]->p;
            break;
        case 1:
            p = (*nodes)[ row+pt   ][ col+3    ]->p;
            break;
        case 2:
            p = (*nodes)[ row+3    ][ col+3-pt ]->p;
            break;
        case 3:
            p = (*nodes)[ row+3-pt ][ col      ]->p;
            break;
    }
    return p;

};

/**
   Returns vector of points for a side in proper order for a patch (clockwise order).
*/
std::vector< Geom::Point > SPMeshPatchI::getPointsForSide( guint i ) {

    assert( i < 4 );

    std::vector< Geom::Point> points;
    points.push_back( getPoint( i, 0 ) );
    points.push_back( getPoint( i, 1 ) );
    points.push_back( getPoint( i, 2 ) );
    points.push_back( getPoint( i, 3 ) );
    return points;
};


/**
   Set point for side in proper order for patch
*/
void SPMeshPatchI::setPoint( guint s, guint pt, Geom::Point p, bool set ) {

    assert( s < 4 );
    assert( pt < 4 );

    NodeType node_type = MG_NODE_TYPE_CORNER;
    if( pt == 1 || pt == 2 ) node_type = MG_NODE_TYPE_HANDLE;

    // std::cout << "SPMeshPatchI::setPoint: s: " << s 
    //           << " pt: " << pt
    //           << " p: " << p
    //           << " node_type: " << node_type
    //           << " set: " << set
    //           << " row: " << row
    //           << " col: " << col << std::endl;
    switch ( s ) {
        case 0:
            (*nodes)[ row      ][ col+pt   ]->p = p;
            (*nodes)[ row      ][ col+pt   ]->set = set;
            (*nodes)[ row      ][ col+pt   ]->node_type = node_type;
            break;
        case 1:
            (*nodes)[ row+pt   ][ col+3    ]->p = p;
            (*nodes)[ row+pt   ][ col+3    ]->set = set;
            (*nodes)[ row+pt   ][ col+3    ]->node_type = node_type;
            break;
        case 2:
            (*nodes)[ row+3    ][ col+3-pt ]->p = p;
            (*nodes)[ row+3    ][ col+3-pt ]->set = set;
            (*nodes)[ row+3    ][ col+3-pt ]->node_type = node_type;
            break;
        case 3:
            (*nodes)[ row+3-pt ][ col      ]->p = p;
            (*nodes)[ row+3-pt ][ col      ]->set = set;
            (*nodes)[ row+3-pt ][ col      ]->node_type = node_type;
            break;
    }

};

/**
   Get path type for side (stored in handle nodes).
*/
gchar SPMeshPatchI::getPathType( guint s ) {

    assert( s < 4 );

    gchar type = 'x';

    switch ( s ) {
        case 0:
            type = (*nodes)[ row   ][ col+1 ]->path_type;
            break;
        case 1:
            type = (*nodes)[ row+1 ][ col+3 ]->path_type;
            break;
        case 2:
            type = (*nodes)[ row+3 ][ col+2 ]->path_type;
            break;
        case 3:
            type = (*nodes)[ row+2 ][ col   ]->path_type;
            break;
    }

    return type;
};

/**
   Set path type for side (stored in handle nodes).
*/
void SPMeshPatchI::setPathType( guint s, gchar t ) {

    assert( s < 4 );

    switch ( s ) {
        case 0:
            (*nodes)[ row   ][ col+1 ]->path_type = t;
            (*nodes)[ row   ][ col+2 ]->path_type = t;
            break;
        case 1:
            (*nodes)[ row+1 ][ col+3 ]->path_type = t;
            (*nodes)[ row+2 ][ col+3 ]->path_type = t;
            break;
        case 2:
            (*nodes)[ row+3 ][ col+1 ]->path_type = t;
            (*nodes)[ row+3 ][ col+2 ]->path_type = t;
            break;
        case 3:
            (*nodes)[ row+1 ][ col   ]->path_type = t;
            (*nodes)[ row+2 ][ col   ]->path_type = t;
            break;
    }

};

/**
   Set tensor control point for "corner" i.
 */
void SPMeshPatchI::setTensorPoint( guint i, Geom::Point p ) {

    assert( i < 4 );
    switch ( i ) {
        case 0:
            (*nodes)[ row + 1 ][ col + 1 ]->p = p;
            (*nodes)[ row + 1 ][ col + 1 ]->set = true;
            (*nodes)[ row + 1 ][ col + 1 ]->node_type = MG_NODE_TYPE_TENSOR;
            break;
        case 1:
            (*nodes)[ row + 1 ][ col + 2 ]->p = p;
            (*nodes)[ row + 1 ][ col + 2 ]->set = true;
            (*nodes)[ row + 1 ][ col + 2 ]->node_type = MG_NODE_TYPE_TENSOR;
            break;
        case 2:
            (*nodes)[ row + 2 ][ col + 2 ]->p = p;
            (*nodes)[ row + 2 ][ col + 2 ]->set = true;
            (*nodes)[ row + 2 ][ col + 2 ]->node_type = MG_NODE_TYPE_TENSOR;
            break;
        case 3:
            (*nodes)[ row + 2 ][ col + 1 ]->p = p;
            (*nodes)[ row + 2 ][ col + 1 ]->set = true;
            (*nodes)[ row + 2 ][ col + 1 ]->node_type = MG_NODE_TYPE_TENSOR;
            break;
    }
}

/**
   Return if any tensor control point is set.
 */
bool SPMeshPatchI::tensorIsSet() {
    for( guint i = 0; i < 4; ++i ) {
        if( tensorIsSet( i ) ) {
            return true;
        }
    }
    return false;
}

/**
   Return if tensor control point for "corner" i is set.
 */
bool SPMeshPatchI::tensorIsSet( guint i ) {

    assert( i < 4 );

    bool set = false;
    switch ( i ) {
        case 0:
            set = (*nodes)[ row + 1 ][ col + 1 ]->set;
            break;
        case 1:
            set = (*nodes)[ row + 1 ][ col + 2 ]->set;
            break;
        case 2:
            set = (*nodes)[ row + 2 ][ col + 2 ]->set;
            break;
        case 3:
            set = (*nodes)[ row + 2 ][ col + 1 ]->set;
            break;
    }
    return set;
}

/**
   Return tensor control point for "corner" i.
   If not sest, returns calculated (Coons) point.
 */
Geom::Point SPMeshPatchI::getTensorPoint( guint k ) {

    assert( k < 4 );

    guint i = 0;
    guint j = 0;
    

    switch ( k ) {
        case 0:
            i = 1;
            j = 1;
            break;
        case 1:
            i = 1;
            j = 2;
            break;
        case 2:
            i = 2;
            j = 2;
            break;
        case 3:
            i = 2;
            j = 1;
            break;
    }

    Geom::Point p;
    if( (*nodes)[ row + i ][ col + j ]->set ) {
        p = (*nodes)[ row + i ][ col + j ]->p;
    } else {
        p = coonsTensorPoint( k );
    }
    return p;
}

/**
   Find default tensor point (equivalent point to Coons Patch).
   Formulas defined in PDF spec.
   Equivalent to 1/3 of side length from corner for square patch.
 */
Geom::Point SPMeshPatchI::coonsTensorPoint( guint i ) {

    Geom::Point t;
    Geom::Point p[4][4]; // Points in PDF notation

    p[0][0] = getPoint( 0, 0 );
    p[0][1] = getPoint( 0, 1 );
    p[0][2] = getPoint( 0, 2 );
    p[0][3] = getPoint( 0, 3 );
    p[1][0] = getPoint( 3, 2 );
    p[1][3] = getPoint( 1, 1 );
    p[2][0] = getPoint( 3, 1 );
    p[2][3] = getPoint( 1, 2 );
    p[3][0] = getPoint( 2, 3 );
    p[3][1] = getPoint( 2, 2 );
    p[3][2] = getPoint( 2, 1 );
    p[3][3] = getPoint( 2, 0 );

    switch ( i ) {
        case 0:
            t = ( -4.0 *   p[0][0] +
                   6.0 * ( p[0][1] + p[1][0] ) +
                  -2.0 * ( p[0][3] + p[3][0] ) +
                   3.0 * ( p[3][1] + p[1][3] ) +
                  -1.0 *   p[3][3] ) / 9.0;
            break;

        case 1:
            t = ( -4.0 *   p[0][3] +
                   6.0 * ( p[0][2] + p[1][3] ) +
                  -2.0 * ( p[0][0] + p[3][3] ) +
                   3.0 * ( p[3][2] + p[1][0] ) +
                  -1.0 *   p[3][0] ) / 9.0;
            break;

        case 2:
            t = ( -4.0 *   p[3][3] +
                   6.0 * ( p[3][2] + p[2][3] ) +
                  -2.0 * ( p[3][0] + p[0][3] ) +
                   3.0 * ( p[0][2] + p[2][0] ) +
                  -1.0 *   p[0][0] ) / 9.0;
            break;

        case 3:
            t = ( -4.0 *   p[3][0] +
                   6.0 * ( p[3][1] + p[2][0] ) +
                  -2.0 * ( p[3][3] + p[0][0] ) +
                   3.0 * ( p[0][1] + p[2][3] ) +
                  -1.0 *   p[0][3] ) / 9.0;
            break;

        default:

            g_warning( "Impossible!" );

    }
    return t;
}

/**
   Update default values for handle and tensor nodes.
*/
void SPMeshPatchI::updateNodes() {

    // std::cout << "SPMeshPatchI::updateNodes: " << row << "," << col << std::endl;
    // Handles first (tensors require update handles).
    for( guint i = 0; i < 4; ++i ) {
        for( guint j = 0; j < 4; ++j ) {
            if( (*nodes)[ row + i ][ col + j ]->set == false ) {

                if( (*nodes)[ row + i ][ col + j ]->node_type == MG_NODE_TYPE_HANDLE ) {
                    
                    // If a handle is not set it is because the side is a line.
                    // Set node points 1/3 of the way between corners.

                    if( i == 0 || i == 3 ) {
                        Geom::Point p0 = ( (*nodes)[ row + i ][ col     ]->p );
                        Geom::Point p3 = ( (*nodes)[ row + i ][ col + 3 ]->p );
                        Geom::Point dp = (p3 - p0)/3.0;
                        if( j == 2 ) dp *= 2.0;
                        (*nodes)[ row + i ][ col + j ]->p = p0 + dp;
                    }

                    if( j == 0 || j == 3 ) {
                        Geom::Point p0 = ( (*nodes)[ row     ][ col + j ]->p );
                        Geom::Point p3 = ( (*nodes)[ row + 3 ][ col + j ]->p );
                        Geom::Point dp = (p3 - p0)/3.0;
                        if( i == 2 ) dp *= 2.0;
                        (*nodes)[ row + i ][ col + j ]->p = p0 + dp;
                    }
                }
            }
        }
    }

    // Update tensor nodes
    for( guint i = 1; i < 3; ++i ) {
        for( guint j = 1; j < 3; ++j ) {
            if( (*nodes)[ row + i ][ col + j ]->set == false ) {

                (*nodes)[ row + i ][ col + j ]->node_type = MG_NODE_TYPE_TENSOR;

                guint t = 0;
                if( i == 1 && j == 2 ) t = 1;
                if( i == 2 && j == 2 ) t = 2;
                if( i == 2 && j == 1 ) t = 3;
                (*nodes)[ row + i ][ col + j ]->p = coonsTensorPoint( t );
                // std::cout << "Update node: " << i << ", " << j << " " << coonsTensorPoint( t ) << std::endl;

            }
        }
    }
}

/**
   Return color for corner of patch.
*/
SPColor SPMeshPatchI::getColor( guint i ) {

    assert( i < 4 );

    SPColor color;
    switch ( i ) {
        case 0:
            color = (*nodes)[ row   ][ col   ]->color;
            break;
        case 1:
            color = (*nodes)[ row   ][ col+3 ]->color;
            break;
        case 2:
            color = (*nodes)[ row+3 ][ col+3 ]->color;
            break;
        case 3:
            color = (*nodes)[ row+3 ][ col   ]->color;
            break;

    }

    return color;

};

/**
   Set color for corner of patch.
*/
void SPMeshPatchI::setColor( guint i, SPColor color ) {

    assert( i < 4 );

    switch ( i ) {
        case 0:
            (*nodes)[ row   ][ col   ]->color = color;
            break;                       
        case 1:                          
            (*nodes)[ row   ][ col+3 ]->color = color;
            break;                       
        case 2:                          
            (*nodes)[ row+3 ][ col+3 ]->color = color;
            break;                       
        case 3:                          
            (*nodes)[ row+3 ][ col   ]->color = color;
            break;
    }
};

/**
   Return opacity for corner of patch.
*/
gdouble SPMeshPatchI::getOpacity( guint i ) {

    assert( i < 4 );

    gdouble opacity = 0.0;
    switch ( i ) {
        case 0:
            opacity = (*nodes)[ row   ][ col   ]->opacity;
            break;
        case 1:
            opacity = (*nodes)[ row   ][ col+3 ]->opacity;
            break;
        case 2:
            opacity = (*nodes)[ row+3 ][ col+3 ]->opacity;
            break;
        case 3:
            opacity = (*nodes)[ row+3 ][ col   ]->opacity;
            break;
    }

    return opacity;
};


/**
   Set opacity for corner of patch.
*/
void SPMeshPatchI::setOpacity( guint i, gdouble opacity ) {

    assert( i < 4 );

    switch ( i ) {
        case 0:
            (*nodes)[ row   ][ col   ]->opacity = opacity;
            break;                         
        case 1:                            
            (*nodes)[ row   ][ col+3 ]->opacity = opacity;
            break;                         
        case 2:                            
            (*nodes)[ row+3 ][ col+3 ]->opacity = opacity;
            break;                         
        case 3:                            
            (*nodes)[ row+3 ][ col   ]->opacity = opacity;
            break;

    }

};


SPMeshNodeArray::SPMeshNodeArray( SPMeshGradient *mg ) {

    read( mg );

};


// Copy constructor
SPMeshNodeArray::SPMeshNodeArray( const SPMeshNodeArray& rhs ) {

    built = false;
    mg = NULL;
    drag_valid = false;

    nodes = rhs.nodes; // This only copies the pointers but it does size the vector of vectors.

    for( unsigned i=0; i < nodes.size(); ++i ) {
        for( unsigned j=0; j < nodes[i].size(); ++j ) {
            nodes[i][j] = new SPMeshNode( *rhs.nodes[i][j] ); // Copy data.
        }
    }
};


// Copy assignment operator
SPMeshNodeArray& SPMeshNodeArray::operator=( const SPMeshNodeArray& rhs ) {

    if( this == &rhs ) return *this;

    clear(); // Clear any existing array.

    built = false;
    mg = NULL;
    drag_valid = false;

    nodes = rhs.nodes; // This only copies the pointers but it does size the vector of vectors.

    for( unsigned i=0; i < nodes.size(); ++i ) {
        for( unsigned j=0; j < nodes[i].size(); ++j ) {
            nodes[i][j] = new SPMeshNode( *rhs.nodes[i][j] ); // Copy data.
        }
    }
    
    return *this;
};


void SPMeshNodeArray::read( SPMeshGradient *mg_in ) {

    mg = mg_in;

    clear();

    Geom::Point current_p( mg->x.computed, mg->y.computed );
    // std::cout << "SPMeshNodeArray::read: p: " << current_p << std::endl;

    guint max_column = 0;
    guint irow = 0; // Corresponds to top of patch being read in.
    for ( SPObject *ro = mg->firstChild() ; ro ; ro = ro->getNext() ) {

        if (SP_IS_MESHROW(ro)) {

            guint icolumn = 0; // Corresponds to left of patch being read in.
            for ( SPObject *po = ro->firstChild() ; po ; po = po->getNext() ) {

                if (SP_IS_MESHPATCH(po)) {

                    SPMeshPatch *patch = SP_MESHPATCH(po);

                    // std::cout << "SPMeshNodeArray::read: row size: " << nodes.size() << std::endl;
                    SPMeshPatchI new_patch( &nodes, irow, icolumn ); // Adds new nodes.
                    // std::cout << "                          after: " << nodes.size() << std::endl;

                    gint istop = 0;

                    // Only 'top' side defined for first row.
                    if( irow != 0 ) ++istop;

                    for ( SPObject *so = po->firstChild() ; so ; so = so->getNext() ) {
                        if (SP_IS_STOP(so)) {

                            if( istop > 3 ) {
                                // std::cout << " Mesh Gradient: Too many stops: " << istop << std::endl;
                                break;
                            }

                            SPStop *stop = SP_STOP(so);

                            // Handle top of first row.
                            if( istop == 0 && icolumn == 0 ) {
                                // First patch in mesh.
                                new_patch.setPoint( 0, 0, current_p );
                            }
                            // First point is always already defined by previous side (stop).
                            current_p = new_patch.getPoint( istop, 0 );

                            // If side closes patch, then we read one less point.
                            bool closed = false;
                            if( icolumn == 0 && istop == 3 ) closed = true;
                            if( icolumn  > 0 && istop == 2 ) closed = true;


                            // Copy path and then replace commas by spaces so we can use stringstream to parse
                            std::string path_string = *(stop->path_string);
                            std::replace(path_string.begin(),path_string.end(),',',' ');

                            // std::cout << "    path_string: " << path_string << std::endl;
                            // std::cout << "    current_p: " << current_p << std::endl;

                            std::stringstream os( path_string );

                            // Determine type of path
                            char path_type;
                            os >> path_type;
                            new_patch.setPathType( istop, path_type );

                            gdouble x, y;
                            Geom::Point p, dp;
                            guint max;
                            switch ( path_type ) {
                                case 'l':
                                    if( !closed ) {
                                        os >> x >> y;
                                        if( !os.fail() ) {
                                            dp = Geom::Point( x, y ); 
                                            new_patch.setPoint( istop, 3, current_p + dp );
                                        } else {
                                            std::cout << "Failed to read l" << std::endl;
                                        }
                                    }
                                    // To facilitate some side operations, set handles to 1/3 and
                                    // 2/3 distance between corner points but flag as unset.
                                    p = new_patch.getPoint( istop, 3 );
                                    dp = (p - current_p)/3.0;  // Calculate since may not be set if closed.
                                    // std::cout << "      istop: " << istop
                                    //           << "  dp: " << dp
                                    //           << "  p: " << p
                                    //           << " current_p: " << current_p
                                    //           << std::endl;
                                    new_patch.setPoint( istop, 1, current_p + dp,       false );
                                    new_patch.setPoint( istop, 2, current_p + 2.0 * dp, false );
                                    break;
                                case 'L':
                                    if( !closed ) {
                                        os >> x >> y;
                                        if( !os.fail() ) {
                                            p = Geom::Point( x, y );
                                            new_patch.setPoint( istop, 3, p );
                                        } else {
                                            std::cout << "Failed to read L" << std::endl;
                                        }
                                    }
                                    // To facilitate some side operations, set handles to 1/3 and
                                    // 2/3 distance between corner points but flag as unset.
                                    p = new_patch.getPoint( istop, 3 );
                                    dp = (p - current_p)/3.0;
                                    new_patch.setPoint( istop, 1, current_p + dp,       false );
                                    new_patch.setPoint( istop, 2, current_p + 2.0 * dp, false );
                                    break;
                                case 'c':
                                    max = 4;
                                    if( closed ) max = 3;
                                    for( guint i = 1; i < max; ++i ) {
                                        os >> x >> y;
                                        if( !os.fail() ) {
                                            p = Geom::Point( x, y );
                                            p += current_p;
                                            new_patch.setPoint( istop, i, p );
                                        } else {
                                            std::cout << "Failed to read c: " << i << std::endl;
                                        }
                                    }
                                    break;
                                case 'C':
                                    max = 4;
                                    if( closed ) max = 3;
                                    for( guint i = 1; i < max; ++i ) {
                                        os >> x >> y;
                                        if( !os.fail() ) {
                                            p = Geom::Point( x, y );
                                            new_patch.setPoint( istop, i, p );
                                        } else {
                                            std::cout << "Failed to read C: " << i << std::endl;
                                        }
                                    }
                                    break;
                                default:
                                    // should not reach
                                    std::cout << "Path Error: unhandled path type: " << path_type << std::endl;
                            }
                            current_p = new_patch.getPoint( istop, 3 );

                            // Color
                            if( (istop == 0 && irow == 0 && icolumn > 0) || (istop == 1 && irow > 0 ) ) {
                                // skip 
                            } else {
                                SPColor color   = stop->getEffectiveColor();
                                double opacity  = stop->opacity;
                                new_patch.setColor( istop, color );
                                new_patch.setOpacity( istop, opacity );
                            }

                        }
                        ++istop;
                    } // Loop over stops

                    // Read in tensor string after stops since tensor nodes defined relative to corner nodes.

                    // Copy string and then replace commas by spaces so we can use stringstream to parse XXXX
                    if( patch->tensor_string ) {
                        std::string tensor_string = *(patch->tensor_string);
                        std::replace(tensor_string.begin(),tensor_string.end(),',',' ');

                        // std::cout << "    tensor_string: " << tensor_string << std::endl;

                        std::stringstream os( tensor_string );
                        for( guint i = 0; i < 4; ++i ) {
                            double x = 0.0;
                            double y = 0.0;
                            os >> x >> y;
                            if( !os.fail() ) {
                                new_patch.setTensorPoint( i, new_patch.getPoint( i, 0 ) + Geom::Point( x, y ) );
                            } else {
                                std::cout << "Failed to read p: " << i << std::endl;
                                break;
                            }
                        }
                    }
                }

                ++icolumn;
                if( max_column < icolumn ) max_column = icolumn;
            }
        }
        ++irow;
    }

    // Insure we have a true array.
    for( guint i = 0; i < nodes.size(); ++i ) {
        nodes[ i ].resize( max_column * 3 + 1 );
    }

    // Set node edge.
    for( guint i = 0; i < nodes.size(); ++i ) {
        for( guint j = 0; j < nodes[i].size(); ++j ) {
            nodes[i][j]->node_edge = MG_NODE_EDGE_NONE;
            if( i == 0 )                    nodes[i][j]->node_edge |= MG_NODE_EDGE_TOP;
            if( i == nodes.size() - 1 )     nodes[i][j]->node_edge |= MG_NODE_EDGE_BOTTOM;
            if( j == 0 )                    nodes[i][j]->node_edge |= MG_NODE_EDGE_RIGHT;
            if( j == nodes[i].size() - 1 )  nodes[i][j]->node_edge |= MG_NODE_EDGE_LEFT;
        }
    }

    // std::cout << "SPMeshNodeArray::Read: result:" << std::endl;
    // print();

    drag_valid = false;
    built = true;

};

/**
   Write repr using our array.
*/
void SPMeshNodeArray::write( SPMeshGradient *mg ) {

    // std::cout << "SPMeshNodeArray::write: entrance:" << std::endl;
    // print();
    using Geom::X;
    using Geom::Y;

    // First we must delete reprs for old mesh rows and patches.
    GSList *descendant_reprs = NULL;
    GSList *descendant_objects = NULL;
    for ( SPObject *row = mg->firstChild(); row; row = row->getNext() ) {
        descendant_reprs   = g_slist_prepend (descendant_reprs,   row->getRepr());
        descendant_objects = g_slist_prepend (descendant_objects, row           );
        for ( SPObject *patch = row->firstChild(); patch; patch = patch->getNext() ) {
            descendant_reprs   = g_slist_prepend (descendant_reprs,   patch->getRepr());
            descendant_objects = g_slist_prepend (descendant_objects, patch           );
            for ( SPObject *stop = patch->firstChild(); stop; stop = stop->getNext() ) {
                descendant_reprs   = g_slist_prepend (descendant_reprs,   stop->getRepr());
                descendant_objects = g_slist_prepend (descendant_objects, stop           );
            }
        }
    }

    for ( GSList *i = descendant_objects; i != NULL; i = i->next ) {
        SPObject *descendant = SP_OBJECT (i->data);
        descendant->deleteObject();
    }

    for ( GSList *i = descendant_reprs; i != NULL; i = i->next ) {
        Inkscape::XML::Node *repr = (Inkscape::XML::Node *) i->data;
        sp_repr_unparent( repr );
    }


    // Now we build new reprs
    Inkscape::XML::Node *mesh = mg->getRepr();

    SPMeshNodeArray* array = &(mg->array);
    SPMeshPatchI patch0( &(array->nodes), 0, 0 );
    Geom::Point current_p = patch0.getPoint( 0, 0 ); // Side 0, point 0

    sp_repr_set_svg_double( mesh, "x", current_p[X] );
    sp_repr_set_svg_double( mesh, "y", current_p[Y] );

    Geom::Point current_p2( mg->x.computed, mg->y.computed );

    Inkscape::XML::Document *xml_doc = mesh->document();
    guint rows = array->patch_rows();
    for( guint i = 0; i < rows; ++i ) {

        // Write row
        Inkscape::XML::Node *row = xml_doc->createElement("svg:meshRow");
        mesh->appendChild( row );  // No attributes

        guint columns = array->patch_columns();
        for( guint j = 0; j < columns; ++j ) {

            // Write patch
            Inkscape::XML::Node *patch = xml_doc->createElement("svg:meshPatch");

            SPMeshPatchI patchi( &(array->nodes), i, j );

            // Add tensor
            if( patchi.tensorIsSet() ) {

                std::stringstream is;

                for( guint k = 0; k < 4; ++k ) {
                    Geom::Point p = patchi.getTensorPoint( k ) - patchi.getPoint( k, 0 );
                    is << p[X] << "," << p[Y];
                    if( k < 3 ) is << " ";
                }

                patch->setAttribute("tensor", is.str().c_str());
                // std::cout << "  SPMeshNodeArray::write: tensor: " << is.str() << std::endl;
            }

            row->appendChild( patch );

            // Write sides
            for( guint k = 0; k < 4; ++k ) {

                // Only first row has top stop
                if( k == 0 && i != 0 ) continue;

                // Only first column has left stop
                if( k == 3 && j != 0 ) continue;

                Inkscape::XML::Node *stop = xml_doc->createElement("svg:stop");

                // Add path
                std::stringstream is;
                char path_type = patchi.getPathType( k ); 
                is << path_type;

                std::vector< Geom::Point> p = patchi.getPointsForSide( k );
                current_p = patchi.getPoint( k, 0 );

                switch ( path_type ) {
                    case 'l':
                        is << " "
                           << ( p[3][X] - current_p[X] ) << "," 
                           << ( p[3][Y] - current_p[Y] );
                        break;
                    case 'L':
                        is << " "
                           << p[3][X] << "," 
                           << p[3][Y];
                        break;
                    case 'c':
                        is << " "
                           << ( p[1][X] - current_p[X] ) << "," 
                           << ( p[1][Y] - current_p[Y] ) << "  "
                           << ( p[2][X] - current_p[X] ) << "," 
                           << ( p[2][Y] - current_p[Y] ) << "  "
                           << ( p[3][X] - current_p[X] ) << "," 
                           << ( p[3][Y] - current_p[Y] );
                        break;
                    case 'C':
                        is << " "
                           << p[1][X] << "," 
                           << p[1][Y] << "  "
                           << p[2][X] << "," 
                           << p[2][Y] << "  "
                           << p[3][X] << "," 
                           << p[3][Y];
                        break;
                    case 'z':
                    case 'Z':
                        std::cout << "sp_meshgradient_repr_write: bad path type" << path_type << std::endl;
                        break;
                    default:
                        std::cout << "sp_meshgradient_repr_write: unhandled path type" << path_type << std::endl;
                }
                stop->setAttribute("path", is.str().c_str());
                // std::cout << "SPMeshNodeArray::write: path:  " << is.str().c_str() << std::endl;
                // Add stop-color
                if( ( k == 0 && i == 0 && j == 0 ) ||
                    ( k == 1 && i == 0           ) ||
                    ( k == 2                     ) ||
                    ( k == 3 &&           j == 0 ) ) {

                    // Why are we setting attribute and not style?
                    //stop->setAttribute("stop-color",   patchi.getColor(k).toString().c_str() );
                    //stop->setAttribute("stop-opacity", patchi.getOpacity(k) );

                    Inkscape::CSSOStringStream os;
                    os << "stop-color:" << patchi.getColor(k).toString() << ";stop-opacity:" << patchi.getOpacity(k);
                    stop->setAttribute("style", os.str().c_str());
                }
                patch->appendChild( stop );
            }
        }
    }
}

/**
   Find default color based on color of first stop in "vector" gradient.
   This should be rewritten if dependence on "vector" is removed.
*/
static SPColor default_color( SPItem *item ) {

    // Set initial color to the color of the object before adding the mesh.
    // This is a bit tricky as at the moment, a "vector" gradient is created
    // before reaching here, replacing the original solid color. But the first
    // stop will be that of the original object color.
    SPColor color( 0.5, 0.0, 0.5 );
    if ( item->style ) {
        SPStyle const &style = *(item->style);
        SPIPaint const &paint = ( style.fill ); // Could pick between style.fill/style.stroke
        if ( paint.isColor() ) {
            color = paint.value.color;
        } else if ( paint.isPaintserver() ) {
            SPObject const *server = style.getFillPaintServer();
            if ( SP_IS_GRADIENT(server) ) {
                SPGradient *vector = SP_GRADIENT( server )->getVector();
                SPStop *firstStop = (vector) ?
                    vector->getFirstStop() : SP_GRADIENT( server )->getFirstStop();
                if ( firstStop ) {
                    if (firstStop->currentColor) {
                        Glib::ustring str = firstStop->getStyleProperty("color", NULL);
                        if( !str.empty() ) {
                            guint32 rgb = sp_svg_read_color( str.c_str(), 0 );
                            color = SPColor( rgb );
                        }
                    } else {
                        color = firstStop->specified_color;
                    }
                }
            }
        }
    } else {
        std::cout << " SPMeshNodeArray: No style" << std::endl;
    }

    return color;
}

/**
   Create a default mesh.
*/
void SPMeshNodeArray::create( SPMeshGradient *mg, SPItem *item, Geom::OptRect bbox ) {

    // std::cout << "SPMeshNodeArray::create: Entrance" << std::endl;

    if( !bbox ) {
        // Set default size to bounding box if size not given.
        std::cout << "SPMeshNodeArray::create(): bbox empty" << std::endl;
        Geom::OptRect bbox = item->geometricBounds();
    }
    if( !bbox ) {
        std::cout << "SPMeshNodeArray::create: ERROR: No bounding box!" << std::endl;
        return;
    }

    Geom::Coord const width = bbox->dimensions()[Geom::X];
    Geom::Coord const height = bbox->dimensions()[Geom::Y];
    Geom::Point       center = bbox->midpoint();

    // Must keep repr and array in sync. We have two choices:
    //  Build the repr first and the "read" it.
    //  Construct the array and the "write" it.
    // We'll do the second.

    // Remove any existing mesh.  We could chose to simply scale an existing mesh...
    //clear();

    // We get called twice when a new mesh is created...WHY?
    //  return if we've already constructed the mesh.
    if( !nodes.empty() ) return;

    // Get default color
    SPColor color = default_color( item );
 
    // Get preferences
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    guint prows = prefs->getInt("/tools/mesh/mesh_rows", 1);
    guint pcols = prefs->getInt("/tools/mesh/mesh_cols", 1);

    SPGradientMeshType mesh_type =
        (SPGradientMeshType) prefs->getInt("/tools/mesh/mesh_type", SP_GRADIENT_MESH_TYPE_NORMAL);

    if( mesh_type == SP_GRADIENT_MESH_TYPE_CONICAL ) {

        // Conical gradient.. for any shape/path using geometric bounding box.

        gdouble rx = width/2.0;
        gdouble ry = height/2.0;

        // Start and end angles
        gdouble start = 0.0;
        gdouble end   = 2.0 * M_PI;

        if ( SP_IS_STAR( item ) ) {
            // But if it is a star... use star parameters!
            SPStar* star = SP_STAR( item );
            center = star->center;
            rx = star->r[0];
            ry = star->r[0];
            start = star->arg[0];
            end   = start + 2.0 * M_PI;
        }

        if ( SP_IS_GENERICELLIPSE( item ) ) {
            // For arcs use set start/stop
            SPGenericEllipse* arc = SP_GENERICELLIPSE( item );
            center[Geom::X] = arc->cx.computed;
            center[Geom::Y] = arc->cy.computed;
            rx = arc->rx.computed;
            ry = arc->ry.computed;
            start = arc->start;
            end   = arc->end;
        }

        // std::cout << " start: " << start << "  end: " << end << std::endl;

        // IS THIS NECESSARY?
        Inkscape::XML::Node *repr = mg->getRepr();
        sp_repr_set_svg_double( repr, "x", center[Geom::X] + rx * cos(start) );
        sp_repr_set_svg_double( repr, "y", center[Geom::Y] + ry * sin(start) );

        guint sections = pcols;

        // If less sections, arc approximation error too great. (Check!)
        if( sections < 4 ) sections = 4;

        double arc = (end - start) / (double)sections;

        // See: http://en.wikipedia.org/wiki/B%C3%A9zier_curve
        gdouble kappa = 4.0/3.0 * tan(arc/4.0);
        gdouble lenx = rx * kappa;
        gdouble leny = ry * kappa;

        gdouble s = start;
        for( guint i = 0; i < sections; ++i ) {

            SPMeshPatchI patch( &nodes, 0, i );

            gdouble x0 = center[Geom::X] + rx * cos(s);
            gdouble y0 = center[Geom::Y] + ry * sin(s);
            gdouble x1 = x0 - lenx * sin(s);
            gdouble y1 = y0 + leny * cos(s);

            s += arc;
            gdouble x3 = center[Geom::X] + rx * cos(s);
            gdouble y3 = center[Geom::Y] + ry * sin(s);
            gdouble x2 = x3 + lenx * sin(s);
            gdouble y2 = y3 - leny * cos(s);

            patch.setPoint( 0, 0, Geom::Point( x0, y0 ) );
            patch.setPoint( 0, 1, Geom::Point( x1, y1 ) );
            patch.setPoint( 0, 2, Geom::Point( x2, y2 ) );
            patch.setPoint( 0, 3, Geom::Point( x3, y3 ) );

            patch.setPoint( 2, 0, center );
            patch.setPoint( 3, 0, center );

            for( guint k = 0; k < 4; ++k ) {
                patch.setPathType( k, 'l' );
                patch.setColor( k, color );
                patch.setOpacity( k, 1.0 );
            }
            patch.setPathType( 0, 'c' );

            // Set handle and tensor nodes.
            patch.updateNodes();

        }

        split_row( 0, prows );

    } else {

        // Normal grid meshes

        if( SP_IS_GENERICELLIPSE( item ) ) {

            // std::cout << "We've got ourselves an arc!" << std::endl;

            SPGenericEllipse* arc = SP_GENERICELLIPSE( item );
            center[Geom::X] = arc->cx.computed;
            center[Geom::Y] = arc->cy.computed;
            gdouble rx = arc->rx.computed;
            gdouble ry = arc->ry.computed;

            gdouble s = -3.0/2.0 * M_PI_2;

            Inkscape::XML::Node *repr = mg->getRepr();
            sp_repr_set_svg_double( repr, "x", center[Geom::X] + rx * cos(s) );
            sp_repr_set_svg_double( repr, "y", center[Geom::Y] + ry * sin(s) );

            gdouble lenx = rx * 4*tan(M_PI_2/4)/3;
            gdouble leny = ry * 4*tan(M_PI_2/4)/3;

            SPMeshPatchI patch( &nodes, 0, 0 );
            for( guint i = 0; i < 4; ++i ) {

                gdouble x0 = center[Geom::X] + rx * cos(s);
                gdouble y0 = center[Geom::Y] + ry * sin(s);
                gdouble x1 = x0 + lenx * cos(s + M_PI_2);
                gdouble y1 = y0 + leny * sin(s + M_PI_2);

                s += M_PI_2;
                gdouble x3 = center[Geom::X] + rx * cos(s);
                gdouble y3 = center[Geom::Y] + ry * sin(s);
                gdouble x2 = x3 + lenx * cos(s - M_PI_2);
                gdouble y2 = y3 + leny * sin(s - M_PI_2);

                Geom::Point p1( x1, y1 );
                Geom::Point p2( x2, y2 );
                Geom::Point p3( x3, y3 );
                patch.setPoint( i, 1, p1 );
                patch.setPoint( i, 2, p2 );
                patch.setPoint( i, 3, p3 );

                patch.setPathType( i, 'c' );

                patch.setColor( i, color );
                patch.setOpacity( i, 1.0 );
            }

            // Fill out tensor points
            patch.updateNodes();

            split_row( 0, prows );
            split_column( 0, pcols );

            // END Arc

        } else if ( SP_IS_STAR( item ) ) {

            // Do simplest thing... assume star is not rounded or randomized.
            // (It should be easy to handle the rounded/randomized cases by making
            //  the appropriate star class function public.)
            SPStar* star = SP_STAR( item );
            guint sides =  star->sides;

            // std::cout << "We've got ourselves an star! Sides: " << sides << std::endl;

            Geom::Point p0 = sp_star_get_xy( star, SP_STAR_POINT_KNOT1, 0 );
            Inkscape::XML::Node *repr = mg->getRepr();
            sp_repr_set_svg_double( repr, "x", p0[Geom::X] );
            sp_repr_set_svg_double( repr, "y", p0[Geom::Y] );

            for( guint i = 0; i < sides; ++i ) {

                if( star->flatsided ) {

                    SPMeshPatchI patch( &nodes, 0, i );

                    patch.setPoint( 0, 0, sp_star_get_xy( star, SP_STAR_POINT_KNOT1, i ) );
                    guint ii = i+1;
                    if( ii == sides ) ii = 0;
                    patch.setPoint( 1, 0, sp_star_get_xy( star, SP_STAR_POINT_KNOT1, ii ) );
                    patch.setPoint( 2, 0, star->center );
                    patch.setPoint( 3, 0, star->center );

                    for( guint s = 0; s < 4; ++s ) {
                        patch.setPathType( s, 'l' );
                        patch.setColor( s, color );
                        patch.setOpacity( s, 1.0 );
                    }

                    // Set handle and tensor nodes.
                    patch.updateNodes();

                } else {

                    SPMeshPatchI patch0( &nodes, 0, 2*i );

                    patch0.setPoint( 0, 0, sp_star_get_xy( star, SP_STAR_POINT_KNOT1, i ) );
                    patch0.setPoint( 1, 0, sp_star_get_xy( star, SP_STAR_POINT_KNOT2, i ) );
                    patch0.setPoint( 2, 0, star->center );
                    patch0.setPoint( 3, 0, star->center );

                    guint ii = i+1;
                    if( ii == sides ) ii = 0;

                    SPMeshPatchI patch1( &nodes, 0, 2*i+1 );

                    patch1.setPoint( 0, 0, sp_star_get_xy( star, SP_STAR_POINT_KNOT2, i ) );
                    patch1.setPoint( 1, 0, sp_star_get_xy( star, SP_STAR_POINT_KNOT1, ii ) );
                    patch1.setPoint( 2, 0, star->center );
                    patch1.setPoint( 3, 0, star->center );

                    for( guint s = 0; s < 4; ++s ) {
                        patch0.setPathType( s, 'l' );
                        patch0.setColor( s, color );
                        patch0.setOpacity( s, 1.0 );
                        patch1.setPathType( s, 'l' );
                        patch1.setColor( s, color );
                        patch1.setOpacity( s, 1.0 );
                    }

                    // Set handle and tensor nodes.
                    patch0.updateNodes();
                    patch1.updateNodes();

                }
            }
        
            //print();

            split_row( 0, prows );
            //split_column( 0, pcols );

        } else {

            // Generic

            Inkscape::XML::Node *repr = mg->getRepr();
            sp_repr_set_svg_double(repr, "x", bbox->min()[Geom::X]);
            sp_repr_set_svg_double(repr, "y", bbox->min()[Geom::Y]);

            // Get node array size
            guint nrows = prows * 3 + 1;
            guint ncols = pcols * 3 + 1;

            gdouble dx = width  / (gdouble)(ncols-1.0);
            gdouble dy = height / (gdouble)(nrows-1.0);

            Geom::Point p0( mg->x.computed, mg->y.computed );

            for( guint i = 0; i < nrows; ++i ) {
                std::vector< SPMeshNode* > row;
                for( guint j = 0; j < ncols; ++j ) {
                    SPMeshNode* node = new SPMeshNode;
                    node->p = p0 + Geom::Point( j * dx, i * dy );

                    node->node_edge = MG_NODE_EDGE_NONE;
                    if( i == 0        ) node->node_edge |= MG_NODE_EDGE_TOP;
                    if( i == nrows -1 ) node->node_edge |= MG_NODE_EDGE_BOTTOM;
                    if( j == 0        ) node->node_edge |= MG_NODE_EDGE_LEFT;
                    if( j == ncols -1 ) node->node_edge |= MG_NODE_EDGE_RIGHT;

                    if( i%3 == 0 ) {

                        if( j%3 == 0) {
                            // Corner
                            node->node_type = MG_NODE_TYPE_CORNER;
                            node->set = true;
                            node->color = color;
                            node->opacity = 1.0;

                        } else {
                            // Side
                            node->node_type = MG_NODE_TYPE_HANDLE;
                            node->set = true;
                            node->path_type = 'c';
                        }

                    } else {

                        if( j%3 == 0) {
                            // Side
                            node->node_type = MG_NODE_TYPE_HANDLE;
                            node->set = true;
                            node->path_type = 'c';
                        } else {
                            // Tensor
                            node->node_type = MG_NODE_TYPE_TENSOR;
                            node->set = false;
                        }

                    }

                    row.push_back( node );
                }
                nodes.push_back( row );
            }
            // End normal
        }

    } // If conical

    //print();

    // Write repr
    write( mg );
}


/**
   Clear mesh gradient.
*/
void SPMeshNodeArray::clear() {

    for( guint i = 0; i < nodes.size(); ++i ) {
        for( guint j = 0; j < nodes[i].size(); ++j ) {
            if( nodes[i][j] ) {
                delete nodes[i][j];
            }
        }
        for( guint i = 0; i < nodes.size(); ++i ) {
            nodes[i].clear();
        }
        nodes.clear();
    }
};


/**
   Print mesh gradient (for debugging).
*/
void SPMeshNodeArray::print() {
    for( guint i = 0; i < nodes.size(); ++i ) {
        std::cout << "New node row:" << std::endl;
        for( guint j = 0; j < nodes[i].size(); ++j ) {
            if( nodes[i][j] ) {
                std::cout.width(4);
                std::cout << "  Node: " << i << "," << j << ":  "
                          << nodes[i][j]->p
                          << "  Node type: " << nodes[i][j]->node_type
                          << "  Node edge: " << nodes[i][j]->node_edge
                          << "  Set: "  << nodes[i][j]->set
                          << "  Path type: " << nodes[i][j]->path_type
                          << std::endl;
            } else {
                std::cout << "Error: missing mesh node." << std::endl;
            }
        } // Loop over patches
    } // Loop over rows
};



// Find the slopes at start and end for Hermite interpolation.
// Smooth using Hermite interpolation.
// Inputs are:
//  pb: color value before patch
//  p0: color value start of patch
//  p1: color value end of patch
//  pa: color value after patch
//  is_first:  If first patch in row/column
//  is_last:   If last patch in row/column
//  type: Type smoothing
// Output:
//  m0: slope of Hermite function at start.
//  m1: slope of Hermite function at end.
void find_slopes( const double &pb, const double &p0, const double &p1, const double &pa,
                  const bool &is_first, const bool &is_last, const SPMeshSmooth type,
                  double &m0, double &m1 ) {

    // We use Hermite interpolation. We have end points, we need tangents.

    // Try various ways of finding tangents m0, m1

    // Default to Catmul-Rom (assumes pb and pa already calculatedd)
    m0 = (p1 - pb)/2.0;
    m1 = (pa - p0)/2.0;

    bool parabolic = false;  // Require end patches to be parabolic
    switch (type) {
        case SP_MESH_SMOOTH_SMOOTH1:
            // Flat
            m0 = 0.0;
            m1 = 0.0;
            break;
        case SP_MESH_SMOOTH_SMOOTH2:
            // Catmul-Rom, standard end treatment. Double first/last point.
            if( is_first ) {
                m0 = (p1-p0)/2.0;
            }
            if( is_last ) {
                m1 = (p1-p0)/2.0;
            }
            break;
        case SP_MESH_SMOOTH_SMOOTH3:
            // Catmul-Rom, standard end treatment. Reflect first/last point.
            if( is_first ) {
                m0 = (p1-p0);
            }
            if( is_last ) {
                m1 = (p1-p0);
            }
            break;
        case SP_MESH_SMOOTH_SMOOTH4:
            // Catmul-Rom, Parabolic ends
            parabolic = true;
            break;
        case SP_MESH_SMOOTH_SMOOTH:
        case SP_MESH_SMOOTH_SMOOTH5:
            // Catmul-Rom, Parabolic ends, no color min/max in middle of patch.
            parabolic = true;

            if( (pb > p0 && p1 > p0) ||
                (pb < p0 && p1 < p0) ) {
                // tangents flat at min/max
                m0 = 0;
            } else {
                // ensure we don't overshoot
                if( fabs(m0) > fabs(3*(p1-p0)) ) {
                    m0 = 3*(p1-p0);
                }
                if( fabs(m0) > fabs(3*(p0-pb)) ) {
                    m0 = 3*(p0-pb);
                }
            }
            if( (p0 > p1 && pa > p1) ||
                (p0 < p1 && pa < p1) ) {
                // tangents flat at min/max
                m1 = 0;
            } else {
                // ensure we don't overshoot
                if( fabs(m1) > fabs(3*(pa-p1)) ) {
                    m1 = 3*(pa-p1);
                }
                if( fabs(m1) > fabs(3*(p1-p0)) ) {
                    m1 = 3*(p1-p0);
                }
            }
            break;
        case SP_MESH_SMOOTH_NONE:
        default:
            std::cerr << "find_slopes() Invalid smoothing type." << std::endl;
            break;
    }

    // Force end patches to be parabolic
    if( parabolic ) {
        if( is_first ) {
            // Constraint for parabola
            m0 = 2.0*(p1-p0) - m1;
            if ( ((p1-p0) < 0 && m0 > 0) || ((p1-p0) > 0 && m0 < 0 ) ) {
                m0 = 0;  // Prevent overshooting start value;
            }
        } else if( is_last ) {
            // Constraint for parabola
            m1 = 2.0*(p1-p0) - m0;
            if ( ((p1-p0) < 0 && m1 > 0) || ((p1-p0) > 0 && m1 < 0 ) ) {
                m1 = 0;  // Prevent overshooting end value;
            }
        }
    }

    // std::cout << "  pb: " << pb
    //           << "  p0: " << p0
    //           << "  p1: " << p1
    //           << "  pa: " << pa
    //           << "  m0: " << m0
    //           << "  m1: " << m1 << std::endl;
}

double hermite( const double p0, const double p1, const double m0, const double m1, const double t ) {
    double t2 = t*t;
    double t3 = t2*t;

    double result = (2.0*t3 - 3.0*t2 +1.0) * p0
                  + (t3 - 2.0*t2 + t)      * m0
                  + (-2.0*t3 + 3.0*t2)     * p1
                  + (t3 -t2)               * m1;

    return result;
}


/**
   Fill 'smooth' with a smoothed version of the array by subdividing each patch into smaller patches.
*/
void SPMeshNodeArray::smooth( SPMeshNodeArray* smooth, SPMeshSmooth type ) {

    *smooth = *this;  // Deep copy via copy assignment constructor, smooth cleared before copy
    // std::cout << "SPMeshNodeArray::smooth(): " << this->patch_rows() << " " << smooth->patch_rows() << std::endl;
    // std::cout << "   " << smooth << " " << this << std::endl;
    // Next split each patch into 8x8 smaller patches.
    
    // Do rows first.

    // Split each row into eight rows.
    // Must do it from end so inserted rows don't mess up indexing 
    for( int i = smooth->patch_rows() - 1; i >= 0; --i ) {
        smooth->split_row( i, unsigned(8) );
    }

    // Update color values (every third node is a corner)
    for( unsigned i = 0; i < this->patch_rows(); ++i ) { // i is orignal patch index

        bool is_first_row = (i == 0);
        bool is_last_row = (i == this->patch_rows() - 1 );
        //std::cout << "  last row: " << smooth->patch_rows()/8 - 1 << " " << is_last_row << std::endl;
        for( unsigned j = 0; j < smooth->patch_columns()+1; ++j ) { // j is smooth patch index

            // Can't use guint32 since delta can be negative
            float pb[3]; // Point before patch
            float p0[3]; // Point at start of patch
            float p1[3]; // Point at end of patch
            float pa[3]; // Point after patch
            float result[3][8];
            sp_color_get_rgb_floatv( &this->nodes[  i   *3 ][ j*3 ]->color, p0 ); 
            sp_color_get_rgb_floatv( &this->nodes[ (i+1)*3 ][ j*3 ]->color, p1 );
            if( !is_first_row ) {
                sp_color_get_rgb_floatv( &this->nodes[ (i-1)*3 ][ j*3 ]->color, pb );
            } else {
                pb[0] = 2.0*p0[0] - p1[0];
                pb[1] = 2.0*p0[1] - p1[1];
                pb[2] = 2.0*p0[2] - p1[2];
            }
            if( !is_last_row ) {
                sp_color_get_rgb_floatv( &this->nodes[ (i+2)*3 ][ j*3 ]->color, pa );
            } else {
                pa[0] = 2.0*p1[0] - p0[0];
                pa[1] = 2.0*p1[1] - p0[1];
                pa[2] = 2.0*p1[2] - p0[2];
            }

            for( unsigned n = 0; n < 3; ++n ) { // Loop over colors

                // We use Hermite interpolation. We have end points, we need tangents.
                double m0 = 0;
                double m1 = 0;
                find_slopes( pb[n], p0[n], p1[n], pa[n], is_first_row, is_last_row, type, m0, m1 );
        
                for( unsigned k = 1; k < 8; ++k ) {
                    double t = k/8.0;
                    // Cubic Hermite (four constraints)
                    result[n][k] = hermite( p0[n], p1[n], m0, m1, t );
                    // Clamp to allowed values
                    if( result[n][k] > 1.0 )
                        result[n][k] = 1.0;
                    if( result[n][k] < 0.0 )
                        result[n][k] = 0.0;
                }
            }

            for( unsigned k = 1; k < 8; ++k ) {
                smooth->nodes[ (i*8+k)*3 ][ j*3 ]->color.set( result[0][k], result[1][k], result[2][k] ); 
            }
        }
    }

    // Split each column into eight columns.
    // Must do it from end so inserted columns don't mess up indexing 
    for( int i = smooth->patch_columns() - 1; i >= 0; --i ) {
        smooth->split_column( i, (unsigned)8 );
    }

    // Update color values (every third node is a corner)
    for( unsigned i = 0; i < this->patch_columns(); ++i ) { // i is orignal patch index

        bool is_first_column = (i == 0);
        bool is_last_column = (i == this->patch_columns() - 1 );
        //std::cout << "  last column: " << smooth->patch_columns()/8 - 1 << " " << is_last_column << std::endl;
        for( unsigned j = 0; j < smooth->patch_rows()+1; ++j ) { // j is smooth patch index

            // Can't use guint32 since delta can be negative
            float pb[3]; // Point before patch
            float p0[3]; // Point at start of patch
            float p1[3]; // Point at end of patch
            float pa[3]; // Point after patch
            float result[3][8];
            sp_color_get_rgb_floatv( &smooth->nodes[ j*3 ][  i   *3*8 ]->color, p0 ); 
            sp_color_get_rgb_floatv( &smooth->nodes[ j*3 ][ (i+1)*3*8 ]->color, p1   );
            if( !is_first_column ) {
                sp_color_get_rgb_floatv( &smooth->nodes[ j*3 ][ (i-1)*3*8 ]->color, pb );
            } else {
                pb[0] = 2.0*p0[0] - p1[0];
                pb[1] = 2.0*p0[1] - p1[1];
                pb[2] = 2.0*p0[2] - p1[2];
            }
            if( !is_last_column ) {
                sp_color_get_rgb_floatv( &smooth->nodes[ j*3 ][ (i+2)*3*8 ]->color, pa );
            } else {
                pa[0] = 2.0*p1[0] - p0[0];
                pa[1] = 2.0*p1[1] - p0[1];
                pa[2] = 2.0*p1[2] - p0[2];
            }

            for( unsigned n = 0; n < 3; ++n ) { // Loop over colors

                // We use Hermite interpolation. We have end points, we need tangents.
                double m0 = 0;
                double m1 = 0;
                find_slopes( pb[n], p0[n], p1[n], pa[n], is_first_column, is_last_column, type, m0, m1 );
        
                for( unsigned k = 1; k < 8; ++k ) {
                    double t = k/8.0;
                    // Cubic Hermite (four constraints)
                    result[n][k] = hermite( p0[n], p1[n], m0, m1, t );
                    // Clamp to allowed values
                    if( result[n][k] > 1.0 )
                        result[n][k] = 1.0;
                    if( result[n][k] < 0.0 )
                        result[n][k] = 0.0;
                }
            }

            for( unsigned k = 1; k < 8; ++k ) {
                smooth->nodes[ j*3 ][ (i*8+k)*3 ]->color.set( result[0][k], result[1][k], result[2][k] ); 
            }
        }
    }
}

class SPMeshSmoothCorner {

public:
    SPMeshSmoothCorner() {
        for( unsigned i = 0; i < 3; ++i ) {
            for( unsigned j = 0; j < 4; ++j ) {
                g[i][j] = 0;
            }
        }
    }
    
    double g[3][4]; // 3 colors, 4 parameters: f, f_x, f_y, f_xy
    // double x[4]; // Lengths between neigboring points  x, y, xy, yx
};

// Find slope at point 1 given values at previous and next points
double find_slope1( double p0, double p1, double p2 ) {

    double slope = (p2 - p0)/2.0;  // Simple Catmul-Rom condition
    if( ( p0 > p1 && p1 < p2 ) ||
        ( p0 < p1 && p1 > p2 ) ) {
        // At minimum or maximum, use slope of zero
        slope = 0;
    } else {
        // Ensure we don't overshoot
        if( fabs(slope) > fabs(3*(p1-p0)) ) {
            slope = 3*(p1-p0);
        }
        if( fabs(slope) > fabs(3*(p2-p1)) ) {
            slope = 3*(p2-p1);
        }
    }
    return slope;
};


// Find slope at point 0 given values at previous and next points
double find_slope2( double pmm, double ppm, double pmp, double ppp, double p0 ) {

    // pmm == d[i-1][j-1], ...  'm' is minus, 'p' is plus
    double slope = (ppp - ppm - pmp + pmm)/2.0;
    if( (ppp > p0 && ppm > p0 && pmp > p0 && pmm > 0) ||
        (ppp < p0 && ppm < p0 && pmp < p0 && pmm < 0) ) {
        // At minimum or maximum, use slope of zero
        slope = 0;
    } else {
        // Don't really know what to do here
        if( fabs(slope) > fabs(3*(ppp-p0)) ) {
            slope = 3*(ppp-p0);
        }
        if( fabs(slope) > fabs(3*(pmp-p0)) ) {
            slope = 3*(pmp-p0);
        }
        if( fabs(slope) > fabs(3*(ppm-p0)) ) {
            slope = 3*(ppm-p0);
        }
        if( fabs(slope) > fabs(3*(pmm-p0)) ) {
            slope = 3*(pmm-p0);
        }
    }
    return slope;
}

// https://en.wikipedia.org/wiki/Bicubic_interpolation
void invert( const double v[16], double alpha[16] ) {

    const double  A[16][16] = {

        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 },
        { 0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 },
        {-3, 3, 0, 0, -2,-1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 },
        { 2,-2, 0, 0,  1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 },
        { 0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0 },
        { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0 },
        { 0, 0, 0, 0,  0, 0, 0, 0, -3, 3, 0, 0, -2,-1, 0, 0 },
        { 0, 0, 0, 0,  0, 0, 0, 0,  2,-2, 0, 0,  1, 1, 0, 0 },
        {-3, 0, 3, 0,  0, 0, 0, 0, -2, 0,-1, 0,  0, 0, 0, 0 },
        { 0, 0, 0, 0, -3, 0, 3, 0,  0, 0, 0, 0, -2, 0,-1, 0 },
        { 9,-9,-9, 9,  6, 3,-6,-3,  6,-6, 3,-3,  4, 2, 2, 1 },
        {-6, 6, 6,-6, -3,-3, 3, 3, -4, 4,-2, 2, -2,-2,-1,-1 },
        { 2, 0,-2, 0,  0, 0, 0, 0,  1, 0, 1, 0,  0, 0, 0, 0 },
        { 0, 0, 0, 0,  2, 0,-2, 0,  0, 0, 0, 0,  1, 0, 1, 0 },
        {-6, 6, 6,-6, -4,-2, 4, 2, -3, 3,-3, 3, -2,-1,-2,-1 },
        { 4,-4,-4, 4,  2, 2,-2,-2,  2,-2, 2,-2,  1, 1, 1, 1 }
    };

    for( unsigned i = 0; i < 16; ++i ) {
        alpha[i] = 0;
        for( unsigned j = 0; j < 16; ++j ) {
            alpha[i] += A[i][j]*v[j];
        }
    }
}

double sum( const double alpha[16], const double& x, const double& y ) {

    double result = 0;
    
    double xx = x*x;
    double xxx = xx * x;
    double yy = y*y;
    double yyy = yy * y;

    result += alpha[  0 ];
    result += alpha[  1 ] * x;
    result += alpha[  2 ] * xx;
    result += alpha[  3 ] * xxx;
    result += alpha[  4 ] * y;
    result += alpha[  5 ] * y * x;
    result += alpha[  6 ] * y * xx;
    result += alpha[  7 ] * y * xxx;
    result += alpha[  8 ] * yy;
    result += alpha[  9 ] * yy * x;
    result += alpha[ 10 ] * yy * xx;
    result += alpha[ 11 ] * yy * xxx;
    result += alpha[ 12 ] * yyy;
    result += alpha[ 13 ] * yyy * x;
    result += alpha[ 14 ] * yyy * xx;
    result += alpha[ 15 ] * yyy * xxx;
    
    return result;
}

/**
   Fill 'smooth' with a smoothed version of the array by subdividing each patch into smaller patches.
*/
void SPMeshNodeArray::smooth2( SPMeshNodeArray* smooth, SPMeshSmooth type ) {

    
    *smooth = *this;  // Deep copy via copy assignment constructor, smooth cleared before copy
    // std::cout << "SPMeshNodeArray::smooth2(): " << this->patch_rows() << " " << smooth->patch_columns() << std::endl;
    // std::cout << "  " << smooth << " " << this << std::endl;

    // Find derivatives at corners

    // Create array of corner points
    std::vector< std::vector <SPMeshSmoothCorner> > d; 
    d.resize( smooth->patch_rows() + 1 );
    for( unsigned i = 0; i < d.size(); ++i ) {
        d[i].resize( smooth->patch_columns() + 1 );
        for( unsigned j = 0; j < d[i].size(); ++j ) {
            float rgb_color[3];
            sp_color_get_rgb_floatv( &this->nodes[ i*3 ][ j*3 ]->color, rgb_color );
            d[i][j].g[0][0] =  rgb_color[ 0 ];
            d[i][j].g[1][0] =  rgb_color[ 1 ];
            d[i][j].g[2][0] =  rgb_color[ 2 ];
        }
    }

    // Calculate interior derivatives
    for( unsigned i = 1; i < d.size()-1; ++i ) {
        for( unsigned j = 1; j < d[i].size()-1; ++j ) {
            for( unsigned k = 0; k < 3; ++k ) { // Loop over colors
                if( type == SP_MESH_SMOOTH_SMOOTH7 || type == SP_MESH_SMOOTH_SMOOTH ) {
                    d[i][j].g[k][1] = find_slope1( d[i-1][j].g[k][0], d[i][j].g[k][0], d[i+1][j].g[k][0] );
                    d[i][j].g[k][2] = find_slope1( d[i][j-1].g[k][0], d[i][j].g[k][0], d[i][j+1].g[k][0] );
                    d[i][j].g[k][3] = find_slope2( d[i-1][j-1].g[k][0], d[i+1][j-1].g[k][0],
                                                   d[i-1][j+1].g[k][0], d[i-1][j-1].g[k][0],
                                                   d[i][j].g[k][0] );
                } else {
                    // Catmul-Rom
                    d[i][j].g[k][1] = (d[i+1][j].g[k][0] - d[i-1][j].g[k][0])/2.0;
                    d[i][j].g[k][2] = (d[i][j+1].g[k][0] - d[i][j-1].g[k][0])/2.0;
                }
            }
        }
    }
    
    // Calculate exterior derivatives
    for( unsigned j = 1; j< d[0].size()-1; ++j ) {
        for( unsigned k = 0; k < 3; ++k ) { // Loop over colors
            unsigned z = d.size()-1;
            if( type == SP_MESH_SMOOTH_SMOOTH7 || type == SP_MESH_SMOOTH_SMOOTH ) {
                // Parabolic
                d[0][j].g[k][1] = 2.0*(d[1][j].g[k][0] - d[0  ][j].g[k][0]) - d[1][j].g[k][1];
                d[z][j].g[k][1] = 2.0*(d[z][j].g[k][0] - d[z-1][j].g[k][0]) - d[z][j].g[k][1];
            } else {
                // Catmul-Rom
                d[0][j].g[k][1] = (d[1][j].g[k][0] - d[0  ][j].g[k][0])/2.0;
                d[z][j].g[k][1] = (d[z][j].g[k][0] - d[z-1][j].g[k][0])/2.0;
            }
        }
    }

    for( unsigned i = 1; i< d.size()-1; ++i ) {
        for( unsigned k = 0; k < 3; ++k ) { // Loop over colors
            unsigned z = d[0].size()-1;
            if( type == SP_MESH_SMOOTH_SMOOTH7 || type == SP_MESH_SMOOTH_SMOOTH ) {
                // Parabolic
                d[i][0].g[k][2] = 2.0*(d[i][1].g[k][0] - d[i][0  ].g[k][0]) - d[i][0].g[k][1];
                d[i][z].g[k][2] = 2.0*(d[i][z].g[k][0] - d[i][z-1].g[k][0]) - d[i][z].g[k][1];
            } else {
                // Catmul-Rom
                d[i][0].g[k][2] = (d[i][1].g[k][0] - d[i][0  ].g[k][0])/2.0;
                d[i][z].g[k][2] = (d[i][z].g[k][0] - d[i][z-1].g[k][0])/2.0;
            }
        }
    }

    // Leave outside corner derivatives at zero.
    
    // Next split each patch into 8x8 smaller patches.
    
    // Split each row into eight rows.
    // Must do it from end so inserted rows don't mess up indexing 
    for( int i = smooth->patch_rows() - 1; i >= 0; --i ) {
        smooth->split_row( i, unsigned(8) );
    }

    // Split each column into eight columns.
    // Must do it from end so inserted columns don't mess up indexing 
    for( int i = smooth->patch_columns() - 1; i >= 0; --i ) {
        smooth->split_column( i, (unsigned)8 );
    }

    // Fill new patches
    for( unsigned i = 0; i < this->patch_rows(); ++i ) {
        for( unsigned j = 0; j < this->patch_columns(); ++j ) {

            // Temp loop over 0..8 to get last column/row edges
            float r[3][9][9]; // result
            for( unsigned m = 0; m < 3; ++m ) {

                double v[16];
                v[ 0] = d[i  ][j  ].g[m][0];
                v[ 1] = d[i+1][j  ].g[m][0];
                v[ 2] = d[i  ][j+1].g[m][0];
                v[ 3] = d[i+1][j+1].g[m][0];
                v[ 4] = d[i  ][j  ].g[m][1];
                v[ 5] = d[i+1][j  ].g[m][1];
                v[ 6] = d[i  ][j+1].g[m][1];
                v[ 7] = d[i+1][j+1].g[m][1];
                v[ 8] = d[i  ][j  ].g[m][2];
                v[ 9] = d[i+1][j  ].g[m][2];
                v[10] = d[i  ][j+1].g[m][2];
                v[11] = d[i+1][j+1].g[m][2];
                v[12] = d[i  ][j  ].g[m][3];
                v[13] = d[i+1][j  ].g[m][3];
                v[14] = d[i  ][j+1].g[m][3];
                v[15] = d[i+1][j+1].g[m][3];

                double alpha[16];
                invert( v, alpha );
                
                for( unsigned k = 0; k < 9; ++k ) {
                    for( unsigned l = 0; l < 9; ++l ) {
                        double x = k/8.0;
                        double y = l/8.0;
                        r[m][k][l] = sum( alpha, x, y );
                        // Clamp to allowed values
                        if( r[m][k][l] > 1.0 )
                            r[m][k][l] = 1.0;
                        if( r[m][k][l] < 0.0 )
                            r[m][k][l] = 0.0;
                    }
                }

            } // Loop over colors

            for( unsigned k = 0; k < 9; ++k ) {
                for( unsigned l = 0; l < 9; ++l ) {
                    // Every third node is a corner node
                    smooth->nodes[ (i*8+k)*3 ][(j*8+l)*3 ]->color.set( r[0][k][l], r[1][k][l], r[2][k][l] );
                }
            }
        }
    }
}

/**
   Number of patch rows.
*/
guint SPMeshNodeArray::patch_rows() {

    return nodes.size()/3;
}

/**
   Number of patch columns.
*/
guint SPMeshNodeArray::patch_columns() {

    return nodes[0].size()/3;
}

/**
   Inputs:
     i, j: Corner draggable indices.
   Returns:
     true if corners adjacent.
     n[] is array of nodes in top/bottom or left/right order.
*/
bool SPMeshNodeArray::adjacent_corners( guint i, guint j, SPMeshNode* n[4] ) {

    // This works as all corners have indices and they
    // are numbered in order by row and column (and
    // the node array is rectangular).

    bool adjacent = false;

    guint c1 = i;
    guint c2 = j;
    if( j < i ) {
        c1 = j;
        c2 = i;
    }

    // Number of corners in a row of patches.
    guint ncorners = patch_columns() + 1;

    guint crow1 = c1 / ncorners;
    guint crow2 = c2 / ncorners;
    guint ccol1 = c1 % ncorners;
    guint ccol2 = c2 % ncorners;

    guint nrow  = crow1 * 3;
    guint ncol  = ccol1 * 3;

    // std::cout << "  i: " << i
    //           << "  j: " << j
    //           << "  ncorners: " << ncorners
    //           << "  c1: " << c1
    //           << "  crow1: " << crow1
    //           << "  ccol1: " << ccol1
    //           << "  c2: " << c2
    //           << "  crow2: " << crow2
    //           << "  ccol2: " << ccol2
    //           << "  nrow: " << nrow
    //           << "  ncol: " << ncol
    //           << std::endl;

    // Check for horizontal neighbors
    if ( crow1 == crow2 && (ccol2 - ccol1) == 1 ) {
        adjacent = true;
        for( guint k = 0; k < 4; ++k ) {
            n[k] = nodes[nrow][ncol+k];
        }
    }

    // Check for vertical neighbors
    if ( ccol1 == ccol2 && (crow2 - crow1) == 1 ) {
        adjacent = true;
        for( guint k = 0; k < 4; ++k ) {
            n[k] = nodes[nrow+k][ncol];
        }
    }

    return adjacent;
}

/**
   Toggle sides between lineto and curve to if both corners selected.
   Input is a list of selected corner draggable indices.
*/
guint SPMeshNodeArray::side_toggle( std::vector<guint> corners ) {

    guint toggled = 0;

    if( corners.size() < 2 ) return 0;

    for( guint i = 0; i < corners.size()-1; ++i ) {
        for( guint j = i+1; j < corners.size(); ++j ) {

            SPMeshNode* n[4];
            if( adjacent_corners( corners[i], corners[j], n ) ) {

                gchar path_type = n[1]->path_type;
                switch (path_type)
                {
                    case 'L':
                        n[1]->path_type = 'C';
                        n[2]->path_type = 'C';
                        n[1]->set = true;
                        n[2]->set = true;
                        break;

                    case 'l':
                        n[1]->path_type = 'c';
                        n[2]->path_type = 'c';
                        n[1]->set = true;
                        n[2]->set = true;
                        break;
                    
                    case 'C': {
                        n[1]->path_type = 'L';
                        n[2]->path_type = 'L';
                        n[1]->set = false;
                        n[2]->set = false;
                        // 'L' acts as if handles are 1/3 of path length from corners.
                        Geom::Point dp = (n[3]->p - n[0]->p)/3.0;
                        n[1]->p = n[0]->p + dp;
                        n[2]->p = n[3]->p - dp;
                        break;
                    }
                    case 'c': {
                        n[1]->path_type = 'l';
                        n[2]->path_type = 'l';
                        n[1]->set = false;
                        n[2]->set = false;
                        // 'l' acts as if handles are 1/3 of path length from corners.
                        Geom::Point dp = (n[3]->p - n[0]->p)/3.0;
                        n[1]->p = n[0]->p + dp;
                        n[2]->p = n[3]->p - dp;
                        // std::cout << "Toggle sides: "
                        //           << n[0]->p << " "
                        //           << n[1]->p << " "
                        //           << n[2]->p << " "
                        //           << n[3]->p << " "
                        //           << dp << std::endl;
                        break;
                    }
                    default:
                        std::cout << "Toggle sides: Invalid path type: " << path_type << std::endl;
                }
                ++toggled;
            }
        }
    }
    if( toggled > 0 ) built = false;
    return toggled;
}

/**
 * Converts generic Beziers to Beziers approximating elliptical arcs, preserving handle direction.
 * There are infinite possible solutions. The solution chosen here is to generate a section of an
 * ellipse that is centered on the intersection of the two lines passing through the two nodes but
 * parallel to the other node's handle direction. This is the section of an ellipse that
 * corresponds to a quarter of a circle squished and then skewed.
 */
guint SPMeshNodeArray::side_arc( std::vector<guint> corners ) {

    if( corners.size() < 2 ) return 0;

    guint arced = 0;
    for( guint i = 0; i < corners.size()-1; ++i ) {
        for( guint j = i+1; j < corners.size(); ++j ) {

            SPMeshNode* n[4];
            if( adjacent_corners( corners[i], corners[j], n ) ) {

                gchar path_type = n[1]->path_type;
                switch (path_type)
                {
                    case 'L':
                    case 'l':
                        std::cout << "SPMeshNodeArray::arc_sides: Can't convert straight lines to arcs.";
                        break;

                    case 'C':
                    case 'c': {

                        Geom::Ray  ray1( n[0]->p, n[1]->p );
                        Geom::Ray  ray2( n[3]->p, n[2]->p );
                        if( !are_parallel( (Geom::Line)ray1, (Geom::Line)ray2 ) ) {

                            Geom::OptCrossing crossing = intersection( ray1, ray2 );

                            if( crossing ) {

                                Geom::Point intersection = ray1.pointAt( (*crossing).ta );

                                const double f = 4.0/3.0 * tan( M_PI/2.0/4.0 );

                                Geom::Point h1 = intersection - n[0]->p;
                                Geom::Point h2 = intersection - n[3]->p;

                                n[1]->p = n[0]->p + f*h1;
                                n[2]->p = n[3]->p + f*h2;
                                ++arced;

                            } else {
                                std::cout << "SPMeshNodeArray::arc_sides: No crossing, can't turn into arc." << std::endl;
                            }
                        } else {
                            std::cout << "SPMeshNodeArray::arc_sides: Handles parallel, can't turn into arc." << std::endl;
                        }
                        break;
                    }
                    default:
                        std::cout << "SPMeshNodeArray::arc_sides: Invalid path type: " << n[1]->path_type << std::endl;
                }
            }
        }
    }
    if( arced > 0 ) built = false;
    return arced;
}

/**
   Toggle sides between lineto and curve to if both corners selected.
   Input is a list of selected corner draggable indices.
*/
guint SPMeshNodeArray::tensor_toggle( std::vector<guint> corners ) {

    // std::cout << "SPMeshNodeArray::tensor_toggle" << std::endl;

    if( corners.size() < 4 ) return 0;

    guint toggled = 0;

    // Number of corners in a row of patches.
    guint ncorners = patch_columns() + 1;

    for( guint i = 0; i < corners.size()-3; ++i ) {
        for( guint j = i+1; j < corners.size()-2; ++j ) {
            for( guint k = j+1; k < corners.size()-1; ++k ) {
                for( guint l = k+1; l < corners.size(); ++l ) {

                    guint c[4];
                    c[0] = corners[i];
                    c[1] = corners[j];
                    c[2] = corners[k];
                    c[3] = corners[l];
                    std::sort( c, c+4 );

                    // Check we have four corners of one patch selected
                    if( c[1]-c[0] == 1 &&
                        c[3]-c[2] == 1 &&
                        c[2]-c[0] == ncorners &&
                        c[3]-c[1] == ncorners &&
                        c[0] % ncorners < ncorners - 1 ) {

                        // Patch
                        guint prow = c[0] / ncorners;
                        guint pcol = c[0] % ncorners;

                        // Upper left node of patch
                        guint irow = prow * 3;
                        guint jcol = pcol * 3;

                        // std::cout << "tensor::toggle: "
                        //           << c[0] << ", "
                        //           << c[1] << ", "
                        //           << c[2] << ", "
                        //           << c[3] << std::endl;

                        // std::cout << "tensor::toggle: "
                        //           << " irow: " << irow
                        //           << " jcol: " << jcol
                        //           << " prow: " << prow
                        //           << " pcol: " << pcol
                        //           << std::endl;

                        SPMeshPatchI patch( &nodes, prow, pcol );
                        patch.updateNodes();

                        if( patch.tensorIsSet() ) {
                            // Unset tensor points
                            nodes[irow+1][jcol+1]->set = false;
                            nodes[irow+1][jcol+2]->set = false;
                            nodes[irow+2][jcol+1]->set = false;
                            nodes[irow+2][jcol+2]->set = false;
                        } else {
                            // Set tensor points
                            nodes[irow+1][jcol+1]->set = true;
                            nodes[irow+1][jcol+2]->set = true;
                            nodes[irow+2][jcol+1]->set = true;
                            nodes[irow+2][jcol+2]->set = true;
                        }

                        ++toggled;
                    }
                }
            }
        }
    }
    if( toggled > 0 ) built = false;
    return toggled;
}

/**
   Atempts to smooth color transitions across corners.
   Input is a list of selected corner draggable indices.
*/
guint SPMeshNodeArray::color_smooth( std::vector<guint> corners ) {

    // std::cout << "SPMeshNodeArray::color_smooth" << std::endl;

    guint smoothed = 0;

    // Number of corners in a row of patches.
    guint ncorners = patch_columns() + 1;

    // Number of node rows and columns
    guint ncols = patch_columns() * 3 + 1;
    guint nrows = patch_rows() * 3 + 1;

    for( guint i = 0; i < corners.size(); ++i ) {

        guint corner = corners[i];
        // std::cout << "SPMeshNodeArray::color_smooth: " << i << " " << corner << std::endl;

        // Node row & col
        guint nrow = (corner / ncorners) * 3;
        guint ncol = (corner % ncorners) * 3;

        SPMeshNode* n[7];
        for( guint s = 0; s < 2; ++s ) {

            bool smooth = false;

            // Find neighboring nodes
            if( s == 0 ) {

                // Horizontal
                if( ncol > 2 && ncol+3 < ncols) {
                    for( guint j = 0; j < 7; ++j ) {
                        n[j] = nodes[ nrow ][ ncol - 3 + j ];
                    }
                    smooth = true;
                }

            } else {

                // Vertical
                if( nrow > 2 && nrow+3 < nrows) {
                    for( guint j = 0; j < 7; ++j ) {
                        n[j] = nodes[ nrow - 3 + j ][ ncol ];
                    }
                    smooth = true;
                }
            }

            if( smooth ) {

                // Let the smoothing begin
                // std::cout << " checking: " << ncol << " " << nrow << std::endl;

                // Get initial slopes using closest handles.
                double slope[2][3];
                double slope_ave[3];
                double slope_diff[3];

                // Color of corners
                SPColor color0 = n[0]->color;
                SPColor color3 = n[3]->color;
                SPColor color6 = n[6]->color;
                
                // Distance nodes from selected corner
                Geom::Point d[7];
                for( guint k = 0; k < 7; ++k ) {
                    d[k]= n[k]->p - n[3]->p;
                    // std::cout << " d[" << k << "]: " << d[k].length() << std::endl;
                }

                double sdm = -1.0; // Slope Diff Max
                guint cdm = 0; // Color Diff Max  (Which color has the maximum difference in slopes)
                for( guint c = 0; c < 3; ++c ) {
                    if( d[2].length() != 0.0 ) {
                        slope[0][c] = (color3.v.c[c] - color0.v.c[c]) / d[2].length();
                    } 
                    if( d[4].length() != 0.0 ) {
                        slope[1][c] = (color6.v.c[c] - color3.v.c[c]) / d[4].length();
                    }
                    slope_ave[c]  = (slope[0][c]+slope[1][c]) / 2.0;
                    slope_diff[c] = (slope[0][c]-slope[1][c]);
                    // std::cout << "  color: " << c << " :"
                    //           << color0.v.c[c] << " "
                    //           << color3.v.c[c] << " "
                    //           << color6.v.c[c]
                    //           << "  slope: "
                    //           << slope[0][c] << " "
                    //           << slope[1][c]
                    //           << "  slope_ave: " << slope_ave[c]
                    //           << "  slope_diff: " << slope_diff[c]
                    //           << std::endl;

                    // Find color with maximum difference
                    if( std::abs( slope_diff[c] ) > sdm ) {
                        sdm = std::abs( slope_diff[c] );
                        cdm = c;
                    }
                }
                // std::cout << " cdm: " << cdm << std::endl;

                // Find new handle positions:
                double length_left  = d[0].length();
                double length_right = d[6].length();
                if( slope_ave[ cdm ] != 0.0 ) {
                    length_left  = std::abs( (color3.v.c[cdm] - color0.v.c[cdm]) / slope_ave[ cdm ] );
                    length_right = std::abs( (color6.v.c[cdm] - color3.v.c[cdm]) / slope_ave[ cdm ] );
                }

                // Move closest handle a maximum of mid point... but don't shorten
                double max = 0.8;
                if( length_left  > max * d[0].length() && length_left > d[2].length() ) {
                    std::cout << " Can't smooth left side" << std::endl;
                    length_left  = std::max( max * d[0].length(), d[2].length() );
                }
                if( length_right  > max * d[6].length() && length_right > d[4].length() ) {
                    std::cout << " Can't smooth right side" << std::endl;
                    length_right  = std::max( max * d[6].length(), d[4].length() );
                }

                if( d[2].length() != 0.0 ) d[2] *= length_left/d[2].length();
                if( d[4].length() != 0.0 ) d[4] *= length_right/d[4].length();

                // std::cout << "  length_left: " << length_left
                //           << "  d[0]: " << d[0].length()
                //           << "  length_right: " << length_right
                //           << "  d[6]: " << d[6].length()
                //           << std::endl;

                n[2]->p = n[3]->p + d[2];
                n[4]->p = n[3]->p + d[4];

                ++smoothed;
            }
        }

    }

    if( smoothed > 0 ) built = false;
    return smoothed;
}

/**
   Pick color from background for selected corners.
*/
guint SPMeshNodeArray::color_pick( std::vector<guint> icorners, SPItem* item ) {

    // std::cout << "SPMeshNodeArray::color_pick" << std::endl;

    guint picked = 0;

    // Code inspired from clone tracing

    // Setup...

    // We need a copy of the drawing so we can hide the mesh.
    Inkscape::Drawing *pick_drawing = new Inkscape::Drawing();
    unsigned pick_visionkey = SPItem::display_key_new(1);

    SPDocument *pick_doc = mg->document;

    pick_drawing->setRoot(pick_doc->getRoot()->invoke_show(*pick_drawing, pick_visionkey, SP_ITEM_SHOW_DISPLAY));

    item->invoke_hide(pick_visionkey);

    pick_doc->getRoot()->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    pick_doc->ensureUpToDate();

    //gdouble pick_zoom = 1.0; // zoom;
    //pick_drawing->root()->setTransform(Geom::Scale(pick_zoom));
    pick_drawing->update();

    // std::cout << " transform: " << std::endl;
    // std::cout << item->transform << std::endl;
    // std::cout << " i2doc: " << std::endl;
    // std::cout << item->i2doc_affine() << std::endl;
    // std::cout << " i2dt: " << std::endl;
    // std::cout << item->i2dt_affine() << std::endl;
    // std::cout << " dt2i: " << std::endl;
    // std::cout << item->dt2i_affine() << std::endl;
    SPGradient* gr = SP_GRADIENT( mg );
    // if( gr->gradientTransform_set ) {
    //     std::cout << " gradient transform set: " << std::endl;
    //     std::cout << gr->gradientTransform << std::endl;
    // } else {
    //     std::cout << " gradient transform not set! " << std::endl;
    // }

    // Do picking
    for( guint i = 0; i < icorners.size(); ++i ) {

        guint corner = icorners[i];

        SPMeshNode* n = corners[ corner ];

        // Region to average over
        Geom::Point p = n->p;
        // std::cout << " p: " << p << std::endl;
        p *= gr->gradientTransform;
        // std::cout << " p: " << p << std::endl;

        // If on edge, move inward
        guint cols = patch_columns()+1;
        guint rows = patch_rows()+1;
        guint col = corner % cols;
        guint row = corner / cols;
        guint ncol  = col * 3;
        guint nrow  = row * 3;

        double size = 3.0;

        // Top edge
        if( row == 0 ) {
            Geom::Point dp = nodes[nrow+1][ncol]->p - p;
            p += unit_vector( dp ) * size;
        }
        // Right edge
        if( col == cols-1 ) {
            Geom::Point dp = nodes[nrow][ncol-1]->p - p;
            p += unit_vector( dp ) * size;
        }
        // Bottom edge
        if( row == rows-1 ) {
            Geom::Point dp = nodes[nrow-1][ncol]->p - p;
            p += unit_vector( dp ) * size;
        }
        // Left edge
        if( col == 0 ) {
            Geom::Point dp = nodes[nrow][ncol+1]->p - p;
            p += unit_vector( dp ) * size;
        }

        Geom::Rect box( p[Geom::X]-size/2.0, p[Geom::Y]-size/2.0,
                        p[Geom::X]+size/2.0, p[Geom::Y]+size/2.0 );

        /* Item integer bbox in points */
        Geom::IntRect ibox = box.roundOutwards();

        /* Find visible area */
        cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, ibox.width(), ibox.height());
        Inkscape::DrawingContext dc(s, ibox.min());

        /* Render copy and pick color */
        pick_drawing->render(dc, ibox);
        double R = 0, G = 0, B = 0, A = 0;
        ink_cairo_surface_average_color(s, R, G, B, A);
        cairo_surface_destroy(s);

        // std::cout << " p: " << p
        //           << " box: " << ibox
        //           << " R: " << R
        //           << " G: " << G
        //           << " B: " << B
        //           << std::endl;
        n->color.set( R, G, B );
    }

    pick_doc->getRoot()->invoke_hide(pick_visionkey);
    delete pick_drawing;

    if( picked > 0 ) built = false;
    return picked;
}

/**
   Moves handles in response to a corner node move.
   p_old: orignal position of moved corner node.
   corner: the corner node moved (draggable index, i.e. point_i).
   selected: list of all corners selected (draggable indices).
   op: how other corners should be moved.
*/
void SPMeshNodeArray::update_handles( guint corner, std::vector< guint > /*selected*/, Geom::Point p_old, MeshNodeOperation /*op*/ )
{
    assert( drag_valid );

    // std::cout << "SPMeshNodeArray::update_handles: "
    //           << "  corner: " << corner
    //           << "  op: " << op
    //           << std::endl;

    // Find number of patch rows and columns
    guint mrow = patch_rows();
    guint mcol = patch_columns();

    // Number of corners in a row of patches.
    guint ncorners = mcol + 1;

    // Find corner row/column
    guint crow = corner / ncorners;
    guint ccol = corner % ncorners;

    // Find node row/column
    guint nrow  = crow * 3;
    guint ncol  = ccol * 3;

    // std::cout << "  mrow: " << mrow
    //           << "  mcol: " << mcol
    //           << "  crow: " << crow
    //           << "  ccol: " << ccol
    //           << "  ncorners: " << ncorners
    //           << "  nrow: " << nrow
    //           << "  ncol: " << ncol
    //           << std::endl;

    // New corner mesh coordinate.
    Geom::Point p_new = nodes[nrow][ncol]->p;

    // Corner point move dpg in mesh coordinate system.
    Geom::Point dp = p_new - p_old;

    // std::cout << "  p_old: " << p_old << std::endl;
    // std::cout << "  p_new: " << p_new << std::endl;
    // std::cout << "     dp: " << dp << std::endl;

    // STEP 1: ONLY DO DIRECT MOVE
    bool patch[4];
    patch[0] = patch[1] = patch[2] = patch[3] = false;
    if( ccol > 0    && crow > 0    ) patch[0] = true;
    if( ccol < mcol && crow > 0    ) patch[1] = true;
    if( ccol < mcol && crow < mrow ) patch[2] = true;
    if( ccol > 0    && crow < mrow ) patch[3] = true;

    // std::cout << patch[0] << " "
    //           << patch[1] << " "
    //           << patch[2] << " "
    //           << patch[3] << std::endl;

    // Move handles
    if( patch[0] || patch[1] ) {
        if( nodes[nrow-1][ncol]->path_type == 'l' ||
            nodes[nrow-1][ncol]->path_type == 'L' ) {
            Geom::Point s = (nodes[nrow-3][ncol]->p - nodes[nrow][ncol]->p)/3.0;
            nodes[nrow-1][ncol  ]->p = nodes[nrow][ncol]->p + s;
            nodes[nrow-2][ncol  ]->p = nodes[nrow-3][ncol]->p - s;
        } else {
            nodes[nrow-1][ncol  ]->p += dp;
        }
    }

    if( patch[1] || patch[2] )  {
        if( nodes[nrow  ][ncol+1]->path_type == 'l' ||
            nodes[nrow  ][ncol+1]->path_type == 'L' ) {
            Geom::Point s = (nodes[nrow][ncol+3]->p - nodes[nrow][ncol]->p)/3.0;
            nodes[nrow  ][ncol+1]->p = nodes[nrow][ncol]->p + s;
            nodes[nrow  ][ncol+2]->p = nodes[nrow][ncol+3]->p - s;
        } else {
            nodes[nrow  ][ncol+1]->p += dp;
        }
    }

    if( patch[2] || patch[3] ) {
        if( nodes[nrow+1][ncol  ]->path_type == 'l' ||
            nodes[nrow+1][ncol  ]->path_type == 'L' ) {
            Geom::Point s = (nodes[nrow+3][ncol]->p - nodes[nrow][ncol]->p)/3.0;
            nodes[nrow+1][ncol  ]->p = nodes[nrow][ncol]->p + s;
            nodes[nrow+2][ncol  ]->p = nodes[nrow+3][ncol]->p - s;
        } else {
            nodes[nrow+1][ncol  ]->p += dp;
        }
    }

    if( patch[3] || patch[0] ) {
        if( nodes[nrow  ][ncol-1]->path_type == 'l' ||
            nodes[nrow  ][ncol-1]->path_type == 'L' ) {
            Geom::Point s = (nodes[nrow][ncol-3]->p - nodes[nrow][ncol]->p)/3.0;
            nodes[nrow  ][ncol-1]->p = nodes[nrow][ncol]->p + s;
            nodes[nrow  ][ncol-2]->p = nodes[nrow][ncol-3]->p - s;
        } else {
            nodes[nrow  ][ncol-1]->p += dp;
        }
    }


    // Move tensors
    if( patch[0] ) nodes[nrow-1][ncol-1]->p += dp;
    if( patch[1] ) nodes[nrow-1][ncol+1]->p += dp;
    if( patch[2] ) nodes[nrow+1][ncol+1]->p += dp;
    if( patch[3] ) nodes[nrow+1][ncol-1]->p += dp;

    // // Check if neighboring corners are selected.

    // bool do_scale = false;

    // bool do_scale_xp = do_scale;
    // bool do_scale_xn = do_scale;
    // bool do_scale_yp = do_scale;
    // bool do_scale_yn = do_scale;

    // if( ccol < mcol+1 ) {
    //     if( std::find( sc.begin(), sc.end(), point_i + 1 ) != sc.end() ) {
    //         do_scale_xp = false;
    //         std::cout << "  Not scaling x+" << std::endl;
    //     }
    // }                        

    // if( ccol > 0 ) {
    //     if( std::find( sc.begin(), sc.end(), point_i - 1 ) != sc.end() ) {
    //         do_scale_xn = false;
    //         std::cout << "  Not scaling x-" << std::endl;
    //     }
    // }                        

    // if( crow < mrow+1 ) {
    //     if( std::find( sc.begin(), sc.end(), point_i + ncorners ) != sc.end() ) {
    //         do_scale_yp = false;
    //         std::cout << "  Not scaling y+" << std::endl;
    //     }
    // }                        

    // if( crow > 0 ) {
    //     if( std::find( sc.begin(), sc.end(), point_i - ncorners ) != sc.end() ) {
    //         do_scale_yn = false;
    //         std::cout << "  Not scaling y-" << std::endl;
    //     }
    // }                        

    // // We have four patches to adjust...
    // for ( guint k = 0; k < 4;  ++k ) {

    //     bool do_scale_x = do_scale;
    //     bool do_scale_y = do_scale;

    //     SPMeshNode* pnodes[4][4];

    //     // Load up matrix
    //     switch (k) {

    //         case 0:
    //             if( crow < mrow+1 && ccol < mcol+1 ) {
    //                 // Bottom right patch

    //                 do_scale_x = do_scale_xp;
    //                 do_scale_y = do_scale_yp;

    //                 for( guint i = 0; i < 4; ++i ) {
    //                     for( guint j = 0; j< 4; ++j ) {
    //                         pnodes[i][j] = mg->array.nodes[nrow+i][nrow+j];
    //                     }
    //                 }
    //             }
    //             break;

    //         case 1:
    //             if( crow < mrow+1 && ccol > 0 ) {
    //                 // Bottom left patch (note x, y swapped)

    //                 do_scale_y = do_scale_xn;
    //                 do_scale_x = do_scale_yp;

    //                 for( guint i = 0; i < 4; ++i ) {
    //                     for( guint j = 0; j< 4; ++j ) {
    //                         pnodes[j][i] = mg->array.nodes[nrow+i][nrow-j];
    //                     }
    //                 }
    //             }
    //             break;

    //         case 2:
    //             if( crow > 0 && ccol > 0 ) {
    //                 // Top left patch

    //                 do_scale_x = do_scale_xn;
    //                 do_scale_y = do_scale_yn;

    //                 for( guint i = 0; i < 4; ++i ) {
    //                     for( guint j = 0; j< 4; ++j ) {
    //                         pnodes[i][j] = mg->array.nodes[nrow-i][nrow-j];
    //                     }
    //                 }
    //             }
    //             break;

    //         case 3:
    //             if( crow > 0 && ccol < mcol+1 ) {
    //                 // Top right patch (note x, y swapped)

    //                 do_scale_y = do_scale_xp;
    //                 do_scale_x = do_scale_yn;

    //                 for( guint i = 0; i < 4; ++i ) {
    //                     for( guint j = 0; j< 4; ++j ) {
    //                         pnodes[j][i] = mg->array.nodes[nrow-i][nrow+j];
    //                     }
    //                 }
    //             }
    //             break;
    //     }

    //     // Now we must move points in both x and y.
    //     // There are upto six points to move: P01, P02, P11, P12, P21, P22.
    //     // (The points P10, P20 will be moved in another branch of the loop.
    //     // The points P03, P13, P23, P33, P32, P31, P30 are not moved.)
    //     //
    //     //   P00  P01  P02  P03
    //     //   P10  P11  P12  P13
    //     //   P20  P21  P22  P23
    //     //   P30  P31  P32  P33
    //     //
    //     // The goal is to preserve the direction of the handle!


    //     Geom::Point dsx_new = pnodes[0][3]->p - pnodes[0][0]->p; // New side x
    //     Geom::Point dsy_new = pnodes[3][0]->p - pnodes[0][0]->p; // New side y
    //     Geom::Point dsx_old = pnodes[0][3]->p - pcg_old; // Old side x
    //     Geom::Point dsy_old = pnodes[3][0]->p - pcg_old; // Old side y


    //     double scale_factor_x = 1.0;
    //     if( dsx_old.length() != 0.0 ) scale_factor_x = dsx_new.length()/dsx_old.length();

    //     double scale_factor_y = 1.0;
    //     if( dsy_old.length() != 0.0 ) scale_factor_y = dsy_new.length()/dsy_old.length();


    //     if( do_scalex && do_scaley ) {

    //         // We have six point to move.

    //         // P01
    //         Geom::Point dp01 = pnodes[0][1] - pcg_old;
    //         dp01 *= scale_factor_x;
    //         pnodes[0][1] = pnodes[0][0] + dp01;

    //         // P02
    //         Geom::Point dp02 = pnodes[0][2] - pnodes[0][3];
    //         dp02 *= scale_factor_x;
    //         pnodes[0][2] = pnodes[0][3] + dp02;

    //         // P11
    //         Geom::Point dp11 = pnodes[1][1] - pcg_old;
    //         dp11 *= scale_factor_x;
    //         pnodes[1][1] = pnodes[0][0] + dp11;



    //         // P21
    //         Geom::Point dp21 = pnodes[2][1] - pnodes[3][0];
    //         dp21 *= scale_factor_x;
    //         dp21 *= scale_factor_y;
    //         pnodes[2][1] = pnodes[3][0] + dp21;


    //         Geom::Point dsx1 = pnodes[0][1]->p - 
}


// Defined in gradient-chemistry.cpp
guint32 average_color(guint32 c1, guint32 c2, gdouble p);

/**
   Split a row into n equal parts.
*/
void SPMeshNodeArray::split_row( guint row, guint n ) {

    double nn = n;
    if( n > 1 ) split_row( row, (nn-1)/nn );
    if( n > 2 ) split_row( row, n-1 );
}

/**
   Split a column into n equal parts.
*/
void SPMeshNodeArray::split_column( guint col, guint n ) {

    double nn = n;
    if( n > 1 ) split_column( col, (nn-1)/nn );
    if( n > 2 ) split_column( col, n-1 );
}

/**
   Split a row into two rows at coord (fraction of row height).
*/
void SPMeshNodeArray::split_row( guint row, double coord ) {

    // std::cout << "Splitting row: " << row << " at " << coord << std::endl;
    // print();
    assert( coord >= 0.0 && coord <= 1.0 );
    assert( row < patch_rows() );

    built = false;

    // First step is to ensure that handle and tensor points are up-to-date if they are not set.
    // (We can't do this on the fly as we overwrite the necessary points to do the calculation
    //  during the update.)
    for( guint j = 0; j < patch_columns(); ++ j ) {
        SPMeshPatchI patch( &nodes, row, j );
        patch.updateNodes();
    }

    // Add three new rows of empty nodes
    for( guint i = 0; i < 3; ++i ) {
        std::vector< SPMeshNode* > new_row;
        for( guint j = 0; j < nodes[0].size(); ++j ) {
            SPMeshNode* new_node = new SPMeshNode;
            new_row.push_back( new_node );
        }
        nodes.insert( nodes.begin()+3*(row+1), new_row );
    }

    guint i = 3 * row; // Convert from patch row to node row
    for( guint j = 0; j < nodes[i].size(); ++j ) {

        // std::cout << "Splitting row: column: " << j << std::endl;

        Geom::Point p[4];
        for( guint k = 0; k < 4; ++k ) {
            guint n = k;
            if( k == 3 ) n = 6; // Bottom patch row has been shifted by new rows
            p[k] = nodes[i+n][j]->p;
            // std::cout << p[k] << std::endl;
        }

        Geom::BezierCurveN<3> b( p[0], p[1], p[2], p[3] );

        std::pair<Geom::BezierCurveN<3>, Geom::BezierCurveN<3> > b_new =
            b.subdivide( coord );

        // Update points
        for( guint n = 0; n < 4; ++n ) {
            nodes[i+n  ][j]->p = b_new.first[n];
            nodes[i+n+3][j]->p = b_new.second[n];
            // std::cout << b_new.first[n] << "  " << b_new.second[n] << std::endl;
        }

        if( nodes[i][j]->node_type == MG_NODE_TYPE_CORNER ) {
            // We are splitting a side

            // Path type stored in handles.
            gchar path_type = nodes[i+1][j]->path_type; 
            nodes[i+4][j]->path_type = path_type;
            nodes[i+5][j]->path_type = path_type;
            bool  set = nodes[i+1][j]->set; 
            nodes[i+4][j]->set = set;
            nodes[i+5][j]->set = set;
            nodes[i+4][j]->node_type = MG_NODE_TYPE_HANDLE;
            nodes[i+5][j]->node_type = MG_NODE_TYPE_HANDLE;

            // Color stored in corners
            guint c0 = nodes[i  ][j]->color.toRGBA32( 1.0 );
            guint c1 = nodes[i+6][j]->color.toRGBA32( 1.0 );
            gdouble o0 = nodes[i  ][j]->opacity;
            gdouble o1 = nodes[i+6][j]->opacity;
            guint cnew = average_color( c0, c1, coord );
            gdouble onew = o0 * (1.0 - coord) + o1 * coord;
            nodes[i+3][j]->color.set( cnew );
            nodes[i+3][j]->opacity = onew;
            nodes[i+3][j]->node_type = MG_NODE_TYPE_CORNER;
            nodes[i+3][j]->set = true;

        } else {
            // We are splitting a middle

            bool set = nodes[i+1][j]->set || nodes[i+2][j]->set;
            nodes[i+4][j]->set = set;
            nodes[i+5][j]->set = set;
            nodes[i+4][j]->node_type = MG_NODE_TYPE_TENSOR;
            nodes[i+5][j]->node_type = MG_NODE_TYPE_TENSOR;

            // Path type, if different, choose l -> L -> c -> C.
            gchar path_type0 = nodes[i  ][j]->path_type;
            gchar path_type1 = nodes[i+6][j]->path_type;
            gchar path_type = 'l';
            if( path_type0 == 'L' || path_type1 == 'L') path_type = 'L';
            if( path_type0 == 'c' || path_type1 == 'c') path_type = 'c';
            if( path_type0 == 'C' || path_type1 == 'C') path_type = 'C';
            nodes[i+3][j]->path_type = path_type;
            nodes[i+3][j]->node_type = MG_NODE_TYPE_HANDLE;
            if( path_type == 'c' || path_type == 'C' ) nodes[i+3][j]->set = true;

        }

        nodes[i+3][j]->node_edge = MG_NODE_EDGE_NONE;
        nodes[i+4][j]->node_edge = MG_NODE_EDGE_NONE;
        nodes[i+5][j]->node_edge = MG_NODE_EDGE_NONE;;
        if( j == 0 ) {
            nodes[i+3][j]->node_edge |= MG_NODE_EDGE_LEFT;
            nodes[i+4][j]->node_edge |= MG_NODE_EDGE_LEFT;
            nodes[i+5][j]->node_edge |= MG_NODE_EDGE_LEFT;
        }
        if( j == nodes[i].size() - 1 ) {
            nodes[i+3][j]->node_edge |= MG_NODE_EDGE_RIGHT;
            nodes[i+4][j]->node_edge |= MG_NODE_EDGE_RIGHT;
            nodes[i+5][j]->node_edge |= MG_NODE_EDGE_RIGHT;
        }
    }

    // std::cout << "Splitting row: result:" << std::endl;
    // print();
}



/**
   Split a column into two columns at coord (fraction of column width).
*/
void SPMeshNodeArray::split_column( guint col, double coord ) {

    // std::cout << "Splitting column: " << col << " at " << coord << std::endl;
    // print();
    assert( coord >= 0.0 && coord <= 1.0 );
    assert( col < patch_columns() );

    built = false;

    // First step is to ensure that handle and tensor points are up-to-date if they are not set.
    // (We can't do this on the fly as we overwrite the necessary points to do the calculation
    //  during the update.)
    for( guint i = 0; i < patch_rows(); ++ i ) {
        SPMeshPatchI patch( &nodes, i, col );
        patch.updateNodes();
    }

    guint j = 3 * col; // Convert from patch column to node column
    for( guint i = 0; i < nodes.size(); ++i ) {

        // std::cout << "Splitting column: row: " << i << std::endl;

        Geom::Point p[4];
        for( guint k = 0; k < 4; ++k ) {
            p[k] = nodes[i][j+k]->p;
        }

        Geom::BezierCurveN<3> b( p[0], p[1], p[2], p[3] );

        std::pair<Geom::BezierCurveN<3>, Geom::BezierCurveN<3> > b_new =
            b.subdivide( coord );

        // Add three new nodes
        for( guint n = 0; n < 3; ++n ) {
            SPMeshNode* new_node = new SPMeshNode;
            nodes[i].insert( nodes[i].begin()+j+3, new_node );
        }

        // Update points
        for( guint n = 0; n < 4; ++n ) {
            nodes[i][j+n]->p = b_new.first[n];
            nodes[i][j+n+3]->p = b_new.second[n];
        }

        if( nodes[i][j]->node_type == MG_NODE_TYPE_CORNER ) {
            // We are splitting a side

            // Path type stored in handles.
            gchar path_type = nodes[i][j+1]->path_type; 
            nodes[i][j+4]->path_type = path_type;
            nodes[i][j+5]->path_type = path_type;
            bool  set = nodes[i][j+1]->set; 
            nodes[i][j+4]->set = set;
            nodes[i][j+5]->set = set;
            nodes[i][j+4]->node_type = MG_NODE_TYPE_HANDLE;
            nodes[i][j+5]->node_type = MG_NODE_TYPE_HANDLE;

            // Color stored in corners
            guint c0 = nodes[i][j  ]->color.toRGBA32( 1.0 );
            guint c1 = nodes[i][j+6]->color.toRGBA32( 1.0 );
            gdouble o0 = nodes[i][j  ]->opacity;
            gdouble o1 = nodes[i][j+6]->opacity;
            guint cnew = average_color( c0, c1, coord );
            gdouble onew = o0 * (1.0 - coord) + o1 * coord;
            nodes[i][j+3]->color.set( cnew );
            nodes[i][j+3]->opacity = onew;
            nodes[i][j+3]->node_type = MG_NODE_TYPE_CORNER;
            nodes[i][j+3]->set = true;

        } else {
            // We are splitting a middle

            bool  set = nodes[i][j+1]->set || nodes[i][j+2]->set; 
            nodes[i][j+4]->set = set;
            nodes[i][j+5]->set = set;
            nodes[i][j+4]->node_type = MG_NODE_TYPE_TENSOR;
            nodes[i][j+5]->node_type = MG_NODE_TYPE_TENSOR;

            // Path type, if different, choose l -> L -> c -> C.
            gchar path_type0 = nodes[i][j  ]->path_type;
            gchar path_type1 = nodes[i][j+6]->path_type;
            gchar path_type = 'l';
            if( path_type0 == 'L' || path_type1 == 'L') path_type = 'L';
            if( path_type0 == 'c' || path_type1 == 'c') path_type = 'c';
            if( path_type0 == 'C' || path_type1 == 'C') path_type = 'C';
            nodes[i][j+3]->path_type = path_type;
            nodes[i][j+3]->node_type = MG_NODE_TYPE_HANDLE;
            if( path_type == 'c' || path_type == 'C' ) nodes[i][j+3]->set = true;

        }

        nodes[i][j+3]->node_edge = MG_NODE_EDGE_NONE;
        nodes[i][j+4]->node_edge = MG_NODE_EDGE_NONE;
        nodes[i][j+5]->node_edge = MG_NODE_EDGE_NONE;;
        if( i == 0 ) {
            nodes[i][j+3]->node_edge |= MG_NODE_EDGE_TOP;
            nodes[i][j+4]->node_edge |= MG_NODE_EDGE_TOP;
            nodes[i][j+5]->node_edge |= MG_NODE_EDGE_TOP;
        }
        if( i == nodes.size() - 1 ) {
            nodes[i][j+3]->node_edge |= MG_NODE_EDGE_BOTTOM;
            nodes[i][j+4]->node_edge |= MG_NODE_EDGE_BOTTOM;
            nodes[i][j+5]->node_edge |= MG_NODE_EDGE_BOTTOM;
        }

    }

    // std::cout << "Splitting col: result:" << std::endl;
    // print();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

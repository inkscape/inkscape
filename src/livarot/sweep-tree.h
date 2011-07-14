#ifndef INKSCAPE_LIVAROT_SWEEP_TREE_H
#define INKSCAPE_LIVAROT_SWEEP_TREE_H

#include "livarot/AVL.h"
#include <2geom/point.h>

class Shape;
class SweepEvent;
class SweepEventQueue;
class SweepTreeList;


/**
 * One node in the AVL tree of edges.
 * Note that these nodes will be stored in a dynamically allocated array, hence the Relocate() function.
 */
class SweepTree:public AVLTree
{
public:
    SweepEvent *evt[2];   ///< Intersection with the edge on the left and right (if any).

    Shape *src;   /**< Shape from which the edge comes.  (When doing boolean operation on polygons,
                   *   edges can come from 2 different polygons.)
                   */
    int bord;     ///< Edge index in the Shape.
    bool sens;    ///< true= top->bottom; false= bottom->top.
    int startPoint;   ///< point index in the result Shape associated with the upper end of the edge

    SweepTree();
    virtual ~SweepTree();

    // Inits a brand new node.
    void MakeNew(Shape *iSrc, int iBord, int iWeight, int iStartPoint);
    // changes the edge associated with this node
    // goal: reuse the node when an edge follows another, which is the most common case
    void ConvertTo(Shape *iSrc, int iBord, int iWeight, int iStartPoint);

    // Delete the contents of node.
    void MakeDelete();

    // utilites

    // the find function that was missing in the AVLTrree class
    // the return values are defined in LivarotDefs.h
    int Find(Geom::Point const &iPt, SweepTree *newOne, SweepTree *&insertL,
             SweepTree *&insertR, bool sweepSens = true);
    int Find(Geom::Point const &iPt, SweepTree *&insertL, SweepTree *&insertR);

    /// Remove sweepevents attached to this node.
    void RemoveEvents(SweepEventQueue &queue);

    void RemoveEvent(SweepEventQueue &queue, Side s);

    // overrides of the AVLTree functions, to account for the sorting in the tree
    // and some other stuff
    int Remove(SweepTreeList &list, SweepEventQueue &queue, bool rebalance = true);
    int Insert(SweepTreeList &list, SweepEventQueue &queue, Shape *iDst,
               int iAtPoint, bool rebalance = true, bool sweepSens = true);
    int InsertAt(SweepTreeList &list, SweepEventQueue &queue, Shape *iDst,
                 SweepTree *insNode, int fromPt, bool rebalance = true, bool sweepSens = true);

    /// Swap nodes, or more exactly, swap the edges in them.
    void SwapWithRight(SweepTreeList &list, SweepEventQueue &queue);

    void Avance(Shape *dst, int nPt, Shape *a, Shape *b);

    void Relocate(SweepTree *to);
};


#endif /* !INKSCAPE_LIVAROT_SWEEP_TREE_H */

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

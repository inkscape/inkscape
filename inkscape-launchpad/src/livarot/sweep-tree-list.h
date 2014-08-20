/** @file
 * @brief SweepTreeList definition
 */

#ifndef INKSCAPE_LIVAROT_SWEEP_TREE_LIST_H
#define INKSCAPE_LIVAROT_SWEEP_TREE_LIST_H

class Shape;
class SweepTree;

/**
 * The sweepline: a set of edges intersecting the current sweepline
 * stored as an AVL tree.
 */
class SweepTreeList {
public:
    int nbTree;   ///< Number of nodes in the tree.
    int const maxTree;   ///< Max number of nodes in the tree.
    SweepTree *trees;    ///< The array of nodes.
    SweepTree *racine;   ///< Root of the tree.

    SweepTreeList(int s);
    virtual ~SweepTreeList();

    SweepTree *add(Shape *iSrc, int iBord, int iWeight, int iStartPoint, Shape *iDst);
};


#endif /* !INKSCAPE_LIVAROT_SWEEP_TREE_LIST_H */

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

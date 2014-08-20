/*
 *  AVL.h
 *  nlivarot
 *
 *  Created by fred on Mon Jun 16 2003.
 *
 */

#ifndef my_avl
#define my_avl

#include <cstdlib>
#include "LivarotDefs.h"

/*
 * base class providing AVL tree functionnality, that is binary balanced tree
 * there is no Find() function because the class only deal with topological info
 * subclasses of this class have to implement a Find(), and most certainly to 
 * override the Insert() function
 */

class AVLTree
{
public:
    
    AVLTree *elem[2];

    // left most node (ie, with smallest key) in the subtree of this node
    AVLTree *Leftmost();

protected:    

    AVLTree *child[2];

    AVLTree();
    virtual ~AVLTree();
    
    // constructor/destructor meant to be called for an array of AVLTree created by malloc
    void MakeNew();
    void MakeDelete();

    // insertion of the present node in the tree
    // insertType is the insertion type (defined in LivarotDefs.h: not_found, found_exact, found_on_left, etc)
    // insertL is the node in the tree that is immediatly before the current one, NULL is the present node goes to the 
    // leftmost position. if insertType == found_exact, insertL should be the node with ak key
    // equal to that of the present node
    int Insert(AVLTree * &racine, int insertType, AVLTree *insertL,
               AVLTree * insertR, bool rebalance);

    // called when this node is relocated to a new position in memory, to update pointers to him
    void Relocate(AVLTree *to);

    // removal of the present element racine is the tree's root; it's a reference because if the
    // node is the root, removal of the node will change the root
    // rebalance==true if rebalancing is needed
    int Remove(AVLTree * &racine, bool rebalance = true);

private:

    AVLTree *parent;

    int balance;

    // insertion gruntwork.
    int Insert(AVLTree * &racine, int insertType, AVLTree *insertL, AVLTree *insertR);

    // rebalancing functions. both are recursive, but the depth of the trees we'll use should not be a problem
    // this one is for rebalancing after insertions
    int RestoreBalances(AVLTree *from, AVLTree * &racine);
    // this one is for removals
    int RestoreBalances(int diff, AVLTree * &racine);

    // startNode is the node where the rebalancing starts; rebalancing then moves up the tree to the root
    // diff is the change in "subtree height", as needed for the rebalancing
    // racine is the reference to the root, since rebalancing can change it too
    int Remove(AVLTree * &racine, AVLTree * &startNode, int &diff);
    
    void insertOn(Side s, AVLTree *of);
    void insertBetween(AVLTree *l, AVLTree *r);
    AVLTree *leaf(AVLTree *from, Side s);
    AVLTree *leafFromParent(AVLTree *from, Side s);
};

#endif

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

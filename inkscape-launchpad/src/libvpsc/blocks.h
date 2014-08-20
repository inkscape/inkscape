/*
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */

#ifndef SEEN_REMOVEOVERLAP_BLOCKS_H
#define SEEN_REMOVEOVERLAP_BLOCKS_H

#ifdef RECTANGLE_OVERLAP_LOGGING
#define LOGFILE "cRectangleOverlap.log"
#endif

#include <set>
#include <list>

namespace vpsc {

class Block;
class Variable;
class Constraint;

/**
 * A block structure defined over the variables such that each block contains
 * 1 or more variables, with the invariant that all constraints inside a block
 * are satisfied by keeping the variables fixed relative to one another.
 *
 * @todo check on this class being copy-n-paste duplicated.
 */
class Blocks : public std::set<Block*>
{
public:
    Blocks(const int n, Variable* const vs[]);

    virtual ~Blocks(void);

    /**
     * Processes incoming constraints, most violated to least, merging with the
     * neighbouring (left) block until no more violated constraints are found.
     */
    void mergeLeft(Block *r);

    /**
     * Symmetrical to mergeLeft.
     * @see mergeLeft
     */
    void mergeRight(Block *l);

    /**
     * Splits block b across constraint c into two new blocks, l and r (c's left
     * and right sides respectively).
     */
    void split(Block *b, Block *&l, Block *&r, Constraint *c);

    /**
     * Returns a list of variables with total ordering determined by the constraint 
     * DAG.
     */
    std::list<Variable*> *totalOrder();

    void cleanup();

    /**
     * Returns the cost total squared distance of variables from their desired
     * positions.
     */
    double cost();

private:
    void dfsVisit(Variable *v, std::list<Variable*> *order);

    void removeBlock(Block *doomed);

    Variable* const *vs;

    int nvs;
};

extern long blockTimeCtr;
}
#endif // SEEN_REMOVEOVERLAP_BLOCKS_H
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

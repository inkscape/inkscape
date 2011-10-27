/**
 * @file
 * Solve an instance of the "Variable Placement with Separation
 * Constraints" problem.
 */
/*
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */

//
// TODO: Really, we should have three classes: VPSC, IncrementalVPSC and
// StaticVPSC, where the latter two inherit from VPSC.  StaticVPSC would be
// the equivalent of what is currently VPSC.
// Also, a lot of the code specific to one or other of these concrete
// implementations should be moved from Block and Blocks: e.g. mergeLeft etc.
//
#ifndef SEEN_REMOVEOVERLAP_SOLVE_VPSC_H
#define SEEN_REMOVEOVERLAP_SOLVE_VPSC_H

#include <vector>

namespace vpsc {
class Variable;
class Constraint;
class Blocks;

/**
 * Variable Placement with Separation Constraints problem instance
 */
class Solver {
public:

    /**
     * Produces a feasible - though not necessarily optimal - solution by
     * examining blocks in the partial order defined by the directed acyclic
     * graph of constraints. For each block (when processing left to right) we
     * maintain the invariant that all constraints to the left of the block
     * (incoming constraints) are satisfied. This is done by repeatedly merging
     * blocks into bigger blocks across violated constraints (most violated
     * first) fixing the position of variables inside blocks relative to one
     * another so that constraints internal to the block are satisfied.
     */
    virtual void satisfy();

    /**
     * Calculate the optimal solution. After using satisfy() to produce a
     * feasible solution, refine() examines each block to see if further
     * refinement is possible by splitting the block. This is done repeatedly
     * until no further improvement is possible.
     */
    virtual void solve();

	Solver(const unsigned n, Variable* const vs[], const unsigned m, Constraint *cs[]);
	virtual ~Solver();
	Constraint** getConstraints(unsigned &m) { m=this->m; return cs; }
	const Variable* const * getVariables(unsigned &n) { n=this->n; return vs; }
protected:
	Blocks *bs;
	unsigned m;
	Constraint **cs;
	unsigned n;
	const Variable* const *vs;
	void printBlocks();
private:
	void refine();
	bool constraintGraphIsCyclic(const unsigned n, Variable* const vs[]);
	bool blockGraphIsCyclic();
};

class IncSolver : public Solver {
public:
    unsigned splitCnt;

    /**
     * incremental version of satisfy that allows refinement after blocks are
     * moved.
     *
     *  - move blocks to new positions
     *  - repeatedly merge across most violated constraint until no more
     *    violated constraints exist
     *
     * Note: there is a special case to handle when the most violated constraint
     * is between two variables in the same block.  Then, we must split the block
     * over an active constraint between the two variables.  We choose the 
     * constraint with the most negative lagrangian multiplier. 
     */
    void satisfy();

    void solve();

    void moveBlocks();

    void splitBlocks();

    IncSolver(const unsigned n, Variable* const vs[], const unsigned m, Constraint *cs[]);
private:

    typedef std::vector<Constraint*> ConstraintList;

    ConstraintList inactive;

    /**
     * Scan constraint list for the most violated constraint, or the first equality
     * constraint.
     */
    Constraint* mostViolated(ConstraintList &l);
};
}
#endif // SEEN_REMOVEOVERLAP_SOLVE_VPSC_H
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

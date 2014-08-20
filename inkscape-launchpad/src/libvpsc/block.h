/*
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */

#ifndef SEEN_REMOVEOVERLAP_BLOCK_H
#define SEEN_REMOVEOVERLAP_BLOCK_H

#include <vector>
#include <iostream>
template <class T> class PairingHeap;
namespace vpsc {
class Variable;
class Constraint;

/**
 * A block is a group of variables that must be moved together to improve
 * the goal function without violating already active constraints.
 * The variables in a block are spanned by a tree of active constraints.
 */
class Block
{
	typedef std::vector<Variable*> Variables;
	typedef std::vector<Constraint*>::iterator Cit;
	typedef std::vector<Variable*>::iterator Vit;

	friend std::ostream& operator <<(std::ostream &os,const Block &b);
public:
	Variables *vars;
	double posn;
	double weight;
	double wposn;
	Block(Variable* const v=NULL);
    virtual ~Block(void);

    /**
     * finds the constraint with the minimum lagrange multiplier, that is, the constraint
     * that most wants to split
     */
    Constraint* findMinLM();

	Constraint* findMinLMBetween(Variable* const lv, Variable* const rv);
	Constraint* findMinInConstraint();
	Constraint* findMinOutConstraint();
	void deleteMinInConstraint();
	void deleteMinOutConstraint();
	double desiredWeightedPosition();

    /**
     * Merges b into this block across c.  Can be either a
     * right merge or a left merge
     * @param b block to merge into this
     * @param c constraint being merged
     * @param distance separation required to satisfy c
     */
    void merge(Block *b, Constraint *c, double dist);

	void merge(Block *b, Constraint *c);
	void mergeIn(Block *b);
	void mergeOut(Block *b);

    /**
     * Creates two new blocks, l and r, and splits this block across constraint c,
     * placing the left subtree of constraints (and associated variables) into l
     * and the right into r.
     */
    void split(Block *&l, Block *&r, Constraint *c);

    /**
     * Block needs to be split because of a violated constraint between vl and vr.
     * We need to search the active constraint tree between l and r and find the constraint
     * with min lagrangrian multiplier and split at that point.
     * Returns the split constraint
     */
    Constraint* splitBetween(Variable* vl, Variable* vr, Block* &lb, Block* &rb);

	void setUpInConstraints();
	void setUpOutConstraints();

    /**
     * Computes the cost (squared euclidean distance from desired positions) of the
     * current positions for variables in this block
     */
    double cost();

	bool deleted;
	long timeStamp;
	PairingHeap<Constraint*> *in;
	PairingHeap<Constraint*> *out;
	bool isActiveDirectedPathBetween(Variable* u, Variable *v);
private:
	typedef enum {NONE, LEFT, RIGHT} Direction;
	typedef std::pair<double, Constraint*> Pair;
	void reset_active_lm(Variable* const v, Variable* const u);
	double compute_dfdv(Variable* const v, Variable* const u,
		       	Constraint *&min_lm);
	Pair compute_dfdv_between(
			Variable*, Variable* const, Variable* const,
		       	const Direction, bool);
	bool canFollowLeft(Constraint *c, const Variable* const last);
	bool canFollowRight(Constraint *c, const Variable* const last);
	void populateSplitBlock(Block *b, Variable* const v, Variable* const u);
	void addVariable(Variable* const v);
	void setUpConstraintHeap(PairingHeap<Constraint*>* &h,bool in);
};

}
#endif // SEEN_REMOVEOVERLAP_BLOCK_H
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

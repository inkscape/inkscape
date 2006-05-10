/**
 * \brief Solve an instance of the "Variable Placement with Separation
 * Constraints" problem.
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */
#ifndef SEEN_REMOVEOVERLAP_SOLVE_VPSC_H
#define SEEN_REMOVEOVERLAP_SOLVE_VPSC_H

#include <vector>
class Variable;
class Constraint;
class Blocks;

/**
 * Variable Placement with Separation Constraints problem instance
 */
class VPSC {
public:
	virtual void satisfy();
	virtual void solve();

	VPSC(const unsigned n, Variable *vs[], const unsigned m, Constraint *cs[]);
	virtual ~VPSC();
	Constraint** getConstraints() { return cs; }
	Variable** getVariables() { return vs; }
protected:
	Blocks *bs;
	Constraint **cs;
	unsigned m;
	Variable **vs;
	void printBlocks();
private:
	void refine();
	bool constraintGraphIsCyclic(const unsigned n, Variable *vs[]);
	bool blockGraphIsCyclic();
};

class IncVPSC : public VPSC {
public:
	unsigned splitCnt;
	void satisfy();
	void solve();
	void moveBlocks();
	void splitBlocks();
	IncVPSC(const unsigned n, Variable *vs[], const unsigned m, Constraint *cs[]);
private:
	typedef std::vector<Constraint*> ConstraintList;
	ConstraintList inactive;
	Constraint* mostViolated(ConstraintList &l);
};
#endif // SEEN_REMOVEOVERLAP_SOLVE_VPSC_H

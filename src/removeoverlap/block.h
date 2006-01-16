/**
 * \brief Remove overlaps function
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef SEEN_REMOVEOVERLAP_BLOCK_H
#define SEEN_REMOVEOVERLAP_BLOCK_H

#include <vector>
#include <iostream>
class Variable;
class Constraint;
template <class T> class PairingHeap;
class StupidPriorityQueue;

class Block
{
	friend std::ostream& operator <<(std::ostream &os,const Block &b);
public:
	std::vector<Variable*> *vars;
	double posn;
	double weight;
	double wposn;
	Block(Variable *v=NULL);
	~Block(void);
	Constraint *findMinLM();
	Constraint *findMinInConstraint();
	Constraint *findMinOutConstraint();
	void deleteMinInConstraint();
	void deleteMinOutConstraint();
	double desiredWeightedPosition();
	void merge(Block *b, Constraint *c, double dist);
	void mergeIn(Block *b);
	void mergeOut(Block *b);
	void split(Block *&l, Block *&r, Constraint *c);
	void setUpInConstraints();
	void setUpOutConstraints();
	double cost();
	bool deleted;
	long timeStamp;
	PairingHeap<Constraint*> *in;
	PairingHeap<Constraint*> *out;
private:
	void reset_active_lm(Variable *v, Variable *u);
	double compute_dfdv(Variable *v, Variable *u, Constraint *&min_lm);
	bool canFollowLeft(Constraint *c, Variable *last);
	bool canFollowRight(Constraint *c, Variable *last);
	void populateSplitBlock(Block *b, Variable *v, Variable *u);
	void addVariable(Variable *v);
	void setUpConstraintHeap(PairingHeap<Constraint*>* &h,bool in);
};

#endif // SEEN_REMOVEOVERLAP_BLOCK_H

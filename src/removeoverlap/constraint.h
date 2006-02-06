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

#ifndef SEEN_REMOVEOVERLAP_CONSTRAINT_H
#define SEEN_REMOVEOVERLAP_CONSTRAINT_H

#include <iostream>
#include "variable.h"

class Constraint
{
	friend std::ostream& operator <<(std::ostream &os,const Constraint &c);
public:
	Variable *left;
	Variable *right;
	double gap;
	double lm;
	Constraint(Variable *left, Variable *right, double gap);
	~Constraint(void){};
	inline double Constraint::slack() const { return right->position() - gap - left->position(); }
	//inline bool operator<(Constraint const &o) const { return slack() < o.slack(); }
	long timeStamp;
	bool active;
	bool visited;
};
#include <float.h>
#include "block.h"
static inline bool compareConstraints(Constraint *const &l, Constraint *const &r) {
	double const sl = 
		l->left->block->timeStamp > l->timeStamp
		||l->left->block==l->right->block
		?-DBL_MAX:l->slack();
	double const sr = 
		r->left->block->timeStamp > r->timeStamp
		||r->left->block==r->right->block
		?-DBL_MAX:r->slack();
	if(sl==sr) {
		// arbitrary choice based on id
		if(l->left->id==r->left->id) {
			if(l->right->id<r->right->id) return true;
			return false;
		}
		if(l->left->id<r->left->id) return true;
		return false;
	}
	return sl < sr;
}

#endif // SEEN_REMOVEOVERLAP_CONSTRAINT_H

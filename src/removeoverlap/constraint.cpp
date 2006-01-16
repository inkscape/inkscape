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

#include "constraint.h"

Constraint::Constraint(Variable *left, Variable *right, double gap)
{
	this->left=left; 
	left->out.push_back(this);
	this->right=right;
	right->in.push_back(this);
	this->gap=gap;
	active=false;
	visited=false;
	timeStamp=0;
}
std::ostream& operator <<(std::ostream &os, const Constraint &c)
{
	os<<*c.left<<"+"<<c.gap<<"<="<<*c.right<<"("<<c.slack()<<")";
	return os;
}

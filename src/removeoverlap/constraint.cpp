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
#include <cassert>
Constraint::Constraint(Variable *left, Variable *right, double gap)
: left(left),
  right(right),
  gap(gap),
  timeStamp(0),
  active(false),
  visited(false)
{
	left->out.push_back(this);
	right->in.push_back(this);
}
std::ostream& operator <<(std::ostream &os, const Constraint &c)
{
	os<<*c.left<<"+"<<c.gap<<"<="<<*c.right<<"("<<c.slack()<<")";
	return os;
}

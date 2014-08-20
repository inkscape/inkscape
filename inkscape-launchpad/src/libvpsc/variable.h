/*
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */
#ifndef SEEN_REMOVEOVERLAP_VARIABLE_H
#define SEEN_REMOVEOVERLAP_VARIABLE_H

#include <vector>
#include <iostream>
#include "block.h"

namespace vpsc {

class Constraint;
typedef std::vector<Constraint*> Constraints;
class Variable
{
	friend std::ostream& operator <<(std::ostream &os, const Variable &v);
public:
	const int id; // useful in log files
	double desiredPosition;
	const double weight;
	double offset;
	Block *block;
	bool visited;
	Constraints in;
	Constraints out;
	char *toString();
	inline Variable(const int id, const double desiredPos, const double weight)
		: id(id)
		, desiredPosition(desiredPos)
		, weight(weight)
		, offset(0)
		, block(NULL)
		, visited(false)
	{
	}
	inline double position() const {
		return block->posn+offset;
	}
	//double position() const;
    virtual ~Variable(void){
		in.clear();
		out.clear();
	}
};
}
#endif // SEEN_REMOVEOVERLAP_VARIABLE_H

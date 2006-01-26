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

#ifndef SEEN_REMOVEOVERLAP_VARIABLE_H
#define SEEN_REMOVEOVERLAP_VARIABLE_H

#include <vector>
#include <iostream>
class Block;
class Constraint;

class Variable
{
	friend std::ostream& operator <<(std::ostream &os, const Variable &v);
public:
	static const unsigned int _TOSTRINGBUFFSIZE=20;
	const int id; // useful in log files
	double desiredPosition;
	const double weight;
	double offset;
	Block *block;
	bool visited;
	std::vector<Constraint*> in;
	std::vector<Constraint*> out;
	char *toString();
	inline Variable(const int id, const double desiredPos, const double weight)
		: id(id)
		, desiredPosition(desiredPos)
		, weight(weight)
		, offset(0)
		, visited(false)
	{
	}
	double position() const;
	~Variable(void){
		in.clear();
		out.clear();
	}
};
#endif // SEEN_REMOVEOVERLAP_VARIABLE_H

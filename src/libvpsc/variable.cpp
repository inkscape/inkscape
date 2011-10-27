/*
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */
#include "variable.h"

namespace vpsc {
std::ostream& operator <<(std::ostream &os, const Variable &v) {
	os << "(" << v.id << "=" << v.position() << ")";
	return os;
}
}

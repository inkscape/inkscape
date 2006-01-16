#ifndef SEEN_NR_CONVEX_HULL_H
#define SEEN_NR_CONVEX_HULL_H

/* ex:set et ts=4 sw=4: */

/*
 * A class representing the convex hull of a set of points.
 *
 * Copyright 2004  MenTaLguY <mental@rydia.net>
 *
 * This code is licensed under the GNU GPL; see COPYING for more information.
 */

#include <libnr/nr-rect.h>

namespace NR {

class ConvexHull {
public:
	explicit ConvexHull(Point const &p) : _bounds(p, p) {}

    Point midpoint() const {
        return _bounds.midpoint();
    }

	void add(Point const &p) {
		_bounds.expandTo(p);
	}
	void add(Rect const &p) {
		// Note that this is a hack.  when convexhull actually works
		// you will need to add all four points.
		_bounds.expandTo(p.min());
		_bounds.expandTo(p.max());
	}
	void add(ConvexHull const &h) {
		_bounds.expandTo(h._bounds);
	}
		
	Rect const &bounds() const {
		return _bounds;
	}
	
private:
	Rect _bounds;
};

} /* namespace NR */

#endif

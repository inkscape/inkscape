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
	explicit ConvexHull(Point const &p)
    : _initial(p) {}

    Point midpoint() const {
        if (_bounds) {
            return _bounds->midpoint();
        } else {
            return _initial;
        }
    }

	void add(Point const &p) {
        if (_bounds) {
		    _bounds->expandTo(p);
        } else if ( p != _initial ) {
            _bounds = Rect(_initial, p);
        }
	}
	void add(Rect const &p) {
		// Note that this is a hack.  when convexhull actually works
		// you will need to add all four points.
		if (_bounds) {
            _bounds = union_bounds(*_bounds, p);
        } else {
            _bounds = p;
            _bounds->expandTo(_initial);
        }
	}
	void add(ConvexHull const &h) {
        if (h._bounds) {
            add(*h._bounds);
        } else {
            add(h._initial);
        }
	}

	Maybe<Rect> const &bounds() const {
		return _bounds;
	}
	
private:
    Point _initial;
	Maybe<Rect> _bounds;
};

} /* namespace NR */

#endif

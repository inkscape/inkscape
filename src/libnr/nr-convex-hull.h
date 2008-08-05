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
    ConvexHull() : _bounds() {}
	explicit ConvexHull(Point const &p) : _bounds(Rect(p, p)) {}

    boost::optional<Point> midpoint() const {
        if (_bounds) {
            return _bounds->midpoint();
        } else {
            return boost::optional<Point>();
        }
    }

	void add(Point const &p) {
        if (_bounds) {
		    _bounds->expandTo(p);
        } else {
            _bounds = Rect(p, p);
        }
	}
	void add(Rect const &r) {
		// Note that this is a hack.  when convexhull actually works
		// you will need to add all four points.
        _bounds = union_bounds(_bounds, r);
	}
	void add(ConvexHull const &h) {
        if (h._bounds) {
            add(*h._bounds);
        }
	}

	boost::optional<Rect> const &bounds() const {
		return _bounds;
	}
	
private:
    boost::optional<Rect> _bounds;
};

} /* namespace NR */

#endif

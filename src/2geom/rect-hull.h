/*
 * Copyright 2004  MenTaLguY <mental@rydia.net>
 *
 * This code is licensed under the GNU GPL; see COPYING for more information.
 */

#ifndef GEOM_RECT_HULL_H
#define GEOM_RECT_HULL_H

#include <2geom/rect.h>

namespace Geom {

/**
 * A class representing a rectangular convex hull of a set of points.
 */
class RectHull {
public:
    RectHull() : _bounds() {}
    explicit RectHull(Point const &p) : _bounds(Rect(p, p)) {}

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
        _bounds.unionWith(r);
    }
    void add(RectHull const &h) {
        if (h._bounds) {
            add(*h._bounds);
        }
    }

    OptRect const &bounds() const {
        return _bounds;
    }
    
private:
    OptRect _bounds;
};

} /* namespace Geom */

#endif // GEOM_RECT_HULL_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

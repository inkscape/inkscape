/* Axis-aligned rectangle
 *
 * Authors:
 *   Michael Sloan <mgsloan@gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * Copyright 2007-2011 Authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

#include <2geom/rect.h>

namespace Geom {

/** @brief Transform the rectangle by an affine.
 * The result of the transformation might not be axis-aligned. The return value
 * of this operation will be the smallest axis-aligned rectangle containing
 * all points of the true result. */
Rect &Rect::operator*=(Affine const &m) {
    Point pts[4];
    for (unsigned i=0; i<4; ++i) pts[i] = corner(i) * m;
    Coord minx = std::min(std::min(pts[0][X], pts[1][X]), std::min(pts[2][X], pts[3][X]));
    Coord miny = std::min(std::min(pts[0][Y], pts[1][Y]), std::min(pts[2][Y], pts[3][Y]));
    Coord maxx = std::max(std::max(pts[0][X], pts[1][X]), std::max(pts[2][X], pts[3][X]));
    Coord maxy = std::max(std::max(pts[0][Y], pts[1][Y]), std::max(pts[2][Y], pts[3][Y]));
    f[X].setMin(minx); f[X].setMax(maxx);
    f[Y].setMin(miny); f[Y].setMax(maxy);
    return *this;
}

Coord distanceSq(Point const &p, Rect const &rect)
{
    double dx = 0, dy = 0;
    if ( p[X] < rect.left() ) {
        dx = p[X] - rect.left();
    } else if ( p[X] > rect.right() ) {
        dx = rect.right() - p[X];
    }
    if (p[Y] < rect.top() ) {
        dy = rect.top() - p[Y];
    } else if (  p[Y] > rect.bottom() ) {
        dy = p[Y] - rect.bottom();
    }
    return dx*dx+dy*dy;
}

/** @brief Returns the smallest distance between p and rect.
 * @relates Rect */
Coord distance(Point const &p, Rect const &rect)
{
    // copy of distanceSq, because we need to use hypot()
    double dx = 0, dy = 0;
    if ( p[X] < rect.left() ) {
        dx = p[X] - rect.left();
    } else if ( p[X] > rect.right() ) {
        dx = rect.right() - p[X];
    }
    if (p[Y] < rect.top() ) {
        dy = rect.top() - p[Y];
    } else if (  p[Y] > rect.bottom() ) {
        dy = p[Y] - rect.bottom();
    }
    return hypot(dx, dy);
}

Coord distanceSq(Point const &p, OptRect const &rect)
{
    if (!rect) return std::numeric_limits<Coord>::max();
    return distanceSq(p, *rect);
}
Coord distance(Point const &p, OptRect const &rect)
{
    if (!rect) return std::numeric_limits<Coord>::max();
    return distance(p, *rect);
}

} // namespace Geom

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

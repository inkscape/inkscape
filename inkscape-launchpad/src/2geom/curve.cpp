/* Abstract curve type - implementation of default methods
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2007-2009 Authors
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

#include <2geom/curve.h>
#include <2geom/exception.h>
#include <2geom/nearest-time.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/ord.h>
#include <2geom/path-sink.h>

//#include <iostream>

namespace Geom 
{

Coord Curve::nearestTime(Point const& p, Coord a, Coord b) const
{
    return nearest_time(p, toSBasis(), a, b);
}

std::vector<Coord> Curve::allNearestTimes(Point const& p, Coord from, Coord to) const
{
    return all_nearest_times(p, toSBasis(), from, to);
}

Coord Curve::length(Coord tolerance) const
{
    return ::Geom::length(toSBasis(), tolerance);
}

int Curve::winding(Point const &p) const
{
    try {
        std::vector<Coord> ts = roots(p[Y], Y);
        if(ts.empty()) return 0;
        std::sort(ts.begin(), ts.end());

        // skip endpoint roots when they are local maxima on the Y axis
        // this follows the convention used in other winding routines,
        // i.e. that the bottommost coordinate is not part of the shape
        bool ignore_0 = unitTangentAt(0)[Y] <= 0;
        bool ignore_1 = unitTangentAt(1)[Y] >= 0;

        int wind = 0;
        for (std::size_t i = 0; i < ts.size(); ++i) {
            Coord t = ts[i];
            //std::cout << t << std::endl;
            if ((t == 0 && ignore_0) || (t == 1 && ignore_1)) continue;
            if (valueAt(t, X) > p[X]) { // root is ray intersection
                Point tangent = unitTangentAt(t);
                if (tangent[Y] > 0) {
                    // at the point of intersection, curve goes in +Y direction,
                    // so it winds in the direction of positive angles
                    ++wind;
                } else if (tangent[Y] < 0) {
                    --wind;
                }
            }
        }
        return wind;
    } catch (InfiniteSolutions const &e) {
        // this means we encountered a line segment exactly coincident with the point
        // skip, since this will be taken care of by endpoint roots in other segments
        return 0;
    }
}

std::vector<CurveIntersection> Curve::intersect(Curve const &/*other*/, Coord /*eps*/) const
{
    // TODO: approximate as Bezier
    THROW_NOTIMPLEMENTED();
}

std::vector<CurveIntersection> Curve::intersectSelf(Coord eps) const
{
    std::vector<CurveIntersection> result;
    // Monotonic segments cannot have self-intersections.
    // Thus, we can split the curve at roots and intersect the portions.
    std::vector<Coord> splits;
    std::auto_ptr<Curve> deriv(derivative());
    splits = deriv->roots(0, X);
    if (splits.empty()) {
        return result;
    }
    deriv.reset();
    splits.push_back(1.);

    boost::ptr_vector<Curve> parts;
    Coord previous = 0;
    for (unsigned i = 0; i < splits.size(); ++i) {
        if (splits[i] == 0.) continue;
        parts.push_back(portion(previous, splits[i]));
        previous = splits[i];
    }

    Coord prev_i = 0;
    for (unsigned i = 0; i < parts.size()-1; ++i) {
        Interval dom_i(prev_i, splits[i]);
        prev_i = splits[i];

        Coord prev_j = 0;
        for (unsigned j = i+1; j < parts.size(); ++j) {
            Interval dom_j(prev_j, splits[j]);
            prev_j = splits[j];

            std::vector<CurveIntersection> xs = parts[i].intersect(parts[j], eps);
            for (unsigned k = 0; k < xs.size(); ++k) {
                // to avoid duplicated intersections, skip values at exactly 1
                if (xs[k].first == 1. || xs[k].second == 1.) continue;

                Coord ti = dom_i.valueAt(xs[k].first);
                Coord tj = dom_j.valueAt(xs[k].second);

                CurveIntersection real(ti, tj, xs[k].point());
                result.push_back(real);
            }
        }
    }
    return result;
}

Point Curve::unitTangentAt(Coord t, unsigned n) const
{
    std::vector<Point> derivs = pointAndDerivatives(t, n);
    for (unsigned deriv_n = 1; deriv_n < derivs.size(); deriv_n++) {
        Coord length = derivs[deriv_n].length();
        if ( ! are_near(length, 0) ) {
            // length of derivative is non-zero, so return unit vector
            return derivs[deriv_n] / length;
        }
    }
    return Point (0,0);
};

void Curve::feed(PathSink &sink, bool moveto_initial) const
{
    std::vector<Point> pts;
    sbasis_to_bezier(pts, toSBasis(), 2); //TODO: use something better!
    if (moveto_initial) {
        sink.moveTo(initialPoint());
    }
    sink.curveTo(pts[0], pts[1], pts[2]);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

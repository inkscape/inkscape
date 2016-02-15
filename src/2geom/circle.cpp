/** @file
 * @brief Circle shape
 *//*
 * Authors:
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright 2008-2014 Authors
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

#include <2geom/circle.h>
#include <2geom/ellipse.h>
#include <2geom/elliptical-arc.h>
#include <2geom/numeric/fitting-tool.h>
#include <2geom/numeric/fitting-model.h>

namespace Geom {

Rect Circle::boundsFast() const
{
    Point rr(_radius, _radius);
    Rect bbox(_center - rr, _center + rr);
    return bbox;
}

void Circle::setCoefficients(Coord A, Coord B, Coord C, Coord D)
{
    if (A == 0) {
        THROW_RANGEERROR("square term coefficient == 0");
    }

    //std::cerr << "B = " << B << "  C = " << C << "  D = " << D << std::endl;

    Coord b = B / A;
    Coord c = C / A;
    Coord d = D / A;

    _center[X] = -b/2;
    _center[Y] = -c/2;
    Coord r2 = _center[X] * _center[X] + _center[Y] * _center[Y] - d;

    if (r2 < 0) {
        THROW_RANGEERROR("ray^2 < 0");
    }

    _radius = std::sqrt(r2);
}

void Circle::coefficients(Coord &A, Coord &B, Coord &C, Coord &D) const
{
    A = 1;
    B = -2 * _center[X];
    C = -2 * _center[Y];
    D = _center[X] * _center[X] + _center[Y] * _center[Y] - _radius * _radius;
}

std::vector<Coord> Circle::coefficients() const
{
    std::vector<Coord> c(4);
    coefficients(c[0], c[1], c[2], c[3]);
    return c;
}


Zoom Circle::unitCircleTransform() const
{
    Zoom ret(_radius, _center / _radius);
    return ret;
}

Zoom Circle::inverseUnitCircleTransform() const
{
    if (_radius == 0) {
        THROW_RANGEERROR("degenerate circle does not have an inverse unit circle transform");
    }

    Zoom ret(1/_radius, Translate(-_center));
    return ret;
}

Point Circle::initialPoint() const
{
    Point p(_center);
    p[X] += _radius;
    return p;
}

Point Circle::pointAt(Coord t) const {
    return _center + Point::polar(t) * _radius;
}

Coord Circle::valueAt(Coord t, Dim2 d) const {
    Coord delta = (d == X ? std::cos(t) : std::sin(t));
    return _center[d] + delta * _radius;
}

Coord Circle::timeAt(Point const &p) const {
    if (_center == p) return 0;
    return atan2(p - _center);
}

Coord Circle::nearestTime(Point const &p) const {
    return timeAt(p);
}

bool Circle::contains(Rect const &r) const
{
    for (unsigned i = 0; i < 4; ++i) {
        if (!contains(r.corner(i))) return false;
    }
    return true;
}

bool Circle::contains(Circle const &other) const
{
    Coord cdist = distance(_center, other._center);
    Coord rdist = fabs(_radius - other._radius);
    return cdist <= rdist;
}

bool Circle::intersects(Line const &l) const
{
    // http://mathworld.wolfram.com/Circle-LineIntersection.html
    Coord dr = l.vector().length();
    Coord r = _radius;
    Coord D = cross(l.initialPoint(), l.finalPoint());
    Coord delta = r*r * dr*dr - D*D;
    if (delta >= 0) return true;
    return false;
}

bool Circle::intersects(Circle const &other) const
{
    Coord cdist = distance(_center, other._center);
    Coord rsum = _radius + other._radius;
    return cdist <= rsum;
}


std::vector<ShapeIntersection> Circle::intersect(Line const &l) const
{
    // http://mathworld.wolfram.com/Circle-LineIntersection.html
    Coord dr = l.vector().length();
    Coord dx = l.vector().x();
    Coord dy = l.vector().y();
    Coord D = cross(l.initialPoint() - _center, l.finalPoint() - _center);
    Coord delta = _radius*_radius * dr*dr - D*D;

    std::vector<ShapeIntersection> result;
    if (delta < 0) return result;
    if (delta == 0) {
        Coord ix = (D*dy) / (dr*dr);
        Coord iy = (-D*dx) / (dr*dr);
        Point ip(ix, iy); ip += _center;
        result.push_back(ShapeIntersection(timeAt(ip), l.timeAt(ip), ip));
        return result;
    }

    Coord sqrt_delta = std::sqrt(delta);
    Coord signmod = dy < 0 ? -1 : 1;

    Coord i1x = (D*dy + signmod * dx * sqrt_delta) / (dr*dr);
    Coord i1y = (-D*dx + fabs(dy) * sqrt_delta) / (dr*dr);
    Point i1p(i1x, i1y); i1p += _center;

    Coord i2x = (D*dy - signmod * dx * sqrt_delta) / (dr*dr);
    Coord i2y = (-D*dx - fabs(dy) * sqrt_delta) / (dr*dr);
    Point i2p(i2x, i2y); i2p += _center;

    result.push_back(ShapeIntersection(timeAt(i1p), l.timeAt(i1p), i1p));
    result.push_back(ShapeIntersection(timeAt(i2p), l.timeAt(i2p), i2p));
    return result;
}

std::vector<ShapeIntersection> Circle::intersect(LineSegment const &l) const
{
    std::vector<ShapeIntersection> result = intersect(Line(l));
    filter_line_segment_intersections(result);
    return result;
}

std::vector<ShapeIntersection> Circle::intersect(Circle const &other) const
{
    std::vector<ShapeIntersection> result;

    if (*this == other) {
        THROW_INFINITESOLUTIONS();
    }
    if (contains(other)) return result;
    if (!intersects(other)) return result;

    // See e.g. http://mathworld.wolfram.com/Circle-CircleIntersection.html
    // Basically, we figure out where is the third point of a triangle
    // with two points in the centers and with edge lengths equal to radii
    Point cv = other._center - _center;
    Coord d = cv.length();
    Coord R = radius(), r = other.radius();

    if (d == R + r) {
        Point px = lerp(R / d, _center, other._center);
        Coord T = timeAt(px), t = other.timeAt(px);
        result.push_back(ShapeIntersection(T, t, px));
        return result;
    }

    // q is the distance along the line between centers to the perpendicular line
    // that goes through both intersections.
    Coord q = (d*d - r*r + R*R) / (2*d);
    Point qp = lerp(q/d, _center, other._center);

    // The triangle given by the points:
    // _center, qp, intersection
    // is a right triangle. Determine the distance between qp and intersection
    // using the Pythagorean theorem.
    Coord h = std::sqrt(R*R - q*q);
    Point qd = (h/d) * cv.cw();

    // now compute the intersection points
    Point x1 = qp + qd;
    Point x2 = qp - qd;

    result.push_back(ShapeIntersection(timeAt(x1), other.timeAt(x1), x1));
    result.push_back(ShapeIntersection(timeAt(x2), other.timeAt(x2), x2));
    return result;
}

/**
 @param inner a point whose angle with the circle center is inside the angle that the arc spans
 */
EllipticalArc *
Circle::arc(Point const& initial, Point const& inner, Point const& final) const
{
    // TODO native implementation!
    Ellipse e(_center[X], _center[Y], _radius, _radius, 0);
    return e.arc(initial, inner, final);
}

bool Circle::operator==(Circle const &other) const
{
    if (_center != other._center) return false;
    if (_radius != other._radius) return false;
    return true;
}

D2<SBasis> Circle::toSBasis() const
{
    D2<SBasis> B;
    Linear bo = Linear(0, 2 * M_PI);

    B[0] = cos(bo,4);
    B[1] = sin(bo,4);

    B = B * _radius + _center;

    return B;
}


void Circle::fit(std::vector<Point> const& points)
{
    size_t sz = points.size();
    if (sz < 2) {
        THROW_RANGEERROR("fitting error: too few points passed");
    }
    if (sz == 2) {
        _center = points[0] * 0.5 + points[1] * 0.5;
        _radius = distance(points[0], points[1]) / 2;
        return;
    }

    NL::LFMCircle model;
    NL::least_squeares_fitter<NL::LFMCircle> fitter(model, sz);

    for (size_t i = 0; i < sz; ++i) {
        fitter.append(points[i]);
    }
    fitter.update();

    NL::Vector z(sz, 0.0);
    model.instance(*this, fitter.result(z));
}


bool are_near(Circle const &a, Circle const &b, Coord eps)
{
    // to check whether no point on a is further than eps from b,
    // we check two things:
    // 1. if radii differ by more than eps, there is definitely a point that fails
    // 2. if they differ by less, we check the centers. They have to be closer
    //    together if the radius differs, since the maximum distance will be
    //    equal to sum of radius difference and distance between centers.
    if (!are_near(a.radius(), b.radius(), eps)) return false;
    Coord adjusted_eps = eps - fabs(a.radius() - b.radius());
    return are_near(a.center(), b.center(), adjusted_eps);
}

std::ostream &operator<<(std::ostream &out, Circle const &c)
{
    out << "Circle(" << c.center() << ", " << format_coord_nice(c.radius()) << ")";
    return out;
}

}  // end namespace Geom

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

/** @file
 * @brief Ellipse shape
 *//*
 * Authors:
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright 2008  authors
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


#ifndef LIB2GEOM_SEEN_ELLIPSE_H
#define LIB2GEOM_SEEN_ELLIPSE_H

#include <vector>
#include <2geom/angle.h>
#include <2geom/bezier-curve.h>
#include <2geom/exception.h>
#include <2geom/forward.h>
#include <2geom/line.h>
#include <2geom/transforms.h>

namespace Geom {

class EllipticalArc;
class Circle;

/** @brief Set of points with a constant sum of distances from two foci.
 *
 * An ellipse can be specified in several ways. Internally, 2Geom uses
 * the SVG style representation: center, rays and angle between the +X ray
 * and the +X axis. Another popular way is to use an implicit equation,
 * which is as follows:
 * \f$Ax^2 + Bxy + Cy^2 + Dx + Ey + F = 0\f$
 *
 * @ingroup Shapes */
class Ellipse
    : boost::multipliable< Ellipse, Translate
    , boost::multipliable< Ellipse, Scale
    , boost::multipliable< Ellipse, Rotate
    , boost::multipliable< Ellipse, Zoom
    , boost::multipliable< Ellipse, Affine
    , boost::equality_comparable< Ellipse
      > > > > > >
{
    Point _center;
    Point _rays;
    Angle _angle;
public:
    Ellipse() {}
    Ellipse(Point const &c, Point const &r, Coord angle)
        : _center(c)
        , _rays(r)
        , _angle(angle)
    {}
    Ellipse(Coord cx, Coord cy, Coord rx, Coord ry, Coord angle)
        : _center(cx, cy)
        , _rays(rx, ry)
        , _angle(angle)
    {}
    Ellipse(double A, double B, double C, double D, double E, double F) {
        setCoefficients(A, B, C, D, E, F);
    }
    /// Construct ellipse from a circle.
    Ellipse(Geom::Circle const &c);

    /// Set center, rays and angle.
    void set(Point const &c, Point const &r, Coord angle) {
        _center = c;
        _rays = r;
        _angle = angle;
    }
    /// Set center, rays and angle as constituent values.
    void set(Coord cx, Coord cy, Coord rx, Coord ry, Coord a) {
        _center[X] = cx;
        _center[Y] = cy;
        _rays[X] = rx;
        _rays[Y] = ry;
        _angle = a;
    }
    /// Set an ellipse by solving its implicit equation.
    void setCoefficients(double A, double B, double C, double D, double E, double F);
    /// Set the center.
    void setCenter(Point const &p) { _center = p; }
    /// Set the center by coordinates.
    void setCenter(Coord cx, Coord cy) { _center[X] = cx; _center[Y] = cy; }
    /// Set both rays of the ellipse.
    void setRays(Point const &p) { _rays = p; }
    /// Set both rays of the ellipse as coordinates.
    void setRays(Coord x, Coord y) { _rays[X] = x; _rays[Y] = y; }
    /// Set one of the rays of the ellipse.
    void setRay(Coord r, Dim2 d) { _rays[d] = r; }
    /// Set the angle the X ray makes with the +X axis.
    void setRotationAngle(Angle a) { _angle = a; }

    Point center() const { return _center; }
    Coord center(Dim2 d) const { return _center[d]; }
    /// Get both rays as a point.
    Point rays() const { return _rays; }
    /// Get one ray of the ellipse.
    Coord ray(Dim2 d) const { return _rays[d]; }
    /// Get the angle the X ray makes with the +X axis.
    Angle rotationAngle() const { return _angle; }
    /// Get the point corresponding to the +X ray of the ellipse.
    Point initialPoint() const;
    /// Get the point corresponding to the +X ray of the ellipse.
    Point finalPoint() const { return initialPoint(); }

    /** @brief Create an ellipse passing through the specified points
     * At least five points have to be specified. */
    void fit(std::vector<Point> const& points);

    /** @brief Create an elliptical arc from a section of the ellipse.
     * This is mainly useful to determine the flags of the new arc.
     * The passed points should lie on the ellipse, otherwise the results
     * will be undefined.
     * @param ip Initial point of the arc
     * @param inner Point in the middle of the arc, used to pick one of two possibilities
     * @param fp Final point of the arc
     * @return Newly allocated arc, delete when no longer used */
    EllipticalArc *arc(Point const &ip, Point const &inner, Point const &fp);

    /** @brief Return an ellipse with less degrees of freedom.
     * The canonical form always has the angle less than \f$\frac{\pi}{2}\f$,
     * and zero if the rays are equal (i.e. the ellipse is a circle). */
    Ellipse canonicalForm() const;
    void makeCanonical();

    /** @brief Compute the transform that maps the unit circle to this ellipse.
     * Each ellipse can be interpreted as a translated, scaled and rotate unit circle.
     * This function returns the transform that maps the unit circle to this ellipse.
     * @return Transform from unit circle to the ellipse */
    Affine unitCircleTransform() const;
    /** @brief Compute the transform that maps this ellipse to the unit circle.
     * This may be a little more precise and/or faster than simply using
     * unitCircleTransform().inverse(). An exception will be thrown for
     * degenerate ellipses. */
    Affine inverseUnitCircleTransform() const;

    LineSegment majorAxis() const { return ray(X) >= ray(Y) ? axis(X) : axis(Y); }
    LineSegment minorAxis() const { return ray(X) < ray(Y) ? axis(X) : axis(Y); }
    LineSegment semimajorAxis(int sign = 1) const {
        return ray(X) >= ray(Y) ? semiaxis(X, sign) : semiaxis(Y, sign);
    }
    LineSegment semiminorAxis(int sign = 1) const {
        return ray(X) < ray(Y) ? semiaxis(X, sign) : semiaxis(Y, sign);
    }
    LineSegment axis(Dim2 d) const;
    LineSegment semiaxis(Dim2 d, int sign = 1) const;

    /// Get the tight-fitting bounding box of the ellipse.
    Rect boundsExact() const;

    /// Get the coefficients of the ellipse's implicit equation.
    std::vector<double> coefficients() const;
    void coefficients(Coord &A, Coord &B, Coord &C, Coord &D, Coord &E, Coord &F) const;

    /** @brief Evaluate a point on the ellipse.
     * The parameter range is \f$[0, 2\pi)\f$; larger and smaller values
     * wrap around. */
    Point pointAt(Coord t) const;
    /// Evaluate a single coordinate of a point on the ellipse.
    Coord valueAt(Coord t, Dim2 d) const;

    /** @brief Find the time value of a point on an ellipse.
     * If the point is not on the ellipse, the returned time value will correspond
     * to an intersection with a ray from the origin passing through the point
     * with the ellipse. Note that this is NOT the nearest point on the ellipse. */
    Coord timeAt(Point const &p) const;

    /// Get the value of the derivative at time t normalized to unit length.
    Point unitTangentAt(Coord t) const;

    /// Check whether the ellipse contains the given point.
    bool contains(Point const &p) const;

    /// Compute intersections with an infinite line.
    std::vector<ShapeIntersection> intersect(Line const &line) const;
    /// Compute intersections with a line segment.
    std::vector<ShapeIntersection> intersect(LineSegment const &seg) const;
    /// Compute intersections with another ellipse.
    std::vector<ShapeIntersection> intersect(Ellipse const &other) const;
    /// Compute intersections with a 2D Bezier polynomial.
    std::vector<ShapeIntersection> intersect(D2<Bezier> const &other) const;

    Ellipse &operator*=(Translate const &t) {
        _center *= t;
        return *this;
    }
    Ellipse &operator*=(Scale const &s) {
        _center *= s;
        _rays *= s;
        return *this;
    }
    Ellipse &operator*=(Zoom const &z) {
        _center *= z;
        _rays *= z.scale();
        return *this;
    }
    Ellipse &operator*=(Rotate const &r);
    Ellipse &operator*=(Affine const &m);

    /// Compare ellipses for exact equality.
    bool operator==(Ellipse const &other) const;
};

/** @brief Test whether two ellipses are approximately the same.
 * This will check whether no point on ellipse a is further away from
 * the corresponding point on ellipse b than precision.
 * @relates Ellipse */
bool are_near(Ellipse const &a, Ellipse const &b, Coord precision = EPSILON);

/** @brief Outputs ellipse data, useful for debugging.
 * @relates Ellipse */
std::ostream &operator<<(std::ostream &out, Ellipse const &e);

} // end namespace Geom

#endif // LIB2GEOM_SEEN_ELLIPSE_H

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

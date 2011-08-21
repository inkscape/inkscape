/**
 * \file
 * \brief  Horizontal and vertical line segment
 *//*
 * Authors:
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * Copyright 2008-2011 Authors
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

#ifndef LIB2GEOM_SEEN_HVLINESEGMENT_H
#define LIB2GEOM_SEEN_HVLINESEGMENT_H

#include <2geom/bezier-curve.h>

namespace Geom
{

template <Dim2 axis>
class AxisLineSegment : public LineSegment
{
public:
    static const Dim2 other_axis = static_cast<Dim2>((axis + 1) % 2);
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    virtual void setInitial(Point const &p) {
        Point f = finalPoint();
        f[axis] = p[axis];
        LineSegment::setInitial(p);
        LineSegment::setFinal(f);
    }
    virtual void setFinal(Point const &p) {
        Point i = initialPoint();
        i[axis] = p[axis];
        LineSegment::setFinal(p);
        LineSegment::setInitial(i);
    }
    virtual Rect boundsFast() const { return boundsExact(); }
    virtual Rect boundsExact() const { Rect r(initialPoint(), finalPoint()); return r; }
    virtual int degreesOfFreedom() const { return 3; }
    virtual std::vector<Coord> roots(Coord v, Dim2 d) const {
        std::vector<Coord> result;
        if (d == axis) {
            if ( v >= initialPoint()[axis] && v <= finalPoint()[axis] ) {
                Coord t = 0;
                if (!isDegenerate())
                    t = (v - initialPoint()[axis]) / (finalPoint()[axis] - initialPoint()[axis]);
                result.push_back(t);
            }
        } else {
            if (v == initialPoint()[other_axis]) {
                if (!isDegenerate())
                    THROW_INFINITESOLUTIONS(0);
                result.push_back(0);
            }
        }
        return result;
    }
    virtual Coord nearestPoint( Point const &p, Coord from = 0, Coord to = 1 ) const {
        if ( from > to ) std::swap(from, to);
        Coord xfrom = valueAt(from, axis);
        Coord xto = valueAt(to, axis);
        if ( xfrom > xto ) {
            std::swap(xfrom, xto);
            std::swap(from, to);
        }
        if ( p[axis] > xfrom && p[axis] < xto ) {
            return (p[axis] - initialPoint()[axis]) / (finalPoint()[axis] - initialPoint()[axis]);
        }
        else if ( p[X] <= xfrom )
            return from;
        else
            return to;
    }
    virtual Point pointAt(Coord t) const {
        if ( t < 0 || t > 1 )
            THROW_RANGEERROR("AxisLineSegment: Time value out of range");
        Point ret = initialPoint() + t * (finalPoint() - initialPoint());
        return ret;
    }
    virtual Coord valueAt(Coord t, Dim2 d) const {
        if ( t < 0 || t > 1 )
            THROW_RANGEERROR("AxisLineSegment: Time value out of range");
        if (d != axis) return initialPoint()[other_axis];
        return initialPoint()[axis] + t * (finalPoint()[axis] - initialPoint()[axis]);
    }
    virtual std::vector<Point> pointAndDerivatives(Coord t, unsigned n) const {
        std::vector<Point> result;
        result.push_back(pointAt(t));
        if (n > 0) {
            Point der = finalPoint() - initialPoint();
            result.push_back( der );
        }
        if (n > 1) {
            /* higher order derivatives are zero,
             * so the other n-1 vector elements are (0,0) */
            result.insert( result.end(), n-1, Point(0, 0) );
        }
        return result;
    }
#endif
protected:
    AxisLineSegment(Point const &p0, Point const &p1) : LineSegment(p0, p1) {}
    AxisLineSegment() {}
};

class HLineSegment : public AxisLineSegment<X>
{
public:
    HLineSegment() {}
    HLineSegment(Coord x0, Coord x1, Coord y)
        : AxisLineSegment<X>(Point(x0, y), Point(x1, y))
    {}

    HLineSegment(Point const& p, Coord len)
        : AxisLineSegment<X>(p, Point(p[X] + len, p[Y]))
    {}

    HLineSegment(Point const& p0, Point const& p1)
        : AxisLineSegment<X>(p0, p1)
    {
        if ( p0[Y] != p1[Y] ) {
            THROW_RANGEERROR("HLineSegment::HLineSegment passed points should "
                             "have the same Y value");
        }
    }

    Coord getY() { return initialPoint()[Y]; }
    void setInitialX(Coord x) {
        LineSegment::setInitial(Point(x, initialPoint()[Y]));
    }
    void setFinalX(Coord x) {
        LineSegment::setFinal(Point(x, initialPoint()[Y]));
    }
    void setY(Coord y) {
        LineSegment::setInitial( Point(initialPoint()[X], y) );
        LineSegment::setFinal( Point(finalPoint()[X], y) );
    }
    std::pair<HLineSegment, HLineSegment> subdivide(Coord t) const {
        std::pair<HLineSegment, HLineSegment> result;
        Point p = pointAt(t);
        result.first.setInitial(initialPoint());
        result.first.setFinal(p);
        result.second.setInitial(p);
        result.second.setFinal(finalPoint());
        return result;
    }

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    virtual Curve* duplicate() const { return new HLineSegment(*this); }
    virtual Curve *portion(Coord f, Coord t) const {
        Point ip = pointAt(f);
        Point ep = pointAt(t);
        return new HLineSegment(ip[X], ep[X], ip[Y]);
    }
    virtual Curve *reverse() const {
        Point ip = initialPoint();
        return new HLineSegment(finalPoint()[X], ip[X], ip[Y]);
    }
    virtual Curve *transformed(Affine const & m) const {
        Point ip = initialPoint() * m;
        Point ep = finalPoint() * m;
        // cannot afford to lose precision here since it can lead to discontinuous paths
        if (m.isZoom(0)) {
            return new HLineSegment(ip[X], ep[X], ip[Y]);
        } else {
            return new LineSegment(ip, ep);
        }
    }
    virtual Curve *derivative() const {
        Coord x = finalPoint()[X] - initialPoint()[X];
        return new HLineSegment(x, x, 0);
    }
#endif
};  // end class HLineSegment


class VLineSegment : public AxisLineSegment<Y>
{
public:
    VLineSegment() {}

    VLineSegment(Coord x, Coord y0, Coord y1)
        : AxisLineSegment<Y>(Point(x, y0), Point(x, y1))
    {}

    VLineSegment(Point const& _p, Coord _length)
        : AxisLineSegment<Y>(_p, Point(_p[X], _p[Y] + _length))
    {}

    VLineSegment(Point const& _p0, Point const& _p1)
        : AxisLineSegment<Y>(_p0, _p1)
    {
        if ( _p0[X] != _p1[X] ) {
            THROW_RANGEERROR("VLineSegment::VLineSegment passed points should "
                             "have the same X value");
        }
    }
    
    Coord getX() { return initialPoint()[X]; }
    void setInitialY(Coord _y) {
        LineSegment::setInitial( Point(initialPoint()[X], _y) );
    }
    void setFinalY(Coord _y) {
        LineSegment::setFinal( Point(finalPoint()[Y], _y) );
    }
    void setX(Coord _x) {
        LineSegment::setInitial( Point(_x, initialPoint()[Y]) );
        LineSegment::setFinal( Point(_x, finalPoint()[Y]) );
    }
    std::pair<VLineSegment, VLineSegment> subdivide(Coord t) const {
        std::pair<VLineSegment, VLineSegment> result;
        Point p = pointAt(t);
        result.first.setInitial(initialPoint());
        result.first.setFinal(p);
        result.second.setInitial(p);
        result.second.setFinal(finalPoint());
        return result;
    }

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    virtual Curve *duplicate() const { return new VLineSegment(*this); }
    virtual Curve *portion(Coord f, Coord t) const {
        Point ip = pointAt(f);
        Coord epy = valueAt(t, Y);
        return new VLineSegment(ip[X], ip[Y], epy);
    }
    virtual Curve *reverse() const {
        Point ip = initialPoint();
        return new VLineSegment(ip[X], finalPoint()[Y], ip[Y]);
    }
    virtual Curve *transformed(Affine const & m) const {
        Point ip = initialPoint() * m;
        Point ep = finalPoint() * m;
        if (m.isZoom()) {
            return new VLineSegment(ip[X], ip[Y], ep[Y]);
        } else {
            return new LineSegment(ip, ep);
        }
    }
    virtual Curve* derivative() const {
        Coord y = finalPoint()[Y] - initialPoint()[Y];
        return new VLineSegment(0, y, y);
    }
#endif
}; // end class VLineSegment

}  // end namespace Geom

#endif // LIB2GEOM_SEEN_HVLINESEGMENT_H

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

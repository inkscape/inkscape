/**
 * \file
 * \brief Bezier curve
 *//*
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
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

#ifndef SEEN_LIB2GEOM_BEZIER_CURVE_H
#define SEEN_LIB2GEOM_BEZIER_CURVE_H

#include <2geom/curve.h>
#include <2geom/sbasis-curve.h> // for non-native winding method
#include <2geom/bezier.h>
#include <2geom/transforms.h>

namespace Geom 
{

class BezierCurve : public Curve {
protected:
    D2<Bezier> inner;
    BezierCurve() {}
    BezierCurve(D2<Bezier> const &b) : inner(b) {}
    BezierCurve(Bezier const &x, Bezier const &y) : inner(x, y) {}
    BezierCurve(std::vector<Point> const &pts);

public:
    /// @name Access and modify control points
    /// @{
    /** @brief Get the order of the Bezier curve.
     * A Bezier curve has order() + 1 control points. */
    unsigned order() const { return inner[X].order(); }
    /** @brief Get the control points.
     * @return Vector with order() + 1 control points. */
    std::vector<Point> points() const { return bezier_points(inner); }
    /** @brief Modify a control point.
     * @param ix The zero-based index of the point to modify. Note that the caller is responsible for checking that this value is <= order().
     * @param v The new value of the point */
    void setPoint(unsigned ix, Point v) {
        inner[X].setPoint(ix, v[X]);
        inner[Y].setPoint(ix, v[Y]);
    }
    /** @brief Set new control points.
     * @param ps Vector which must contain order() + 1 points.
     *           Note that the caller is responsible for checking the size of this vector.
     * @throws LogicalError Thrown when the size of the vector does not match the order. */
    virtual void setPoints(std::vector<Point> const &ps) {
        // must be virtual, because HLineSegment will need to redefine it
        if (ps.size() != order() + 1)
            THROW_LOGICALERROR("BezierCurve::setPoints: incorrect number of points in vector");
        for(unsigned i = 0; i <= order(); i++) {
            setPoint(i, ps[i]);
        }
    }
    /** @brief Access control points of the curve.
     * @param ix The (zero-based) index of the control point. Note that the caller is responsible for checking that this value is <= order().
     * @return The control point. No-reference return, use setPoint() to modify control points. */
    Point const operator[](unsigned ix) const { return Point(inner[X][ix], inner[Y][ix]); }
    /// @}

    /// @name Construct a Bezier curve with runtime-determined order.
    /// @{
    /** @brief Construct a curve from a vector of control points. */
    static BezierCurve *create(std::vector<Point> const &pts);
    /// @}

    // implementation of virtual methods goes here
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    virtual Point initialPoint() const { return inner.at0(); }
    virtual Point finalPoint() const { return inner.at1(); }
    virtual bool isDegenerate() const { return inner.isConstant(); }
    virtual void setInitial(Point const &v) { setPoint(0, v); }
    virtual void setFinal(Point const &v) { setPoint(order(), v); }
    virtual Rect boundsFast() const { return *bounds_fast(inner); }
    virtual Rect boundsExact() const { return *bounds_exact(inner); }
    virtual OptRect boundsLocal(OptInterval const &i, unsigned deg) const {
        if (!i) return OptRect();
        if(i->min() == 0 && i->max() == 1) return boundsFast();
        if(deg == 0) return bounds_local(inner, i);
        // TODO: UUUUUUGGGLLY
        if(deg == 1 && order() > 1) return OptRect(bounds_local(Geom::derivative(inner[X]), i),
                                                   bounds_local(Geom::derivative(inner[Y]), i));
        return OptRect();
    }
    virtual Curve *duplicate() const {
        return new BezierCurve(*this);
    }
    virtual Curve *portion(Coord f, Coord t) const {
        return new BezierCurve(Geom::portion(inner, f, t));
    }
    virtual Curve *reverse() const {
        return new BezierCurve(Geom::reverse(inner));
    }

    virtual Curve *transformed(Affine const &m) const {
        BezierCurve *ret = new BezierCurve();
        std::vector<Point> ps = points();
        for (unsigned i = 0;  i <= order(); i++) {
            ps[i] = ps[i] * m;
        }
        ret->setPoints(ps);
        return ret;
    }
    virtual Curve &operator*=(Translate const &m) {
        inner += m.vector();
        return *this;
    };

    virtual Curve *derivative() const {
        return new BezierCurve(Geom::derivative(inner[X]), Geom::derivative(inner[Y]));
    }
    virtual int degreesOfFreedom() const {
        return 2 * (order() + 1);
    }
    virtual std::vector<Coord> roots(Coord v, Dim2 d) const {
        return (inner[d] - v).roots();
    }
    virtual Coord length(Coord tolerance) const;
    virtual Point pointAt(Coord t) const { return inner.valueAt(t); }
    virtual std::vector<Point> pointAndDerivatives(Coord t, unsigned n) const { return inner.valueAndDerivatives(t, n); }
    virtual Coord valueAt(Coord t, Dim2 d) const { return inner[d].valueAt(t); }
    virtual D2<SBasis> toSBasis() const {return inner.toSBasis(); }
#endif
};

template <unsigned degree>
class BezierCurveN : public BezierCurve {

public:
  template <unsigned required_degree>
  static void assert_degree(BezierCurveN<required_degree> const *) {}

    /// @name Construct Bezier curves
    /// @{
    /** @brief Construct a Bezier curve of the specified order with all points zero. */
    BezierCurveN() {
        inner = D2<Bezier> (Bezier::Order(degree), Bezier::Order(degree));
    }

    /** @brief Construct from 2D Bezier polynomial. */
    explicit BezierCurveN(D2<Bezier > const &x) {
        inner = x;
    }

    /** @brief Construct from two 1D Bezier polynomials of the same order. */
    BezierCurveN(Bezier x, Bezier y) {
        inner = D2<Bezier > (x,y);
    }

    /** @brief Construct a Bezier curve from a vector of its control points. */
    BezierCurveN(std::vector<Point> const &points) {
        unsigned ord = points.size() - 1;
        if (ord != degree) THROW_LOGICALERROR("BezierCurve<degree> does not match number of points");
        for (unsigned d = 0; d < 2; ++d) {
            inner[d] = Bezier(Bezier::Order(ord));
            for(unsigned i = 0; i <= ord; i++)
                inner[d][i] = points[i][d];
        }
    }

    /** @brief Construct a linear segment from its endpoints. */
    BezierCurveN(Point c0, Point c1) {
      assert_degree<1>(this);
      for(unsigned d = 0; d < 2; d++)
          inner[d] = Bezier(c0[d], c1[d]);
    }

    /** @brief Construct a quadratic Bezier curve from its control points. */
    BezierCurveN(Point c0, Point c1, Point c2) {
      assert_degree<2>(this);
      for(unsigned d = 0; d < 2; d++)
          inner[d] = Bezier(c0[d], c1[d], c2[d]);
    }

    /** @brief Construct a cubic Bezier curve from its control points. */
    BezierCurveN(Point c0, Point c1, Point c2, Point c3) {
      assert_degree<3>(this);
      for(unsigned d = 0; d < 2; d++)
          inner[d] = Bezier(c0[d], c1[d], c2[d], c3[d]);
    }

    // default copy
    // default assign

    /// @}

    /** @brief Divide a Bezier curve into two curves
     * @param t Time value
     * @return Pair of Bezier curves \f$(\mathbf{D}, \mathbf{E})\f$ such that
     *         \f$\mathbf{D}[ [0,1] ] = \mathbf{C}[ [0,t] ]\f$ and
     *         \f$\mathbf{E}[ [0,1] ] = \mathbf{C}[ [t,1] ]\f$ */
    std::pair<BezierCurveN, BezierCurveN> subdivide(Coord t) const {
        std::pair<Bezier, Bezier> sx = inner[X].subdivide(t), sy = inner[Y].subdivide(t);
        return std::make_pair(
                   BezierCurveN(sx.first, sy.first),
                   BezierCurveN(sx.second, sy.second));
    }

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    virtual Curve *duplicate() const {
        return new BezierCurveN(*this);
    }
    virtual Curve *portion(Coord f, Coord t) const {
        if (degree == 1) {
            return new BezierCurveN<1>(pointAt(f), pointAt(t));
        } else {
            return new BezierCurveN(Geom::portion(inner, f, t));
        }
    }
    virtual Curve *reverse() const {
        if (degree == 1) {
            return new BezierCurveN<1>(finalPoint(), initialPoint()); 
        } else {
            return new BezierCurveN(Geom::reverse(inner));
        }
    }
    virtual Curve *transformed(Affine const &m) const {
        if (degree == 1) {
            return new BezierCurveN<1>(initialPoint() * m, finalPoint() * m);
        } else {
            BezierCurveN *ret = new BezierCurveN();
            std::vector<Point> ps = points();
            for (unsigned i = 0;  i <= degree; i++) {
                ps[i] = ps[i] * m;
            }
            ret->setPoints(ps);
            return ret;
        }
    }
    virtual Curve &operator*=(Translate const &m) {
        inner += m.vector();
        return *this;
    }
    virtual Curve *derivative() const;
    
    // the method below is defined so that LineSegment can specialize it
    virtual Coord nearestPoint(Point const& p, Coord from = 0, Coord to = 1) const {
        return Curve::nearestPoint(p, from, to);
    }
#endif
};

// BezierCurveN<0> is meaningless; specialize it out
template<> class BezierCurveN<0> : public BezierCurveN<1> { private: BezierCurveN();};

/** @brief Line segment.
 * Line segments are Bezier curves of order 1. They have only two control points,
 * the starting point and the ending point.
 * @ingroup Curves */
typedef BezierCurveN<1> LineSegment;

/** @brief Quadratic (order 2) Bezier curve.
 * @ingroup Curves */
typedef BezierCurveN<2> QuadraticBezier;

/** @brief Cubic (order 3) Bezier curve.
 * @ingroup Curves */
typedef BezierCurveN<3> CubicBezier;

template <unsigned degree>
inline
Curve *BezierCurveN<degree>::derivative() const {
    return new BezierCurveN<degree-1>(Geom::derivative(inner[X]), Geom::derivative(inner[Y]));
}

// optimized specializations for LineSegment
template <> Curve *BezierCurveN<1>::derivative() const;
template <> Coord BezierCurveN<1>::nearestPoint(Point const &, Coord, Coord) const;

inline Point middle_point(LineSegment const& _segment) {
    return ( _segment.initialPoint() + _segment.finalPoint() ) / 2;
}

inline Coord length(LineSegment const& seg) {
    return distance(seg.initialPoint(), seg.finalPoint());
}

Coord bezier_length(std::vector<Point> const &points, Coord tolerance = 0.01);
Coord bezier_length(Point p0, Point p1, Point p2, Coord tolerance = 0.01);
Coord bezier_length(Point p0, Point p1, Point p2, Point p3, Coord tolerance = 0.01);

} // end namespace Geom

#endif // _2GEOM_BEZIER_CURVE_H_

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

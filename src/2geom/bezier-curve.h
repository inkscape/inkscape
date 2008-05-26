/*
 * Bezier-Curve
 *
 * Authors:
 * 		MenTaLguY <mental@rydia.net>
 * 		Marco Cecchetti <mrcekets at gmail.com>
 * 
 * Copyright 2007-2008  authors
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




#ifndef _2GEOM_BEZIER_CURVE_H_
#define _2GEOM_BEZIER_CURVE_H_


#include "curve.h"
#include "sbasis-curve.h" // for non-native winding method
#include "bezier.h"

#include <algorithm>


namespace Geom 
{



template <unsigned order>
class BezierCurve : public Curve {
	
private:
  D2<Bezier > inner;
  
public:
  template <unsigned required_degree>
  static void assert_degree(BezierCurve<required_degree> const *) {}

  BezierCurve() : inner(Bezier::Order(order), Bezier::Order(order)) {
  }

  explicit BezierCurve(D2<Bezier > const &x) : inner(x) {}

  BezierCurve(Bezier x, Bezier y) : inner(x, y) {}

  // default copy
  // default assign

  BezierCurve(Point c0, Point c1) {
    assert_degree<1>(this);
    for(unsigned d = 0; d < 2; d++)
        inner[d] = Bezier(c0[d], c1[d]);
  }

  BezierCurve(Point c0, Point c1, Point c2) {
    assert_degree<2>(this);
    for(unsigned d = 0; d < 2; d++)
        inner[d] = Bezier(c0[d], c1[d], c2[d]);
  }

  BezierCurve(Point c0, Point c1, Point c2, Point c3) {
    assert_degree<3>(this);
    for(unsigned d = 0; d < 2; d++)
        inner[d] = Bezier(c0[d], c1[d], c2[d], c3[d]);
  }

  unsigned degree() const { return order; }

  Curve *duplicate() const { return new BezierCurve(*this); }

  Point initialPoint() const { return inner.at0(); }
  Point finalPoint() const { return inner.at1(); }

  bool isDegenerate() const { return inner.isConstant(); }

  void setInitial(Point v) { setPoint(0, v); }
  void setFinal(Point v)   { setPoint(order, v); }

  void setPoint(unsigned ix, Point v) { inner[X].setPoint(ix, v[X]); inner[Y].setPoint(ix, v[Y]); }
  Point const operator[](unsigned ix) const { return Point(inner[X][ix], inner[Y][ix]); }

  Rect boundsFast() const { return bounds_fast(inner); }
  Rect boundsExact() const { return bounds_exact(inner); }
  Rect boundsLocal(Interval i, unsigned deg) const {
      if(i.min() == 0 && i.max() == 1) return boundsFast();
      if(deg == 0) return bounds_local(inner, i);
      // TODO: UUUUUUGGGLLY
      if(deg == 1 && order > 1) return Rect(bounds_local(Geom::derivative(inner[X]), i),
                                            bounds_local(Geom::derivative(inner[Y]), i));
      return Rect(Interval(0,0), Interval(0,0));
  }
//TODO: local

//TODO: implement next 3 natively
  int winding(Point p) const {
    return SBasisCurve(toSBasis()).winding(p);
  }

  std::vector<double>
  roots(double v, Dim2 d) const {
      return (inner[d] - v).roots();
  }
  
  double nearestPoint( Point const& p, double from = 0, double to = 1 ) const
  {
	  return Curve::nearestPoint(p, from, to);
  }
  
  void setPoints(std::vector<Point> ps) {
    for(unsigned i = 0; i <= order; i++) {
      setPoint(i, ps[i]);
    }
  }
  std::vector<Point> points() const { return bezier_points(inner); }

  std::pair<BezierCurve<order>, BezierCurve<order> > subdivide(Coord t) const {
    std::pair<Bezier, Bezier > sx = inner[X].subdivide(t), sy = inner[Y].subdivide(t);
    return std::pair<BezierCurve<order>, BezierCurve<order> >(
               BezierCurve<order>(sx.first, sy.first),
               BezierCurve<order>(sx.second, sy.second));
  }

  Curve *portion(double f, double t) const {
    return new BezierCurve(Geom::portion(inner, f, t));
  }

  Curve *reverse() const {
    return new BezierCurve(Geom::reverse(inner));
  }

  Curve *transformed(Matrix const &m) const {
    BezierCurve *ret = new BezierCurve();
    std::vector<Point> ps = points();
    for(unsigned i = 0;  i <= order; i++) ps[i] = ps[i] * m;
    ret->setPoints(ps);
    return ret;
  }

  Curve *derivative() const {
     if(order > 1)
        return new BezierCurve<order-1>(Geom::derivative(inner[X]), Geom::derivative(inner[Y]));
     else if (order == 1) {
        double dx = inner[X][1] - inner[X][0], dy = inner[Y][1] - inner[Y][0];
        return new BezierCurve<1>(Point(dx,dy),Point(dx,dy));
     }
  }

  Point pointAt(double t) const { return inner.valueAt(t); }
  std::vector<Point> pointAndDerivatives(Coord t, unsigned n) const { return inner.valueAndDerivatives(t, n); }

  double valueAt(double t, Dim2 d) const { return inner[d].valueAt(t); }

  D2<SBasis> toSBasis() const {return inner.toSBasis(); }

protected:
  BezierCurve(Point c[]) {
    Coord x[order+1], y[order+1];
    for(unsigned i = 0; i <= order; i++) {
        x[i] = c[i][X]; y[i] = c[i][Y];
    }
    inner = Bezier(x, y);
  }
};

// BezierCurve<0> is meaningless; specialize it out
template<> class BezierCurve<0> : public BezierCurve<1> { public: BezierCurve(); BezierCurve(Bezier x, Bezier y); };

typedef BezierCurve<1> LineSegment;
typedef BezierCurve<2> QuadraticBezier;
typedef BezierCurve<3> CubicBezier;


template<>
inline
double LineSegment::nearestPoint(Point const& p, double from, double to) const
{
	if ( from > to ) std::swap(from, to);
	Point ip = pointAt(from);
	Point fp = pointAt(to);
	Point v = fp - ip;
	double t = dot( p - ip, v ) / L2sq(v);
	if ( t <= 0 )  		return from;
	else if ( t >= 1 )  return to;
	else 				return from + t*(to-from);
}

inline
Point middle_point(LineSegment const& _segment)
{
	return ( _segment.initialPoint() + _segment.finalPoint() ) / 2;
}

inline
double length(LineSegment const& _segment)
{
	return distance(_segment.initialPoint(), _segment.finalPoint());
}

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

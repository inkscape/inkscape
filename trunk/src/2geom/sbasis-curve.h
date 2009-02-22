/**
 * \file
 * \brief Symmetric Power Basis Curve
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




#ifndef _2GEOM_SBASIS_CURVE_H_
#define _2GEOM_SBASIS_CURVE_H_


#include <2geom/curve.h>


namespace Geom 
{

class SBasisCurve : public Curve {
	
private:
  SBasisCurve();
  D2<SBasis> inner;
  
public:
  explicit SBasisCurve(D2<SBasis> const &sb) : inner(sb) {}
  explicit SBasisCurve(Curve const &other) : inner(other.toSBasis()) {}
  Curve *duplicate() const { return new SBasisCurve(*this); }

  Point initialPoint() const    { return inner.at0(); }
  Point finalPoint() const      { return inner.at1(); }
  bool isDegenerate() const     { return inner.isConstant(); }
  Point pointAt(Coord t) const  { return inner.valueAt(t); }
  std::vector<Point> pointAndDerivatives(Coord t, unsigned n) const {
      return inner.valueAndDerivatives(t, n);
  }
  double valueAt(Coord t, Dim2 d) const { return inner[d].valueAt(t); }

  void setInitial(Point v) { for(unsigned d = 0; d < 2; d++) { inner[d][0][0] = v[d]; } }
  void setFinal(Point v)   { for(unsigned d = 0; d < 2; d++) { inner[d][0][1] = v[d]; } }

  virtual OptRect boundsFast() const  { return bounds_fast(inner); }
  virtual OptRect boundsExact() const { return bounds_exact(inner); }
  virtual OptRect boundsLocal(OptInterval i, unsigned deg) const { return bounds_local(inner, i, deg); }

  std::vector<double> roots(double v, Dim2 d) const { return Geom::roots(inner[d] - v); }
  
  double nearestPoint( Point const& p, double from = 0, double to = 1 ) const
  {
	  return nearest_point(p, inner, from, to);
  }
  
  std::vector<double> 
  allNearestPoints( Point const& p, double from = 0, double to = 1 ) const
  {
	  return all_nearest_points(p, inner, from, to);
  }

  Curve *portion(double f, double t) const {
    return new SBasisCurve(Geom::portion(inner, f, t));
  }

  Curve *transformed(Matrix const &m) const {
    return new SBasisCurve(inner * m);
  }

  Curve *derivative() const {
    return new SBasisCurve(Geom::derivative(inner));
  }

  D2<SBasis> toSBasis() const { return inner; }

  virtual int degreesOfFreedom() const { return inner[0].degreesOfFreedom() + inner[1].degreesOfFreedom();
  }
};


} // end namespace Geom


#endif // _2GEOM_SBASIS_CURVE_H_




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

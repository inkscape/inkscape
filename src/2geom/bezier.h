/*
 * bezier.h
 *
 * Copyright 2007  MenTaLguY <mental@rydia.net>
 * Copyright 2007  Michael Sloan <mgsloan@gmail.com>
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
 *
 */

#ifndef SEEN_BEZIER_H
#define SEEN_BEZIER_H

#include "coord.h"
#include "isnan.h"
#include "bezier-to-sbasis.h"

namespace Geom {

template <unsigned order>
class Bezier {
private:
  Coord c_[order+1];

protected:
  Bezier(Coord c[]) {
    std::copy(c, c+order+1, c_);
  }

public:
  template <unsigned required_order>
  static void assert_order(Bezier<required_order> const *) {}

  Bezier() {}

  //Construct an order-0 bezier (constant Bézier)
  explicit Bezier(Coord c0) {
    assert_order<0>(this);
    c_[0] = c0;
  }

  //Construct an order-1 bezier (linear Bézier)
  Bezier(Coord c0, Coord c1) {
    assert_order<1>(this);
    c_[0] = c0; c_[1] = c1;
  }

  //Construct an order-2 bezier (quadratic Bézier)
  Bezier(Coord c0, Coord c1, Coord c2) {
    assert_order<2>(this);
    c_[0] = c0; c_[1] = c1; c_[2] = c2;
  }

  //Construct an order-3 bezier (cubic Bézier)
  Bezier(Coord c0, Coord c1, Coord c2, Coord c3) {
    assert_order<3>(this);
    c_[0] = c0; c_[1] = c1; c_[2] = c2; c_[3] = c3;
  }

  inline unsigned degree() const { return order; }

  //IMPL: FragmentConcept
  typedef Coord output_type;
  inline bool isZero() const { 
     for(int i = 0; i <= order; i++) {
       if(c_[i] != 0) return false;
     }
     return true;
  }
  inline bool isFinite() const {
    for(int i = 0; i <= order; i++) {
      if(!is_finite(c_[i])) return false;
    }
    return true;
  }
  inline Coord at0() const { return c_[0]; }
  inline Coord at1() const { return c_[order]; }

  inline SBasis toSBasis() const { return bezier_to_sbasis<order>(c_); }

  inline Interval bounds_fast() const { return Interval::fromArray(c_, order+1); }
  //TODO: better bounds exact
  inline Interval bounds_exact() const { return toSBasis().bounds_exact(); }
  inline Interval bounds_local(double u, double v) const { return toSBasis().bounds_local(u, v); }

  //Only mutator
  inline Coord &operator[](int index) { return c_[index]; }
  inline Coord const &operator[](int index) const { return c_[index]; }

  Maybe<int> winding(Point p) const {
    return sbasis_winding(toSBasis(), p);
  }
  
  Point pointAndDerivativesAt(Coord t, unsigned n_derivs, Point *derivs) const {
    //TODO
    return Point(0,0);
  }
};

template<unsigned order>
Bezier<order> reverse(const Bezier<order> & a) {
  Bezier<order> result;
  for(int i = 0; i <= order; i++)
    result[i] = a[order - i];
  return result;
}

template<unsigned order>
vector<Point> bezier_points(const D2<Bezier<order> > & a) {
  vector<Point> result;
  for(int i = 0; i <= order; i++) {
    Point p;
    for(unsigned d = 0; d < 2; d++) p[d] = a[d][i];
    result[i] = p;
  }
  return result;
}

}
#endif //SEEN_BEZIER_H

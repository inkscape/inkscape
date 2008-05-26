/*
 * Abstract Curve Type
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




#ifndef _2GEOM_CURVE_H_
#define _2GEOM_CURVE_H_


#include "coord.h"
#include "point.h"
#include "interval.h"
#include "nearest-point.h"
#include "sbasis.h"
#include "d2.h"
#include "matrix.h"
#include "exception.h"

#include <vector>


namespace Geom 
{

class Curve;

struct CurveHelpers {
protected:
  static int root_winding(Curve const &c, Point p);
};

class Curve : private CurveHelpers {
public:
  virtual ~Curve() {}

  virtual Point initialPoint() const = 0;
  virtual Point finalPoint() const = 0;

  virtual bool isDegenerate() const = 0;

  virtual Curve *duplicate() const = 0;

  virtual Rect boundsFast() const = 0;
  virtual Rect boundsExact() const = 0;
  virtual Rect boundsLocal(Interval i, unsigned deg) const = 0;
  Rect boundsLocal(Interval i) const { return boundsLocal(i, 0); }

  virtual std::vector<double> roots(double v, Dim2 d) const = 0;

  virtual int winding(Point p) const { return root_winding(*this, p); }

  //mental: review these
  virtual Curve *portion(double f, double t) const = 0;
  virtual Curve *reverse() const { return portion(1, 0); }
  virtual Curve *derivative() const = 0;

  virtual void setInitial(Point v) = 0;
  virtual void setFinal(Point v) = 0;
  
  virtual
  double nearestPoint( Point const& p, double from = 0, double to = 1 ) const
  {
	  return nearest_point(p, toSBasis(), from, to);
  }
  
  virtual
  std::vector<double> 
  allNearestPoints( Point const& p, double from = 0, double to = 1 ) const
  {
	  return all_nearest_points(p, toSBasis(), from, to);
  }

  virtual Curve *transformed(Matrix const &m) const = 0;

  virtual Point pointAt(Coord t) const { return pointAndDerivatives(t, 1).front(); }
  virtual Coord valueAt(Coord t, Dim2 d) const { return pointAt(t)[d]; }
  virtual std::vector<Point> pointAndDerivatives(Coord t, unsigned n) const = 0;
  virtual D2<SBasis> toSBasis() const = 0;
};


} // end namespace Geom


#endif // _2GEOM_CURVE_H_



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

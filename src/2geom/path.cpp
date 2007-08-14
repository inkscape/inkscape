/*
 * Path - Series of continuous curves
 *   
 * Copyright 2007  MenTaLguY <mental@rydia.net>
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

#include "path.h"

namespace Geom {

namespace {

enum Cmp {
  LESS_THAN=-1,
  GREATER_THAN=1,
  EQUAL_TO=0
};

template <typename T1, typename T2>
inline Cmp cmp(T1 const &a, T2 const &b) {
  if ( a < b ) {
    return LESS_THAN;
  } else if ( b < a ) {
    return GREATER_THAN;
  } else {
    return EQUAL_TO;
  }
}

}

boost::optional<int> CurveHelpers::sbasis_winding(D2<SBasis> const &sb, Point p) {
  Interval ix = bounds_fast(sb[X]);

  if ( p[X] > ix.max() ) { /* ray does not intersect bbox */
    return 0;
  }

  SBasis fy = sb[Y];
  fy -= p[Y];

  if (fy.empty()) { /* coincident horizontal segment */
    return boost::optional<int>();
  }

  if ( p[X] < ix.min() ) { /* ray does not originate in bbox */
    double y = p[Y];
    /* winding determined by position of endpoints */
    Cmp initial_to_ray = cmp(fy[0][0], y);
    Cmp final_to_ray = cmp(fy[0][1], y);
    switch (cmp(final_to_ray, initial_to_ray)) {
    case GREATER_THAN:
      /* exclude final endpoint */
      return ( final_to_ray != EQUAL_TO );
    case LESS_THAN:
      /* exclude final endpoint */
      return -( final_to_ray != EQUAL_TO );
    default:
      /* any intersections cancel out */
      return 0;
    }
  } else { /* ray originates in bbox */
    std::vector<double> ts = roots(fy);

    static const unsigned MAX_DERIVATIVES=8;
    boost::optional<SBasis> ds[MAX_DERIVATIVES];
    ds[0] = derivative(fy);

    /* winding determined by summing signs of derivatives at intersections */
    int winding=0;
    for ( std::vector<double>::iterator ti = ts.begin()
        ; ti != ts.end()
        ; ++ti )
    { 
      double t = *ti;
      if ( sb[X](t) >= p[X] ) { /* root is ray intersection */
        for ( boost::optional<SBasis> *di = ds
            ; di != ( ds + MAX_DERIVATIVES )
            ; ++di )
        {
          if (!*di) {
            *di = derivative(**(di-1));
          }
          switch (cmp((**di)(t), 0)) {
          case GREATER_THAN:
            if ( t < 1 ) { /* exclude final endpoint */
              winding += 1;
            }
            goto next_root;
          case LESS_THAN:
            if ( t < 1 ) { /* exclude final endpoint */
              winding -= 1;
            }
            goto next_root;
          default: (void)0;
            /* give up */
          };
        }
      } 
next_root: (void)0;
    }
    
    return winding;
  }
}

Rect BezierHelpers::bounds(unsigned degree, Point const *points) {
  Point min=points[0];
  Point max=points[0];
  for ( unsigned i = 1 ; i <= degree ; ++i ) {
    for ( unsigned axis = 0 ; axis < 2 ; ++axis ) {
      min[axis] = std::min(min[axis], points[i][axis]);
      max[axis] = std::max(max[axis], points[i][axis]);
    }
  }
  return Rect(min, max);
}

Point BezierHelpers::point_and_derivatives_at(Coord t,
                                              unsigned degree,
                                              Point const *points,
                                              unsigned n_derivs,
                                              Point *derivs)
{
  return Point(0,0); // TODO
}

Geom::Point
BezierHelpers::subdivideArr(Coord t,              // Parameter value
                            unsigned degree,      // Degree of bezier curve
                            Geom::Point const *V, // Control pts
                            Geom::Point *Left,    // RETURN left half ctl pts
                            Geom::Point *Right)   // RETURN right half ctl pts
{
    Geom::Point Vtemp[degree+1][degree+1];

    /* Copy control points	*/
    std::copy(V, V+degree+1, Vtemp[0]);

    /* Triangle computation	*/
    for (unsigned i = 1; i <= degree; i++) {	
        for (unsigned j = 0; j <= degree - i; j++) {
            Vtemp[i][j] = lerp(t, Vtemp[i-1][j], Vtemp[i-1][j+1]);
        }
    }
    
    for (unsigned j = 0; j <= degree; j++)
        Left[j]  = Vtemp[j][0];
    for (unsigned j = 0; j <= degree; j++)
        Right[j] = Vtemp[degree-j][j];

    return (Vtemp[degree][0]);
}

void Path::swap(Path &other) {
  std::swap(curves_, other.curves_);
  std::swap(closed_, other.closed_);
  std::swap(*final_, *other.final_);
  curves_[curves_.size()-1] = final_;
  other.curves_[other.curves_.size()-1] = other.final_;
}

Rect Path::bounds_fast() const {
  Rect bounds=front().bounds_fast();
  for ( const_iterator iter=++begin(); iter != end() ; ++iter ) {
    bounds.unionWith(iter->bounds_fast());
  }
  return bounds;
}

Rect Path::bounds_exact() const {
  Rect bounds=front().bounds_exact();
  for ( const_iterator iter=++begin(); iter != end() ; ++iter ) {
    bounds.unionWith(iter->bounds_exact());
  }
  return bounds;
}

int Path::winding(Point p) const {
  int winding = 0;
  boost::optional<Cmp> ignore = boost::optional<Cmp>();
  for ( const_iterator iter = begin()
      ; iter != end_closed()
      ; ++iter )
  {
    boost::optional<int> w = iter->winding(p);
    if (w) {
      winding += *w;
      ignore = boost::optional<Cmp>();
    } else {
      Point initial = iter->initialPoint();
      Point final = iter->finalPoint();
      switch (cmp(initial[X], final[X])) {
      case GREATER_THAN:
        if ( !ignore || *ignore != GREATER_THAN ) { /* ignore repeated */
          winding += 1;
          ignore = GREATER_THAN;
        }
        break;
      case LESS_THAN:
        if ( !ignore || *ignore != LESS_THAN ) { /* ignore repeated */
          if ( p[X] < final[X] ) { /* ignore final point */
            winding -= 1;
            ignore = LESS_THAN;
          }
        }
        break;
      case EQUAL_TO:
        /* always ignore null segments */
        break;
      }
    }
  }
  return winding;
}

void Path::append(Curve const &curve) {
  if ( curves_.front() != final_ && curve.initialPoint() != (*final_)[0] ) {
    throw ContinuityError();
  }
  do_append(curve.duplicate());
}

void Path::append(D2<SBasis> const &curve) {
  if ( curves_.front() != final_ ) {
    for ( int i = 0 ; i < 2 ; ++i ) {
      if ( curve[i][0][0] != (*final_)[0][i] ) {
        throw ContinuityError();
      }
    }
  }
  do_append(new SBasisCurve(curve));
}

void Path::do_update(Sequence::iterator first_replaced,
                     Sequence::iterator last_replaced,
                     Sequence::iterator first,
                    Sequence::iterator last)
{
  // note: modifies the contents of [first,last)

  check_continuity(first_replaced, last_replaced, first, last);
  delete_range(first_replaced, last_replaced);
  if ( ( last - first ) == ( last_replaced - first_replaced ) ) {
    std::copy(first, last, first_replaced);
  } else {
    // this approach depends on std::vector's behavior WRT iterator stability
    curves_.erase(first_replaced, last_replaced);
    curves_.insert(first_replaced, first, last);
  }

  if ( curves_.front() != final_ ) {
    (*final_)[0] = back().finalPoint();
    (*final_)[1] = front().initialPoint();
  }
}

void Path::do_append(Curve *curve) {
  if ( curves_.front() == final_ ) {
    (*final_)[1] = curve->initialPoint();
  }
  curves_.insert(curves_.end()-1, curve);
  (*final_)[0] = curve->finalPoint();
}

void Path::delete_range(Sequence::iterator first, Sequence::iterator last) {
  for ( Sequence::iterator iter=first ; iter != last ; ++iter ) {
    delete *iter;
  }
}

void Path::check_continuity(Sequence::iterator first_replaced,
                            Sequence::iterator last_replaced,
                            Sequence::iterator first,
                            Sequence::iterator last)
{
  if ( first != last ) {
    if ( first_replaced != curves_.begin() ) {
      if ( (*first_replaced)->initialPoint() != (*first)->initialPoint() ) {
        throw ContinuityError();
      }
    }
    if ( last_replaced != (curves_.end()-1) ) {
      if ( (*(last_replaced-1))->finalPoint() != (*(last-1))->finalPoint() ) {
        throw ContinuityError();
      }
    }
  } else if ( first_replaced != last_replaced && first_replaced != curves_.begin() && last_replaced != curves_.end()-1) {
    if ( (*first_replaced)->initialPoint() !=
         (*(last_replaced-1))->finalPoint() )
    {
      throw ContinuityError();
    }
  }
}

Rect SBasisCurve::bounds_fast() const {
  throw NotImplemented();
  return Rect(Point(0,0), Point(0,0));
}

Rect SBasisCurve::bounds_exact() const {
  throw NotImplemented();
  return Rect(Point(0,0), Point(0,0));
}

Point SBasisCurve::pointAndDerivativesAt(Coord t, unsigned n_derivs, Point *derivs) const {
  throw NotImplemented();
  return Point(0,0);
}

Path const &SBasisCurve::subdivide(Coord t, Path &out) const {
  throw NotImplemented();
}

Rect SVGEllipticalArc::bounds_fast() const {
    throw NotImplemented();
}
Rect SVGEllipticalArc::bounds_exact() const {
    throw NotImplemented();
}

Point SVGEllipticalArc::pointAndDerivativesAt(Coord t, unsigned n_derivs, Point *derivs) const {
    throw NotImplemented();
}

D2<SBasis> SVGEllipticalArc::sbasis() const {
    throw NotImplemented();
}

}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=2:tabstop=8:softtabstop=2 :
*/


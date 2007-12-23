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

#ifndef SEEN_GEOM_PATH_H
#define SEEN_GEOM_PATH_H

#include "point.h"
#include <iterator>
#include <algorithm>
#include "exception.h"
#include "d2.h"
#include "matrix.h"
#include "bezier.h"
#include "crossing.h"
#include "utils.h"

namespace Geom {

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

  virtual Curve *transformed(Matrix const &m) const = 0;

  virtual Point pointAt(Coord t) const { return pointAndDerivatives(t, 1).front(); }
  virtual Coord valueAt(Coord t, Dim2 d) const { return pointAt(t)[d]; }
  virtual std::vector<Point> pointAndDerivatives(Coord t, unsigned n) const = 0;
  virtual D2<SBasis> toSBasis() const = 0;
};

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

  Rect boundsFast() const  { return bounds_fast(inner); }
  Rect boundsExact() const { return bounds_exact(inner); }
  Rect boundsLocal(Interval i, unsigned deg) const { return bounds_local(inner, i, deg); }

  std::vector<double> roots(double v, Dim2 d) const { return Geom::roots(inner[d] - v); }

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

};

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
  void setFinal(Point v)   { setPoint(1, v); }

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
        if(dx == 0) return new BezierCurve<1>(Point(0,0), Point(0,0));
        double slope = dy / dx;
        Geom::Point pnt;
        if(slope == 0) pnt = Geom::Point(0, 0); else pnt = Geom::Point(slope, 1./slope);
        return new BezierCurve<1>(pnt, pnt);
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

class SVGEllipticalArc : public Curve {
public:
  SVGEllipticalArc() {}

  SVGEllipticalArc(Point initial, double rx, double ry,
                   double x_axis_rotation, bool large_arc,
                   bool sweep, Point final)
  : initial_(initial), rx_(rx), ry_(ry), x_axis_rotation_(x_axis_rotation),
    large_arc_(large_arc), sweep_(sweep), final_(final)
  {}

  Curve *duplicate() const { return new SVGEllipticalArc(*this); }

  Point initialPoint() const { return initial_; }
  Point finalPoint() const { return final_; }

  void setInitial(Point v) { initial_ = v; }
  void setFinal(Point v) { final_ = v; }

  //TODO: implement funcs

  bool isDegenerate() const { return toSBasis().isConstant(); }
  Rect boundsFast() const;
  Rect boundsExact() const;
  Rect boundsLocal(Interval i, unsigned deg) const;

  int winding(Point p) const {
    return SBasisCurve(toSBasis()).winding(p);
  }

  std::vector<double> roots(double v, Dim2 d) const;

  inline std::pair<SVGEllipticalArc, SVGEllipticalArc>
  subdivide(Coord t) {
    SVGEllipticalArc a(*this), b(*this);
    a.final_ = b.initial_ = pointAt(t);
    return std::pair<SVGEllipticalArc, SVGEllipticalArc>(a, b);
  }

// TODO: how are the flags affected by reducing an arc from more than 180deg to less than 180deg?
  Curve *portion(double f, double t) const {
    SVGEllipticalArc *ret = new SVGEllipticalArc (*this);
    ret->initial_ = pointAt(f);
    ret->final_ = pointAt(t);
    return ret;
  }

// TODO: incomplete/buggy
  Curve *reverse(double /*f*/, double /*t*/) const {
    SVGEllipticalArc *ret = new SVGEllipticalArc (*this);
    ret->initial_ = final_;
    ret->final_ = initial_;
    return ret;
  }

  //TODO: this next def isn't right
  Curve *transformed(Matrix const & m) const {
    SVGEllipticalArc *ret = new SVGEllipticalArc (*this);
    ret->initial_ = initial_ * m;
    ret->final_ = final_ * m;
    return ret;
  }

  Curve *derivative() const { throwNotImplemented(); }

  std::vector<Point> pointAndDerivatives(Coord t, unsigned n) const;

  D2<SBasis> toSBasis() const;

private:
  Point initial_;
  double rx_;
  double ry_;
  double x_axis_rotation_;
  bool large_arc_;
  bool sweep_;
  Point final_;
};

template <typename IteratorImpl>
class BaseIterator
: public std::iterator<std::forward_iterator_tag, Curve const>
{
public:
  BaseIterator() {}

  // default construct
  // default copy

  bool operator==(BaseIterator const &other) {
    return other.impl_ == impl_;
  }
  bool operator!=(BaseIterator const &other) {
    return other.impl_ != impl_;
  }

  Curve const &operator*() const { return **impl_; }
  Curve const *operator->() const { return *impl_; }

  BaseIterator &operator++() {
    ++impl_;
    return *this;
  }

  BaseIterator operator++(int) {
    BaseIterator old=*this;
    ++(*this);
    return old;
  }

private:
  BaseIterator(IteratorImpl const &pos) : impl_(pos) {}

  IteratorImpl impl_;
  friend class Path;
};

template <typename Iterator>
class DuplicatingIterator
: public std::iterator<std::input_iterator_tag, Curve *>
{
public:
  DuplicatingIterator() {}
  DuplicatingIterator(Iterator const &iter) : impl_(iter) {}

  bool operator==(DuplicatingIterator const &other) {
    return other.impl_ == impl_;
  }
  bool operator!=(DuplicatingIterator const &other) {
    return other.impl_ != impl_;
  }

  Curve *operator*() const { return (*impl_)->duplicate(); }

  DuplicatingIterator &operator++() {
    ++impl_;
    return *this;
  }
  DuplicatingIterator operator++(int) {
    DuplicatingIterator old=*this;
    ++(*this);
    return old;
  }

private:
  Iterator impl_;
};

class Path {
private:
  typedef std::vector<Curve *> Sequence;

public:
  typedef BaseIterator<Sequence::iterator> iterator;
  typedef BaseIterator<Sequence::const_iterator> const_iterator;
  typedef Sequence::size_type size_type;
  typedef Sequence::difference_type difference_type;

  Path()
  : final_(new LineSegment()), closed_(false)
  {
    curves_.push_back(final_);
  }

  Path(Path const &other)
  : final_(new LineSegment()), closed_(other.closed_)
  {
    curves_.push_back(final_);
    insert(begin(), other.begin(), other.end());
  }

  explicit Path(Point p)
  : final_(new LineSegment(p, p)), closed_(false)
  {
    curves_.push_back(final_);
  }

  template <typename Impl>
  Path(BaseIterator<Impl> first, BaseIterator<Impl> last, bool closed=false)
  : closed_(closed), final_(new LineSegment())
  {
    curves_.push_back(final_);
    insert(begin(), first, last);
  }

  virtual ~Path() {
      delete_range(curves_.begin(), curves_.end()-1);
      delete final_;
  }

  Path &operator=(Path const &other) {
    clear();
    insert(begin(), other.begin(), other.end());
    close(other.closed_);
    return *this;
  }

  void swap(Path &other);

  Curve const &operator[](unsigned i) const { return *curves_[i]; }

  iterator begin() { return curves_.begin(); }
  iterator end() { return curves_.end()-1; }

  Curve const &front() const { return *curves_[0]; }
  Curve const &back() const { return *curves_[curves_.size()-2]; }

  const_iterator begin() const { return curves_.begin(); }
  const_iterator end() const { return curves_.end()-1; }

  const_iterator end_open() const { return curves_.end()-1; }
  const_iterator end_closed() const { return curves_.end(); }
  const_iterator end_default() const {
    return ( closed_ ? end_closed() : end_open() );
  }

  size_type size() const { return curves_.size()-1; }
  size_type max_size() const { return curves_.max_size()-1; }

  bool empty() const { return curves_.size() == 1; }
  bool closed() const { return closed_; }
  void close(bool closed=true) { closed_ = closed; }

  Rect boundsFast() const;
  Rect boundsExact() const;

  Piecewise<D2<SBasis> > toPwSb() const {
    Piecewise<D2<SBasis> > ret;
    ret.push_cut(0);
    unsigned i = 1;
    // ignore that path is closed or open. pw<d2<>> is always open.
    for(const_iterator it = begin(); it != end(); ++it) {
      if (!it->isDegenerate()) {
        ret.push(it->toSBasis(), i++);
      }
    }
    return ret;
  }

  Path operator*(Matrix const &m) const {
    Path ret;
    for(const_iterator it = begin(); it != end(); ++it) {
      Curve *temp = it->transformed(m);
      //Possible point of discontinuity?
      ret.append(*temp);
      delete temp;
    }
    return ret;
  }

  Point pointAt(double t) const {
    if(empty()) return Point(0,0);
    double i, f = modf(t, &i);
    if(i == size() && f == 0) { i--; }
    assert(i >= 0 && i <= size());
    return (*this)[unsigned(i)].pointAt(f);
  }

  double valueAt(double t, Dim2 d) const {
    if(empty()) return 0;
    double i, f = modf(t, &i);
    if(i == size() && f == 0) { i--; }
    assert(i >= 0 && i <= size());
    return (*this)[unsigned(i)].valueAt(f, d);
  }

  std::vector<double> roots(double v, Dim2 d) const {
    std::vector<double> res;
    for(unsigned i = 0; i <= size(); i++) {
      std::vector<double> temp = (*this)[i].roots(v, d);
      for(unsigned j = 0; j < temp.size(); j++)
        res.push_back(temp[j] + i);
    }
    return res;
  }

  void appendPortionTo(Path &p, double f, double t) const;

  Path portion(double f, double t) const {
    Path ret;
    ret.close(false);
    appendPortionTo(ret, f, t);
    return ret;
  }
  Path portion(Interval i) const { return portion(i.min(), i.max()); }

  Path reverse() const {
    Path ret;
    ret.close(closed_);
    for(int i = size() - (closed_ ? 0 : 1); i >= 0; i--) {
      //TODO: do we really delete?
      Curve *temp = (*this)[i].reverse();
      ret.append(*temp);
      delete temp;
    }
    return ret;
  }

  void insert(iterator pos, Curve const &curve) {
    Sequence source(1, curve.duplicate());
    try {
      do_update(pos.impl_, pos.impl_, source.begin(), source.end());
    } catch (...) {
      delete_range(source.begin(), source.end());
      throw;
    }
  }

  template <typename Impl>
  void insert(iterator pos, BaseIterator<Impl> first, BaseIterator<Impl> last)
  {
    Sequence source(DuplicatingIterator<Impl>(first.impl_),
                    DuplicatingIterator<Impl>(last.impl_));
    try {
      do_update(pos.impl_, pos.impl_, source.begin(), source.end());
    } catch (...) {
      delete_range(source.begin(), source.end());
      throw;
    }
  }

  void clear() {
    do_update(curves_.begin(), curves_.end()-1,
              curves_.begin(), curves_.begin());
  }

  void erase(iterator pos) {
    do_update(pos.impl_, pos.impl_+1, curves_.begin(), curves_.begin());
  }

  void erase(iterator first, iterator last) {
    do_update(first.impl_, last.impl_, curves_.begin(), curves_.begin());
  }

  void replace(iterator replaced, Curve const &curve) {
    Sequence source(1, curve.duplicate());
    try {
      do_update(replaced.impl_, replaced.impl_+1, source.begin(), source.end());
    } catch (...) {
      delete_range(source.begin(), source.end());
      throw;
    }
  }

  void replace(iterator first_replaced, iterator last_replaced,
               Curve const &curve)
  {
    Sequence source(1, curve.duplicate());
    try {
      do_update(first_replaced.impl_, last_replaced.impl_,
                source.begin(), source.end());
    } catch (...) {
      delete_range(source.begin(), source.end());
      throw;
    }
  }

  template <typename Impl>
  void replace(iterator replaced,
               BaseIterator<Impl> first, BaseIterator<Impl> last)
  {
    Sequence source(DuplicatingIterator<Impl>(first.impl_),
                    DuplicatingIterator<Impl>(last.impl_));
    try {
      do_update(replaced.impl_, replaced.impl_+1, source.begin(), source.end());
    } catch (...) {
      delete_range(source.begin(), source.end());
      throw;
    }
  }

  template <typename Impl>
  void replace(iterator first_replaced, iterator last_replaced,
               BaseIterator<Impl> first, BaseIterator<Impl> last)
  {
    Sequence source(first.impl_, last.impl_);
    try {
      do_update(first_replaced.impl_, last_replaced.impl_,
                source.begin(), source.end());
    } catch (...) {
      delete_range(source.begin(), source.end());
      throw;
    }
  }

  void start(Point p) {
    clear();
    final_->setPoint(0, p);
    final_->setPoint(1, p);
  }

  Point initialPoint() const { return (*final_)[1]; }
  Point finalPoint() const { return (*final_)[0]; }

  void append(Curve const &curve);
  void append(D2<SBasis> const &curve);

  template <typename CurveType, typename A>
  void appendNew(A a) {
    do_append(new CurveType((*final_)[0], a));
  }

  template <typename CurveType, typename A, typename B>
  void appendNew(A a, B b) {
    do_append(new CurveType((*final_)[0], a, b));
  }

  template <typename CurveType, typename A, typename B, typename C>
  void appendNew(A a, B b, C c) {
    do_append(new CurveType((*final_)[0], a, b, c));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D>
  void appendNew(A a, B b, C c, D d) {
    do_append(new CurveType((*final_)[0], a, b, c, d));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E>
  void appendNew(A a, B b, C c, D d, E e) {
    do_append(new CurveType((*final_)[0], a, b, c, d, e));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F>
  void appendNew(A a, B b, C c, D d, E e, F f) {
    do_append(new CurveType((*final_)[0], a, b, c, d, e, f));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F,
                                typename G>
  void appendNew(A a, B b, C c, D d, E e, F f, G g) {
    do_append(new CurveType((*final_)[0], a, b, c, d, e, f, g));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F,
                                typename G, typename H>
  void appendNew(A a, B b, C c, D d, E e, F f, G g, H h) {
    do_append(new CurveType((*final_)[0], a, b, c, d, e, f, g, h));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F,
                                typename G, typename H, typename I>
  void appendNew(A a, B b, C c, D d, E e, F f, G g, H h, I i) {
    do_append(new CurveType((*final_)[0], a, b, c, d, e, f, g, h, i));
  }

private:
  void do_update(Sequence::iterator first_replaced,
                 Sequence::iterator last_replaced,
                 Sequence::iterator first,
                 Sequence::iterator last);

  void do_append(Curve *curve);

  void delete_range(Sequence::iterator first, Sequence::iterator last);

  void check_continuity(Sequence::iterator first_replaced,
                        Sequence::iterator last_replaced,
                        Sequence::iterator first,
                        Sequence::iterator last);

  Sequence curves_;
  LineSegment *final_;
  bool closed_;
};

inline static Piecewise<D2<SBasis> > paths_to_pw(std::vector<Path> paths) {
    Piecewise<D2<SBasis> > ret = paths[0].toPwSb();
    for(unsigned i = 1; i < paths.size(); i++) {
        ret.concat(paths[i].toPwSb());
    }
    return ret;
}

/*
class PathPortion : public Curve {
  Path *source;
  double f, t;
  boost::optional<Path> result;

  public:
  double from() const { return f; }
  double to() const { return t; }

  explicit PathPortion(Path *s, double fp, double tp) : source(s), f(fp), t(tp) {}
  Curve *duplicate() const { return new PathPortion(*this); }

  Point initialPoint() const { return source->pointAt(f); }
  Point finalPoint() const { return source->pointAt(t); }

  Path actualPath() {
    if(!result) *result = source->portion(f, t);
    return *result;
  }

  Rect boundsFast() const { return actualPath().boundsFast; }
  Rect boundsExact() const { return actualPath().boundsFast; }
  Rect boundsLocal(Interval i) const { throwNotImplemented(); }

  std::vector<double> roots(double v, Dim2 d) const = 0;

  virtual int winding(Point p) const { return root_winding(*this, p); }

  virtual Curve *portion(double f, double t) const = 0;
  virtual Curve *reverse() const { return portion(1, 0); }

  virtual Crossings crossingsWith(Curve const & other) const;

  virtual void setInitial(Point v) = 0;
  virtual void setFinal(Point v) = 0;

  virtual Curve *transformed(Matrix const &m) const = 0;

  virtual Point pointAt(Coord t) const { return pointAndDerivatives(t, 1).front(); }
  virtual Coord valueAt(Coord t, Dim2 d) const { return pointAt(t)[d]; }
  virtual std::vector<Point> pointAndDerivatives(Coord t, unsigned n) const = 0;
  virtual D2<SBasis> toSBasis() const = 0;

};
*/

}

namespace std {

template <>
inline void swap<Geom::Path>(Geom::Path &a, Geom::Path &b)
{
  a.swap(b);
}

}

#endif // SEEN_GEOM_PATH_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=2:tabstop=8:softtabstop=2 :

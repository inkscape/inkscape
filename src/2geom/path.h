/*
 * Path - Series of continuous curves
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




#ifndef SEEN_GEOM_PATH_H
#define SEEN_GEOM_PATH_H


#include "curves.h"

#include <iterator>


namespace Geom
{

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

  BaseIterator &operator--() {
    --impl_;
    return *this;
  }

  BaseIterator operator--(int) {
    BaseIterator old=*this;
    --(*this);
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

/*
 * Open and closed paths: all paths, whether open or closed, store a final
 * segment which connects the initial and final endpoints of the "real"
 * path data.  While similar to the "z" in an SVG path, it exists for
 * both open and closed paths, and is not considered part of the "normal"
 * path data, which is always covered by the range [begin(), end_open()).
 * Conversely, the range [begin(), end_closed()) always contains the "extra"
 * closing segment.
 *
 * The only difference between a closed and an open path is whether end()
 * returns end_closed() or end_open().  The idea behind this is to let
 * any path be stroked using [begin(), end()), and filled using
 * [begin(), end_closed()), without requiring a separate "filled" version
 * of the path to use for filling.
 */
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

  Curve const &front() const { return *curves_[0]; }
  Curve const &back() const { return *curves_[curves_.size()-2]; }

  const_iterator begin() const { return curves_.begin(); }
  const_iterator end() const { return curves_.end()-1; }
  iterator begin() { return curves_.begin(); }
  iterator end() { return curves_.end()-1; }

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
    // pw<d2<>> is always open. so if path is closed, add closing segment as well to pwd2.
    for(const_iterator it = begin(); it != end_default(); ++it) {
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
    ret.closed_ = closed_;
    return ret;
  }

  Point pointAt(double t) const 
  {
	  unsigned int sz = size();
	  if ( closed() ) ++sz;
	  if ( t < 0 || t > sz  )
	  {
		  THROW_RANGEERROR("parameter t out of bounds");
	  }
	  if ( empty() ) return Point(0,0);
	  double k, lt = modf(t, &k);
	  unsigned int i = static_cast<unsigned int>(k);
	  if ( i == sz ) 
	  { 
		  --i;
		  lt = 1;
	  }
	  return (*this)[i].pointAt(lt);
  }

  double valueAt(double t, Dim2 d) const 
  {
	  unsigned int sz = size();
	  if ( closed() ) ++sz;
	  if ( t < 0 || t > sz  )
	  {
		  THROW_RANGEERROR("parameter t out of bounds");
	  }
	  if ( empty() ) return 0;
	  double k, lt = modf(t, &k);
	  unsigned int i = static_cast<unsigned int>(k);
	  if ( i == sz ) 
	  { 
		  --i;
		  lt = 1;
	  }
	  return (*this)[i].valueAt(lt, d);
  }

  
  Point operator() (double t) const
  {
	  return pointAt(t);
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
  
  std::vector<double> 
  allNearestPoints(Point const& _point, double from, double to) const;
  
  std::vector<double>
  allNearestPoints(Point const& _point) const
  {
	  unsigned int sz = size();
	  if ( closed() ) ++sz;
	  return allNearestPoints(_point, 0, sz);
  }
  
  
  double nearestPoint(Point const& _point, double from, double to) const;
  
  double nearestPoint(Point const& _point) const
  {
	  unsigned int sz = size();
	  if ( closed() ) ++sz;
	  return nearestPoint(_point, 0, sz);
  }
   
  Rect boundsFast();
  Rect boundsExact();
  
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

  // erase last segment of path
  void erase_last() {
    erase(curves_.end()-2);
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

  void setInitial(Point const& p)
  {
	  if ( empty() ) return;
	  Curve* head = front().duplicate();
	  head->setInitial(p);
	  Sequence::iterator replaced = curves_.begin();
	  Sequence source(1, head);
	  try 
	  {
		  do_update(replaced, replaced + 1, source.begin(), source.end());
	  } 
	  catch (...) 
	  {
		  delete_range(source.begin(), source.end());
		  throw;
	  }
  }

  void setFinal(Point const& p)
  {
	  if ( empty() ) return;
	  Curve* tail = back().duplicate();
	  tail->setFinal(p);
	  Sequence::iterator replaced = curves_.end() - 2;
	  Sequence source(1, tail);
	  try 
	  {
		  do_update(replaced, replaced + 1, source.begin(), source.end());
	  } 
	  catch (...) 
	  {
		  delete_range(source.begin(), source.end());
		  throw;
	  }	 
  }

  void append(Curve const &curve);
  void append(D2<SBasis> const &curve);
  void append(Path const &other);

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
  
};  // end class Path

inline static Piecewise<D2<SBasis> > paths_to_pw(std::vector<Path> paths) {
    Piecewise<D2<SBasis> > ret = paths[0].toPwSb();
    for(unsigned i = 1; i < paths.size(); i++) {
        ret.concat(paths[i].toPwSb());
    }
    return ret;
}

inline
Coord nearest_point(Point const& p, Path const& c)
{
	return c.nearestPoint(p);
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
  Rect boundsLocal(Interval i) const { THROW_NOTIMPLEMENTED(); }

  std::vector<double> roots(double v, Dim2 d) const = 0;

  virtual int winding(Point p) const { return root_winding(*this, p); }

  virtual Curve *portion(double f, double t) const = 0;
  virtual Curve *reverse() const { return portion(1, 0); }

  virtual Crossings crossingsWith(Curve const & other) const;

  virtual void setInitial(Point v) = 0;
  virtual void setFinal(Point v) = 0;

  virtual Curve *transformed(Matrix const &m) const = 0;

  virtual Point pointAt(Coord t) const { return pointAndDerivatives(t, 0).front(); }
  virtual Coord valueAt(Coord t, Dim2 d) const { return pointAt(t)[d]; }
  virtual std::vector<Point> pointAndDerivatives(Coord t, unsigned n) const = 0;
  virtual D2<SBasis> toSBasis() const = 0;

};
*/

}  // end namespace Geom

namespace std {

template <>
inline void swap<Geom::Path>(Geom::Path &a, Geom::Path &b)
{
  a.swap(b);
}

}  // end namespace std


#endif // SEEN_GEOM_PATH_H




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

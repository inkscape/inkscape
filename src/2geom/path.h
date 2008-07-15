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


#include <2geom/curves.h>

#include <iterator>


namespace Geom
{

// Conditional expression for types. If true, first, if false, second.
template<bool _Cond, typename _Iftrue, typename _Iffalse>
  struct __conditional_type
  { typedef _Iftrue __type; };

template<typename _Iftrue, typename _Iffalse>
  struct __conditional_type<false, _Iftrue, _Iffalse>
  { typedef _Iffalse __type; };


template <typename IteratorImpl>
class BaseIterator
: public std::iterator<std::forward_iterator_tag, Curve const>
{
public:
  BaseIterator() {}

  // default construct
  // default copy

  // Allow Sequence::iterator to Sequence::const_iterator conversion
  // unfortunately I do not know how to imitate the way __normal_iterator 
  // does it, because I don't see a way to get the typename of the container 
  // IteratorImpl is pointing at...
  typedef std::vector<Curve *> Sequence;
  BaseIterator (  typename __conditional_type<
                    (std::__are_same<IteratorImpl, Sequence::const_iterator >::__value),  // check if this instantiation is of const_iterator type
                    const BaseIterator< Sequence::iterator >,     // if true:  accept iterator in const_iterator instantiation
                    const BaseIterator<IteratorImpl> > ::__type   // if false: default to standard copy constructor
                  & __other)
    : impl_(__other.impl_) { }
  friend class BaseIterator< Sequence::const_iterator >;


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
 * any path be stroked using [begin(), end_default()), and filled using
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

  class ClosingSegment : public LineSegment {
  public:
    ClosingSegment() : LineSegment() {}
    ClosingSegment(Point const &p1, Point const &p2) : LineSegment(p1, p2) {}
    virtual Curve *duplicate() const { return new ClosingSegment(*this); }
  };

  enum Stitching {
    NO_STITCHING=0,
    STITCH_DISCONTINUOUS
  };

  class StitchSegment : public LineSegment {
  public:
    StitchSegment() : LineSegment() {}
    StitchSegment(Point const &p1, Point const &p2) : LineSegment(p1, p2) {}
    virtual Curve *duplicate() const { return new StitchSegment(*this); }
  };

  Path()
  : final_(new ClosingSegment()), closed_(false)
  {
    curves_.push_back(final_);
  }

  Path(Path const &other)
  : final_(new ClosingSegment()), closed_(other.closed_)
  {
    curves_.push_back(final_);
    insert(begin(), other.begin(), other.end());
  }

  explicit Path(Point p)
  : final_(new ClosingSegment(p, p)), closed_(false)
  {
    curves_.push_back(final_);
  }

  template <typename Impl>
  Path(BaseIterator<Impl> first, BaseIterator<Impl> last, bool closed=false)
  : closed_(closed), final_(new ClosingSegment())
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
  Curve const &back_open() const { return *curves_[curves_.size()-2]; }
  Curve const &back_closed() const { return *curves_[curves_.size()-1]; }
  Curve const &back_default() const {
    return ( closed_ ? back_closed() : back_open() );
  }

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
    bool degenerate = true;
    // pw<d2<>> is always open. so if path is closed, add closing segment as well to pwd2.
    for(const_iterator it = begin(); it != end_default(); ++it) {
      if (!it->isDegenerate()) {
        ret.push(it->toSBasis(), i++);
        degenerate = false;
      }
    }
    if (degenerate) {
      // if path only contains degenerate curves, no second cut is added
      // so we need to create at least one segment manually
      ret = Piecewise<D2<SBasis> >(initialPoint());
    }
    return ret;
  }

  bool operator==(Path const &m) const {
      if (size() != m.size() || closed() != m.closed())
          return false;
      const_iterator it2 = m.curves_.begin();
    for(const_iterator it = curves_.begin(); it != curves_.end(); ++it) {
        const Curve& a = (*it);
        const Curve& b = (*it2);
        if(!(a == b))
            return false;
        ++it2;
    }
    return true;
  }

  /*
     Path operator*=(Matrix)
     This is not possible without at least partly regenerating the curves of 
     the path, because a path can consist of many types of curves, 
     e.g. a HLineSegment.
     Such a segment cannot be transformed and stay a HLineSegment in general 
     (take for example rotations).
     This means that these curves of the path have to be replaced with 
     LineSegments: new Curves.
     So an implementation of this method should check the curve's type to see 
     whether operator*= is doable for that curve type, ...
  */
  Path operator*(Matrix const &m) const {
    Path ret;
    ret.curves_.reserve(curves_.size());
    for(const_iterator it = begin(); it != end(); ++it) {
      Curve *curve = it->transformed(m);
      ret.do_append(curve);
    }
    ret.close(closed_);
    return ret;
  }

  /*
  // this should be even quickier but it works at low level
  Path operator*(Matrix const &m) const
  {
      Path result;
      size_t sz = curves_.size() - 1;
      if (sz == 0) return result;
      result.curves_.resize(curves_.size());
      result.curves_.back() = result.final_;
      result.curves_[0] = (curves_[0])->transformed(m);
      for (size_t i = 1; i < sz; ++i)
      {
          result.curves_[i] = (curves_[i])->transformed(m);
          if ( result.curves_[i]->initialPoint() != result.curves_[i-1]->finalPoint() ) {
              THROW_CONTINUITYERROR();
          }
      }
      result.final_->setInitial( (result.curves_[sz])->finalPoint() );
      result.final_->setFinal( (result.curves_[0])->initialPoint() );
      result.closed_ = closed_;
      return result;
  }
  */
  
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
      Curve *temp = (*this)[i].reverse();
      ret.append(*temp);
      // delete since append makes a copy
      delete temp;
    }
    return ret;
  }

  void insert(iterator pos, Curve const &curve, Stitching stitching=NO_STITCHING) {
    Sequence source(1, curve.duplicate());
    try {
      if (stitching) stitch(pos.impl_, pos.impl_, source);
      do_update(pos.impl_, pos.impl_, source.begin(), source.end());
    } catch (...) {
      delete_range(source.begin(), source.end());
      throw;
    }
  }

  template <typename Impl>
  void insert(iterator pos, BaseIterator<Impl> first, BaseIterator<Impl> last, Stitching stitching=NO_STITCHING)
  {
    Sequence source(DuplicatingIterator<Impl>(first.impl_),
                    DuplicatingIterator<Impl>(last.impl_));
    try {
      if (stitching) stitch(pos.impl_, pos.impl_, source);
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

  void erase(iterator pos, Stitching stitching=NO_STITCHING) {
    if (stitching) {
      Sequence stitched;
      stitch(pos.impl_, pos.impl_+1, stitched);
      try {
        do_update(pos.impl_, pos.impl_+1, stitched.begin(), stitched.end());
      } catch (...) {
        delete_range(stitched.begin(), stitched.end());
        throw;
      }
    } else {
      do_update(pos.impl_, pos.impl_+1, curves_.begin(), curves_.begin());
    }
  }

  void erase(iterator first, iterator last, Stitching stitching=NO_STITCHING) {
    if (stitching) {
      Sequence stitched;
      stitch(first.impl_, last.impl_, stitched);
      try {
        do_update(first.impl_, last.impl_, stitched.begin(), stitched.end());
      } catch (...) {
        delete_range(stitched.begin(), stitched.end());
        throw;
      }
    } else {
      do_update(first.impl_, last.impl_, curves_.begin(), curves_.begin());
    }
  }

  // erase last segment of path
  void erase_last() {
    erase(curves_.end()-2);
  }

  void replace(iterator replaced, Curve const &curve, Stitching stitching=NO_STITCHING) {
    Sequence source(1, curve.duplicate());
    try {
      if (stitching) stitch(replaced.impl_, replaced.impl_+1, source);
      do_update(replaced.impl_, replaced.impl_+1, source.begin(), source.end());
    } catch (...) {
      delete_range(source.begin(), source.end());
      throw;
    }
  }

  void replace(iterator first_replaced, iterator last_replaced,
               Curve const &curve, Stitching stitching=NO_STITCHING)
  {
    Sequence source(1, curve.duplicate());
    try {
      if (stitching) stitch(first_replaced.impl_, last_replaced.impl_, source);
      do_update(first_replaced.impl_, last_replaced.impl_,
                source.begin(), source.end());
    } catch (...) {
      delete_range(source.begin(), source.end());
      throw;
    }
  }

  template <typename Impl>
  void replace(iterator replaced,
               BaseIterator<Impl> first, BaseIterator<Impl> last,
               Stitching stitching=NO_STITCHING)
  {
    Sequence source(DuplicatingIterator<Impl>(first.impl_),
                    DuplicatingIterator<Impl>(last.impl_));
    try {
      if (stitching) stitch(replaced.impl_, replaced.impl_+1, source);
      do_update(replaced.impl_, replaced.impl_+1, source.begin(), source.end());
    } catch (...) {
      delete_range(source.begin(), source.end());
      throw;
    }
  }

  template <typename Impl>
  void replace(iterator first_replaced, iterator last_replaced,
               BaseIterator<Impl> first, BaseIterator<Impl> last,
               Stitching stitching=NO_STITCHING)
  {
    Sequence source(first.impl_, last.impl_);
    try {
      if (stitching) stitch(first_replaced.impl_, last_replaced.impl_, source);
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

  void append(Curve const &curve, Stitching stitching=NO_STITCHING) {
    if (stitching) stitchTo(curve.initialPoint());
    do_append(curve.duplicate());
  }
  void append(D2<SBasis> const &curve, Stitching stitching=NO_STITCHING) {
    if (stitching) stitchTo(Point(curve[X][0][0], curve[Y][0][0]));
    do_append(new SBasisCurve(curve));
  }
  void append(Path const &other, Stitching stitching=NO_STITCHING) {
    insert(end(), other.begin(), other.end(), stitching);
  }

  void stitchTo(Point const &p) {
    if (!empty() && finalPoint() != p) {
      do_append(new StitchSegment(finalPoint(), p));
    }
  }

  template <typename CurveType, typename A>
  void appendNew(A a) {
    do_append(new CurveType(finalPoint(), a));
  }

  template <typename CurveType, typename A, typename B>
  void appendNew(A a, B b) {
    do_append(new CurveType(finalPoint(), a, b));
  }

  template <typename CurveType, typename A, typename B, typename C>
  void appendNew(A a, B b, C c) {
    do_append(new CurveType(finalPoint(), a, b, c));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D>
  void appendNew(A a, B b, C c, D d) {
    do_append(new CurveType(finalPoint(), a, b, c, d));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E>
  void appendNew(A a, B b, C c, D d, E e) {
    do_append(new CurveType(finalPoint(), a, b, c, d, e));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F>
  void appendNew(A a, B b, C c, D d, E e, F f) {
    do_append(new CurveType(finalPoint(), a, b, c, d, e, f));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F,
                                typename G>
  void appendNew(A a, B b, C c, D d, E e, F f, G g) {
    do_append(new CurveType(finalPoint(), a, b, c, d, e, f, g));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F,
                                typename G, typename H>
  void appendNew(A a, B b, C c, D d, E e, F f, G g, H h) {
    do_append(new CurveType(finalPoint(), a, b, c, d, e, f, g, h));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F,
                                typename G, typename H, typename I>
  void appendNew(A a, B b, C c, D d, E e, F f, G g, H h, I i) {
    do_append(new CurveType(finalPoint(), a, b, c, d, e, f, g, h, i));
  }

private:
  void stitch(Sequence::iterator first_replaced,
              Sequence::iterator last_replaced,
              Sequence &sequence);

  void do_update(Sequence::iterator first_replaced,
                 Sequence::iterator last_replaced,
                 Sequence::iterator first,
                 Sequence::iterator last);

  void do_append(Curve *curve);

  static void delete_range(Sequence::iterator first, Sequence::iterator last);

  void check_continuity(Sequence::iterator first_replaced,
                        Sequence::iterator last_replaced,
                        Sequence::iterator first,
                        Sequence::iterator last);

  Sequence curves_;
  ClosingSegment *final_;
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

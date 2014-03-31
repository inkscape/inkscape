/**
 * \file
 * \brief  Path - Series of continuous curves
 *//*
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Marco Cecchetti <mrcekets at gmail.com>
 * 
 * Copyright 2007-2008 Authors
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

#ifndef LIB2GEOM_SEEN_PATH_H
#define LIB2GEOM_SEEN_PATH_H

#include <iterator>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <2geom/curve.h>
#include <2geom/bezier-curve.h>
#include <2geom/transforms.h>

namespace Geom {

class Path;

namespace PathInternal {

typedef std::vector<boost::shared_ptr<Curve const> > Sequence;

template <typename C, typename P>
class BaseIterator {
protected:
  BaseIterator() : path(NULL), index(0) {}
  BaseIterator(P *p, unsigned i) : path(p), index(i) {}
  // default copy, default assign

public:
  bool operator==(BaseIterator const &other) {
    return path == other.path && index == other.index;
  }
  bool operator!=(BaseIterator const &other) {
    return path != other.path || index != other.index;
  }

  Curve const &operator*() const { return (*path)[index]; }
  Curve const *operator->() const { return &(*path)[index]; }
  boost::shared_ptr<Curve const> get_ref() const {
    return path->get_ref_at_index(index);
  }

  C &operator++() {
    ++index;
    return static_cast<C &>(*this);
  }
  C operator++(int) {
    C old(static_cast<C &>(*this));
    ++(*this);
    return old;
  }

  C &operator--() {
    --index;
    return static_cast<C &>(*this);
  }
  C operator--(int) {
    C old(static_cast<C &>(*this));
    --(*this);
    return old;
  }

private:
  P *path;
  unsigned index;

  friend class ::Geom::Path;
};

class ConstIterator : public BaseIterator<ConstIterator, Path const> {
public:
  typedef BaseIterator<ConstIterator, Path const> Base;

  ConstIterator() : Base() {}
  // default copy, default assign

private:
  ConstIterator(Path const &p, unsigned i) : Base(&p, i) {}
  friend class ::Geom::Path;
};

class Iterator : public BaseIterator<Iterator, Path> {
public:
  typedef BaseIterator<Iterator, Path> Base;

  Iterator() : Base() {}
  // default copy, default assign

  operator ConstIterator const &() const {
    return reinterpret_cast<ConstIterator const &>(*this);
  }

private:
  Iterator(Path &p, unsigned i) : Base(&p, i) {}
  friend class ::Geom::Path;
};

}

/*
 * Open and closed paths: all paths, whether open or closed, store a final
 * segment which connects the initial and final endpoints of the "real"
 * path data.  While similar to the "z" in an SVG path, it exists for
 * both open and closed paths, and is not considered part of the "normal"
 * path data, which is always covered by the range [begin(), end_open()).
 * Conversely, the range [begin(), end_closed()) always contains the "extra"
 * closing segment.
 *
 * The only difference between a closed and an open path is whether
 * end_default() returns end_closed() or end_open().  The idea behind this
 * is to let any path be stroked using [begin(), end_default()), and filled
 * using [begin(), end_closed()), without requiring a separate "filled" version
 * of the path to use for filling.
 *
 * \invariant : curves_ always contains at least one segment. The last segment
 *              is always of type ClosingSegment. All constructors take care of this.
                (curves_.size() > 0) && dynamic_cast<ClosingSegment>(curves_.back())
 */
class Path {
public:
  typedef PathInternal::Sequence Sequence;
  typedef PathInternal::Iterator iterator;
  typedef PathInternal::ConstIterator const_iterator;
  typedef Sequence::size_type size_type;
  typedef Sequence::difference_type difference_type;

  class ClosingSegment : public LineSegment {
  public:
    ClosingSegment() : LineSegment() {}
    ClosingSegment(Point const &p1, Point const &p2) : LineSegment(p1, p2) {}
    virtual Curve *duplicate() const { return new ClosingSegment(*this); }
    virtual Curve *reverse() const { return new ClosingSegment((*this)[1], (*this)[0]); }
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
    virtual Curve *reverse() const { return new StitchSegment((*this)[1], (*this)[0]); }
  };

  // Path(Path const &other) - use default copy constructor

  explicit Path(Point p=Point())
  : curves_(boost::shared_ptr<Sequence>(new Sequence(1, boost::shared_ptr<Curve>()))),
    final_(new ClosingSegment(p, p)),
    closed_(false)
  {
    get_curves().back() = boost::shared_ptr<Curve>(final_);
  }

  Path(const_iterator const &first,
       const_iterator const &last,
       bool closed=false)
  : curves_(boost::shared_ptr<Sequence>(new Sequence(seq_iter(first),
                                                     seq_iter(last)))),
    closed_(closed)
  {
    if (!get_curves().empty()) {
      final_ = new ClosingSegment(get_curves().back()->finalPoint(),
                                  get_curves().front()->initialPoint());
    } else {
      final_ = new ClosingSegment();
    }
    get_curves().push_back(boost::shared_ptr<Curve>(final_));
  }

  virtual ~Path() {}
  
  // Path &operator=(Path const &other) - use default assignment operator

  /// \todo Add noexcept specifiers for C++11
  void swap(Path &other) {
    using std::swap;
    swap(other.curves_, curves_);
    swap(other.final_, final_);
    swap(other.closed_, closed_);
  }
  friend inline void swap(Path &a, Path &b) { a.swap(b); }

  Curve const &operator[](unsigned i) const { return *get_curves()[i]; }
  Curve const &at_index(unsigned i) const { return *get_curves()[i]; }
  boost::shared_ptr<Curve const> get_ref_at_index(unsigned i) {
    return get_curves()[i];
  }

  Curve const &front() const { return *get_curves()[0]; }
  Curve const &back() const { return back_open(); }
  Curve const &back_open() const {
    if (empty()) { THROW_RANGEERROR("Path contains not enough segments"); }
    return *get_curves()[get_curves().size()-2];
  }
  Curve const &back_closed() const { return *get_curves()[get_curves().size()-1]; }
  Curve const &back_default() const {
    return ( closed_ ? back_closed() : back_open() );
  }

  const_iterator begin() const { return const_iterator(*this, 0); }
  const_iterator end() const { return const_iterator(*this, size()); }
  iterator begin() { return iterator(*this, 0); }
  iterator end() { return iterator(*this, size()); }

  const_iterator end_open() const { return const_iterator(*this, size()); }
  const_iterator end_closed() const { return const_iterator(*this, size()+1); }
  const_iterator end_default() const {
    return ( closed_ ? end_closed() : end_open() );
  }

  size_type size_open() const { return get_curves().size()-1; }
  size_type size_closed() const { return get_curves().size(); }
  size_type size_default() const  {
    return ( closed_ ? size_closed() : size_open() );
  }
  size_type size() const { return size_open(); }

  size_type max_size() const { return get_curves().max_size()-1; }

  bool empty() const { return (get_curves().size() == 1); }
  bool closed() const { return closed_; }
  void close(bool closed=true) { closed_ = closed; }

  OptRect boundsFast() const;
  OptRect boundsExact() const;

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

  bool operator==(Path const &other) const {
    if (this == &other) return true;
    if (closed_ != other.closed_) return false;
    return get_curves() == other.get_curves();
  }
  bool operator!=(Path const &other) const {
    return !( *this == other );
  }

  Path operator*(Affine const &m) const {
    Path ret(*this);
    ret *= m;
    return ret;
  }
  Path operator*(Translate const &m) const { // specialization over Affine, for faster computation
    Path ret(*this);
    ret *= m;
    return ret;
  }

  Path &operator*=(Affine const &m);
  Path &operator*=(Translate const &m); // specialization over Affine, for faster computation
  
  Point pointAt(double t) const 
  {
	  unsigned int sz = size();
	  if ( closed() ) ++sz;
	  if ( t < 0 || t > sz  )
	  {
		  THROW_RANGEERROR("parameter t out of bounds");
	  }
	  if ( empty() ) return initialPoint(); // naked moveto
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
	  if ( empty() ) return initialPoint()[d]; // naked moveto
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
  
  std::vector<double>
  nearestPointPerCurve(Point const& _point) const;  
  
  double nearestPoint(Point const& _point, double from, double to, double *distance_squared = NULL) const;
  
  double nearestPoint(Point const& _point, double *distance_squared = NULL) const
  {
	  unsigned int sz = size();
	  if ( closed() ) ++sz;
	  return nearestPoint(_point, 0, sz, distance_squared);
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
    Path ret(*this);
    ret.unshare();
    for ( Sequence::iterator iter = ret.get_curves().begin() ;
          iter != ret.get_curves().end()-1 ; ++iter )
    {
      *iter = boost::shared_ptr<Curve>((*iter)->reverse());
    }
    std::reverse(ret.get_curves().begin(), ret.get_curves().end()-1);
    ret.final_ = static_cast<ClosingSegment *>(ret.final_->reverse());
    ret.get_curves().back() = boost::shared_ptr<Curve>(ret.final_);
    return ret;
  }

  void insert(iterator const &pos,
              Curve const &curve, Stitching stitching=NO_STITCHING)
  {
    unshare();
    Sequence::iterator seq_pos(seq_iter(pos));
    Sequence source(1, boost::shared_ptr<Curve>(curve.duplicate()));
    if (stitching) stitch(seq_pos, seq_pos, source);
    do_update(seq_pos, seq_pos, source.begin(), source.end());
  }

  void insert(iterator const &pos,
              const_iterator const &first,
              const_iterator const &last,
              Stitching stitching=NO_STITCHING)
  {
    unshare();
    Sequence::iterator seq_pos(seq_iter(pos));
    Sequence source(seq_iter(first), seq_iter(last));
    if (stitching) stitch(seq_pos, seq_pos, source);
    do_update(seq_pos, seq_pos, source.begin(), source.end());
  }

  void clear() {
    unshare();
    do_update(get_curves().begin(), get_curves().end()-1,
              get_curves().begin(), get_curves().begin());
  }

  void erase(iterator const &pos, Stitching stitching=NO_STITCHING) {
    unshare();
    Sequence::iterator seq_pos(seq_iter(pos));
    if (stitching) {
      Sequence stitched;
      stitch(seq_pos, seq_pos+1, stitched);
      do_update(seq_pos, seq_pos+1, stitched.begin(), stitched.end());
    } else {
      do_update(seq_pos, seq_pos+1, get_curves().begin(), get_curves().begin());
    }
  }

  void erase(iterator const &first,
             iterator const &last,
             Stitching stitching=NO_STITCHING)
  {
    unshare();
    Sequence::iterator seq_first=seq_iter(first);
    Sequence::iterator seq_last=seq_iter(last);
    if (stitching) {
      Sequence stitched;
      stitch(seq_first, seq_last, stitched);
      do_update(seq_first, seq_last, stitched.begin(), stitched.end());
    } else {
      do_update(seq_first, seq_last,
                get_curves().begin(), get_curves().begin());
    }
  }

  // erase last segment of path
  void erase_last() {
    erase(iterator(*this, size()-1));
  }

  void replace(iterator const &replaced,
               Curve const &curve,
               Stitching stitching=NO_STITCHING)
  {
    unshare();
    Sequence::iterator seq_replaced(seq_iter(replaced));
    Sequence source(1, boost::shared_ptr<Curve>(curve.duplicate()));
    if (stitching) stitch(seq_replaced, seq_replaced+1, source);
    do_update(seq_replaced, seq_replaced+1, source.begin(), source.end());
  }

  void replace(iterator const &first_replaced,
               iterator const &last_replaced,
               Curve const &curve, Stitching stitching=NO_STITCHING)
  {
    unshare();
    Sequence::iterator seq_first_replaced(seq_iter(first_replaced));
    Sequence::iterator seq_last_replaced(seq_iter(last_replaced));
    Sequence source(1, boost::shared_ptr<Curve>(curve.duplicate()));
    if (stitching) stitch(seq_first_replaced, seq_last_replaced, source);
    do_update(seq_first_replaced, seq_last_replaced,
              source.begin(), source.end());
  }

  void replace(iterator const &replaced,
               const_iterator const &first,
               const_iterator const &last,
               Stitching stitching=NO_STITCHING)
  {
    unshare();
    Sequence::iterator seq_replaced(seq_iter(replaced));
    Sequence source(seq_iter(first), seq_iter(last));
    if (stitching) stitch(seq_replaced, seq_replaced+1, source);
    do_update(seq_replaced, seq_replaced+1, source.begin(), source.end());
  }

  void replace(iterator const &first_replaced,
               iterator const &last_replaced,
               const_iterator const &first,
               const_iterator const &last,
               Stitching stitching=NO_STITCHING)
  {
    unshare();
    Sequence::iterator seq_first_replaced(seq_iter(first_replaced));
    Sequence::iterator seq_last_replaced(seq_iter(last_replaced));
    Sequence source(seq_iter(first), seq_iter(last));
    if (stitching) stitch(seq_first_replaced, seq_last_replaced, source);
    do_update(seq_first_replaced, seq_last_replaced,
              source.begin(), source.end());
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
          unshare();
	  boost::shared_ptr<Curve> head(front().duplicate());
	  head->setInitial(p);
	  Sequence::iterator replaced = get_curves().begin();
	  Sequence source(1, head);
	  do_update(replaced, replaced + 1, source.begin(), source.end());
  }

  void setFinal(Point const& p)
  {
	  if ( empty() ) return;
          unshare();
	  boost::shared_ptr<Curve> tail(back().duplicate());
	  tail->setFinal(p);
	  Sequence::iterator replaced = get_curves().end() - 2;
	  Sequence source(1, tail);
	  do_update(replaced, replaced + 1, source.begin(), source.end());
  }

  void append(Curve const &curve, Stitching stitching=NO_STITCHING) {
    unshare();
    if (stitching) stitchTo(curve.initialPoint());
    do_append(curve.duplicate());
  }
  void append(D2<SBasis> const &curve, Stitching stitching=NO_STITCHING) {
    unshare();
    if (stitching) stitchTo(Point(curve[X][0][0], curve[Y][0][0]));
    do_append(new SBasisCurve(curve));
  }
  void append(Path const &other, Stitching stitching=NO_STITCHING) {
    insert(end(), other.begin(), other.end(), stitching);
  }

  void stitchTo(Point const &p) {
    if (!empty() && finalPoint() != p) {
      unshare();
      do_append(new StitchSegment(finalPoint(), p));
    }
  }


  /**
   * It is important to note that the coordinates passed to appendNew should be finite!
   * If one of the coordinates is infinite, 2geom will throw a ContinuityError exception.
   */

  template <typename CurveType, typename A>
  void appendNew(A a) {
    unshare();
    do_append(new CurveType(finalPoint(), a));
  }

  template <typename CurveType, typename A, typename B>
  void appendNew(A a, B b) {
    unshare();
    do_append(new CurveType(finalPoint(), a, b));
  }

  template <typename CurveType, typename A, typename B, typename C>
  void appendNew(A a, B b, C c) {
    unshare();
    do_append(new CurveType(finalPoint(), a, b, c));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D>
  void appendNew(A a, B b, C c, D d) {
    unshare();
    do_append(new CurveType(finalPoint(), a, b, c, d));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E>
  void appendNew(A a, B b, C c, D d, E e) {
    unshare();
    do_append(new CurveType(finalPoint(), a, b, c, d, e));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F>
  void appendNew(A a, B b, C c, D d, E e, F f) {
    unshare();
    do_append(new CurveType(finalPoint(), a, b, c, d, e, f));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F,
                                typename G>
  void appendNew(A a, B b, C c, D d, E e, F f, G g) {
    unshare();
    do_append(new CurveType(finalPoint(), a, b, c, d, e, f, g));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F,
                                typename G, typename H>
  void appendNew(A a, B b, C c, D d, E e, F f, G g, H h) {
    unshare();
    do_append(new CurveType(finalPoint(), a, b, c, d, e, f, g, h));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F,
                                typename G, typename H, typename I>
  void appendNew(A a, B b, C c, D d, E e, F f, G g, H h, I i) {
    unshare();
    do_append(new CurveType(finalPoint(), a, b, c, d, e, f, g, h, i));
  }

private:
  static Sequence::iterator seq_iter(iterator const &iter) {
    return iter.path->get_curves().begin() + iter.index;
  }
  static Sequence::const_iterator seq_iter(const_iterator const &iter) {
    return iter.path->get_curves().begin() + iter.index;
  }

  Sequence &get_curves() { return *curves_; }
  Sequence const &get_curves() const { return *curves_; }

  void unshare() {
    if (!curves_.unique()) {
      curves_ = boost::shared_ptr<Sequence>(new Sequence(*curves_));
    }
    if (!get_curves().back().unique()) {
      final_ = static_cast<ClosingSegment *>(final_->duplicate());
      get_curves().back() = boost::shared_ptr<Curve>(final_);
    }
  }

  void stitch(Sequence::iterator first_replaced,
              Sequence::iterator last_replaced,
              Sequence &sequence);

  void do_update(Sequence::iterator first_replaced,
                 Sequence::iterator last_replaced,
                 Sequence::iterator first,
                 Sequence::iterator last);

  // n.b. takes ownership of curve object
  void do_append(Curve *curve);

  void check_continuity(Sequence::iterator first_replaced,
                        Sequence::iterator last_replaced,
                        Sequence::iterator first,
                        Sequence::iterator last);

  boost::shared_ptr<Sequence> curves_;
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


#endif // LIB2GEOM_SEEN_PATH_H

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

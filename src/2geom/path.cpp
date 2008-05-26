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



#include "path.h"


namespace Geom 
{

void Path::swap(Path &other) {
  std::swap(curves_, other.curves_);
  std::swap(closed_, other.closed_);
  std::swap(*final_, *other.final_);
  curves_[curves_.size()-1] = final_;
  other.curves_[other.curves_.size()-1] = other.final_;
}

Rect Path::boundsFast() const {
  Rect bounds=front().boundsFast();
  for ( const_iterator iter=++begin(); iter != end() ; ++iter ) {
    bounds.unionWith(iter->boundsFast());
  }
  return bounds;
}

Rect Path::boundsExact() const {
  Rect bounds=front().boundsExact();
  for ( const_iterator iter=++begin(); iter != end() ; ++iter ) {
    bounds.unionWith(iter->boundsExact());
  }
  return bounds;
}

template<typename iter>
iter inc(iter const &x, unsigned n) {
  iter ret = x;
  for(unsigned i = 0; i < n; i++)
    ret++;
  return ret;
}

std::vector<double> 
Path::allNearestPoints(Point const& _point, double from, double to) const
{
	if ( from > to ) std::swap(from, to);
	const Path& _path = *this;
	unsigned int sz = _path.size();
	if ( _path.closed() ) ++sz;
	if ( from < 0 || to > sz )
	{
		THROW_RANGEERROR("[from,to] interval out of bounds");
	}
	double sif, st = modf(from, &sif);
	double eif, et = modf(to, &eif);
	unsigned int si = static_cast<unsigned int>(sif);
	unsigned int ei = static_cast<unsigned int>(eif);
	if ( si == sz )
	{
		--si;
		st = 1;
	}
	if ( ei == sz )
	{
		--ei;
		et = 1;
	}
	if ( si == ei )
	{
		std::vector<double>	all_nearest = 
			_path[si].allNearestPoints(_point, st, et);
		for ( unsigned int i = 0; i < all_nearest.size(); ++i )
		{
			all_nearest[i] = si + all_nearest[i];
		}
		return all_nearest;
	}
	std::vector<double> all_t;
	std::vector< std::vector<double> > all_np;
	all_np.push_back( _path[si].allNearestPoints(_point, st) );
	std::vector<unsigned int> ni;
	ni.push_back(si);
	double dsq;
	double mindistsq 
		= distanceSq( _point, _path[si].pointAt( all_np.front().front() ) );
	Rect bb;
	for ( unsigned int i = si + 1; i < ei; ++i )
	{
		bb = _path[i].boundsFast();
		dsq = distanceSq(_point, bb);
		if ( mindistsq < dsq ) continue;
		all_t = _path[i].allNearestPoints(_point);
		dsq = distanceSq( _point, _path[i].pointAt( all_t.front() ) );
		if ( mindistsq > dsq )
		{
			all_np.clear();
			all_np.push_back(all_t);
			ni.clear();
			ni.push_back(i);
			mindistsq = dsq;
		}
		else if ( mindistsq == dsq )
		{
			all_np.push_back(all_t);
			ni.push_back(i);
		}
	}
	bb = _path[ei].boundsFast();
	dsq = distanceSq(_point, bb);
	if ( mindistsq >= dsq )
	{
		all_t = _path[ei].allNearestPoints(_point, 0, et);
		dsq = distanceSq( _point, _path[ei].pointAt( all_t.front() ) );
		if ( mindistsq > dsq )
		{
			for ( unsigned int i = 0; i < all_t.size(); ++i )
			{
				all_t[i] = ei + all_t[i];
			}
			return all_t;
		}
		else if ( mindistsq == dsq )
		{
			all_np.push_back(all_t);
			ni.push_back(ei);
		}
	}
	std::vector<double> all_nearest;
	for ( unsigned int i = 0; i < all_np.size(); ++i )
	{
		for ( unsigned int j = 0; j < all_np[i].size(); ++j )
		{
			all_nearest.push_back( ni[i] + all_np[i][j] );
		}
	}
	return all_nearest;	
}

double Path::nearestPoint(Point const& _point, double from, double to) const
{
	if ( from > to ) std::swap(from, to);
	const Path& _path = *this;
	unsigned int sz = _path.size();
	if ( _path.closed() ) ++sz;
	if ( from < 0 || to > sz )
	{
		THROW_RANGEERROR("[from,to] interval out of bounds");
	}
	double sif, st = modf(from, &sif);
	double eif, et = modf(to, &eif);
	unsigned int si = static_cast<unsigned int>(sif);
	unsigned int ei = static_cast<unsigned int>(eif);
	if ( si == sz )
	{
		--si;
		st = 1;
	}
	if ( ei == sz )
	{
		--ei;
		et = 1;
	}
	if ( si == ei )
	{
		double nearest =
			_path[si].nearestPoint(_point, st, et);
		return si + nearest;
	}
	double t;
	double nearest = _path[si].nearestPoint(_point, st);
	unsigned int ni = si;
	double dsq;
	double mindistsq = distanceSq(_point, _path[si].pointAt(nearest));
	Rect bb;
	for ( unsigned int i = si + 1; i < ei; ++i )
	{
		bb = _path[i].boundsFast();
		dsq = distanceSq(_point, bb);
		if ( mindistsq <= dsq ) continue;
		t = _path[i].nearestPoint(_point);
		dsq = distanceSq(_point, _path[i].pointAt(t));
		if ( mindistsq > dsq )
		{
			nearest = t;
			ni = i;
			mindistsq = dsq;
		}
	}
	bb = _path[ei].boundsFast();
	dsq = distanceSq(_point, bb);
	if ( mindistsq > dsq )
	{
		t = _path[ei].nearestPoint(_point, 0, et);
		dsq = distanceSq(_point, _path[ei].pointAt(t));
		if ( mindistsq > dsq )
		{
			nearest = t;
			ni = ei;
		}
	}
	return ni + nearest;
}

//This assumes that you can't be perfect in your t-vals, and as such, tweaks the start
void Path::appendPortionTo(Path &ret, double from, double to) const {
  assert(from >= 0 && to >= 0);
  if(to == 0) to = size()+0.999999;
  if(from == to) { return; }
  double fi, ti;
  double ff = modf(from, &fi), tf = modf(to, &ti);
  if(tf == 0) { ti--; tf = 1; }
  const_iterator fromi = inc(begin(), (unsigned)fi);
  if(fi == ti && from < to) {
    Curve *v = fromi->portion(ff, tf);
    ret.append(*v);
    delete v;
    return;
  }
  const_iterator toi = inc(begin(), (unsigned)ti);
  if(ff != 1.) {
    Curve *fromv = fromi->portion(ff, 1.);
    //fromv->setInitial(ret.finalPoint());
    ret.append(*fromv);
    delete fromv;
  }
  if(from >= to) {
    const_iterator ender = end();
    if(ender->initialPoint() == ender->finalPoint()) ender++;
    ret.insert(ret.end(), ++fromi, ender);
    ret.insert(ret.end(), begin(), toi);
  } else {
    ret.insert(ret.end(), ++fromi, toi);
  }
  Curve *tov = toi->portion(0., tf);
  ret.append(*tov);
  delete tov;
}

const double eps = .1;

void Path::append(Curve const &curve) {
  if ( curves_.front() != final_ && !are_near(curve.initialPoint(), (*final_)[0], eps) ) {
    THROW_CONTINUITYERROR();
  }
  do_append(curve.duplicate());
}

void Path::append(D2<SBasis> const &curve) {
  if ( curves_.front() != final_ ) {
    for ( int i = 0 ; i < 2 ; ++i ) {
      if ( !are_near(curve[i][0][0], (*final_)[0][i], eps) ) {
        THROW_CONTINUITYERROR();
      }
    }
  }
  do_append(new SBasisCurve(curve));
}

void Path::append(Path const &other)
{
    // Check that path stays continuous:
    if ( !are_near( finalPoint(), other.initialPoint() ) ) {
        THROW_CONTINUITYERROR();
    }

    insert(begin(), other.begin(), other.end());
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
    final_->setPoint(0, back().finalPoint());
    final_->setPoint(1, front().initialPoint());
  }
}

void Path::do_append(Curve *curve) {
  if ( curves_.front() == final_ ) {
    final_->setPoint(1, curve->initialPoint());
  }
  curves_.insert(curves_.end()-1, curve);
  final_->setPoint(0, curve->finalPoint());
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
      if ( !are_near( (*first_replaced)->initialPoint(), (*first)->initialPoint(), eps ) ) {
        THROW_CONTINUITYERROR();
      }
    }
    if ( last_replaced != (curves_.end()-1) ) {
      if ( !are_near( (*(last_replaced-1))->finalPoint(), (*(last-1))->finalPoint(), eps ) ) {
        THROW_CONTINUITYERROR();
      }
    }
  } else if ( first_replaced != last_replaced && first_replaced != curves_.begin() && last_replaced != curves_.end()-1) {
    if ( !are_near((*first_replaced)->initialPoint(), (*(last_replaced-1))->finalPoint(), eps ) ) {
      THROW_CONTINUITYERROR();
    }
  }
}

} // end namespace Geom

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

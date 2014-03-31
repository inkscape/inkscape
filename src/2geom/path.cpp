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




#include <2geom/path.h>
#include <2geom/transforms.h>
#include <algorithm>

using std::swap;
using namespace Geom::PathInternal;

namespace Geom
{

OptRect Path::boundsFast() const {
  OptRect bounds;
  if (empty()) return bounds;
  bounds = front().boundsFast();
  const_iterator iter = begin();
  // the closing path segment can be ignored, because it will always lie within the bbox of the rest of the path
  if ( iter != end() ) {
    for ( ++iter; iter != end() ; ++iter ) {
      bounds.unionWith(iter->boundsFast());
    }
  }
  return bounds;
}

OptRect Path::boundsExact() const {
  OptRect bounds;
  if (empty()) return bounds;
  bounds = front().boundsExact();
  const_iterator iter = begin();
  // the closing path segment can be ignored, because it will always lie within the bbox of the rest of the path
  if ( iter != end() ) {
    for ( ++iter; iter != end() ; ++iter ) {
      bounds.unionWith(iter->boundsExact());
    }
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

Path &Path::operator*=(Affine const &m) {
  unshare();
  Sequence::iterator last = get_curves().end() - 1;
  Sequence::iterator it;
  Point prev;
  for (it = get_curves().begin() ; it != last ; ++it) {
    *it = boost::shared_ptr<Curve>((*it)->transformed(m));
    if ( it != get_curves().begin() && (*it)->initialPoint() != prev ) {
      THROW_CONTINUITYERROR();
    }
    prev = (*it)->finalPoint();
  }
  for ( int i = 0 ; i < 2 ; ++i ) {
    final_->setPoint(i, (*final_)[i] * m);
  }
  if (get_curves().size() > 1) {
    if ( front().initialPoint() != initialPoint() || back().finalPoint() != finalPoint() ) {
      THROW_CONTINUITYERROR();
    }
  }
  return *this;
}

Path &Path::operator*=(Translate const &m) {
  unshare();
  Sequence::iterator last = get_curves().end() - 1;
  Sequence::iterator it;
  Point prev;
  for (it = get_curves().begin() ; it != last ; ++it) {
    *(const_cast<Curve*>(&**it)) *= m;
    if ( it != get_curves().begin() && (*it)->initialPoint() != prev ) {
      THROW_CONTINUITYERROR();
    }
    prev = (*it)->finalPoint();
  }
  for ( int i = 0 ; i < 2 ; ++i ) {
    final_->setPoint(i, (*final_)[i] + m.vector());
  }
  if (get_curves().size() > 1) {
    if ( front().initialPoint() != initialPoint() || back().finalPoint() != finalPoint() ) {
      THROW_CONTINUITYERROR();
    }
  }
  return *this;
}

std::vector<double>
Path::allNearestPoints(Point const& _point, double from, double to) const
{
  using std::swap;

	if ( from > to ) swap(from, to);
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
	Rect bb(Geom::Point(0,0),Geom::Point(0,0));
	for ( unsigned int i = si + 1; i < ei; ++i )
	{
		bb = (_path[i].boundsFast());
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
	bb = (_path[ei].boundsFast());
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
	all_nearest.erase(std::unique(all_nearest.begin(), all_nearest.end()),
	                  all_nearest.end());
	return all_nearest;
}

std::vector<double>
Path::nearestPointPerCurve(Point const& _point) const
{
	//return a single nearest point for each curve in this path
	std::vector<double> np;
	for (const_iterator it = begin() ; it != end_default() ; ++it)
	//for (std::vector<Path>::const_iterator it = _path.begin(); it != _path.end(), ++it){
	{
	    np.push_back(it->nearestPoint(_point));
    }
	return np;
}  

double Path::nearestPoint(Point const &_point, double from, double to, double *distance_squared) const
{
  using std::swap;

	if ( from > to ) swap(from, to);
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
        if(sz == 0) {// naked moveto
            if (distance_squared != NULL)
                *distance_squared = distanceSq(_point, _path.initialPoint());
            return 0;
        }
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
		double nearest = _path[si].nearestPoint(_point, st, et);
		if (distance_squared != NULL)
		    *distance_squared = distanceSq(_point, _path[si].pointAt(nearest));
		return si + nearest;
	}

	double t;
	double nearest = _path[si].nearestPoint(_point, st);
	unsigned int ni = si;
	double dsq;
	double mindistsq = distanceSq(_point, _path[si].pointAt(nearest));
	for ( unsigned int i = si + 1; i < ei; ++i )
	{
            Rect bb = (_path[i].boundsFast());
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
	Rect bb = (_path[ei].boundsFast());
	dsq = distanceSq(_point, bb);
	if ( mindistsq > dsq )
	{
		t = _path[ei].nearestPoint(_point, 0, et);
		dsq = distanceSq(_point, _path[ei].pointAt(t));
		if ( mindistsq > dsq )
		{
			nearest = t;
			ni = ei;
			mindistsq = dsq;
		}
	}

    if (distance_squared != NULL)
        *distance_squared = mindistsq;

	return ni + nearest;
}

void Path::appendPortionTo(Path &ret, double from, double to) const {
  if (!(from >= 0 && to >= 0)) {
    THROW_RANGEERROR("from and to must be >=0 in Path::appendPortionTo");
  }
  if(to == 0) to = size()+0.999999;
  if(from == to) { return; }
  double fi, ti;
  double ff = modf(from, &fi), tf = modf(to, &ti);
  if(tf == 0) { ti--; tf = 1; }
  const_iterator fromi = inc(begin(), (unsigned)fi);
  if(fi == ti && from < to) {
    Curve *v = fromi->portion(ff, tf);
    ret.append(*v, STITCH_DISCONTINUOUS);
    delete v;
    return;
  }
  const_iterator toi = inc(begin(), (unsigned)ti);
  if(ff != 1.) {
    Curve *fromv = fromi->portion(ff, 1.);
    //fromv->setInitial(ret.finalPoint());
    ret.append(*fromv, STITCH_DISCONTINUOUS);
    delete fromv;
  }
  if(from >= to) {
    const_iterator ender = end();
    if(ender->initialPoint() == ender->finalPoint()) ++ender;
    ret.insert(ret.end(), ++fromi, ender, STITCH_DISCONTINUOUS);
    ret.insert(ret.end(), begin(), toi, STITCH_DISCONTINUOUS);
  } else {
    ret.insert(ret.end(), ++fromi, toi, STITCH_DISCONTINUOUS);
  }
  Curve *tov = toi->portion(0., tf);
  ret.append(*tov, STITCH_DISCONTINUOUS);
  delete tov;
}

void Path::do_update(Sequence::iterator first_replaced,
                     Sequence::iterator last_replaced,
                     Sequence::iterator first,
                     Sequence::iterator last)
{
  // note: modifies the contents of [first,last)
  check_continuity(first_replaced, last_replaced, first, last);
  if ( ( last - first ) == ( last_replaced - first_replaced ) ) {
    std::copy(first, last, first_replaced);
  } else {
    // this approach depends on std::vector's behavior WRT iterator stability
    get_curves().erase(first_replaced, last_replaced);
    get_curves().insert(first_replaced, first, last);
  }

  if ( get_curves().front().get() != final_ ) {
    final_->setPoint(0, back().finalPoint());
    final_->setPoint(1, front().initialPoint());
  }
}

void Path::do_append(Curve *c) {
  if ( get_curves().front().get() == final_ ) {
    final_->setPoint(1, c->initialPoint());
  } else {
    if (c->initialPoint() != finalPoint()) {
      THROW_CONTINUITYERROR();
    }
  }
  get_curves().insert(get_curves().end()-1, boost::shared_ptr<Curve>(c));
  final_->setPoint(0, c->finalPoint());
}

void Path::stitch(Sequence::iterator first_replaced,
                  Sequence::iterator last_replaced,
                  Sequence &source)
{
  if (!source.empty()) {
    if ( first_replaced != get_curves().begin() ) {
      if ( (*first_replaced)->initialPoint() != source.front()->initialPoint() ) {
        Curve *stitch = new StitchSegment((*first_replaced)->initialPoint(),
                                          source.front()->initialPoint());
        source.insert(source.begin(), boost::shared_ptr<Curve>(stitch));
      }
    }
    if ( last_replaced != (get_curves().end()-1) ) {
      if ( (*last_replaced)->finalPoint() != source.back()->finalPoint() ) {
        Curve *stitch = new StitchSegment(source.back()->finalPoint(),
                                          (*last_replaced)->finalPoint());
        source.insert(source.end(), boost::shared_ptr<Curve>(stitch));
      }
    }
  } else if ( first_replaced != last_replaced && first_replaced != get_curves().begin() && last_replaced != get_curves().end()-1) {
    if ( (*first_replaced)->initialPoint() != (*(last_replaced-1))->finalPoint() ) {
      Curve *stitch = new StitchSegment((*(last_replaced-1))->finalPoint(),
                                        (*first_replaced)->initialPoint());
      source.insert(source.begin(), boost::shared_ptr<Curve>(stitch));
    }
  }
}

void Path::check_continuity(Sequence::iterator first_replaced,
                            Sequence::iterator last_replaced,
                            Sequence::iterator first,
                            Sequence::iterator last)
{
  if ( first != last ) {
    if ( first_replaced != get_curves().begin() ) {
      if ( (*first_replaced)->initialPoint() != (*first)->initialPoint() ) {
        THROW_CONTINUITYERROR();
      }
    }
    if ( last_replaced != (get_curves().end()-1) ) {
      if ( (*(last_replaced-1))->finalPoint() != (*(last-1))->finalPoint() ) {
        THROW_CONTINUITYERROR();
      }
    }
  } else if ( first_replaced != last_replaced && first_replaced != get_curves().begin() && last_replaced != get_curves().end()-1) {
    if ( (*first_replaced)->initialPoint() != (*(last_replaced-1))->finalPoint() ) {
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

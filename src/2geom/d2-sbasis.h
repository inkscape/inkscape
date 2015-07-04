/**
 * \file
 * \brief  Do not include this file
 *
 * We don't actually want anyone to
 * include this, other than D2.h.
 *//*
 * Authors:
 *      ? <?@?.?>
 * 
 * Copyright ?-?  authors
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

#ifdef LIB2GEOM_SEEN_D2_H  /*This is intentional: we don't actually want anyone to
                             include this, other than D2.h.  If somone else tries, D2
                             won't be defined.  If it is, this will already be included. */
#ifndef LIB2GEOM_SEEN_D2_SBASIS_H
#define LIB2GEOM_SEEN_D2_SBASIS_H

#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/piecewise.h>
#include <2geom/affine.h>

//TODO: implement intersect

namespace Geom {

inline D2<SBasis> compose(D2<SBasis> const & a, SBasis const & b) {
    return D2<SBasis>(compose(a[X], b), compose(a[Y], b));
}

SBasis L2(D2<SBasis> const & a, unsigned k);
double L2(D2<double> const & a);

D2<SBasis> multiply(Linear const & a, D2<SBasis> const & b);
inline D2<SBasis> operator*(Linear const & a, D2<SBasis> const & b) { return multiply(a, b); }
D2<SBasis> multiply(SBasis const & a, D2<SBasis> const & b);
inline D2<SBasis> operator*(SBasis const & a, D2<SBasis> const & b) { return multiply(a, b); }
D2<SBasis> truncate(D2<SBasis> const & a, unsigned terms);

unsigned sbasis_size(D2<SBasis> const & a);
double tail_error(D2<SBasis> const & a, unsigned tail);

//Piecewise<D2<SBasis> > specific decls:

Piecewise<D2<SBasis> > sectionize(D2<Piecewise<SBasis> > const &a);
D2<Piecewise<SBasis> > make_cuts_independent(Piecewise<D2<SBasis> > const &a);
Piecewise<D2<SBasis> > rot90(Piecewise<D2<SBasis> > const &a);
Piecewise<SBasis> dot(Piecewise<D2<SBasis> > const &a, Piecewise<D2<SBasis> > const &b);
Piecewise<SBasis> dot(Piecewise<D2<SBasis> > const &a, Point const &b);
Piecewise<SBasis> cross(Piecewise<D2<SBasis> > const &a, Piecewise<D2<SBasis> > const &b);

Piecewise<D2<SBasis> > operator*(Piecewise<D2<SBasis> > const &a, Affine const &m);

Piecewise<D2<SBasis> > force_continuity(Piecewise<D2<SBasis> > const &f, double tol=0, bool closed=false);

std::vector<Piecewise<D2<SBasis> > > fuse_nearby_ends(std::vector<Piecewise<D2<SBasis> > > const &f, double tol=0);

std::vector<Geom::Piecewise<Geom::D2<Geom::SBasis> > > split_at_discontinuities (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwsbin, double tol = .0001);

/**
* note that -unitTangentAt(reverse(a),0.) == unitTangentAt(a,1.) but the former is more reliable (sometimes the sign is wrong for the latter)
*/
Point unitTangentAt(D2<SBasis> const & a, Coord t, unsigned n = 3);

class CoordIterator
: public std::iterator<std::input_iterator_tag, SBasis const>
{
public:
  CoordIterator(std::vector<D2<SBasis> >::const_iterator const &iter, unsigned d) : impl_(iter), ix_(d) {}

  inline bool operator==(CoordIterator const &other) { return other.impl_ == impl_; }
  inline bool operator!=(CoordIterator const &other) { return other.impl_ != impl_; }

  inline SBasis operator*() const {
        return (*impl_)[ix_];
  }

  inline CoordIterator &operator++() {
    ++impl_;
    return *this;
  }
  inline CoordIterator operator++(int) {
    CoordIterator old=*this;
    ++(*this);
    return old;
  }

private:
  std::vector<D2<SBasis> >::const_iterator impl_;
  unsigned ix_;
};

inline CoordIterator iterateCoord(Piecewise<D2<SBasis> > const &a, unsigned d) {
    return CoordIterator(a.segs.begin(), d);
}

//bounds specializations with order
inline OptRect bounds_fast(D2<SBasis> const & s, unsigned order=0) {
    OptRect retval;
    OptInterval xint = bounds_fast(s[X], order);
    if (xint) {
        OptInterval yint = bounds_fast(s[Y], order);
        if (yint) {
            retval = Rect(*xint, *yint);
        }
    }
    return retval;
}
inline OptRect bounds_local(D2<SBasis> const & s, OptInterval i, unsigned order=0) {
    OptRect retval;
    OptInterval xint = bounds_local(s[X], i, order);
    OptInterval yint = bounds_local(s[Y], i, order);
    if (xint && yint) {
        retval = Rect(*xint, *yint);
    }
    return retval;
}

std::vector<Interval> level_set( D2<SBasis> const &f, Rect region);
std::vector<Interval> level_set( D2<SBasis> const &f, Point p, double tol);
std::vector<std::vector<Interval> > level_sets( D2<SBasis> const &f, std::vector<Rect> regions);
std::vector<std::vector<Interval> > level_sets( D2<SBasis> const &f, std::vector<Point> pts, double tol);

}

#endif // LIB2GEOM_SEEN_D2_SBASIS_H
#endif // LIB2GEOM_SEEN_D2_H


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

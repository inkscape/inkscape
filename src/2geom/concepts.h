/*
 * concepts.h - Declares various mathematical concepts, for restriction of template parameters
 *
 * Copyright 2007 Michael Sloan <mgsloan@gmail.com>
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
 * in the file COPYING-LGPL-2.1; if not, output to the Free Software
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

#ifndef SEEN_CONCEPTS_H
#define SEEN_CONCEPTS_H

#include "sbasis.h"
#include "interval.h"
#include "point.h"
#include <vector>
#include <boost/concept_check.hpp>

namespace Geom {

//forward decls
template <typename T> class D2;

template <typename T> struct ResultTraits;

template <> struct ResultTraits<double> {
  typedef Interval bounds_type;
  typedef SBasis sb_type;
};

template <> struct ResultTraits<Point > {
  typedef D2<Interval> bounds_type;
  typedef D2<SBasis> sb_type;
};

//A concept for one-dimensional functions defined on [0,1]
template <typename T>
struct FragmentConcept {
    typedef typename T::output_type                        OutputType;
    typedef typename ResultTraits<OutputType>::bounds_type BoundsType;
    typedef typename ResultTraits<OutputType>::sb_type     SbType;
    T t;
    double d;
    OutputType o;
    bool b;
    BoundsType i;
    Interval dom;
    std::vector<OutputType> v;
    unsigned u;
    SbType sb;
    void constraints() {
        t = T(o);
        b = t.isZero();
        b = t.isConstant();
        b = t.isFinite();
        o = t.at0();
        o = t.at1();
        o = t.valueAt(d);
        o = t(d);
        v = t.valueAndDerivatives(d, u);
		//Is a pure derivative (ignoring others) accessor ever much faster?
		//u = number of values returned. first val is value.
        sb = t.toSBasis();
        t = reverse(t);
        i = bounds_fast(t);
        i = bounds_exact(t);
        i = bounds_local(t, dom);
        /*With portion, Interval makes some sense, but instead I'm opting for
          doubles, for the following reasons:
          A) This way a reversed portion may be specified
          B) Performance might be a bit better for piecewise and such
          C) Interval version provided below
         */
        t = portion(t, d, d);
    }
};

template <typename T>
inline T portion(const T& t, const Interval& i) { return portion(t, i.min(), i.max()); }

template <typename T>
struct NearConcept {
    T a, b;
    double tol;
    bool res;
    void constraints() {
        res = are_near(a, b, tol);
    }
};

template <typename T>
struct OffsetableConcept {
    T t;
    typename T::output_type d;
    void constraints() {
        t = t + d; t += d;
        t = t - d; t -= d;
    }
};

template <typename T>
struct ScalableConcept {
    T t;
    typename T::output_type d;
    void constraints() {
        t = -t;
        t = t * d; t *= d;
        t = t / d; t /= d;
    }
};

template <class T>
struct AddableConcept {
    T i, j;
    void constraints() {
        i += j; i = i + j;
        i -= j; i = i - j;
    }
};

template <class T>
struct MultiplicableConcept {
    T i, j;
    void constraints() {
        i *= j; i = i * j;
    }
};

};

#endif //SEEN_CONCEPTS_H

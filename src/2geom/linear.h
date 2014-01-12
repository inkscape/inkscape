/**
 * \file
 * \brief  Linear fragment function class
 *
 *  Authors:
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *   Michael Sloan <mgsloan@gmail.com>
 *
 * Copyright (C) 2006-2007 authors
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

#ifndef SEEN_LINEAR_H
#define SEEN_LINEAR_H
#include <2geom/interval.h>
#include <2geom/math-utils.h>


//#define USE_SBASIS_OF

#ifdef USE_SBASIS_OF

#include "linear-of.h"

#else

namespace Geom{

inline double lerp(double t, double a, double b) { return a*(1-t) + b*t; }

class SBasis;

class Linear{
public:
    double a[2];
    Linear() {a[0]=0; a[1]=0;}
    Linear(double aa, double b) {a[0] = aa; a[1] = b;}
    Linear(double aa) {a[0] = aa; a[1] = aa;}

    double operator[](const int i) const {
        assert(i >= 0);
        assert(i < 2);
        return a[i];
    }
    double& operator[](const int i) {
        assert(i >= 0);
        assert(i < 2);
        return a[i];
    }

    //IMPL: FragmentConcept
    typedef double output_type;
    inline bool isZero(double eps=EPSILON) const { return are_near(a[0], 0., eps) && are_near(a[1], 0., eps); }
    inline bool isConstant(double eps=EPSILON) const { return are_near(a[0], a[1], eps); }
    inline bool isFinite() const { return IS_FINITE(a[0]) && IS_FINITE(a[1]); }

    inline double at0() const { return a[0]; }
    inline double at1() const { return a[1]; }

    inline double valueAt(double t) const { return lerp(t, a[0], a[1]); }
    inline double operator()(double t) const { return valueAt(t); }

    //defined in sbasis.h
    inline SBasis toSBasis() const;

    inline OptInterval bounds_exact() const { return Interval(a[0], a[1]); }
    inline OptInterval bounds_fast() const { return bounds_exact(); }
    inline OptInterval bounds_local(double u, double v) const { return Interval(valueAt(u), valueAt(v)); }

    double tri() const {
        return a[1] - a[0];
    }
    double hat() const {
        return (a[1] + a[0])/2;
    }
};

inline Linear reverse(Linear const &a) { return Linear(a[1], a[0]); }

//IMPL: AddableConcept
inline Linear operator+(Linear const & a, Linear const & b) {
    return Linear(a[0] + b[0], a[1] + b[1]);
}
inline Linear operator-(Linear const & a, Linear const & b) {
    return Linear(a[0] - b[0], a[1] - b[1]);
}
inline Linear& operator+=(Linear & a, Linear const & b) {
    a[0] += b[0]; a[1] += b[1];
    return a;
}
inline Linear& operator-=(Linear & a, Linear const & b) {
    a[0] -= b[0]; a[1] -= b[1];
    return a;
}
//IMPL: OffsetableConcept
inline Linear operator+(Linear const & a, double b) {
    return Linear(a[0] + b, a[1] + b);
}
inline Linear operator-(Linear const & a, double b) {
    return Linear(a[0] - b, a[1] - b);
}
inline Linear& operator+=(Linear & a, double b) {
    a[0] += b; a[1] += b;
    return a;
}
inline Linear& operator-=(Linear & a, double b) {
    a[0] -= b; a[1] -= b;
    return a;
}
//IMPL: boost::EqualityComparableConcept
inline bool operator==(Linear const & a, Linear const & b) {
    return a[0] == b[0] && a[1] == b[1];
}
inline bool operator!=(Linear const & a, Linear const & b) {
    return a[0] != b[0] || a[1] != b[1];
}
//IMPL: ScalableConcept
inline Linear operator-(Linear const &a) {
    return Linear(-a[0], -a[1]);
}
inline Linear operator*(Linear const & a, double b) {
    return Linear(a[0]*b, a[1]*b);
}
inline Linear operator/(Linear const & a, double b) {
    return Linear(a[0]/b, a[1]/b);
}
inline Linear operator*=(Linear & a, double b) {
    a[0] *= b; a[1] *= b;
    return a;
}
inline Linear operator/=(Linear & a, double b) {
    a[0] /= b; a[1] /= b;
    return a;
}

}
#endif

#endif //SEEN_LINEAR_H

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

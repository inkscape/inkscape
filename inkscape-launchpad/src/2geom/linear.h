/**
 * \file
 * \brief  Linear fragment function class
 *//*
 *  Authors:
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *   Michael Sloan <mgsloan@gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2006-2015 authors
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

#ifndef LIB2GEOM_SEEN_LINEAR_H
#define LIB2GEOM_SEEN_LINEAR_H

#include <2geom/interval.h>
#include <2geom/math-utils.h>

namespace Geom {

class SBasis;

/**
 * @brief Function that interpolates linearly between two values.
 * @ingroup Fragments
 */
class Linear
    : boost::additive< Linear
    , boost::arithmetic< Linear, Coord
    , boost::equality_comparable< Linear
      > > >
{
public:
    Coord a[2];
    Linear() {a[0]=0; a[1]=0;}
    Linear(Coord aa, Coord b) {a[0] = aa; a[1] = b;}
    Linear(Coord aa) {a[0] = aa; a[1] = aa;}

    Coord operator[](unsigned i) const {
        assert(i < 2);
        return a[i];
    }
    Coord &operator[](unsigned i) {
        assert(i < 2);
        return a[i];
    }

    //IMPL: FragmentConcept
    typedef Coord output_type;
    bool isZero(Coord eps=EPSILON) const { return are_near(a[0], 0., eps) && are_near(a[1], 0., eps); }
    bool isConstant(Coord eps=EPSILON) const { return are_near(a[0], a[1], eps); }
    bool isFinite() const { return IS_FINITE(a[0]) && IS_FINITE(a[1]); }

    Coord at0() const { return a[0]; }
    Coord &at0() { return a[0]; }
    Coord at1() const { return a[1]; }
    Coord &at1() { return a[1]; }

    Coord valueAt(Coord t) const { return lerp(t, a[0], a[1]); }
    Coord operator()(Coord t) const { return valueAt(t); }

    // not very useful, but required for FragmentConcept
    std::vector<Coord> valueAndDerivatives(Coord t, unsigned n) {
        std::vector<Coord> result(n+1, 0.0);
        result[0] = valueAt(t);
        if (n >= 1) {
            result[1] = a[1] - a[0];
        }
        return result;
    }

    //defined in sbasis.h
    inline SBasis toSBasis() const;

    OptInterval bounds_exact() const { return Interval(a[0], a[1]); }
    OptInterval bounds_fast() const { return bounds_exact(); }
    OptInterval bounds_local(double u, double v) const { return Interval(valueAt(u), valueAt(v)); }

    double tri() const {
        return a[1] - a[0];
    }
    double hat() const {
        return (a[1] + a[0])/2;
    }

    // addition of other Linears
    Linear &operator+=(Linear const &other) {
        a[0] += other.a[0];
        a[1] += other.a[1];
        return *this;
    }
    Linear &operator-=(Linear const &other) {
        a[0] -= other.a[0];
        a[1] -= other.a[1];
        return *this;
    }

    // 
    Linear &operator+=(Coord x) {
        a[0] += x; a[1] += x;
        return *this;
    }
    Linear &operator-=(Coord x) {
        a[0] -= x; a[1] -= x;
        return *this;
    }
    Linear &operator*=(Coord x) {
        a[0] *= x; a[1] *= x;
        return *this;
    }
    Linear &operator/=(Coord x) {
        a[0] /= x; a[1] /= x;
        return *this;
    }
    Linear operator-() const {
        Linear ret(-a[0], -a[1]);
        return ret;
    }

    bool operator==(Linear const &other) const {
        return a[0] == other.a[0] && a[1] == other.a[1];
    }
};

inline Linear reverse(Linear const &a) { return Linear(a[1], a[0]); }
inline Linear portion(Linear const &a, Coord from, Coord to) {
    Linear result(a.valueAt(from), a.valueAt(to));
    return result;
}

} // end namespace Geom

#endif //LIB2GEOM_SEEN_LINEAR_H

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

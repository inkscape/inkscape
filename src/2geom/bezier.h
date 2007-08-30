/*
 * bezier.h
 *
 * Copyright 2007  MenTaLguY <mental@rydia.net>
 * Copyright 2007  Michael Sloan <mgsloan@gmail.com>
 * Copyright 2007  Nathan Hurst <njh@njhurst.com>
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

#ifndef SEEN_BEZIER_H
#define SEEN_BEZIER_H

#include "coord.h"
#include "isnan.h"
#include "bezier-to-sbasis.h"
#include "d2.h"
#include "solver.h"
#include <boost/optional/optional.hpp>

namespace Geom {

template<unsigned order>
Coord subdivideArr(Coord t, Coord const *v, Coord *left, Coord *right) {
    Coord vtemp[order+1][order+1];

    /* Copy control points	*/
    std::copy(v, v+order+1, vtemp[0]);

    /* Triangle computation	*/
    for (unsigned i = 1; i <= order; i++) {	
        for (unsigned j = 0; j <= order - i; j++) {
            vtemp[i][j] = lerp(t, vtemp[i-1][j], vtemp[i-1][j+1]);
        }
    }
    if(left != NULL)
        for (unsigned j = 0; j <= order; j++)
            left[j]  = vtemp[j][0];
    if(right != NULL)
        for (unsigned j = 0; j <= order; j++)
            right[j] = vtemp[order-j][j];

    return (vtemp[order][0]);
}


template <unsigned order>
class Bezier {
private:
    Coord c_[order+1];

    template<unsigned ord>
    friend Bezier<ord> portion(const Bezier<ord> & a, Coord from, Coord to);

    template<unsigned ord>
    friend Interval bounds_fast(Bezier<ord> const & b);

    template<unsigned ord>
    friend Bezier<ord-1> derivative(const Bezier<ord> & a);

protected:
    Bezier(Coord const c[]) {
        std::copy(c, c+order+1, c_);
    }

public:

    //TODO: fix this assert - get it compiling!
    //template <unsigned required_order>
    //static void assert_order(Bezier<required_order> const *) {}

    Bezier() {}

    //Construct an order-0 bezier (constant Bézier)
    explicit Bezier(Coord c0) {
        //assert_order<0>(this);
        c_[0] = c0;
    }

    //Construct an order-1 bezier (linear Bézier)
    Bezier(Coord c0, Coord c1) {
        //assert_order<1>(this);
        c_[0] = c0; c_[1] = c1;
    }

    //Construct an order-2 bezier (quadratic Bézier)
    Bezier(Coord c0, Coord c1, Coord c2) {
        //assert_order<2>(this);
        c_[0] = c0; c_[1] = c1; c_[2] = c2;
    }

    //Construct an order-3 bezier (cubic Bézier)
    Bezier(Coord c0, Coord c1, Coord c2, Coord c3) {
        //assert_order<3>(this);
        c_[0] = c0; c_[1] = c1; c_[2] = c2; c_[3] = c3;
    }

    inline unsigned degree() const { return order; }

    //IMPL: FragmentConcept
    typedef Coord output_type;
    inline bool isZero() const { 
        for(unsigned i = 0; i <= order; i++) {
            if(c_[i] != 0) return false;
        }
        return true;
    }
    inline bool isFinite() const {
        for(unsigned i = 0; i <= order; i++) {
            if(!is_finite(c_[i])) return false;
        }
        return true;
    }
    inline Coord at0() const { return c_[0]; }
    inline Coord at1() const { return c_[order]; }

    inline Coord valueAt(double t) const { return subdivideArr<order>(t, c_, NULL, NULL); }
    inline Coord operator()(double t) const { return valueAt(t); }

    inline SBasis toSBasis() const { return bezier_to_sbasis<order>(c_); }

    //Only mutator
    inline Coord &operator[](unsigned ix) { return c_[ix]; }
    inline Coord const &operator[](unsigned ix) const { return c_[ix]; }
    inline void setPoint(unsigned ix, double val) { c_[ix] = val; }
    
    /* This is inelegant, as it uses several extra stores.  I think there might be a way to
     * evaluate roughly in situ. */
    
    std::vector<Coord> valueAndDerivatives(Coord t, unsigned n_derivs) const {
        throw NotImplemented();
        // Can't be implemented without a dynamic version of subdivide.
        /*std::vector<Coord> val_n_der;
        Coord d_[order+1];
        for(int di = n_derivs; di > 0; di--) {
            val_n_der.push_back(subdivideArr<di>(t, d_, NULL, NULL));
            for(unsigned i = 0; i < di; i++) {
                d[i] = order*(a._c[i+1] - a._c[i]);
            }
            }*/
    }
  
    std::pair<Bezier<order>, Bezier<order> > subdivide(Coord t) const {
        Bezier<order> a, b;
        subdivideArr(t, order, c_, a.c_, b.c_);
        return std::pair<Bezier<order>, Bezier<order> >(a, b);
    }

    std::vector<double> roots() const {
        std::vector<double> solutions;
        find_bernstein_roots(c_, order, solutions, 0, 0.0, 1.0);
        return solutions;
    }
};

//TODO: implement others
template<unsigned order>
Bezier<order> operator+(const Bezier<order> & a, double v) {
    Bezier<order> result;
    for(unsigned i = 0; i <= order; i++)
        result[i] = a[i] + v;
    return result;
}
template<unsigned order>
Bezier<order> operator-(const Bezier<order> & a, double v) {
    Bezier<order> result;
    for(unsigned i = 0; i <= order; i++)
        result[i] = a[i] - v;
    return result;
}
template<unsigned order>
Bezier<order> operator*(const Bezier<order> & a, double v) {
    Bezier<order> result;
    for(unsigned i = 0; i <= order; i++)
        result[i] = a[i] * v;
    return result;
}
template<unsigned order>
Bezier<order> operator/(const Bezier<order> & a, double v) {
    Bezier<order> result;
    for(unsigned i = 0; i <= order; i++)
        result[i] = a[i] / v;
    return result;
}

template<unsigned order>
Bezier<order> reverse(const Bezier<order> & a) {
    Bezier<order> result;
    for(unsigned i = 0; i <= order; i++)
        result[i] = a[order - i];
    return result;
}

template<unsigned order>
Bezier<order> portion(const Bezier<order> & a, double from, double to) {
    //TODO: implement better?
    Coord res[order+1];
    if(from == 0) {
        if(to == 1) { return Bezier<order>(a); }
        subdivideArr<order>(to, a.c_, res, NULL);
        return Bezier<order>(res);
    }
    subdivideArr<order>(from, a.c_, NULL, res);
    if(to == 1) return Bezier<order>(res);
    Coord res2[order+1];
    subdivideArr<order>((to - from)/(1 - from), res, res2, NULL);
    return Bezier<order>(res2);
}

template<unsigned order>
std::vector<Point> bezier_points(const D2<Bezier<order> > & a) {
    std::vector<Point> result;
    for(unsigned i = 0; i <= order; i++) {
        Point p;
        for(unsigned d = 0; d < 2; d++) p[d] = a[d][i];
        result.push_back(p);
    }
    return result;
}

template<unsigned order>
Bezier<order-1> derivative(const Bezier<order> & a) {
    Bezier<order-1> der;
    
    for(unsigned i = 0; i < order; i++) {
        der.c_[i] = order*(a.c_[i+1] - a.c_[i]);
    }
    return der;
}

template<unsigned order>
inline Interval bounds_fast(Bezier<order> const & b) {
    return Interval::fromArray(b.c_, order+1);
}

//TODO: better bounds exact
template<unsigned order>
inline Interval bounds_exact(Bezier<order> const & b) {
    return bounds_exact(b.toSBasis());
}

template<unsigned order>
inline Interval bounds_local(Bezier<order> const & b, Interval i) {
    return bounds_fast(portion(b, i.min(), i.max()));
    //return bounds_local(b.toSBasis(), i);
}

}
#endif //SEEN_BEZIER_H
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

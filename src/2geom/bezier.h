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
#include <valarray>
#include "isnan.h"
#include "bezier-to-sbasis.h"
#include "d2.h"
#include "solver.h"
#include <boost/optional/optional.hpp>

namespace Geom {

inline Coord subdivideArr(Coord t, Coord const *v, Coord *left, Coord *right, unsigned order) {
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


class Bezier {
private:
    std::valarray<Coord> c_;

    friend Bezier portion(const Bezier & a, Coord from, Coord to);

    friend Interval bounds_fast(Bezier const & b);

    friend Bezier derivative(const Bezier & a);

protected:
    Bezier(Coord const c[], unsigned ord) : c_(c, ord+1){
        //std::copy(c, c+order()+1, &c_[0]);
    }

public:
    unsigned int order() const { return c_.size()-1;}
    unsigned int size() const { return c_.size();}
    
    Bezier() :c_(0., 32) {}
    Bezier(const Bezier& b) :c_(b.c_) {}
    Bezier &operator=(Bezier const &other) {
        if ( c_.size() != other.c_.size() ) {
            c_.resize(other.c_.size());
        }
        c_ = other.c_;
        return *this;
    }

    struct Order {
        unsigned order;
        explicit Order(Bezier const &b) : order(b.order()) {}
        explicit Order(unsigned o) : order(o) {}
        operator unsigned() const { return order; }
    };

    //Construct an arbitrary order bezier
    Bezier(Order ord) : c_(0., ord.order+1) {
        assert(ord.order ==  order());
    }

    explicit Bezier(Coord c0) : c_(0., 1) {
        c_[0] = c0;
    }

    //Construct an order-1 bezier (linear Bézier)
    Bezier(Coord c0, Coord c1) : c_(0., 2) {
        c_[0] = c0; c_[1] = c1;
    }

    //Construct an order-2 bezier (quadratic Bézier)
    Bezier(Coord c0, Coord c1, Coord c2) : c_(0., 3) {
        c_[0] = c0; c_[1] = c1; c_[2] = c2;
    }

    //Construct an order-3 bezier (cubic Bézier)
    Bezier(Coord c0, Coord c1, Coord c2, Coord c3) : c_(0., 4) {
        c_[0] = c0; c_[1] = c1; c_[2] = c2; c_[3] = c3;
    }

    inline unsigned degree() const { return order(); }

    //IMPL: FragmentConcept
    typedef Coord output_type;
    inline bool isZero() const { 
        for(unsigned i = 0; i <= order(); i++) {
            if(c_[i] != 0) return false;
        }
        return true;
    }
    inline bool isConstant() const {
        for(unsigned i = 1; i <= order(); i++) {
            if(c_[i] != c_[0]) return false;
        }
        return true;
    }
    inline bool isFinite() const {
        for(unsigned i = 0; i <= order(); i++) {
            if(!is_finite(c_[i])) return false;
        }
        return true;
    }
    inline Coord at0() const { return c_[0]; }
    inline Coord at1() const { return c_[order()]; }

    inline Coord valueAt(double t) const { 
        return subdivideArr(t, &c_[0], NULL, NULL, order()); 
    }
    inline Coord operator()(double t) const { return valueAt(t); }

    inline SBasis toSBasis() const { 
        return bezier_to_sbasis(&c_[0], order());
    }

    //Only mutator
    inline Coord &operator[](unsigned ix) { return c_[ix]; }
    inline Coord const &operator[](unsigned ix) const { return c_[ix]; }
    inline void setPoint(unsigned ix, double val) { c_[ix] = val; }
    
    /* This is inelegant, as it uses several extra stores.  I think there might be a way to
     * evaluate roughly in situ. */
    
    std::vector<Coord> valueAndDerivatives(Coord t, unsigned n_derivs) const {
        std::vector<Coord> val_n_der;
        Coord d_[order()+1];
        unsigned nn = n_derivs;
        if(nn > order())
            nn = order();
        for(unsigned i = 0; i < size(); i++)
            d_[i] = c_[i];
        for(unsigned di = 0; di < nn; di++) {
            val_n_der.push_back(subdivideArr(t, d_, NULL, NULL, order() - di));
            for(unsigned i = 0; i < order() - di; i++) {
                d_[i] = (order()-di)*(d_[i+1] - d_[i]);
            }
        }
        val_n_der.resize(n_derivs);
        return val_n_der;
    }
  
    std::pair<Bezier, Bezier > subdivide(Coord t) const {
        Bezier a(Bezier::Order(*this)), b(Bezier::Order(*this));
        subdivideArr(t, &c_[0], &a.c_[0], &b.c_[0], order());
        return std::pair<Bezier, Bezier >(a, b);
    }

    std::vector<double> roots() const {
        std::vector<double> solutions;
        find_bernstein_roots(&c_[0], order(), solutions, 0, 0.0, 1.0);
        return solutions;
    }
};

//TODO: implement others
inline Bezier operator+(const Bezier & a, double v) {
    Bezier result = Bezier(Bezier::Order(a));
    for(unsigned i = 0; i <= a.order(); i++)
        result[i] = a[i] + v;
    return result;
}

inline Bezier operator-(const Bezier & a, double v) {
    Bezier result = Bezier(Bezier::Order(a));
    for(unsigned i = 0; i <= a.order(); i++)
        result[i] = a[i] - v;
    return result;
}

inline Bezier operator*(const Bezier & a, double v) {
    Bezier result = Bezier(Bezier::Order(a));
    for(unsigned i = 0; i <= a.order(); i++)
        result[i] = a[i] * v;
    return result;
}

inline Bezier operator/(const Bezier & a, double v) {
    Bezier result = Bezier(Bezier::Order(a));
    for(unsigned i = 0; i <= a.order(); i++)
        result[i] = a[i] / v;
    return result;
}

inline Bezier reverse(const Bezier & a) {
    Bezier result = Bezier(Bezier::Order(a));
    for(unsigned i = 0; i <= a.order(); i++)
        result[i] = a[a.order() - i];
    return result;
}

inline Bezier portion(const Bezier & a, double from, double to) {
    //TODO: implement better?
    Coord res[a.order()+1];
    if(from == 0) {
        if(to == 1) { return Bezier(a); }
        subdivideArr(to, &a.c_[0], res, NULL, a.order());
        return Bezier(res, a.order());
    }
    subdivideArr(from, &a.c_[0], NULL, res, a.order());
    if(to == 1) return Bezier(res, a.order());
    Coord res2[a.order()+1];
    subdivideArr((to - from)/(1 - from), res, res2, NULL, a.order());
    return Bezier(res2, a.order());
}

// XXX Todo: how to handle differing orders
inline std::vector<Point> bezier_points(const D2<Bezier > & a) {
    std::vector<Point> result;
    for(unsigned i = 0; i <= a[0].order(); i++) {
        Point p;
        for(unsigned d = 0; d < 2; d++) p[d] = a[d][i];
        result.push_back(p);
    }
    return result;
}

inline Bezier derivative(const Bezier & a) {
    if(a.order() == 1) return Bezier(0.0);
    Bezier der(Bezier::Order(a.order()-1));
    
    for(unsigned i = 0; i < a.order(); i++) {
        der.c_[i] = a.order()*(a.c_[i+1] - a.c_[i]);
    }
    return der;
}

inline Bezier integral(const Bezier & a) {
    Bezier inte(Bezier::Order(a.order()+1));
    
    inte[0] = 0;
    for(unsigned i = 0; i < inte.order(); i++) {
        inte[i+1] = inte[i] + a[i]/(inte.order());
    }
    return inte;
}

inline Interval bounds_fast(Bezier const & b) {
    return Interval::fromArray(&b.c_[0], b.size());
}

//TODO: better bounds exact
inline Interval bounds_exact(Bezier const & b) {
    return bounds_exact(b.toSBasis());
}

inline Interval bounds_local(Bezier const & b, Interval i) {
    return bounds_fast(portion(b, i.min(), i.max()));
    //return bounds_local(b.toSBasis(), i);
}

inline std::ostream &operator<< (std::ostream &out_file, const Bezier & b) {
    for(unsigned i = 0; i < b.size(); i++) {
        out_file << b[i] << ", ";
    }
    return out_file;
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

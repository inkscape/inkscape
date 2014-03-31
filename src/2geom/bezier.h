/**
 * @file
 * @brief Bezier polynomial
 *//*
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Michael Sloan <mgsloan@gmail.com>
 *   Nathan Hurst <njh@njhurst.com>
 *
 * Copyright 2007 Authors
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

#ifndef LIB2GEOM_SEEN_BEZIER_H
#define LIB2GEOM_SEEN_BEZIER_H

#include <valarray>
#include <boost/optional.hpp>
#include <2geom/coord.h>
#include <2geom/choose.h>
#include <valarray>
#include <2geom/math-utils.h>
#include <2geom/d2.h>
#include <2geom/solver.h>

namespace Geom {

inline Coord subdivideArr(Coord t, Coord const *v, Coord *left, Coord *right, unsigned order) {
/*
 *  Bernstein : 
 *	Evaluate a Bernstein function at a particular parameter value
 *      Fill in control points for resulting sub-curves.
 * 
 */

    unsigned N = order+1;
    std::valarray<Coord> row(N);
    for (unsigned i = 0; i < N; i++)
        row[i] = v[i];

    // Triangle computation
    const double omt = (1-t);
    if(left)
        left[0] = row[0];
    if(right)
        right[order] = row[order];
    for (unsigned i = 1; i < N; i++) {
        for (unsigned j = 0; j < N - i; j++) {
            row[j] = omt*row[j] + t*row[j+1];
        }
        if(left)
            left[i] = row[0];
        if(right)
            right[order-i] = row[order-i];
    }
    return (row[0]);
/*
    Coord vtemp[order+1][order+1];

    // Copy control points
    std::copy(v, v+order+1, vtemp[0]);

    // Triangle computation
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

            return (vtemp[order][0]);*/
}

template <typename T>
inline T bernsteinValueAt(double t, T const *c_, unsigned n) {
    double u = 1.0 - t;
    double bc = 1;
    double tn = 1;
    T tmp = c_[0]*u;
    for(unsigned i=1; i<n; i++){
        tn = tn*t;
        bc = bc*(n-i+1)/i;
        tmp = (tmp + tn*bc*c_[i])*u;
    }
    return (tmp + tn*t*c_[n]);
}

class Bezier {
private:
    std::valarray<Coord> c_;

    friend Bezier portion(const Bezier & a, Coord from, Coord to);

    friend OptInterval bounds_fast(Bezier const & b);

    friend Bezier derivative(const Bezier & a);

    friend class Bernstein;

    void
    find_bezier_roots(std::vector<double> & solutions,
                      double l, double r) const;

protected:
    Bezier(Coord const c[], unsigned ord) : c_(c, ord+1){
        //std::copy(c, c+order()+1, &c_[0]);
    }

public:
    unsigned int order() const { return c_.size()-1;}
    unsigned int size() const { return c_.size();}

    Bezier() {}
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

    void resize (unsigned int n, Coord v = 0)
    {
        c_.resize (n, v);
    }

    void clear()
    {
        c_.resize(0);
    }

    inline unsigned degree() const { return order(); }

    //IMPL: FragmentConcept
    typedef Coord output_type;
    inline bool isZero(double eps=EPSILON) const {
        for(unsigned i = 0; i <= order(); i++) {
            if( ! are_near(c_[i], 0., eps) ) return false;
        }
        return true;
    }
    inline bool isConstant(double eps=EPSILON) const {
        for(unsigned i = 1; i <= order(); i++) {
            if( ! are_near(c_[i], c_[0], eps) ) return false;
        }
        return true;
    }
    inline bool isFinite() const {
        for(unsigned i = 0; i <= order(); i++) {
            if(!IS_FINITE(c_[i])) return false;
        }
        return true;
    }
    inline Coord at0() const { return c_[0]; }
    inline Coord at1() const { return c_[order()]; }

    inline Coord valueAt(double t) const {
        int n = order();
        double u, bc, tn, tmp;
        int i;
        u = 1.0 - t;
        bc = 1;
        tn = 1;
        tmp = c_[0]*u;
        for(i=1; i<n; i++){
            tn = tn*t;
            bc = bc*(n-i+1)/i;
            tmp = (tmp + tn*bc*c_[i])*u;
        }
        return (tmp + tn*t*c_[n]);
    }
    inline Coord operator()(double t) const { return valueAt(t); }

    SBasis toSBasis() const;

    //Only mutator
    inline Coord &operator[](unsigned ix) { return c_[ix]; }
    inline Coord const &operator[](unsigned ix) const { return const_cast<std::valarray<Coord>&>(c_)[ix]; }
    //inline Coord const &operator[](unsigned ix) const { return c_[ix]; }
    inline void setPoint(unsigned ix, double val) { c_[ix] = val; }

    /**
    *  The size of the returned vector equals n_derivs+1.
    */
    std::vector<Coord> valueAndDerivatives(Coord t, unsigned n_derivs) const {
        /* This is inelegant, as it uses several extra stores.  I think there might be a way to
         * evaluate roughly in situ. */

         // initialize return vector with zeroes, such that we only need to replace the non-zero derivs
        std::vector<Coord> val_n_der(n_derivs + 1, Coord(0.0));

        // initialize temp storage variables
        std::valarray<Coord> d_(order()+1);
        for(unsigned i = 0; i < size(); i++) {
            d_[i] = c_[i];
        }

        unsigned nn = n_derivs + 1;
        if(n_derivs > order()) {
            nn = order()+1; // only calculate the non zero derivs
        }
        for(unsigned di = 0; di < nn; di++) {
            //val_n_der[di] = (subdivideArr(t, &d_[0], NULL, NULL, order() - di));
            val_n_der[di] = bernsteinValueAt(t, &d_[0], order() - di);
            for(unsigned i = 0; i < order() - di; i++) {
                d_[i] = (order()-di)*(d_[i+1] - d_[i]);
            }
        }

        return val_n_der;
    }

    std::pair<Bezier, Bezier > subdivide(Coord t) const {
        Bezier a(Bezier::Order(*this)), b(Bezier::Order(*this));
        subdivideArr(t, &const_cast<std::valarray<Coord>&>(c_)[0], &a.c_[0], &b.c_[0], order());
        return std::pair<Bezier, Bezier >(a, b);
    }

    std::vector<double> roots() const {
        std::vector<double> solutions;
        find_bezier_roots(solutions, 0, 1);
        return solutions;
    }
    std::vector<double> roots(Interval const ivl) const {
        std::vector<double> solutions;
        find_bernstein_roots(&const_cast<std::valarray<Coord>&>(c_)[0], order(), solutions, 0, ivl.min(), ivl.max());
        return solutions;
    }

    Bezier forward_difference(unsigned k) {
        Bezier fd(Order(order()-k));
        unsigned n = fd.size();
        
        for(unsigned i = 0; i < n; i++) {
            fd[i] = 0;
            for(unsigned j = i; j < n; j++) {
                fd[i] += (((j)&1)?-c_[j]:c_[j])*choose<double>(n, j-i);
            }
        }
        return fd;
    }
  
    Bezier elevate_degree() const {
        Bezier ed(Order(order()+1));
        unsigned n = size();
        ed[0] = c_[0];
        ed[n] = c_[n-1];
        for(unsigned i = 1; i < n; i++) {
            ed[i] = (i*c_[i-1] + (n - i)*c_[i])/(n);
        }
        return ed;
    }

    Bezier reduce_degree() const {
        if(order() == 0) return *this;
        Bezier ed(Order(order()-1));
        unsigned n = size();
        ed[0] = c_[0];
        ed[n-1] = c_[n]; // ensure exact endpoints
        unsigned middle = n/2;
        for(unsigned i = 1; i < middle; i++) {
            ed[i] = (n*c_[i] - i*ed[i-1])/(n-i);
        }
        for(unsigned i = n-1; i >= middle; i--) {
            ed[i] = (n*c_[i] - i*ed[n-i])/(i);
        }
        return ed;
    }

    Bezier elevate_to_degree(unsigned newDegree) const {
        Bezier ed = *this;
        for(unsigned i = degree(); i < newDegree; i++) {
            ed = ed.elevate_degree();
        }
        return ed;
    }

    Bezier deflate() {
        if(order() == 0) return *this;
        unsigned n = order();
        Bezier b(Order(n-1));
        for(unsigned i = 0; i < n; i++) {
            b[i] = (n*c_[i+1])/(i+1);
        }
        return b;
    }
};


void bezier_to_sbasis (SBasis & sb, Bezier const& bz);

inline
Bezier multiply(Bezier const& f, Bezier const& g) {
    unsigned m = f.order();
    unsigned n = g.order();
    Bezier h(Bezier::Order(m+n));
    // h_k = sum_(i+j=k) (m i)f_i (n j)g_j / (m+n k)
    
    for(unsigned i = 0; i <= m; i++) {
        const double fi = choose<double>(m,i)*f[i];
        for(unsigned j = 0; j <= n; j++) {
            h[i+j] += fi * choose<double>(n,j)*g[j];
        }
    }
    for(unsigned k = 0; k <= m+n; k++) {
        h[k] /= choose<double>(m+n, k);
    }
    return h;
}

inline
SBasis Bezier::toSBasis() const {
    SBasis sb;
    bezier_to_sbasis(sb, (*this));
    return sb;
    //return bezier_to_sbasis(&c_[0], order());
}

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

inline Bezier& operator+=(Bezier & a, double v) {
    for(unsigned i = 0; i <= a.order(); ++i)
        a[i] = a[i] + v;
    return a;
}

inline Bezier& operator-=(Bezier & a, double v) {
    for(unsigned i = 0; i <= a.order(); ++i)
        a[i] = a[i] - v;
    return a;
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
    std::valarray<Coord> res(a.order() + 1);
    if(from == 0) {
        if(to == 1) { return Bezier(a); }
        subdivideArr(to, &const_cast<Bezier&>(a).c_[0], &res[0], NULL, a.order());
        return Bezier(&res[0], a.order());
    }
    subdivideArr(from, &const_cast<Bezier&>(a).c_[0], NULL, &res[0], a.order());
    if(to == 1) return Bezier(&res[0], a.order());
    std::valarray<Coord> res2(a.order()+1);
    subdivideArr((to - from)/(1 - from), &res[0], &res2[0], NULL, a.order());
    return Bezier(&res2[0], a.order());
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
    //if(a.order() == 1) return Bezier(0.0);
    if(a.order() == 1) return Bezier(a.c_[1]-a.c_[0]);
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

inline OptInterval bounds_fast(Bezier const & b) {
    OptInterval ret = Interval::from_array(&const_cast<Bezier&>(b).c_[0], b.size());
    return ret;
}

//TODO: better bounds exact
inline OptInterval bounds_exact(Bezier const & b) {
    return bounds_exact(b.toSBasis());
}

inline OptInterval bounds_local(Bezier const & b, OptInterval i) {
    //return bounds_local(b.toSBasis(), i);
    if (i) {
        return bounds_fast(portion(b, i->min(), i->max()));
    } else {
        return OptInterval();
    }
}

inline std::ostream &operator<< (std::ostream &out_file, const Bezier & b) {
    out_file << "Bezier(";
    for(unsigned i = 0; i < b.size(); i++) {
        out_file << b[i] << ", ";
    }
    return out_file << ")";
}

}
#endif // LIB2GEOM_SEEN_BEZIER_H

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

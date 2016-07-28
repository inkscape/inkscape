/**
 * \file
 * \brief Polynomial in canonical (monomial) basis
 *//*
 * Authors:
 *    MenTaLguY <mental@rydia.net>
 *    Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2007-2015 Authors
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

#ifndef LIB2GEOM_SEEN_POLY_H
#define LIB2GEOM_SEEN_POLY_H
#include <assert.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <complex>
#include <2geom/coord.h>
#include <2geom/utils.h>

namespace Geom {

/** @brief Polynomial in canonical (monomial) basis.
 * @ingroup Fragments */
class Poly : public std::vector<double>{
public:
    // coeff; // sum x^i*coeff[i]

    //unsigned size() const { return coeff.size();}
    unsigned degree() const { return size()-1;}

    //double operator[](const int i) const { return (*this)[i];}
    //double& operator[](const int i) { return (*this)[i];}

    Poly operator+(const Poly& p) const {
        Poly result;
        const unsigned out_size = std::max(size(), p.size());
        const unsigned min_size = std::min(size(), p.size());
        //result.reserve(out_size);

        for(unsigned i = 0; i < min_size; i++) {
            result.push_back((*this)[i] + p[i]);
        }
        for(unsigned i = min_size; i < size(); i++)
            result.push_back((*this)[i]);
        for(unsigned i = min_size; i < p.size(); i++)
            result.push_back(p[i]);
        assert(result.size() == out_size);
        return result;
    }
    Poly operator-(const Poly& p) const {
        Poly result;
        const unsigned out_size = std::max(size(), p.size());
        const unsigned min_size = std::min(size(), p.size());
        result.reserve(out_size);

        for(unsigned i = 0; i < min_size; i++) {
            result.push_back((*this)[i] - p[i]);
        }
        for(unsigned i = min_size; i < size(); i++)
            result.push_back((*this)[i]);
        for(unsigned i = min_size; i < p.size(); i++)
            result.push_back(-p[i]);
        assert(result.size() == out_size);
        return result;
    }
    Poly operator-=(const Poly& p) {
        const unsigned out_size = std::max(size(), p.size());
        const unsigned min_size = std::min(size(), p.size());
        resize(out_size);

        for(unsigned i = 0; i < min_size; i++) {
            (*this)[i] -= p[i];
        }
        for(unsigned i = min_size; i < out_size; i++)
            (*this)[i] = -p[i];
        return *this;
    }
    Poly operator-(const double k) const {
        Poly result;
        const unsigned out_size = size();
        result.reserve(out_size);

        for(unsigned i = 0; i < out_size; i++) {
            result.push_back((*this)[i]);
        }
        result[0] -= k;
        return result;
    }
    Poly operator-() const {
        Poly result;
        result.resize(size());

        for(unsigned i = 0; i < size(); i++) {
            result[i] = -(*this)[i];
        }
        return result;
    }
    Poly operator*(const double p) const {
        Poly result;
        const unsigned out_size = size();
        result.reserve(out_size);

        for(unsigned i = 0; i < out_size; i++) {
            result.push_back((*this)[i]*p);
        }
        assert(result.size() == out_size);
        return result;
    }
    // equivalent to multiply by x^terms, negative terms are disallowed
    Poly shifted(unsigned const terms) const {
        Poly result;
        size_type const out_size = size() + terms;
        result.reserve(out_size);

        result.resize(terms, 0.0);
        result.insert(result.end(), this->begin(), this->end());

        assert(result.size() == out_size);
        return result;
    }
    Poly operator*(const Poly& p) const;

    template <typename T>
    T eval(T x) const {
        T r = 0;
        for(int k = size()-1; k >= 0; k--) {
            r = r*x + T((*this)[k]);
        }
        return r;
    }

    template <typename T>
    T operator()(T t) const { return (T)eval(t);}

    void normalize();

    void monicify();
    Poly() {}
    Poly(const Poly& p) : std::vector<double>(p) {}
    Poly(const double a) {push_back(a);}

public:
    template <class T, class U>
    void val_and_deriv(T x, U &pd) const {
        pd[0] = back();
        int nc = size() - 1;
        int nd = pd.size() - 1;
        for(unsigned j = 1; j < pd.size(); j++)
            pd[j] = 0.0;
        for(int i = nc -1; i >= 0; i--) {
            int nnd = std::min(nd, nc-i);
            for(int j = nnd; j >= 1; j--)
                pd[j] = pd[j]*x + operator[](i);
            pd[0] = pd[0]*x + operator[](i);
        }
        double cnst = 1;
        for(int i = 2; i <= nd; i++) {
            cnst *= i;
            pd[i] *= cnst;
        }
    }

    static Poly linear(double ax, double b) {
        Poly p;
        p.push_back(b);
        p.push_back(ax);
        return p;
    }
};

inline Poly operator*(double a, Poly const & b) { return b * a;}

Poly integral(Poly const & p);
Poly derivative(Poly const & p);
Poly divide_out_root(Poly const & p, double x);
Poly compose(Poly const & a, Poly const & b);
Poly divide(Poly const &a, Poly const &b, Poly &r);
Poly gcd(Poly const &a, Poly const &b, const double tol=1e-10);

/*** solve(Poly p)
 * find all p.degree() roots of p.
 * This function can take a long time with suitably crafted polynomials, but in practice it should be fast.  Should we provide special forms for degree() <= 4?
 */
std::vector<std::complex<double> > solve(const Poly & p);

#ifdef HAVE_GSL
/*** solve_reals(Poly p)
 * find all real solutions to Poly p.
 * currently we just use solve and pick out the suitably real looking values, there may be a better algorithm.
 */
std::vector<double> solve_reals(const Poly & p);
#endif
double polish_root(Poly const & p, double guess, double tol);


/** @brief Analytically solve quadratic equation.
 * The equation is given in the standard form: ax^2 + bx + c = 0.
 * Only real roots are returned. */
std::vector<Coord> solve_quadratic(Coord a, Coord b, Coord c);

/** @brief Analytically solve cubic equation.
 * The equation is given in the standard form: ax^3 + bx^2 + cx + d = 0.
 * Only real roots are returned. */
std::vector<Coord> solve_cubic(Coord a, Coord b, Coord c, Coord d);


inline std::ostream &operator<< (std::ostream &out_file, const Poly &in_poly) {
    if(in_poly.size() == 0)
        out_file << "0";
    else {
        for(int i = (int)in_poly.size()-1; i >= 0; --i) {
            if(i == 1) {
                out_file << "" << in_poly[i] << "*x";
                out_file << " + ";
            } else if(i) {
                out_file << "" << in_poly[i] << "*x^" << i;
                out_file << " + ";
            } else
                out_file << in_poly[i];

        }
    }
    return out_file;
}

} // namespace Geom

#endif //LIB2GEOM_SEEN_POLY_H

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

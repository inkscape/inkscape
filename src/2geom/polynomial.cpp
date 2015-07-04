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

#include <algorithm>
#include <2geom/polynomial.h>
#include <2geom/math-utils.h>
#include <math.h>

#ifdef HAVE_GSL
#include <gsl/gsl_poly.h>
#endif

namespace Geom {

Poly Poly::operator*(const Poly& p) const {
    Poly result; 
    result.resize(degree() +  p.degree()+1);
    
    for(unsigned i = 0; i < size(); i++) {
        for(unsigned j = 0; j < p.size(); j++) {
            result[i+j] += (*this)[i] * p[j];
        }
    }
    return result;
}

/*double Poly::eval(double x) const {
    return gsl_poly_eval(&coeff[0], size(), x);
    }*/

void Poly::normalize() {
    while(back() == 0)
        pop_back();
}

void Poly::monicify() {
    normalize();
    
    double scale = 1./back(); // unitize
    
    for(unsigned i = 0; i < size(); i++) {
        (*this)[i] *= scale;
    }
}


#ifdef HAVE_GSL
std::vector<std::complex<double> > solve(Poly const & pp) {
    Poly p(pp);
    p.normalize();
    gsl_poly_complex_workspace * w 
        = gsl_poly_complex_workspace_alloc (p.size());
       
    gsl_complex_packed_ptr z = new double[p.degree()*2];
    double* a = new double[p.size()];
    for(unsigned int i = 0; i < p.size(); i++)
        a[i] = p[i];
    std::vector<std::complex<double> > roots;
    //roots.resize(p.degree());
    
    gsl_poly_complex_solve (a, p.size(), w, z);
    delete[]a;
     
    gsl_poly_complex_workspace_free (w);
     
    for (unsigned int i = 0; i < p.degree(); i++) {
        roots.push_back(std::complex<double> (z[2*i] ,z[2*i+1]));
        //printf ("z%d = %+.18f %+.18f\n", i, z[2*i], z[2*i+1]);
    }    
    delete[] z;
    return roots;
}

std::vector<double > solve_reals(Poly const & p) {
    std::vector<std::complex<double> > roots = solve(p);
    std::vector<double> real_roots;
    
    for(unsigned int i = 0; i < roots.size(); i++) {
        if(roots[i].imag() == 0) // should be more lenient perhaps
            real_roots.push_back(roots[i].real());
    }
    return real_roots;
}
#endif

double polish_root(Poly const & p, double guess, double tol) {
    Poly dp = derivative(p);
    
    double fn = p(guess);
    while(fabs(fn) > tol) {
        guess -= fn/dp(guess);
        fn = p(guess);
    }
    return guess;
}

Poly integral(Poly const & p) {
    Poly result;
    
    result.reserve(p.size()+1);
    result.push_back(0); // arbitrary const
    for(unsigned i = 0; i < p.size(); i++) {
        result.push_back(p[i]/(i+1));
    }
    return result;

}

Poly derivative(Poly const & p) {
    Poly result;
    
    if(p.size() <= 1)
        return Poly(0);
    result.reserve(p.size()-1);
    for(unsigned i = 1; i < p.size(); i++) {
        result.push_back(i*p[i]);
    }
    return result;
}

Poly compose(Poly const & a, Poly const & b) {
    Poly result;
    
    for(unsigned i = a.size(); i > 0; i--) {
        result = Poly(a[i-1]) + result * b;
    }
    return result;
    
}

/* This version is backwards - dividing taylor terms
Poly divide(Poly const &a, Poly const &b, Poly &r) {
    Poly c;
    r = a; // remainder
    
    const unsigned k = a.size();
    r.resize(k, 0);
    c.resize(k, 0);

    for(unsigned i = 0; i < k; i++) {
        double ci = r[i]/b[0];
        c[i] += ci;
        Poly bb = ci*b;
        std::cout << ci <<"*" << b << ", r= " << r << std::endl;
        r -= bb.shifted(i);
    }
    
    return c;
}
*/

Poly divide(Poly const &a, Poly const &b, Poly &r) {
    Poly c;
    r = a; // remainder
    assert(b.size() > 0);
    
    const unsigned k = a.degree();
    const unsigned l = b.degree();
    c.resize(k, 0.);
    
    for(unsigned i = k; i >= l; i--) {
        //assert(i >= 0);
        double ci = r.back()/b.back();
        c[i-l] += ci;
        Poly bb = ci*b;
        //std::cout << ci <<"*(" << b.shifted(i-l) << ") = " 
        //          << bb.shifted(i-l) << "     r= " << r << std::endl;
        r -= bb.shifted(i-l);
        r.pop_back();
    }
    //std::cout << "r= " << r << std::endl;
    r.normalize();
    c.normalize();
    
    return c;
}

Poly gcd(Poly const &a, Poly const &b, const double /*tol*/) {
    if(a.size() < b.size())
        return gcd(b, a);
    if(b.size() <= 0)
        return a;
    if(b.size() == 1)
        return a;
    Poly r;
    divide(a, b, r);
    return gcd(b, r);
}




std::vector<Coord> solve_quadratic(Coord a, Coord b, Coord c)
{
    std::vector<Coord> result;

    if (a == 0) {
        // linear equation
        if (b == 0) return result;
        result.push_back(-c/b);
        return result;
    }

    Coord delta = b*b - 4*a*c;

    if (delta == 0) {
        // one root
        result.push_back(-b / (2*a));
    } else if (delta > 0) {
        // two roots
        Coord delta_sqrt = sqrt(delta);

        // Use different formulas depending on sign of b to preserve
        // numerical stability. See e.g.:
        // http://people.csail.mit.edu/bkph/articles/Quadratics.pdf
        Coord t = -0.5 * (b + sgn(b) * delta_sqrt);
        result.push_back(t / a);
        result.push_back(c / t);
    }
    // no roots otherwise

    std::sort(result.begin(), result.end());
    return result;
}


std::vector<Coord> solve_cubic(Coord a, Coord b, Coord c, Coord d)
{
    // based on:
    // http://mathworld.wolfram.com/CubicFormula.html

    if (a == 0) {
        return solve_quadratic(b, c, d);
    }
    if (d == 0) {
        // divide by x
        std::vector<Coord> result = solve_quadratic(a, b, c);
        result.push_back(0);
        std::sort(result.begin(), result.end());
        return result;
    }

    std::vector<Coord> result;

    // 1. divide everything by a to bring to canonical form
    b /= a;
    c /= a;
    d /= a;

    // 2. eliminate x^2 term: x^3 + 3Qx - 2R = 0
    Coord Q = (3*c - b*b) / 9;
    Coord R = (-27 * d + b * (9*c - 2*b*b)) / 54;

    // 3. compute polynomial discriminant
    Coord D = Q*Q*Q + R*R;
    Coord term1 = b/3;

    if (D > 0) {
        // only one real root
        Coord S = cbrt(R + sqrt(D));
        Coord T = cbrt(R - sqrt(D));
        result.push_back(-b/3 + S + T);
    } else if (D == 0) {
        // 3 real roots, 2 of which are equal
        Coord rroot = cbrt(R);
        result.reserve(3);
        result.push_back(-term1 + 2*rroot);
        result.push_back(-term1 - rroot);
        result.push_back(-term1 - rroot);
    } else {
        // 3 distinct real roots
        assert(Q < 0);
        Coord theta = acos(R / sqrt(-Q*Q*Q));
        Coord rroot = 2 * sqrt(-Q);
        result.reserve(3);
        result.push_back(-term1 + rroot * cos(theta / 3));
        result.push_back(-term1 + rroot * cos((theta + 2*M_PI) / 3));
        result.push_back(-term1 + rroot * cos((theta + 4*M_PI) / 3));
    }

    std::sort(result.begin(), result.end());
    return result;
}


/*Poly divide_out_root(Poly const & p, double x) {
    assert(1);
    }*/

} //namespace Geom

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

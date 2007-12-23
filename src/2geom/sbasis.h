/*
 *  sbasis.h - S-power basis function class
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

#ifndef SEEN_SBASIS_H
#define SEEN_SBASIS_H
#include <vector>
#include <cassert>
#include <iostream>

#include "linear.h"
#include "interval.h"
#include "utils.h"
#include "exception.h"

namespace Geom {

/*** An empty SBasis is identically 0. */
class SBasis : public std::vector<Linear>{
public:
    SBasis() {}
    explicit SBasis(double a) {
        push_back(Linear(a,a));
    }
    SBasis(SBasis const & a) :
        std::vector<Linear>(a)
    {}
    SBasis(Linear const & bo) {
        push_back(bo);
    }

    //IMPL: FragmentConcept
    typedef double output_type;
    inline bool isZero() const {
        if(empty()) return true;
        for(unsigned i = 0; i < size(); i++) {
            if(!(*this)[i].isZero()) return false;
        }
        return true;
    }
    inline bool isConstant() const {
        if (empty()) return true;
        for (unsigned i = 0; i < size(); i++) {
            if(!(*this)[i].isConstant()) return false;
        }
        return true;
    }

    bool isFinite() const;
    inline double at0() const { 
        if(empty()) return 0; else return (*this)[0][0];
    }
    inline double at1() const{
        if(empty()) return 0; else return (*this)[0][1];
    }

    double valueAt(double t) const {
        double s = t*(1-t);
        double p0 = 0, p1 = 0;
        double sk = 1;
//TODO: rewrite as horner
        for(unsigned k = 0; k < size(); k++) {
            p0 += sk*(*this)[k][0];
            p1 += sk*(*this)[k][1];
            sk *= s;
        }
        return (1-t)*p0 + t*p1;
    }
    double valueAndDerivative(double t, double &der) const {
        double s = t*(1-t);
        double p0 = 0, p1 = 0;
        double sk = 1;
//TODO: rewrite as horner
        for(unsigned k = 0; k < size(); k++) {
            p0 += sk*(*this)[k][0];
            p1 += sk*(*this)[k][1];
            sk *= s;
        }
        // p0 and p1 at this point form a linear approximation at t
        der = p1 - p0;
        return (1-t)*p0 + t*p1;
    }
    double operator()(double t) const {
        return valueAt(t);
    }

    std::vector<double> valueAndDerivatives(double /*t*/, unsigned /*n*/) const {
        //TODO
        throwNotImplemented();
    }

    SBasis toSBasis() const { return SBasis(*this); }

    double tailError(unsigned tail) const;

// compute f(g)
    SBasis operator()(SBasis const & g) const;

    Linear operator[](unsigned i) const {
        assert(i < size());
        return std::vector<Linear>::operator[](i);
    }

//MUTATOR PRISON
    Linear& operator[](unsigned i) { return this->at(i); }

    //remove extra zeros
    void normalize() {
        while(!empty() && 0 == back()[0] && 0 == back()[1])
            pop_back();
    }
    void truncate(unsigned k) { if(k < size()) resize(k); }
};

//TODO: figure out how to stick this in linear, while not adding an sbasis dep
inline SBasis Linear::toSBasis() const { return SBasis(*this); }

//implemented in sbasis-roots.cpp
Interval bounds_exact(SBasis const &a);
Interval bounds_fast(SBasis const &a, int order = 0);
Interval bounds_local(SBasis const &a, const Interval &t, int order = 0);

inline SBasis reverse(SBasis const &a) {
    SBasis result;
    result.reserve(a.size());
    for(unsigned k = 0; k < a.size(); k++)
       result.push_back(reverse(a[k]));
    return result;
}

//IMPL: ScalableConcept
inline SBasis operator-(const SBasis& p) {
    if(p.isZero()) return SBasis();
    SBasis result;
    result.reserve(p.size());
        
    for(unsigned i = 0; i < p.size(); i++) {
        result.push_back(-p[i]);
    }
    return result;
}
SBasis operator*(SBasis const &a, double k);
inline SBasis operator*(double k, SBasis const &a) { return a*k; }
inline SBasis operator/(SBasis const &a, double k) { return a*(1./k); }
SBasis& operator*=(SBasis& a, double b);
inline SBasis& operator/=(SBasis& a, double b) { return (a*=(1./b)); }

//IMPL: AddableConcept
SBasis operator+(const SBasis& a, const SBasis& b);
SBasis operator-(const SBasis& a, const SBasis& b);
SBasis& operator+=(SBasis& a, const SBasis& b);
SBasis& operator-=(SBasis& a, const SBasis& b);

//TODO: remove?
inline SBasis operator+(const SBasis & a, Linear const & b) {
    if(b.isZero()) return a;
    if(a.isZero()) return b;
    SBasis result(a);
    result[0] += b;
    return result;
}
inline SBasis operator-(const SBasis & a, Linear const & b) {
    if(b.isZero()) return a;
    SBasis result(a);
    result[0] -= b;
    return result;
}
inline SBasis& operator+=(SBasis& a, const Linear& b) {
    if(a.isZero())
        a.push_back(b);
    else
        a[0] += b;
    return a;
}
inline SBasis& operator-=(SBasis& a, const Linear& b) {
    if(a.isZero())
        a.push_back(-b);
    else
        a[0] -= b;
    return a;
}

//IMPL: OffsetableConcept
inline SBasis operator+(const SBasis & a, double b) {
    if(a.isZero()) return Linear(b, b);
    SBasis result(a);
    result[0] += b;
    return result;
}
inline SBasis operator-(const SBasis & a, double b) {
    if(a.isZero()) return Linear(-b, -b);
    SBasis result(a);
    result[0] -= b;
    return result;
}
inline SBasis& operator+=(SBasis& a, double b) {
    if(a.isZero())
        a.push_back(Linear(b,b));
    else
        a[0] += b;
    return a;
}
inline SBasis& operator-=(SBasis& a, double b) {
    if(a.isZero())
        a.push_back(Linear(-b,-b));
    else
        a[0] -= b;
    return a;
}

SBasis shift(SBasis const &a, int sh);
SBasis shift(Linear const &a, int sh);

inline SBasis truncate(SBasis const &a, unsigned terms) {
    SBasis c;
    c.insert(c.begin(), a.begin(), a.begin() + std::min(terms, (unsigned)a.size()));
    return c;
}

SBasis multiply(SBasis const &a, SBasis const &b);

SBasis integral(SBasis const &c);
SBasis derivative(SBasis const &a);

SBasis sqrt(SBasis const &a, int k);

// return a kth order approx to 1/a)
SBasis reciprocal(Linear const &a, int k);
SBasis divide(SBasis const &a, SBasis const &b, int k);

inline SBasis operator*(SBasis const & a, SBasis const & b) {
    return multiply(a, b);
}

inline SBasis& operator*=(SBasis& a, SBasis const & b) {
    a = multiply(a, b);
    return a;
}

//valuation: degree of the first non zero coefficient.
inline unsigned 
valuation(SBasis const &a, double tol=0){
    unsigned val=0;
    while( val<a.size() &&
           fabs(a[val][0])<tol &&
           fabs(a[val][1])<tol ) 
        val++;
    return val;
}

// a(b(t))
SBasis compose(SBasis const &a, SBasis const &b);
SBasis compose(SBasis const &a, SBasis const &b, unsigned k);
SBasis inverse(SBasis a, int k);
//compose_inverse(f,g)=compose(f,inverse(g)), but is numerically more stable in some good cases...
//TODO: requires g(0)=0 & g(1)=1 atm. generalization should be obvious.
SBasis compose_inverse(SBasis const &f, SBasis const &g, unsigned order=2, double tol=1e-3);

inline SBasis portion(const SBasis &t, double from, double to) { return compose(t, Linear(from, to)); }

// compute f(g)
inline SBasis
SBasis::operator()(SBasis const & g) const {
    return compose(*this, g);
}
 
inline std::ostream &operator<< (std::ostream &out_file, const Linear &bo) {
    out_file << "{" << bo[0] << ", " << bo[1] << "}";
    return out_file;
}

inline std::ostream &operator<< (std::ostream &out_file, const SBasis & p) {
    for(unsigned i = 0; i < p.size(); i++) {
        out_file << p[i] << "s^" << i << " + ";
    }
    return out_file;
}

SBasis sin(Linear bo, int k);
SBasis cos(Linear bo, int k);

std::vector<double> roots(SBasis const & s);
std::vector<std::vector<double> > multi_roots(SBasis const &f,
                                 std::vector<double> const &levels,
                                 double htol=1e-7,
                                 double vtol=1e-7,
                                 double a=0,
                                 double b=1);
    
}

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
#endif

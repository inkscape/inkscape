/** @file
 * @brief Polynomial in symmetric power basis (S-basis)
 *//*
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

#ifndef LIB2GEOM_SEEN_SBASIS_H
#define LIB2GEOM_SEEN_SBASIS_H
#include <vector>
#include <cassert>
#include <iostream>

#include <2geom/linear.h>
#include <2geom/interval.h>
#include <2geom/utils.h>
#include <2geom/exception.h>

//#define USE_SBASISN 1


#if defined(USE_SBASIS_OF)

#include "sbasis-of.h"

#elif defined(USE_SBASISN)

#include "sbasisN.h"
namespace Geom{

/*** An empty SBasis is identically 0. */
class SBasis : public SBasisN<1>;

};
#else

namespace Geom {

/**
 * @brief Polynomial in symmetric power basis
 * @ingroup Fragments
 */
class SBasis {
    std::vector<Linear> d;
    void push_back(Linear const&l) { d.push_back(l); }

public:
    // As part of our migration away from SBasis isa vector we provide this minimal set of vector interface methods.
    size_t size() const {return d.size();}
    typedef std::vector<Linear>::iterator iterator;
    typedef std::vector<Linear>::const_iterator const_iterator;
    Linear operator[](unsigned i) const {
        return d[i];
    }
    Linear& operator[](unsigned i) { return d.at(i); }
    const_iterator begin() const { return d.begin();}
    const_iterator end() const { return d.end();}
    iterator begin() { return d.begin();}
    iterator end() { return d.end();}
    bool empty() const { return d.size() == 1 && d[0][0] == 0 && d[0][1] == 0; }
    Linear &back() {return d.back();}
    Linear const &back() const {return d.back();}
    void pop_back() {
		if (d.size() > 1) {
			d.pop_back();
		} else {
			d[0][0] = 0;
			d[0][1] = 0;
		}
	}
    void resize(unsigned n) { d.resize(std::max<unsigned>(n, 1));}
    void resize(unsigned n, Linear const& l) { d.resize(std::max<unsigned>(n, 1), l);}
    void reserve(unsigned n) { d.reserve(n);}
    void clear() {
    	d.resize(1);
    	d[0][0] = 0;
    	d[0][1] = 0;
    }
    void insert(iterator before, const_iterator src_begin, const_iterator src_end) { d.insert(before, src_begin, src_end);}
    Linear& at(unsigned i) { return d.at(i);}
    //void insert(Linear* before, int& n, Linear const &l) { d.insert(std::vector<Linear>::iterator(before), n, l);}
    bool operator==(SBasis const&B) const { return d == B.d;}
    bool operator!=(SBasis const&B) const { return d != B.d;}
    
    SBasis()
		: d(1, Linear(0, 0))
	{}
    explicit SBasis(double a)
        : d(1, Linear(a, a))
    {}
    explicit SBasis(double a, double b)
        : d(1, Linear(a, b))
    {}
    SBasis(SBasis const &a)
        : d(a.d)
    {}
    SBasis(std::vector<Linear> const &ls)
        : d(ls)
    {}
    SBasis(Linear const &bo)
		: d(1, bo)
	{}
    SBasis(Linear* bo)
		: d(1, bo ? *bo : Linear(0, 0))
    {}
    explicit SBasis(size_t n, Linear const&l) : d(n, l) {}

    SBasis(Coord c0, Coord c1, Coord c2, Coord c3)
        : d(2)
    {
        d[0][0] = c0;
        d[1][0] = c1;
        d[1][1] = c2;
        d[0][1] = c3;
    }
    SBasis(Coord c0, Coord c1, Coord c2, Coord c3, Coord c4, Coord c5)
        : d(3)
    {
        d[0][0] = c0;
        d[1][0] = c1;
        d[2][0] = c2;
        d[2][1] = c3;
        d[1][1] = c4;
        d[0][1] = c5;
    }
    SBasis(Coord c0, Coord c1, Coord c2, Coord c3, Coord c4, Coord c5,
           Coord c6, Coord c7)
        : d(4)
    {
        d[0][0] = c0;
        d[1][0] = c1;
        d[2][0] = c2;
        d[3][0] = c3;
        d[3][1] = c4;
        d[2][1] = c5;
        d[1][1] = c6;
        d[0][1] = c7;
    }
    SBasis(Coord c0, Coord c1, Coord c2, Coord c3, Coord c4, Coord c5,
           Coord c6, Coord c7, Coord c8, Coord c9)
        : d(5)
    {
        d[0][0] = c0;
        d[1][0] = c1;
        d[2][0] = c2;
        d[3][0] = c3;
        d[4][0] = c4;
        d[4][1] = c5;
        d[3][1] = c6;
        d[2][1] = c7;
        d[1][1] = c8;
        d[0][1] = c9;
    }

    // construct from a sequence of coefficients
    template <typename Iter>
    SBasis(Iter first, Iter last) {
        assert(std::distance(first, last) % 2 == 0);
        assert(std::distance(first, last) >= 2);
        for (; first != last; ++first) {
            --last;
            push_back(Linear(*first, *last));
        }
    }

    //IMPL: FragmentConcept
    typedef double output_type;
    inline bool isZero(double eps=EPSILON) const {
    	assert(size() > 0);
        for(unsigned i = 0; i < size(); i++) {
            if(!(*this)[i].isZero(eps)) return false;
        }
        return true;
    }
    inline bool isConstant(double eps=EPSILON) const {
    	assert(size() > 0);
        if(!(*this)[0].isConstant(eps)) return false;
        for (unsigned i = 1; i < size(); i++) {
            if(!(*this)[i].isZero(eps)) return false;
        }
        return true;
    }

    bool isFinite() const;
    inline Coord at0() const { return (*this)[0][0]; }
    inline Coord &at0() { return (*this)[0][0]; }
    inline Coord at1() const { return (*this)[0][1]; }
    inline Coord &at1() { return (*this)[0][1]; }
    
    int degreesOfFreedom() const { return size()*2;}

    double valueAt(double t) const {
    	assert(size() > 0);
        double s = t*(1-t);
        double p0 = 0, p1 = 0;
        for(unsigned k = size(); k > 0; k--) {
            const Linear &lin = (*this)[k-1];
            p0 = p0*s + lin[0];
            p1 = p1*s + lin[1];
        }
        return (1-t)*p0 + t*p1;
    }
    //double valueAndDerivative(double t, double &der) const {
    //}
    double operator()(double t) const {
        return valueAt(t);
    }

    std::vector<double> valueAndDerivatives(double t, unsigned n) const;

    SBasis toSBasis() const { return SBasis(*this); }

    double tailError(unsigned tail) const;

// compute f(g)
    SBasis operator()(SBasis const & g) const;

//MUTATOR PRISON
    //remove extra zeros
    void normalize() {
        while(size() > 1 && back().isZero(0))
            pop_back();
    }

    void truncate(unsigned k) { if(k < size()) resize(std::max<size_t>(k, 1)); }
private:
    void derive(); // in place version
};

//TODO: figure out how to stick this in linear, while not adding an sbasis dep
inline SBasis Linear::toSBasis() const { return SBasis(*this); }

//implemented in sbasis-roots.cpp
OptInterval bounds_exact(SBasis const &a);
OptInterval bounds_fast(SBasis const &a, int order = 0);
OptInterval bounds_local(SBasis const &a, const OptInterval &t, int order = 0);

/** Returns a function which reverses the domain of a.
 \param a sbasis function
 \relates SBasis

useful for reversing a parameteric curve.
*/
inline SBasis reverse(SBasis const &a) {
    SBasis result(a.size(), Linear());
    
    for(unsigned k = 0; k < a.size(); k++)
        result[k] = reverse(a[k]);
    return result;
}

//IMPL: ScalableConcept
inline SBasis operator-(const SBasis& p) {
    if(p.isZero()) return SBasis();
    SBasis result(p.size(), Linear());
        
    for(unsigned i = 0; i < p.size(); i++) {
        result[i] = -p[i];
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
/*inline SBasis operator+(const SBasis & a, Linear const & b) {
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
    }*/

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
        a = SBasis(Linear(b,b));
    else
        a[0] += b;
    return a;
}
inline SBasis& operator-=(SBasis& a, double b) {
    if(a.isZero())
        a = SBasis(Linear(-b,-b));
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
// This performs a multiply and accumulate operation in about the same time as multiply.  return a*b + c
SBasis multiply_add(SBasis const &a, SBasis const &b, SBasis c);

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

/** Returns the degree of the first non zero coefficient.
 \param a sbasis function
 \param tol largest abs val considered 0
 \return first non zero coefficient
 \relates SBasis
*/
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

/** Returns the sbasis on domain [0,1] that was t on [from, to]
 \param t sbasis function
 \param from,to interval
 \return sbasis
 \relates SBasis
*/
SBasis portion(const SBasis &t, double from, double to);
inline SBasis portion(const SBasis &t, Interval const &ivl) { return portion(t, ivl.min(), ivl.max()); }

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
        if (i != 0) {
            out_file << " + ";
        }
        out_file << p[i] << "s^" << i;
    }
    return out_file;
}

// These are deprecated, use sbasis-math.h versions if possible
SBasis sin(Linear bo, int k);
SBasis cos(Linear bo, int k);

std::vector<double> roots(SBasis const & s);
std::vector<double> roots(SBasis const & s, Interval const inside);
std::vector<std::vector<double> > multi_roots(SBasis const &f,
                                 std::vector<double> const &levels,
                                 double htol=1e-7,
                                 double vtol=1e-7,
                                 double a=0,
                                 double b=1);

//--------- Levelset like functions -----------------------------------------------------

/** Solve f(t) = v +/- tolerance. The collection of intervals where
 *     v - vtol <= f(t) <= v+vtol
 *   is returned (with a precision tol on the boundaries).
    \param f sbasis function
    \param level the value of v.
    \param vtol: error tolerance on v.
    \param a, b limit search on domain [a,b]
    \param tol: tolerance on the result bounds.
    \returns a vector of intervals.
*/
std::vector<Interval> level_set (SBasis const &f,
		double level,
		double vtol = 1e-5,
		double a=0.,
		double b=1.,
		double tol = 1e-5);

/** Solve f(t)\in I=[u,v], which defines a collection of intervals (J_k). More precisely,
 *  a collection (J'_k) is returned with J'_k = J_k up to a given tolerance.
    \param f sbasis function
    \param level: the given interval of deisred values for f.
    \param a, b limit search on domain [a,b]
    \param tol: tolerance on the bounds of the result.
    \returns a vector of intervals.
*/
std::vector<Interval> level_set (SBasis const &f,
		Interval const &level,
		double a=0.,
		double b=1.,
		double tol = 1e-5);

/** 'Solve' f(t) = v +/- tolerance for several values of v at once.
    \param f sbasis function
    \param levels vector of values, that should be sorted.
    \param vtol: error tolerance on v.
    \param a, b limit search on domain [a,b]
    \param tol: the bounds of the returned intervals are exact up to that tolerance.
    \returns a vector of vectors of intervals.
*/
std::vector<std::vector<Interval> > level_sets (SBasis const &f,
		std::vector<double> const &levels,
		double a=0.,
		double b=1.,
		double vtol = 1e-5,
		double tol = 1e-5);

/** 'Solve' f(t)\in I=[u,v] for several intervals I at once.
    \param f sbasis function
    \param levels vector of 'y' intervals, that should be disjoints and sorted.
    \param a, b limit search on domain [a,b]
    \param tol: the bounds of the returned intervals are exact up to that tolerance.
    \returns a vector of vectors of intervals.
*/
std::vector<std::vector<Interval> > level_sets (SBasis const &f,
		std::vector<Interval> const &levels,
		double a=0.,
		double b=1.,
		double tol = 1e-5);

}
#endif

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
#endif

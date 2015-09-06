/*
 *  sbasis.cpp - S-power basis function class + supporting classes
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

#include <cmath>

#include <2geom/sbasis.h>
#include <2geom/math-utils.h>

namespace Geom{

/** bound the error from term truncation
 \param tail first term to chop
 \returns the largest possible error this truncation could give
*/
double SBasis::tailError(unsigned tail) const {
  Interval bs = *bounds_fast(*this, tail);
  return std::max(fabs(bs.min()),fabs(bs.max()));
}

/** test all coefficients are finite
*/
bool SBasis::isFinite() const {
    for(unsigned i = 0; i < size(); i++) {
        if(!(*this)[i].isFinite())
            return false;
    }
    return true;
}

/** Compute the value and the first n derivatives
 \param t position to evaluate
 \param n number of derivatives (not counting value)
 \returns a vector with the value and the n derivative evaluations

There is an elegant way to compute the value and n derivatives for a polynomial using a variant of horner's rule.  Someone will someday work out how for sbasis.
*/
std::vector<double> SBasis::valueAndDerivatives(double t, unsigned n) const {
    std::vector<double> ret(n+1);
    ret[0] = valueAt(t);
    SBasis tmp = *this;
    for(unsigned i = 1; i < n+1; i++) {
        tmp.derive();
        ret[i] = tmp.valueAt(t);
    }
    return ret;
}


/** Compute the pointwise sum of a and b (Exact)
 \param a,b sbasis functions
 \returns sbasis equal to a+b

*/
SBasis operator+(const SBasis& a, const SBasis& b) {
    const unsigned out_size = std::max(a.size(), b.size());
    const unsigned min_size = std::min(a.size(), b.size());
    SBasis result(out_size, Linear());

    for(unsigned i = 0; i < min_size; i++) {
        result[i] = a[i] + b[i];
    }
    for(unsigned i = min_size; i < a.size(); i++)
        result[i] = a[i];
    for(unsigned i = min_size; i < b.size(); i++)
        result[i] = b[i];

    assert(result.size() == out_size);
    return result;
}

/** Compute the pointwise difference of a and b (Exact)
 \param a,b sbasis functions
 \returns sbasis equal to a-b

*/
SBasis operator-(const SBasis& a, const SBasis& b) {
    const unsigned out_size = std::max(a.size(), b.size());
    const unsigned min_size = std::min(a.size(), b.size());
    SBasis result(out_size, Linear());

    for(unsigned i = 0; i < min_size; i++) {
        result[i] = a[i] - b[i];
    }
    for(unsigned i = min_size; i < a.size(); i++)
        result[i] = a[i];
    for(unsigned i = min_size; i < b.size(); i++)
        result[i] = -b[i];

    assert(result.size() == out_size);
    return result;
}

/** Compute the pointwise sum of a and b and store in a (Exact)
 \param a,b sbasis functions
 \returns sbasis equal to a+b

*/
SBasis& operator+=(SBasis& a, const SBasis& b) {
    const unsigned out_size = std::max(a.size(), b.size());
    const unsigned min_size = std::min(a.size(), b.size());
    a.resize(out_size);

    for(unsigned i = 0; i < min_size; i++)
        a[i] += b[i];
    for(unsigned i = min_size; i < b.size(); i++)
        a[i] = b[i];

    assert(a.size() == out_size);
    return a;
}

/** Compute the pointwise difference of a and b and store in a (Exact)
 \param a,b sbasis functions
 \returns sbasis equal to a-b

*/
SBasis& operator-=(SBasis& a, const SBasis& b) {
    const unsigned out_size = std::max(a.size(), b.size());
    const unsigned min_size = std::min(a.size(), b.size());
    a.resize(out_size);

    for(unsigned i = 0; i < min_size; i++)
        a[i] -= b[i];
    for(unsigned i = min_size; i < b.size(); i++)
        a[i] = -b[i];

    assert(a.size() == out_size);
    return a;
}

/** Compute the pointwise product of a and b (Exact)
 \param a,b sbasis functions
 \returns sbasis equal to a*b

*/
SBasis operator*(SBasis const &a, double k) {
    SBasis c(a.size(), Linear());
    for(unsigned i = 0; i < a.size(); i++)
        c[i] = a[i] * k;
    return c;
}

/** Compute the pointwise product of a and b and store the value in a (Exact)
 \param a,b sbasis functions
 \returns sbasis equal to a*b

*/
SBasis& operator*=(SBasis& a, double b) {
    if (a.isZero()) return a;
    if (b == 0)
        a.clear();
    else
        for(unsigned i = 0; i < a.size(); i++)
            a[i] *= b;
    return a;
}

/** multiply a by x^sh in place (Exact)
 \param a sbasis function
 \param sh power
 \returns a

*/
SBasis shift(SBasis const &a, int sh) {
    size_t n = a.size()+sh;
    SBasis c(n, Linear());
    size_t m = std::max(0, sh);
    
    for(int i = 0; i < sh; i++)
        c[i] = Linear(0,0);
    for(size_t i = m, j = std::max(0,-sh); i < n; i++, j++)
        c[i] = a[j];
    return c;
}

/** multiply a by x^sh  (Exact)
 \param a linear function
 \param sh power
 \returns a* x^sh 

*/
SBasis shift(Linear const &a, int sh) {
    size_t n = 1+sh;
    SBasis c(n, Linear());
    
    for(int i = 0; i < sh; i++)
        c[i] = Linear(0,0);
    if(sh >= 0)
        c[sh] = a;
    return c;
}

#if 0
SBasis multiply(SBasis const &a, SBasis const &b) {
    // c = {a0*b0 - shift(1, a.Tri*b.Tri), a1*b1 - shift(1, a.Tri*b.Tri)}

    // shift(1, a.Tri*b.Tri)
    SBasis c(a.size() + b.size(), Linear(0,0));
    if(a.isZero() || b.isZero())
        return c;
    for(unsigned j = 0; j < b.size(); j++) {
        for(unsigned i = j; i < a.size()+j; i++) {
            double tri = b[j].tri()*a[i-j].tri();
            c[i+1/*shift*/] += Linear(-tri);
        }
    }
    for(unsigned j = 0; j < b.size(); j++) {
        for(unsigned i = j; i < a.size()+j; i++) {
            for(unsigned dim = 0; dim < 2; dim++)
                c[i][dim] += b[j][dim]*a[i-j][dim];
        }
    }
    c.normalize();
    //assert(!(0 == c.back()[0] && 0 == c.back()[1]));
    return c;
}
#else

/** Compute the pointwise product of a and b adding c (Exact)
 \param a,b,c sbasis functions
 \returns sbasis equal to a*b+c

The added term is almost free
*/
SBasis multiply_add(SBasis const &a, SBasis const &b, SBasis c) {
    if(a.isZero() || b.isZero())
        return c;
    c.resize(a.size() + b.size(), Linear(0,0));
    for(unsigned j = 0; j < b.size(); j++) {
        for(unsigned i = j; i < a.size()+j; i++) {
            double tri = b[j].tri()*a[i-j].tri();
            c[i+1/*shift*/] += Linear(-tri);
        }
    }
    for(unsigned j = 0; j < b.size(); j++) {
        for(unsigned i = j; i < a.size()+j; i++) {
            for(unsigned dim = 0; dim < 2; dim++)
                c[i][dim] += b[j][dim]*a[i-j][dim];
        }
    }
    c.normalize();
    //assert(!(0 == c.back()[0] && 0 == c.back()[1]));
    return c;
}

/** Compute the pointwise product of a and b (Exact)
 \param a,b sbasis functions
 \returns sbasis equal to a*b

*/
SBasis multiply(SBasis const &a, SBasis const &b) {
    if(a.isZero() || b.isZero()) {
        SBasis c(1, Linear(0,0));
        return c;
    }
    SBasis c(a.size() + b.size(), Linear(0,0));
    return multiply_add(a, b, c);
}
#endif 
/** Compute the integral of a (Exact)
 \param a sbasis functions
 \returns sbasis integral(a)

*/
SBasis integral(SBasis const &c) {
    SBasis a;
    a.resize(c.size() + 1, Linear(0,0));
    a[0] = Linear(0,0);

    for(unsigned k = 1; k < c.size() + 1; k++) {
        double ahat = -c[k-1].tri()/(2*k);
        a[k][0] = a[k][1] = ahat;
    }
    double aTri = 0;
    for(int k = c.size()-1; k >= 0; k--) {
        aTri = (c[k].hat() + (k+1)*aTri/2)/(2*k+1);
        a[k][0] -= aTri/2;
        a[k][1] += aTri/2;
    }
    a.normalize();
    return a;
}

/** Compute the derivative of a (Exact)
 \param a sbasis functions
 \returns sbasis da/dt

*/
SBasis derivative(SBasis const &a) {
    SBasis c;
    c.resize(a.size(), Linear(0,0));
    if(a.isZero())
        return c;

    for(unsigned k = 0; k < a.size()-1; k++) {
        double d = (2*k+1)*(a[k][1] - a[k][0]);
        
        c[k][0] = d + (k+1)*a[k+1][0];
        c[k][1] = d - (k+1)*a[k+1][1];
    }
    int k = a.size()-1;
    double d = (2*k+1)*(a[k][1] - a[k][0]);
    if (d == 0 && k > 0) {
        c.pop_back();
    } else {
        c[k][0] = d;
        c[k][1] = d;
    }

    return c;
}

/** Compute the derivative of this inplace (Exact)

*/
void SBasis::derive() { // in place version
    if(isZero()) return;
    for(unsigned k = 0; k < size()-1; k++) {
        double d = (2*k+1)*((*this)[k][1] - (*this)[k][0]);
        
        (*this)[k][0] = d + (k+1)*(*this)[k+1][0];
        (*this)[k][1] = d - (k+1)*(*this)[k+1][1];
    }
    int k = size()-1;
    double d = (2*k+1)*((*this)[k][1] - (*this)[k][0]);
    if (d == 0 && k > 0) {
        pop_back();
    } else {
        (*this)[k][0] = d;
        (*this)[k][1] = d;
    }
}

/** Compute the sqrt of a
 \param a sbasis functions
 \returns sbasis \f[ \sqrt{a} \f]

It is recommended to use the piecewise version unless you have good reason.
TODO: convert int k to unsigned k, and remove cast
*/
SBasis sqrt(SBasis const &a, int k) {
    SBasis c;
    if(a.isZero() || k == 0)
        return c;
    c.resize(k, Linear(0,0));
    c[0] = Linear(std::sqrt(a[0][0]), std::sqrt(a[0][1]));
    SBasis r = a - multiply(c, c); // remainder

    for(unsigned i = 1; i <= (unsigned)k && i<r.size(); i++) {
        Linear ci(r[i][0]/(2*c[0][0]), r[i][1]/(2*c[0][1]));
        SBasis cisi = shift(ci, i);
        r -= multiply(shift((c*2 + cisi), i), SBasis(ci));
        r.truncate(k+1);
        c += cisi;
        if(r.tailError(i) == 0) // if exact
            break;
    }

    return c;
}

/** Compute the recpirocal of a
 \param a sbasis functions
 \returns sbasis 1/a

It is recommended to use the piecewise version unless you have good reason.
*/
SBasis reciprocal(Linear const &a, int k) {
    SBasis c;
    assert(!a.isZero());
    c.resize(k, Linear(0,0));
    double r_s0 = (a.tri()*a.tri())/(-a[0]*a[1]);
    double r_s0k = 1;
    for(unsigned i = 0; i < (unsigned)k; i++) {
        c[i] = Linear(r_s0k/a[0], r_s0k/a[1]);
        r_s0k *= r_s0;
    }
    return c;
}

/** Compute  a / b to k terms
 \param a,b sbasis functions
 \returns sbasis a/b

It is recommended to use the piecewise version unless you have good reason.
*/
SBasis divide(SBasis const &a, SBasis const &b, int k) {
    SBasis c;
    assert(!a.isZero());
    SBasis r = a; // remainder

    k++;
    r.resize(k, Linear(0,0));
    c.resize(k, Linear(0,0));

    for(unsigned i = 0; i < (unsigned)k; i++) {
        Linear ci(r[i][0]/b[0][0], r[i][1]/b[0][1]); //H0
        c[i] += ci;
        r -= shift(multiply(ci,b), i);
        r.truncate(k+1);
        if(r.tailError(i) == 0) // if exact
            break;
    }

    return c;
}

/** Compute  a composed with b
 \param a,b sbasis functions
 \returns sbasis a(b(t))

 return a0 + s(a1 + s(a2 +...  where s = (1-u)u; ak =(1 - u)a^0_k + ua^1_k
*/
SBasis compose(SBasis const &a, SBasis const &b) {
    SBasis s = multiply((SBasis(Linear(1,1))-b), b);
    SBasis r;

    for(int i = a.size()-1; i >= 0; i--) {
        r = multiply_add(r, s, SBasis(Linear(a[i][0])) - b*a[i][0] + b*a[i][1]);
    }
    return r;
}

/** Compute  a composed with b to k terms
 \param a,b sbasis functions
 \returns sbasis a(b(t))

 return a0 + s(a1 + s(a2 +...  where s = (1-u)u; ak =(1 - u)a^0_k + ua^1_k
*/
SBasis compose(SBasis const &a, SBasis const &b, unsigned k) {
    SBasis s = multiply((SBasis(Linear(1,1))-b), b);
    SBasis r;

    for(int i = a.size()-1; i >= 0; i--) {
        r = multiply_add(r, s, SBasis(Linear(a[i][0])) - b*a[i][0] + b*a[i][1]);
    }
    r.truncate(k);
    return r;
}

SBasis portion(const SBasis &t, double from, double to) {
    double fv = t.valueAt(from);
    double tv = t.valueAt(to);
    SBasis ret = compose(t, Linear(from, to));
    ret.at0() = fv;
    ret.at1() = tv;
    return ret;
}

/*
Inversion algorithm. The notation is certainly very misleading. The
pseudocode should say:

c(v) := 0
r(u) := r_0(u) := u
for i:=0 to k do
  c_i(v) := H_0(r_i(u)/(t_1)^i; u)
  c(v) := c(v) + c_i(v)*t^i
  r(u) := r(u) ? c_i(u)*(t(u))^i
endfor
*/

//#define DEBUG_INVERSION 1

/** find the function a^-1 such that a^-1 composed with a to k terms is the identity function
 \param a sbasis function
 \returns sbasis a^-1 s.t. a^-1(a(t)) = 1

 The function must have 'unit range'("a00 = 0 and a01 = 1") and be monotonic.
*/
SBasis inverse(SBasis a, int k) {
    assert(a.size() > 0);
    double a0 = a[0][0];
    if(a0 != 0) {
        a -= a0;
    }
    double a1 = a[0][1];
    assert(a1 != 0); // not invertable.

    if(a1 != 1) {
        a /= a1;
    }
    SBasis c(k, Linear());                           // c(v) := 0
    if(a.size() >= 2 && k == 2) {
        c[0] = Linear(0,1);
        Linear t1(1+a[1][0], 1-a[1][1]);    // t_1
        c[1] = Linear(-a[1][0]/t1[0], -a[1][1]/t1[1]);
    } else if(a.size() >= 2) {                      // non linear
        SBasis r = Linear(0,1);             // r(u) := r_0(u) := u
        Linear t1(1./(1+a[1][0]), 1./(1-a[1][1]));    // 1./t_1
        Linear one(1,1);
        Linear t1i = one;                   // t_1^0
        SBasis one_minus_a = SBasis(one) - a;
        SBasis t = multiply(one_minus_a, a); // t(u)
        SBasis ti(one);                     // t(u)^0
#ifdef DEBUG_INVERSION
        std::cout << "a=" << a << std::endl;
        std::cout << "1-a=" << one_minus_a << std::endl;
        std::cout << "t1=" << t1 << std::endl;
        //assert(t1 == t[1]);
#endif

        //c.resize(k+1, Linear(0,0));
        for(unsigned i = 0; i < (unsigned)k; i++) {   // for i:=0 to k do
#ifdef DEBUG_INVERSION
            std::cout << "-------" << i << ": ---------" <<std::endl;
            std::cout << "r=" << r << std::endl
                      << "c=" << c << std::endl
                      << "ti=" << ti << std::endl
                      << std::endl;
#endif
            if(r.size() <= i)                // ensure enough space in the remainder, probably not needed
                r.resize(i+1, Linear(0,0));
            Linear ci(r[i][0]*t1i[0], r[i][1]*t1i[1]); // c_i(v) := H_0(r_i(u)/(t_1)^i; u)
#ifdef DEBUG_INVERSION
            std::cout << "t1i=" << t1i << std::endl;
            std::cout << "ci=" << ci << std::endl;
#endif
            for(int dim = 0; dim < 2; dim++) // t1^-i *= 1./t1
                t1i[dim] *= t1[dim];
            c[i] = ci; // c(v) := c(v) + c_i(v)*t^i
            // change from v to u parameterisation
            SBasis civ = one_minus_a*ci[0] + a*ci[1];
            // r(u) := r(u) - c_i(u)*(t(u))^i
            // We can truncate this to the number of final terms, as no following terms can
            // contribute to the result.
            r -= multiply(civ,ti);
            r.truncate(k);
            if(r.tailError(i) == 0)
                break; // yay!
            ti = multiply(ti,t);
        }
#ifdef DEBUG_INVERSION
        std::cout << "##########################" << std::endl;
#endif
    } else
        c = Linear(0,1); // linear
    c -= a0; // invert the offset
    c /= a1; // invert the slope
    return c;
}

/** Compute the sine of a to k terms
 \param b linear function
 \returns sbasis sin(a)

It is recommended to use the piecewise version unless you have good reason.
*/
SBasis sin(Linear b, int k) {
    SBasis s(k+2, Linear());
    s[0] = Linear(std::sin(b[0]), std::sin(b[1]));
    double tr = s[0].tri();
    double t2 = b.tri();
    s[1] = Linear(std::cos(b[0])*t2 - tr, -std::cos(b[1])*t2 + tr);

    t2 *= t2;
    for(int i = 0; i < k; i++) {
        Linear bo(4*(i+1)*s[i+1][0] - 2*s[i+1][1],
                  -2*s[i+1][0] + 4*(i+1)*s[i+1][1]);
        bo -= s[i]*(t2/(i+1));


        s[i+2] = bo/double(i+2);
    }

    return s;
}

/** Compute the cosine of a
 \param b linear function
 \returns sbasis cos(a)

It is recommended to use the piecewise version unless you have good reason.
*/
SBasis cos(Linear bo, int k) {
    return sin(Linear(bo[0] + M_PI/2,
                      bo[1] + M_PI/2),
               k);
}

/** compute fog^-1.
 \param f,g sbasis functions
 \returns sbasis f(g^-1(t)).

("zero" = double comparison threshold. *!*we might divide by "zero"*!*)
TODO: compute order according to tol?
TODO: requires g(0)=0 & g(1)=1 atm... adaptation to other cases should be obvious!
*/
SBasis compose_inverse(SBasis const &f, SBasis const &g, unsigned order, double zero){
    SBasis result(order, Linear(0.)); //result
    SBasis r=f; //remainder
    SBasis Pk=Linear(1)-g,Qk=g,sg=Pk*Qk;
    Pk.truncate(order);
    Qk.truncate(order);
    Pk.resize(order,Linear(0.));
    Qk.resize(order,Linear(0.));
    r.resize(order,Linear(0.));

    int vs = valuation(sg,zero);
    if (vs == 0) { // to prevent infinite loop
        return result;
    }

    for (unsigned k=0; k<order; k+=vs){
        double p10 = Pk.at(k)[0];// we have to solve the linear system:
        double p01 = Pk.at(k)[1];//
        double q10 = Qk.at(k)[0];//   p10*a + q10*b = r10
        double q01 = Qk.at(k)[1];// &
        double r10 =  r.at(k)[0];//   p01*a + q01*b = r01
        double r01 =  r.at(k)[1];//
        double a,b;
        double det = p10*q01-p01*q10;

        //TODO: handle det~0!!
        if (fabs(det)<zero){
            a=b=0;
        }else{
            a=( q01*r10-q10*r01)/det;
            b=(-p01*r10+p10*r01)/det;
        }
        result[k] = Linear(a,b);
        r=r-Pk*a-Qk*b;

        Pk=Pk*sg;
        Qk=Qk*sg;

        Pk.resize(order,Linear(0.)); // truncates if too high order, expands with zeros if too low
        Qk.resize(order,Linear(0.));
        r.resize(order,Linear(0.));

    }
    result.normalize();
    return result;
}

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

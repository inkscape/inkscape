/**
 * \file
 * \brief Obsolete 2D SBasis function class
 *//*
 * Authors:
 *      Nathan Hurst <?@?.?>
 *      JFBarraud <?@?.?>
 * 
 * Copyright 2006-2008  authors
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

#ifndef LIB2GEOM_SEEN_SBASIS_2D_H
#define LIB2GEOM_SEEN_SBASIS_2D_H
#include <vector>
#include <cassert>
#include <algorithm>
#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <iostream>

namespace Geom{

class Linear2d{
public:
    /*  
        u 0,1
        v 0,2
    */
    double a[4];
    Linear2d() {
        a[0] = 0; 
        a[1] = 0;
        a[2] = 0; 
        a[3] = 0; 
    }
    Linear2d(double aa) {
        for(unsigned i = 0 ; i < 4; i ++) 
            a[i] = aa;
    }
    Linear2d(double a00, double a01, double a10, double a11) 
    {
        a[0] = a00; 
        a[1] = a01;
        a[2] = a10; 
        a[3] = a11; 
    }

    double operator[](const int i) const {
        assert(i >= 0);
        assert(i < 4);
        return a[i];
    }
    double& operator[](const int i) {
        assert(i >= 0);
        assert(i < 4);
        return a[i];
    }
    double apply(double u, double v) {
        return (a[0]*(1-u)*(1-v) +
                a[1]*u*(1-v) +
                a[2]*(1-u)*v +
                a[3]*u*v);
    }
};

inline Linear extract_u(Linear2d const &a, double u) {
    return Linear(a[0]*(1-u) +
                  a[1]*u,
                  a[2]*(1-u) +
                  a[3]*u);
}
inline Linear extract_v(Linear2d const &a, double v) {
    return Linear(a[0]*(1-v) +
                  a[2]*v,
                  a[1]*(1-v) +
                  a[3]*v);
}
inline Linear2d operator-(Linear2d const &a) {
    return Linear2d(-a.a[0], -a.a[1],
                    -a.a[2], -a.a[3]);
}
inline Linear2d operator+(Linear2d const & a, Linear2d const & b) {
    return Linear2d(a[0] + b[0], 
                  a[1] + b[1],
                  a[2] + b[2], 
                  a[3] + b[3]);
}
inline Linear2d operator-(Linear2d const & a, Linear2d const & b) {
    return Linear2d(a[0] - b[0],
                  a[1] - b[1],
                  a[2] - b[2],
                  a[3] - b[3]);
}
inline Linear2d& operator+=(Linear2d & a, Linear2d const & b) {
    for(unsigned i = 0; i < 4; i++)
        a[i] += b[i];
    return a;
}
inline Linear2d& operator-=(Linear2d & a, Linear2d const & b) {
    for(unsigned i = 0; i < 4; i++)
        a[i] -= b[i];
    return a;
}
inline Linear2d& operator*=(Linear2d & a, double b) {
    for(unsigned i = 0; i < 4; i++)
        a[i] *= b;
    return a;
}

inline bool operator==(Linear2d const & a, Linear2d const & b) {
    for(unsigned i = 0; i < 4; i++)
        if(a[i] != b[i])
            return false;
    return true;
}
inline bool operator!=(Linear2d const & a, Linear2d const & b) {
    for(unsigned i = 0; i < 4; i++)
        if(a[i] == b[i])
            return false;
    return true;
}
inline Linear2d operator*(double const a, Linear2d const & b) {
    return Linear2d(a*b[0], a*b[1],
                  a*b[2], a*b[3]);
}

class SBasis2d : public std::vector<Linear2d>{
public:
    // vector in u,v
    unsigned us, vs; // number of u terms, v terms
    SBasis2d() {}
    SBasis2d(Linear2d const & bo) 
        : us(1), vs(1) {
        push_back(bo);
    }
    SBasis2d(SBasis2d const & a) 
        : std::vector<Linear2d>(a), us(a.us), vs(a.vs) {}
    
    Linear2d& index(unsigned ui, unsigned vi) {
        assert(ui < us);
        assert(vi < vs);
        return (*this)[ui + vi*us];        
    }
    
    Linear2d index(unsigned ui, unsigned vi) const {
        if(ui >= us) 
            return Linear2d(0);
        if(vi >= vs)
            return Linear2d(0);
        return (*this)[ui + vi*us];        
    }
    
    double apply(double u, double v) const {
        double s = u*(1-u);
        double t = v*(1-v);
        Linear2d p;
        double tk = 1;
// XXX rewrite as horner
        for(unsigned vi = 0; vi < vs; vi++) {
            double sk = 1;
            for(unsigned ui = 0; ui < us; ui++) {
                p += (sk*tk)*index(ui, vi);
                sk *= s;
            }
            tk *= t;
        }
        return p.apply(u,v);
    }

    void clear() {
        fill(begin(), end(), Linear2d(0));
    }
    
    void normalize(); // remove extra zeros

    double tail_error(unsigned tail) const;
    
    void truncate(unsigned k);
};

inline SBasis2d operator-(const SBasis2d& p) {
    SBasis2d result;
    result.reserve(p.size());
        
    for(unsigned i = 0; i < p.size(); i++) {
        result.push_back(-p[i]);
    }
    return result;
}

inline SBasis2d operator+(const SBasis2d& a, const SBasis2d& b) {
    SBasis2d result;
    result.us = std::max(a.us, b.us);
    result.vs = std::max(a.vs, b.vs);
    const unsigned out_size = result.us*result.vs;
    result.resize(out_size);
        
    for(unsigned vi = 0; vi < result.vs; vi++) {
        for(unsigned ui = 0; ui < result.us; ui++) {
            Linear2d bo;
            if(ui < a.us && vi < a.vs)
                bo += a.index(ui, vi);
            if(ui < b.us && vi < b.vs)
                bo += b.index(ui, vi);
            result.index(ui, vi) = bo;
        }
    }
    return result;
}

inline SBasis2d operator-(const SBasis2d& a, const SBasis2d& b) {
    SBasis2d result;
    result.us = std::max(a.us, b.us);
    result.vs = std::max(a.vs, b.vs);
    const unsigned out_size = result.us*result.vs;
    result.resize(out_size);
        
    for(unsigned vi = 0; vi < result.vs; vi++) {
        for(unsigned ui = 0; ui < result.us; ui++) {
            Linear2d bo;
            if(ui < a.us && vi < a.vs)
                bo += a.index(ui, vi);
            if(ui < b.us && vi < b.vs)
                bo -= b.index(ui, vi);
            result.index(ui, vi) = bo;
        }
    }
    return result;
}


inline SBasis2d& operator+=(SBasis2d& a, const Linear2d& b) {
    if(a.size() < 1)
        a.push_back(b);
    else
        a[0] += b;
    return a;
}

inline SBasis2d& operator-=(SBasis2d& a, const Linear2d& b) {
    if(a.size() < 1)
        a.push_back(-b);
    else
        a[0] -= b;
    return a;
}

inline SBasis2d& operator+=(SBasis2d& a, double b) {
    if(a.size() < 1)
        a.push_back(Linear2d(b));
    else {
        for(unsigned i = 0; i < 4; i++)
            a[0] += double(b);
    }
    return a;
}

inline SBasis2d& operator-=(SBasis2d& a, double b) {
    if(a.size() < 1)
        a.push_back(Linear2d(-b));
    else {
        a[0] -= b;
    }
    return a;
}

inline SBasis2d& operator*=(SBasis2d& a, double b) {
    for(unsigned i = 0; i < a.size(); i++)
        a[i] *= b;
    return a;
}

inline SBasis2d& operator/=(SBasis2d& a, double b) {
    for(unsigned i = 0; i < a.size(); i++)
        a[i] *= (1./b);
    return a;
}

SBasis2d operator*(double k, SBasis2d const &a);
SBasis2d operator*(SBasis2d const &a, SBasis2d const &b);

SBasis2d shift(SBasis2d const &a, int sh);

SBasis2d shift(Linear2d const &a, int sh);

SBasis2d truncate(SBasis2d const &a, unsigned terms);

SBasis2d multiply(SBasis2d const &a, SBasis2d const &b);

SBasis2d integral(SBasis2d const &c);

SBasis2d partial_derivative(SBasis2d const &a, int dim);

SBasis2d sqrt(SBasis2d const &a, int k);

// return a kth order approx to 1/a)
SBasis2d reciprocal(Linear2d const &a, int k);

SBasis2d divide(SBasis2d const &a, SBasis2d const &b, int k);

// a(b(t))
SBasis2d compose(SBasis2d const &a, SBasis2d const &b);
SBasis2d compose(SBasis2d const &a, SBasis2d const &b, unsigned k);
SBasis2d inverse(SBasis2d const &a, int k);

// these two should probably be replaced with compose
SBasis extract_u(SBasis2d const &a, double u);
SBasis extract_v(SBasis2d const &a, double v);

SBasis compose(Linear2d const &a, D2<SBasis> const &p);

SBasis compose(SBasis2d const &fg, D2<SBasis> const &p);

D2<SBasis> compose_each(D2<SBasis2d> const &fg, D2<SBasis> const &p);

inline std::ostream &operator<< (std::ostream &out_file, const Linear2d &bo) {
    out_file << "{" << bo[0] << ", " << bo[1] << "}, ";
    out_file << "{" << bo[2] << ", " << bo[3] << "}";
    return out_file;
}

inline std::ostream &operator<< (std::ostream &out_file, const SBasis2d & p) {
    for(unsigned i = 0; i < p.size(); i++) {
        out_file << p[i] << "s^" << i << " + ";
    }
    return out_file;
}

D2<SBasis>
sb2dsolve(SBasis2d const &f, Geom::Point const &A, Geom::Point const &B, unsigned degmax=2);

D2<SBasis>
sb2d_cubic_solve(SBasis2d const &f, Geom::Point const &A, Geom::Point const &B);

} // end namespace Geom

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

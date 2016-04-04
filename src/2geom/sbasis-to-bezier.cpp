/*
 * Symmetric Power Basis - Bernstein Basis conversion routines
 *
 * Authors:
 *      Marco Cecchetti <mrcekets at gmail.com>
 *      Nathan Hurst <njh@mail.csse.monash.edu.au>
 *
 * Copyright 2007-2008  authors
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


#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>
#include <2geom/choose.h>
#include <2geom/path-sink.h>
#include <2geom/exception.h>
#include <2geom/convex-hull.h>

#include <iostream>




namespace Geom
{

/*
 *  Symmetric Power Basis - Bernstein Basis conversion routines
 *
 *  some remark about precision:
 *  interval [0,1], subdivisions: 10^3
 *  - bezier_to_sbasis : up to degree ~72 precision is at least 10^-5
 *                       up to degree ~87 precision is at least 10^-3
 *  - sbasis_to_bezier : up to order ~63 precision is at least 10^-15
 *                       precision is at least 10^-14 even beyond order 200
 *
 *  interval [-1,1], subdivisions: 10^3
 *  - bezier_to_sbasis : up to degree ~21 precision is at least 10^-5
 *                       up to degree ~24 precision is at least 10^-3
 *  - sbasis_to_bezier : up to order ~11 precision is at least 10^-5
 *                       up to order ~13 precision is at least 10^-3
 *
 *  interval [-10,10], subdivisions: 10^3
 *  - bezier_to_sbasis : up to degree ~7 precision is at least 10^-5
 *                       up to degree ~8 precision is at least 10^-3
 *  - sbasis_to_bezier : up to order ~3 precision is at least 10^-5
 *                       up to order ~4 precision is at least 10^-3
 *
 *  references:
 *  this implementation is based on the following article:
 *  J.Sanchez-Reyes - The Symmetric Analogue of the Polynomial Power Basis
 */

inline
double binomial(unsigned int n, unsigned int k)
{
    return choose<double>(n, k);
}

inline
int sgn(unsigned int j, unsigned int k)
{
    assert (j >= k);
    // we are sure that j >= k
    return ((j-k) &  1u) ? -1 : 1;
}


/** Changes the basis of p to be bernstein.
 \param p the Symmetric basis polynomial
 \returns the Bernstein basis polynomial

 if the degree is even q is the order in the symmetrical power basis,
 if the degree is odd q is the order + 1
 n is always the polynomial degree, i. e. the Bezier order
 sz is the number of bezier handles.
*/
void sbasis_to_bezier (Bezier & bz, SBasis const& sb, size_t sz)
{
    assert(sb.size() > 0);

    size_t q, n;
    bool even;
    if (sz == 0)
    {
        q = sb.size();
        if (sb[q-1][0] == sb[q-1][1])
        {
            even = true;
            --q;
            n = 2*q;
        }
        else
        {
            even = false;
            n = 2*q-1;
        }
    }
    else
    {
        q = (sz > 2*sb.size()-1) ?  sb.size() : (sz+1)/2;
        n = sz-1;
        even = false;
    }
    bz.clear();
    bz.resize(n+1);
    double Tjk;
    for (size_t k = 0; k < q; ++k)
    {
        for (size_t j = k; j < n-k; ++j) // j <= n-k-1
        {
            Tjk = binomial(n-2*k-1, j-k);
            bz[j] += (Tjk * sb[k][0]);
            bz[n-j] += (Tjk * sb[k][1]); // n-k <-> [k][1]
        }
    }
    if (even)
    {
        bz[q] += sb[q][0];
    }
    // the resulting coefficients are with respect to the scaled Bernstein
    // basis so we need to divide them by (n, j) binomial coefficient
    for (size_t j = 1; j < n; ++j)
    {
        bz[j] /= binomial(n, j);
    }
    bz[0] = sb[0][0];
    bz[n] = sb[0][1];
}

void sbasis_to_bezier(D2<Bezier> &bz, D2<SBasis> const &sb, size_t sz)
{
    if (sz == 0) {
        sz = std::max(sb[X].size(), sb[Y].size())*2;
    }
    sbasis_to_bezier(bz[X], sb[X], sz);
    sbasis_to_bezier(bz[Y], sb[Y], sz);
}

/** Changes the basis of p to be Bernstein.
 \param p the D2 Symmetric basis polynomial
 \returns the D2 Bernstein basis polynomial

 sz is always the polynomial degree, i. e. the Bezier order
*/
void sbasis_to_bezier (std::vector<Point> & bz, D2<SBasis> const& sb, size_t sz)
{
    D2<Bezier> bez;
    sbasis_to_bezier(bez, sb, sz);
    bz = bezier_points(bez);
}

/** Changes the basis of p to be Bernstein.
 \param p the D2 Symmetric basis polynomial
 \returns the D2 Bernstein basis cubic polynomial

Bezier is always cubic.
For general asymmetric case, fit the SBasis function value at midpoint
For parallel, symmetric case, find the point of closest approach to the midpoint
For parallel, anti-symmetric case, fit the SBasis slope at midpoint
*/
void sbasis_to_cubic_bezier (std::vector<Point> & bz, D2<SBasis> const& sb)
{
    double delx[2], dely[2];
    double xprime[2], yprime[2];
    double midx = 0;
    double midy = 0;
    double numer;
    double denom;
    double div;

    if ((sb[X].size() == 0) || (sb[Y].size() == 0)) {
        THROW_RANGEERROR("size of sb is too small");
    }

    sbasis_to_bezier(bz, sb, 4);  // zeroth-order estimate
    if ((sb[X].size() < 3) && (sb[Y].size() < 3))
        return;  // cubic bezier estimate is exact
    Geom::ConvexHull bezhull(bz);

//  calculate first derivatives of x and y wrt t

    for (int i = 0; i < 2; ++i) {
        xprime[i] = sb[X][0][1] - sb[X][0][0];
        yprime[i] = sb[Y][0][1] - sb[Y][0][0];
    }
    if (sb[X].size() > 1) {
        xprime[0] += sb[X][1][0];
        xprime[1] -= sb[X][1][1];
    }
    if (sb[Y].size() > 1) {
        yprime[0] += sb[Y][1][0];
        yprime[1] -= sb[Y][1][1];
    }

//  calculate midpoint at t = 0.5

    div = 2;
    for (size_t i = 0; i < sb[X].size(); ++i) {
        midx += (sb[X][i][0] + sb[X][i][1])/div;
        div *= 4;
    }

    div = 2;
    for (size_t i = 0; i < sb[Y].size(); ++i) {
        midy += (sb[Y][i][0] + sb[Y][i][1])/div;
        div *= 4;
    }

//  is midpoint in hull: if not, the solution will be ill-conditioned, LP Bug 1428683

    if (!bezhull.contains(Geom::Point(midx, midy)))
        return;

//  calculate Bezier control arms

    midx = 8*midx - 4*bz[0][X] - 4*bz[3][X];  // re-define relative to center
    midy = 8*midy - 4*bz[0][Y] - 4*bz[3][Y];

    if ((std::abs(xprime[0]) < EPSILON) && (std::abs(yprime[0]) < EPSILON)
    && ((std::abs(xprime[1]) > EPSILON) || (std::abs(yprime[1]) > EPSILON)))  { // degenerate handle at 0 : use distance of closest approach
        numer = midx*xprime[1] + midy*yprime[1];
        denom = 3.0*(xprime[1]*xprime[1] + yprime[1]*yprime[1]);
        delx[0] = 0;
        dely[0] = 0;
        delx[1] = -xprime[1]*numer/denom;
        dely[1] = -yprime[1]*numer/denom;
    } else if ((std::abs(xprime[1]) < EPSILON) && (std::abs(yprime[1]) < EPSILON)
           && ((std::abs(xprime[0]) > EPSILON) || (std::abs(yprime[0]) > EPSILON)))  { // degenerate handle at 1 : ditto
        numer = midx*xprime[0] + midy*yprime[0];
        denom = 3.0*(xprime[0]*xprime[0] + yprime[0]*yprime[0]);
        delx[0] = xprime[0]*numer/denom;
        dely[0] = yprime[0]*numer/denom;
        delx[1] = 0;
        dely[1] = 0;
    } else if  (std::abs(xprime[1]*yprime[0] - yprime[1]*xprime[0]) >  // general case : fit mid fxn value
        0.002 * std::abs(xprime[1]*xprime[0] + yprime[1]*yprime[0])) { // approx. 0.1 degree of angle
        double test1 = (bz[1][Y] - bz[0][Y])*(bz[3][X] - bz[0][X]) - (bz[1][X] - bz[0][X])*(bz[3][Y] - bz[0][Y]);
        double test2 = (bz[2][Y] - bz[0][Y])*(bz[3][X] - bz[0][X]) - (bz[2][X] - bz[0][X])*(bz[3][Y] - bz[0][Y]);
        if (test1*test2 < 0) // reject anti-symmetric case, LP Bug 1428267 & Bug 1428683
            return;
        denom = 3.0*(xprime[1]*yprime[0] - yprime[1]*xprime[0]);
        for (int i = 0; i < 2; ++i) {
            numer = xprime[1 - i]*midy - yprime[1 - i]*midx;
            delx[i] = xprime[i]*numer/denom;
            dely[i] = yprime[i]*numer/denom;
        }
    } else if ((xprime[0]*xprime[1] < 0) || (yprime[0]*yprime[1] < 0)) { // symmetric case : use distance of closest approach
        numer = midx*xprime[0] + midy*yprime[0];
        denom = 6.0*(xprime[0]*xprime[0] + yprime[0]*yprime[0]);
        delx[0] = xprime[0]*numer/denom;
        dely[0] = yprime[0]*numer/denom;
        delx[1] = -delx[0];
        dely[1] = -dely[0];
    } else {                                    // anti-symmetric case : fit mid slope
                                                // calculate slope at t = 0.5
        midx = 0;
        div = 1;
        for (size_t i = 0; i < sb[X].size(); ++i) {
            midx += (sb[X][i][1] - sb[X][i][0])/div;
            div *= 4;
        }
        midy = 0;
        div = 1;
        for (size_t i = 0; i < sb[Y].size(); ++i) {
            midy += (sb[Y][i][1] - sb[Y][i][0])/div;
            div *= 4;
        }
        if (midx*yprime[0] != midy*xprime[0]) {
            denom = midx*yprime[0] - midy*xprime[0];
            numer = midx*(bz[3][Y] - bz[0][Y]) - midy*(bz[3][X] - bz[0][X]);
            for (int i = 0; i < 2; ++i) {
                delx[i] = xprime[0]*numer/denom;
                dely[i] = yprime[0]*numer/denom;
            }
        } else {                                // linear case
            for (int i = 0; i < 2; ++i) {
                delx[i] = (bz[3][X] - bz[0][X])/3;
                dely[i] = (bz[3][Y] - bz[0][Y])/3;
            }
        }
    }
    bz[1][X] = bz[0][X] + delx[0];
    bz[1][Y] = bz[0][Y] + dely[0];
    bz[2][X] = bz[3][X] - delx[1];
    bz[2][Y] = bz[3][Y] - dely[1];
}

/** Changes the basis of p to be sbasis.
 \param p the Bernstein basis polynomial
 \returns the Symmetric basis polynomial

 if the degree is even q is the order in the symmetrical power basis,
 if the degree is odd q is the order + 1
 n is always the polynomial degree, i. e. the Bezier order
*/
void bezier_to_sbasis (SBasis & sb, Bezier const& bz)
{
    size_t n = bz.order();
    size_t q = (n+1) / 2;
    size_t even = (n & 1u) ? 0 : 1;
    sb.clear();
    sb.resize(q + even, Linear(0, 0));
    double Tjk;
    for (size_t k = 0; k < q; ++k)
    {
        for (size_t j = k; j < q; ++j)
        {
            Tjk = sgn(j, k) * binomial(n-j-k, j-k) * binomial(n, k);
            sb[j][0] += (Tjk * bz[k]);
            sb[j][1] += (Tjk * bz[n-k]); // n-j <-> [j][1]
        }
        for (size_t j = k+1; j < q; ++j)
        {
            Tjk = sgn(j, k) * binomial(n-j-k-1, j-k-1) * binomial(n, k);
            sb[j][0] += (Tjk * bz[n-k]);
            sb[j][1] += (Tjk * bz[k]);   // n-j <-> [j][1]
        }
    }
    if (even)
    {
        for (size_t k = 0; k < q; ++k)
        {
            Tjk = sgn(q,k) * binomial(n, k);
            sb[q][0] += (Tjk * (bz[k] + bz[n-k]));
        }
        sb[q][0] += (binomial(n, q) * bz[q]);
        sb[q][1] = sb[q][0];
    }
    sb[0][0] = bz[0];
    sb[0][1] = bz[n];
}


/** Changes the basis of d2 p to be sbasis.
 \param p the d2 Bernstein basis polynomial
 \returns the d2 Symmetric basis polynomial

 if the degree is even q is the order in the symmetrical power basis,
 if the degree is odd q is the order + 1
 n is always the polynomial degree, i. e. the Bezier order
*/
void bezier_to_sbasis (D2<SBasis> & sb, std::vector<Point> const& bz)
{
    size_t n = bz.size() - 1;
    size_t q = (n+1) / 2;
    size_t even = (n & 1u) ? 0 : 1;
    sb[X].clear();
    sb[Y].clear();
    sb[X].resize(q + even, Linear(0, 0));
    sb[Y].resize(q + even, Linear(0, 0));
    double Tjk;
    for (size_t k = 0; k < q; ++k)
    {
        for (size_t j = k; j < q; ++j)
        {
            Tjk = sgn(j, k) * binomial(n-j-k, j-k) * binomial(n, k);
            sb[X][j][0] += (Tjk * bz[k][X]);
            sb[X][j][1] += (Tjk * bz[n-k][X]);
            sb[Y][j][0] += (Tjk * bz[k][Y]);
            sb[Y][j][1] += (Tjk * bz[n-k][Y]);
        }
        for (size_t j = k+1; j < q; ++j)
        {
            Tjk = sgn(j, k) * binomial(n-j-k-1, j-k-1) * binomial(n, k);
            sb[X][j][0] += (Tjk * bz[n-k][X]);
            sb[X][j][1] += (Tjk * bz[k][X]);
            sb[Y][j][0] += (Tjk * bz[n-k][Y]);
            sb[Y][j][1] += (Tjk * bz[k][Y]);
        }
    }
    if (even)
    {
        for (size_t k = 0; k < q; ++k)
        {
            Tjk = sgn(q,k) * binomial(n, k);
            sb[X][q][0] += (Tjk * (bz[k][X] + bz[n-k][X]));
            sb[Y][q][0] += (Tjk * (bz[k][Y] + bz[n-k][Y]));
        }
        sb[X][q][0] += (binomial(n, q) * bz[q][X]);
        sb[X][q][1] = sb[X][q][0];
        sb[Y][q][0] += (binomial(n, q) * bz[q][Y]);
        sb[Y][q][1] = sb[Y][q][0];
    }
    sb[X][0][0] = bz[0][X];
    sb[X][0][1] = bz[n][X];
    sb[Y][0][0] = bz[0][Y];
    sb[Y][0][1] = bz[n][Y];
}


}  // end namespace Geom


#if 0 
/*
* This version works by inverting a reasonable upper bound on the error term after subdividing the
* curve at $a$.  We keep biting off pieces until there is no more curve left.
*
* Derivation: The tail of the power series is $a_ks^k + a_{k+1}s^{k+1} + \ldots = e$.  A
* subdivision at $a$ results in a tail error of $e*A^k, A = (1-a)a$.  Let this be the desired
* tolerance tol $= e*A^k$ and invert getting $A = e^{1/k}$ and $a = 1/2 - \sqrt{1/4 - A}$
*/
void
subpath_from_sbasis_incremental(Geom::OldPathSetBuilder &pb, D2<SBasis> B, double tol, bool initial) {
    const unsigned k = 2; // cubic bezier
    double te = B.tail_error(k);
    assert(B[0].IS_FINITE());
    assert(B[1].IS_FINITE());

    //std::cout << "tol = " << tol << std::endl;
    while(1) {
        double A = std::sqrt(tol/te); // pow(te, 1./k)
        double a = A;
        if(A < 1) {
            A = std::min(A, 0.25);
            a = 0.5 - std::sqrt(0.25 - A); // quadratic formula
            if(a > 1) a = 1; // clamp to the end of the segment
        } else
            a = 1;
        assert(a > 0);
        //std::cout << "te = " << te << std::endl;
        //std::cout << "A = " << A << "; a=" << a << std::endl;
        D2<SBasis> Bs = compose(B, Linear(0, a));
        assert(Bs.tail_error(k));
        std::vector<Geom::Point> bez = sbasis_to_bezier(Bs, 2);
        reverse(bez.begin(), bez.end());
        if (initial) {
          pb.start_subpath(bez[0]);
          initial = false;
        }
        pb.push_cubic(bez[1], bez[2], bez[3]);

// move to next piece of curve
        if(a >= 1) break;
        B = compose(B, Linear(a, 1));
        te = B.tail_error(k);
    }
}

#endif

namespace Geom{

/** Make a path from a d2 sbasis.
 \param p the d2 Symmetric basis polynomial
 \returns a Path

  If only_cubicbeziers is true, the resulting path may only contain CubicBezier curves.
*/
void build_from_sbasis(Geom::PathBuilder &pb, D2<SBasis> const &B, double tol, bool only_cubicbeziers) {
    if (!B.isFinite()) {
        THROW_EXCEPTION("assertion failed: B.isFinite()");
    }
    if(tail_error(B, 3) < tol || sbasis_size(B) == 2) { // nearly cubic enough
        if( !only_cubicbeziers && (sbasis_size(B) <= 1) ) {
            pb.lineTo(B.at1());
        } else {
            std::vector<Geom::Point> bez;
//            sbasis_to_bezier(bez, B, 4);
            sbasis_to_cubic_bezier(bez, B);
            pb.curveTo(bez[1], bez[2], bez[3]);
        }
    } else {
        build_from_sbasis(pb, compose(B, Linear(0, 0.5)), tol, only_cubicbeziers);
        build_from_sbasis(pb, compose(B, Linear(0.5, 1)), tol, only_cubicbeziers);
    }
}

/** Make a path from a d2 sbasis.
 \param p the d2 Symmetric basis polynomial
 \returns a Path

  If only_cubicbeziers is true, the resulting path may only contain CubicBezier curves.
*/
Path
path_from_sbasis(D2<SBasis> const &B, double tol, bool only_cubicbeziers) {
    PathBuilder pb;
    pb.moveTo(B.at0());
    build_from_sbasis(pb, B, tol, only_cubicbeziers);
    pb.flush();
    return pb.peek().front();
}

/** Make a path from a d2 sbasis.
 \param p the d2 Symmetric basis polynomial
 \returns a Path

  If only_cubicbeziers is true, the resulting path may only contain CubicBezier curves.
 TODO: some of this logic should be lifted into svg-path
*/
PathVector
path_from_piecewise(Geom::Piecewise<Geom::D2<Geom::SBasis> > const &B, double tol, bool only_cubicbeziers) {
    Geom::PathBuilder pb;
    if(B.size() == 0) return pb.peek();
    Geom::Point start = B[0].at0();
    pb.moveTo(start);
    for(unsigned i = 0; ; i++) {
        if ( (i+1 == B.size()) 
             || !are_near(B[i+1].at0(), B[i].at1(), tol) )
        {
            //start of a new path
            if (are_near(start, B[i].at1()) && sbasis_size(B[i]) <= 1) {
                pb.closePath();
                //last line seg already there (because of .closePath())
                goto no_add;
            }
            build_from_sbasis(pb, B[i], tol, only_cubicbeziers);
            if (are_near(start, B[i].at1())) {
                //it's closed, the last closing segment was not a straight line so it needed to be added, but still make it closed here with degenerate straight line.
                pb.closePath();
            }
          no_add:
            if (i+1 >= B.size()) {
                break;
            }
            start = B[i+1].at0();
            pb.moveTo(start);
        } else {
            build_from_sbasis(pb, B[i], tol, only_cubicbeziers);
        }
    }
    pb.flush();
    return pb.peek();
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

/** @file
 * @brief Basic intersection routines
 *//*
 * Authors:
 *   Nathan Hurst <njh@njhurst.com>
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Jean-François Barraud <jf.barraud@gmail.com>
 * 
 * Copyright 2008-2009 Authors
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

#include <2geom/basic-intersection.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/exception.h>

#ifdef HAVE_GSL
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multiroots.h>
#endif

using std::vector;
namespace Geom {

//#ifdef USE_RECURSIVE_INTERSECTOR

// void find_intersections(std::vector<std::pair<double, double> > &xs,
//                         D2<SBasis> const & A,
//                         D2<SBasis> const & B) {
//     vector<Point> BezA, BezB;
//     sbasis_to_bezier(BezA, A);
//     sbasis_to_bezier(BezB, B);
    
//     xs.clear();
    
//     find_intersections_bezier_recursive(xs, BezA, BezB);
// }
// void find_intersections(std::vector< std::pair<double, double> > & xs,
//                          std::vector<Point> const& A,
//                          std::vector<Point> const& B,
//                         double precision){
//     find_intersections_bezier_recursive(xs, A, B, precision);
// }

//#else

namespace detail{ namespace bezier_clipping {
void portion(std::vector<Point> &B, Interval const &I);
void derivative(std::vector<Point> &D, std::vector<Point> const &B);
}; };

void find_intersections(std::vector<std::pair<double, double> > &xs,
                        D2<Bezier> const & A,
                        D2<Bezier> const & B,
                        double precision)
{
    find_intersections_bezier_clipping(xs, bezier_points(A), bezier_points(B), precision);
}

void find_intersections(std::vector<std::pair<double, double> > &xs,
                        D2<SBasis> const & A,
                        D2<SBasis> const & B,
                        double precision)
{
    vector<Point> BezA, BezB;
    sbasis_to_bezier(BezA, A);
    sbasis_to_bezier(BezB, B);
    
    find_intersections_bezier_clipping(xs, BezA, BezB, precision);
}

void find_intersections(std::vector< std::pair<double, double> > & xs,
                        std::vector<Point> const& A,
                        std::vector<Point> const& B,
                        double precision)
{
    find_intersections_bezier_clipping(xs, A, B, precision);
}

//#endif

/*
 * split the curve at the midpoint, returning an array with the two parts
 * Temporary storage is minimized by using part of the storage for the result
 * to hold an intermediate value until it is no longer needed.
 */
// TODO replace with Bezier method
void split(vector<Point> const &p, double t, 
           vector<Point> &left, vector<Point> &right) {
    const unsigned sz = p.size();
    //Geom::Point Vtemp[sz][sz];
    vector<vector<Point> > Vtemp(sz);
    for ( size_t i = 0; i < sz; ++i )
        Vtemp[i].reserve(sz);

    /* Copy control points	*/
    std::copy(p.begin(), p.end(), Vtemp[0].begin());

    /* Triangle computation	*/
    for (unsigned i = 1; i < sz; i++) {
        for (unsigned j = 0; j < sz - i; j++) {
            Vtemp[i][j] = lerp(t, Vtemp[i-1][j], Vtemp[i-1][j+1]);
        }
    }

    left.resize(sz);
    right.resize(sz);
    for (unsigned j = 0; j < sz; j++)
        left[j]  = Vtemp[j][0];
    for (unsigned j = 0; j < sz; j++)
        right[j] = Vtemp[sz-1-j][j];
}



void find_self_intersections(std::vector<std::pair<double, double> > &xs,
                             D2<Bezier> const &A,
                             double precision)
{
    std::vector<double> dr = derivative(A[X]).roots();
    {
        std::vector<double> dyr = derivative(A[Y]).roots();
        dr.insert(dr.begin(), dyr.begin(), dyr.end());
    }
    dr.push_back(0);
    dr.push_back(1);
    // We want to be sure that we have no empty segments
    std::sort(dr.begin(), dr.end());
    std::vector<double>::iterator new_end = std::unique(dr.begin(), dr.end());
    dr.resize( new_end - dr.begin() );

    std::vector< D2<Bezier> > pieces;
    for (unsigned i = 0; i < dr.size() - 1; ++i) {
        pieces.push_back(portion(A, dr[i], dr[i+1]));
    }
    /*{
        vector<Point> l, r, in = A;
        for(unsigned i = 0; i < dr.size()-1; i++) {
            split(in, (dr[i+1]-dr[i]) / (1 - dr[i]), l, r);
            pieces.push_back(l);
            in = r;
        }
    }*/

    for(unsigned i = 0; i < dr.size()-1; i++) {
        for(unsigned j = i+1; j < dr.size()-1; j++) {
            std::vector<std::pair<double, double> > section;
            
            find_intersections(section, pieces[i], pieces[j], precision);
            for(unsigned k = 0; k < section.size(); k++) {
                double l = section[k].first;
                double r = section[k].second;
// XXX: This condition will prune out false positives, but it might create some false negatives.  Todo: Confirm it is correct.
                if(j == i+1)
                    //if((l == 1) && (r == 0))
                    if( ( l > precision ) && (r < precision) )//FIXME: what precision should be used here???
                        continue;
                xs.push_back(std::make_pair((1-l)*dr[i] + l*dr[i+1],
                                                (1-r)*dr[j] + r*dr[j+1]));
            }
        }
    }

    // Because i is in order, xs should be roughly already in order?
    //sort(xs.begin(), xs.end());
    //unique(xs.begin(), xs.end());
}

void find_self_intersections(std::vector<std::pair<double, double> > &xs,
                             D2<SBasis> const &A,
                             double precision)
{
    D2<Bezier> in;
    sbasis_to_bezier(in, A);
    find_self_intersections(xs, in, precision);
}


void subdivide(D2<Bezier> const &a,
               D2<Bezier> const &b,
               std::vector< std::pair<double, double> > const &xs,
               std::vector< D2<Bezier> > &av,
               std::vector< D2<Bezier> > &bv)
{
    if (xs.empty()) {
        av.push_back(a);
        bv.push_back(b);
        return;
    }

    std::pair<double, double> prev = std::make_pair(0., 0.);
    for (unsigned i = 0; i < xs.size(); ++i) {
        av.push_back(portion(a, prev.first, xs[i].first));
        bv.push_back(portion(b, prev.second, xs[i].second));
        av.back()[X].at0() = bv.back()[X].at0() = lerp(0.5, av.back()[X].at0(), bv.back()[X].at0());
        av.back()[X].at1() = bv.back()[X].at1() = lerp(0.5, av.back()[X].at1(), bv.back()[X].at1());
        av.back()[Y].at0() = bv.back()[Y].at0() = lerp(0.5, av.back()[Y].at0(), bv.back()[Y].at0());
        av.back()[Y].at1() = bv.back()[Y].at1() = lerp(0.5, av.back()[Y].at1(), bv.back()[Y].at1());
        prev = xs[i];
    }
    av.push_back(portion(a, prev.first, 1));
    bv.push_back(portion(b, prev.second, 1));
    av.back()[X].at0() = bv.back()[X].at0() = lerp(0.5, av.back()[X].at0(), bv.back()[X].at0());
    av.back()[X].at1() = bv.back()[X].at1() = lerp(0.5, av.back()[X].at1(), bv.back()[X].at1());
    av.back()[Y].at0() = bv.back()[Y].at0() = lerp(0.5, av.back()[Y].at0(), bv.back()[Y].at0());
    av.back()[Y].at1() = bv.back()[Y].at1() = lerp(0.5, av.back()[Y].at1(), bv.back()[Y].at1());
}

#ifdef HAVE_GSL
#include <gsl/gsl_multiroots.h>

struct rparams
{
    D2<SBasis> const &A;
    D2<SBasis> const &B;
};

static int
intersect_polish_f (const gsl_vector * x, void *params,
                    gsl_vector * f)
{
    const double x0 = gsl_vector_get (x, 0);
    const double x1 = gsl_vector_get (x, 1);

    Geom::Point dx = ((struct rparams *) params)->A(x0) -
        ((struct rparams *) params)->B(x1);

    gsl_vector_set (f, 0, dx[0]);
    gsl_vector_set (f, 1, dx[1]);

    return GSL_SUCCESS;
}
#endif

union dbl_64{
    long long i64;
    double d64;
};

static double EpsilonBy(double value, int eps)
{
    dbl_64 s;
    s.d64 = value;
    s.i64 += eps;
    return s.d64;
}


static void intersect_polish_root (D2<SBasis> const &A, double &s,
                                   D2<SBasis> const &B, double &t) {
#ifdef HAVE_GSL
    const gsl_multiroot_fsolver_type *T;
    gsl_multiroot_fsolver *sol;

    int status;
    size_t iter = 0;
#endif
    std::vector<Point> as, bs;
    as = A.valueAndDerivatives(s, 2);
    bs = B.valueAndDerivatives(t, 2);
    Point F = as[0] - bs[0];
    double best = dot(F, F);
    
    for(int i = 0; i < 4; i++) {
        
        /**
           we want to solve
           J*(x1 - x0) = f(x0)
           
           |dA(s)[0]  -dB(t)[0]|  (X1 - X0) = A(s) - B(t)
           |dA(s)[1]  -dB(t)[1]| 
        **/

        // We're using the standard transformation matricies, which is numerically rather poor.  Much better to solve the equation using elimination.

        Affine jack(as[1][0], as[1][1],
                    -bs[1][0], -bs[1][1],
                    0, 0);
        Point soln = (F)*jack.inverse();
        double ns = s - soln[0];
        double nt = t - soln[1];
        
        as = A.valueAndDerivatives(ns, 2);
        bs = B.valueAndDerivatives(nt, 2);
        F = as[0] - bs[0];
        double trial = dot(F, F);
        if (trial > best*0.1) {// we have standards, you know
            // At this point we could do a line search
            break;
        }
        best = trial;
        s = ns;
        t = nt;
    }
    
#ifdef HAVE_GSL
    const size_t n = 2;
    struct rparams p = {A, B};
    gsl_multiroot_function f = {&intersect_polish_f, n, &p};

    double x_init[2] = {s, t};
    gsl_vector *x = gsl_vector_alloc (n);

    gsl_vector_set (x, 0, x_init[0]);
    gsl_vector_set (x, 1, x_init[1]);

    T = gsl_multiroot_fsolver_hybrids;
    sol = gsl_multiroot_fsolver_alloc (T, 2);
    gsl_multiroot_fsolver_set (sol, &f, x);

    do
    {
        iter++;
        status = gsl_multiroot_fsolver_iterate (sol);

        if (status)   /* check if solver is stuck */
            break;

        status =
            gsl_multiroot_test_residual (sol->f, 1e-12);
    }
    while (status == GSL_CONTINUE && iter < 1000);

    s = gsl_vector_get (sol->x, 0);
    t = gsl_vector_get (sol->x, 1);

    gsl_multiroot_fsolver_free (sol);
    gsl_vector_free (x);
#endif
    
    {
    // This code does a neighbourhood search for minor improvements.
    double best_v = L1(A(s) - B(t));
    //std::cout  << "------\n" <<  best_v << std::endl;
    Point best(s,t);
    while (true) {
        Point trial = best;
        double trial_v = best_v;
        for(int nsi = -1; nsi < 2; nsi++) {
        for(int nti = -1; nti < 2; nti++) {
            Point n(EpsilonBy(best[0], nsi),
                    EpsilonBy(best[1], nti));
            double c = L1(A(n[0]) - B(n[1]));
            //std::cout << c << "; ";
            if (c < trial_v) {
                trial = n;
                trial_v = c;
            }
        }
        }
        if(trial == best) {
            //std::cout << "\n" << s << " -> " << s - best[0] << std::endl;
            //std::cout << t << " -> " << t - best[1] << std::endl;
            //std::cout << best_v << std::endl;
            s = best[0];
            t = best[1];
            return;
        } else {
            best = trial;
            best_v = trial_v;
        }
    }
    }
}


void polish_intersections(std::vector<std::pair<double, double> > &xs, 
                        D2<SBasis> const  &A, D2<SBasis> const &B)
{
    for(unsigned i = 0; i < xs.size(); i++)
        intersect_polish_root(A, xs[i].first,
                              B, xs[i].second);
}

/**
 * Compute the Hausdorf distance from A to B only.
 */
double hausdorfl(D2<SBasis>& A, D2<SBasis> const& B,
                 double m_precision,
                 double *a_t, double* b_t) {
    std::vector< std::pair<double, double> > xs;
    std::vector<Point> Az, Bz;
    sbasis_to_bezier (Az, A);
    sbasis_to_bezier (Bz, B);
    find_collinear_normal(xs, Az, Bz, m_precision);
    double h_dist = 0, h_a_t = 0, h_b_t = 0;
    double dist = 0;
    Point Ax = A.at0();
    double t = Geom::nearest_time(Ax, B);
    dist = Geom::distance(Ax, B(t));
    if (dist > h_dist) {
        h_a_t = 0;
        h_b_t = t;
        h_dist = dist;
    }
    Ax = A.at1();
    t = Geom::nearest_time(Ax, B);
    dist = Geom::distance(Ax, B(t));
    if (dist > h_dist) {
        h_a_t = 1;
        h_b_t = t;
        h_dist = dist;
    }
    for (size_t i = 0; i < xs.size(); ++i)
    {
        Point At = A(xs[i].first);
        Point Bu = B(xs[i].second);
        double distAtBu = Geom::distance(At, Bu);
        t = Geom::nearest_time(At, B);
        dist = Geom::distance(At, B(t));
        //FIXME: we might miss it due to floating point precision...
        if (dist >= distAtBu-.1 && distAtBu > h_dist) {
            h_a_t = xs[i].first;
            h_b_t = xs[i].second;
            h_dist = distAtBu;
        }
            
    }
    if(a_t) *a_t = h_a_t;
    if(b_t) *b_t = h_b_t;
    
    return h_dist;
}

/** 
 * Compute the symmetric Hausdorf distance.
 */
double hausdorf(D2<SBasis>& A, D2<SBasis> const& B,
                 double m_precision,
                 double *a_t, double* b_t) {
    double h_dist = hausdorfl(A, B, m_precision, a_t, b_t);
    
    double dist = 0;
    Point Bx = B.at0();
    double t = Geom::nearest_time(Bx, A);
    dist = Geom::distance(Bx, A(t));
    if (dist > h_dist) {
        if(a_t) *a_t = t;
        if(b_t) *b_t = 0;
        h_dist = dist;
    }
    Bx = B.at1();
    t = Geom::nearest_time(Bx, A);
    dist = Geom::distance(Bx, A(t));
    if (dist > h_dist) {
        if(a_t) *a_t = t;
        if(b_t) *b_t = 1;
        h_dist = dist;
    }
    
    return h_dist;
}
};

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

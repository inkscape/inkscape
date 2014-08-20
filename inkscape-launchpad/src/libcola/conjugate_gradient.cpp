#include <math.h>
#include <stdlib.h>
#include <valarray>
#include <cassert>
#include "conjugate_gradient.h"

/*
* Authors:
*   Nathan Hurst <njh@njhurst.com>
*   Tim Dwyer <tgdwyer@gmail.com>
*
* Copyright (C) 2006 Authors
*
* Released under GNU LGPL.
*/

/* lifted wholely from wikipedia.  Well, apart from the bug in the wikipedia version. */

using std::valarray;

static void 
matrix_times_vector(valarray<double> const &matrix, /* m * n */
		    valarray<double> const &vec,  /* n */
		    valarray<double> &result) /* m */
{
    unsigned n = vec.size();
    unsigned m = result.size();
    assert(m*n == matrix.size());
    const double* mp = &matrix[0];
    for (unsigned i = 0; i < m; i++) {
        double res = 0;
        for (unsigned j = 0; j < n; j++)
            res += *mp++ * vec[j];
        result[i] = res;
    }
}

/*
static double Linfty(valarray<double> const &vec) {
    return std::max(vec.max(), -vec.min());
}
*/

double
inner(valarray<double> const &x, 
      valarray<double> const &y) {
    double total = 0;
    for(unsigned i = 0; i < x.size(); i++)
        total += x[i]*y[i];
    return total;// (x*y).sum(); <- this is more concise, but ineff
}

void 
conjugate_gradient(double **A, 
                   double *x, 
                   double *b, 
                   unsigned n, 
                   double tol,
                   unsigned max_iterations) {
    valarray<double> vA(n*n);
    valarray<double> vx(n);
    valarray<double> vb(n);
    for(unsigned i=0;i<n;i++) {
        vx[i]=x[i];
        vb[i]=b[i];
        for(unsigned j=0;j<n;j++) {
            vA[i*n+j]=A[i][j];
        }
    }
    conjugate_gradient(vA,vx,vb,n,tol,max_iterations);
    for(unsigned i=0;i<n;i++) {
        x[i]=vx[i];
    }
}
void 
conjugate_gradient(valarray<double> const &A, 
		   valarray<double> &x, 
		   valarray<double> const &b, 
		   unsigned n, double tol,
		   unsigned max_iterations) {
    valarray<double> Ap(n), p(n), r(n);
    matrix_times_vector(A,x,Ap);
    r=b-Ap; 
    double r_r = inner(r,r);
    unsigned k = 0;
    tol *= tol;
    while(k < max_iterations && r_r > tol) {
        k++;
        double r_r_new = r_r;
        if(k == 1)
            p = r;
        else {
            r_r_new = inner(r,r);
            p = r + (r_r_new/r_r)*p;
        }
        matrix_times_vector(A, p, Ap);
        double alpha_k = r_r_new / inner(p, Ap);
        x += alpha_k*p;
        r -= alpha_k*Ap;
        r_r = r_r_new;
    }
    //printf("njh: %d iters, Linfty = %g L2 = %g\n", k, 
    //std::max(-r.min(), r.max()), sqrt(r_r));
    // x is solution
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

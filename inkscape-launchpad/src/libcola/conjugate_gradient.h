#ifndef _CONJUGATE_GRADIENT_H
#define _CONJUGATE_GRADIENT_H

#include <valarray>

double
inner(std::valarray<double> const &x, 
      std::valarray<double> const &y);

void 
conjugate_gradient(double **A, 
		   double *x, 
		   double *b, 
		   unsigned n, 
		   double tol,
           unsigned max_iterations);
void 
conjugate_gradient(std::valarray<double> const &A, 
		   std::valarray<double> &x, 
		   std::valarray<double> const &b, 
		   unsigned n, double tol,
		   unsigned max_iterations);
#endif // _CONJUGATE_GRADIENT_H

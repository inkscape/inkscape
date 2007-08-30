#include "solver.h"
#include "point.h"
#include <algorithm>

/*** Find the zeros of the bernstein function.  The code subdivides until it is happy with the
 * linearity of the function.  This requires an O(degree^2) subdivision for each step, even when
 * there is only one solution.
 */

namespace Geom{

template<class t>
static int SGN(t x) { return (x > 0 ? 1 : (x < 0 ? -1 : 0)); } 

/*
 *  Forward declarations
 */
static void 
Bernstein(double const *V,
          unsigned degree,
          double t,
          double *Left,
          double *Right);

static unsigned 
control_poly_flat_enough(double const *V, unsigned degree,
			 double left_t, double right_t);

const unsigned MAXDEPTH = 64;	/*  Maximum depth for recursion */

const double BEPSILON = ldexp(1.0,-MAXDEPTH-1); /*Flatness control value */

/*
 *  find_bernstein_roots : Given an equation in Bernstein-Bernstein form, find all 
 *    of the roots in the open interval (0, 1).  Return the number of roots found.
 */
void
find_bernstein_roots(double const *w, /* The control points  */
                     unsigned degree,	/* The degree of the polynomial */
                     std::vector<double> &solutions, /* RETURN candidate t-values */
                     unsigned depth,	/* The depth of the recursion */
                     double left_t, double right_t)
{  
    unsigned 	n_crossings = 0;	/*  Number of zero-crossings */
    
    int old_sign = SGN(w[0]);
    for (unsigned i = 1; i <= degree; i++) {
        int sign = SGN(w[i]);
        if (sign) {
            if (sign != old_sign && old_sign) {
               n_crossings++;
            }
            old_sign = sign;
        }
    }
    
    switch (n_crossings) {
    case 0: 	/* No solutions here	*/
        return;
	
    case 1:
 	/* Unique solution	*/
        /* Stop recursion when the tree is deep enough	*/
        /* if deep enough, return 1 solution at midpoint  */
        if (depth >= MAXDEPTH) {
            solutions.push_back((left_t + right_t) / 2.0);
            return;
        }
        
        // I thought secant method would be faster here, but it'aint. -- njh

        if (control_poly_flat_enough(w, degree, left_t, right_t)) {
            const double Ax = right_t - left_t;
            const double Ay = w[degree] - w[0];

            solutions.push_back(left_t - Ax*w[0] / Ay);
            return;
        }
        break;
    }

    /* Otherwise, solve recursively after subdividing control polygon  */
    double Left[degree+1],	/* New left and right  */
           Right[degree+1];	/* control polygons  */
    const double split = 0.5;
    Bernstein(w, degree, split, Left, Right);
    
    double mid_t = left_t*(1-split) + right_t*split;
    
    find_bernstein_roots(Left,  degree, solutions, depth+1, left_t, mid_t);
            
    /* Solution is exactly on the subdivision point. */
    if (Right[0] == 0)
        solutions.push_back(mid_t);
        
    find_bernstein_roots(Right, degree, solutions, depth+1, mid_t, right_t);
}

/*
 *  control_poly_flat_enough :
 *	Check if the control polygon of a Bernstein curve is flat enough
 *	for recursive subdivision to bottom out.
 *
 */
static unsigned 
control_poly_flat_enough(double const *V, /* Control points	*/
			 unsigned degree,
			 double left_t, double right_t)	/* Degree of polynomial	*/
{
    /* Find the perpendicular distance from each interior control point to line connecting V[0] and
     * V[degree] */

    /* Derive the implicit equation for line connecting first */
    /*  and last control points */
    const double a = V[0] - V[degree];
    const double b = right_t - left_t;
    const double c = left_t * V[degree] - right_t * V[0] + a * left_t;

    double max_distance_above = 0.0;
    double max_distance_below = 0.0;
    double ii = 0, dii = 1./degree;
    for (unsigned i = 1; i < degree; i++) {
        ii += dii;
        /* Compute distance from each of the points to that line */
        const double d = (a + V[i]) * ii*b  + c;
        double dist = d*d;
    // Find the largest distance
        if (d < 0.0)
            max_distance_below = std::min(max_distance_below, -dist);
        else
            max_distance_above = std::max(max_distance_above, dist);
    }
    
    const double abSquared = (a * a) + (b * b);

    const double intercept_1 = -(c + max_distance_above / abSquared);
    const double intercept_2 = -(c + max_distance_below / abSquared);

    /* Compute bounding interval*/
    const double left_intercept = std::min(intercept_1, intercept_2);
    const double right_intercept = std::max(intercept_1, intercept_2);

    const double error = 0.5 * (right_intercept - left_intercept);
    
    if (error < BEPSILON * a)
        return 1;
    
    return 0;
}



/*
 *  Bernstein : 
 *	Evaluate a Bernstein function at a particular parameter value
 *      Fill in control points for resulting sub-curves.
 * 
 */
static void 
Bernstein(double const *V, /* Control pts	*/
          unsigned degree,	/* Degree of bernstein curve */
          double t,	/* Parameter value */
          double *Left,	/* RETURN left half ctl pts */
          double *Right)	/* RETURN right half ctl pts */
{
    double Vtemp[degree+1][degree+1];

    /* Copy control points	*/
    std::copy(V, V+degree+1, Vtemp[0]);

    /* Triangle computation	*/
    const double omt = (1-t);
    Left[0] = Vtemp[0][0];
    Right[degree] = Vtemp[0][degree];
    for (unsigned i = 1; i <= degree; i++) {
        for (unsigned j = 0; j <= degree - i; j++) {
            Vtemp[i][j] = omt*Vtemp[i-1][j] + t*Vtemp[i-1][j+1];
        }
        Left[i] = Vtemp[i][0];
        Right[degree-i] = Vtemp[i][degree-i];
    }
}

};

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/


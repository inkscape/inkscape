#include <2geom/bezier.h>
#include <2geom/point.h>
#include <2geom/solver.h>
#include <algorithm>

namespace Geom {

/*** Find the zeros of the parametric function in 2d defined by two beziers X(t), Y(t).  The code subdivides until it happy with the linearity of the bezier.  This requires an n^2 subdivision for each step, even when there is only one solution.
 * 
 * Perhaps it would be better to subdivide particularly around nodes with changing sign, rather than simply cutting in half.
 */

#define SGN(a)      (((a)<0) ? -1 : 1)

/*
 *  Forward declarations
 */
unsigned
crossing_count(Geom::Point const *V, unsigned degree);
static unsigned 
control_poly_flat_enough(Geom::Point const *V, unsigned degree);
static double
compute_x_intercept(Geom::Point const *V, unsigned degree);

const unsigned MAXDEPTH = 64;	/*  Maximum depth for recursion */

const double BEPSILON = ldexp(1.0,-MAXDEPTH-1); /*Flatness control value */

unsigned total_steps, total_subs;

/*
 *  find_bezier_roots : Given an equation in Bernstein-Bezier form, find all 
 *    of the roots in the interval [0, 1].  Return the number of roots found.
 */
void
find_parametric_bezier_roots(Geom::Point const *w, /* The control points  */
                  unsigned degree,	/* The degree of the polynomial */
                  std::vector<double> &solutions, /* RETURN candidate t-values */
                  unsigned depth)	/* The depth of the recursion */
{  
    total_steps++;
    const unsigned max_crossings = crossing_count(w, degree);
    switch (max_crossings) {
    case 0: 	/* No solutions here	*/
        return;
	
    case 1:
 	/* Unique solution	*/
        /* Stop recursion when the tree is deep enough	*/
        /* if deep enough, return 1 solution at midpoint  */
        if (depth >= MAXDEPTH) {
            solutions.push_back((w[0][Geom::X] + w[degree][Geom::X]) / 2.0);
            return;
        }
        
        // I thought secant method would be faster here, but it'aint. -- njh

        if (control_poly_flat_enough(w, degree)) {
            solutions.push_back(compute_x_intercept(w, degree));
            return;
        }
        break;
    }

    /* Otherwise, solve recursively after subdividing control polygon  */

    //Geom::Point Left[degree+1],	/* New left and right  */
    //    Right[degree+1];	/* control polygons  */
    std::vector<Geom::Point> Left( degree+1 ), Right(degree+1);

    casteljau_subdivision(0.5, w, Left.data(), Right.data(), degree);
    total_subs ++;
    find_parametric_bezier_roots(Left.data(),  degree, solutions, depth+1);
    find_parametric_bezier_roots(Right.data(), degree, solutions, depth+1);
}


/*
 * crossing_count:
 *  Count the number of times a Bezier control polygon 
 *  crosses the 0-axis. This number is >= the number of roots.
 *
 */
unsigned
crossing_count(Geom::Point const *V,	/*  Control pts of Bezier curve	*/
	       unsigned degree)	/*  Degree of Bezier curve 	*/
{
    unsigned 	n_crossings = 0;	/*  Number of zero-crossings */
    
    int old_sign = SGN(V[0][Geom::Y]);
    for (unsigned i = 1; i <= degree; i++) {
        int sign = SGN(V[i][Geom::Y]);
        if (sign != old_sign)
            n_crossings++;
        old_sign = sign;
    }
    return n_crossings;
}



/*
 *  control_poly_flat_enough :
 *	Check if the control polygon of a Bezier curve is flat enough
 *	for recursive subdivision to bottom out.
 *
 */
static unsigned 
control_poly_flat_enough(Geom::Point const *V, /* Control points	*/
			 unsigned degree)	/* Degree of polynomial	*/
{
    /* Find the perpendicular distance from each interior control point to line connecting V[0] and
     * V[degree] */

    /* Derive the implicit equation for line connecting first */
    /*  and last control points */
    const double a = V[0][Geom::Y] - V[degree][Geom::Y];
    const double b = V[degree][Geom::X] - V[0][Geom::X];
    const double c = V[0][Geom::X] * V[degree][Geom::Y] - V[degree][Geom::X] * V[0][Geom::Y];

    const double abSquared = (a * a) + (b * b);

    //double distance[degree]; /* Distances from pts to line */
    std::vector<double> distance(degree); /* Distances from pts to line */
    for (unsigned i = 1; i < degree; i++) {
        /* Compute distance from each of the points to that line */
        double & dist(distance[i-1]);
        const double d = a * V[i][Geom::X] + b * V[i][Geom::Y] + c;
        dist = d*d / abSquared;
        if (d < 0.0)
            dist = -dist;
    }


    // Find the largest distance
    double max_distance_above = 0.0;
    double max_distance_below = 0.0;
    for (unsigned i = 0; i < degree-1; i++) {
        const double d = distance[i];
        if (d < 0.0)
            max_distance_below = std::min(max_distance_below, d);
        if (d > 0.0)
            max_distance_above = std::max(max_distance_above, d);
    }

    const double intercept_1 = (c + max_distance_above) / -a;
    const double intercept_2 = (c + max_distance_below) / -a;

    /* Compute bounding interval*/
    const double left_intercept = std::min(intercept_1, intercept_2);
    const double right_intercept = std::max(intercept_1, intercept_2);

    const double error = 0.5 * (right_intercept - left_intercept);
    
    if (error < BEPSILON)
        return 1;
    
    return 0;
}



/*
 *  compute_x_intercept :
 *	Compute intersection of chord from first control point to last
 *  	with 0-axis.
 * 
 */
static double
compute_x_intercept(Geom::Point const *V, /*  Control points	*/
		    unsigned degree) /*  Degree of curve	*/
{
    const Geom::Point A = V[degree] - V[0];

    return (A[Geom::X]*V[0][Geom::Y] - A[Geom::Y]*V[0][Geom::X]) / -A[Geom::Y];
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

#include <2geom/solver.h>
#include <2geom/point.h>
#include <algorithm>

/*** Find the zeros of the bernstein function.  The code subdivides until it is happy with the
 * linearity of the function.  This requires an O(degree^2) subdivision for each step, even when
 * there is only one solution.
 */

namespace Geom{

template<class t>
static int SGN(t x) { return (x > 0 ? 1 : (x < 0 ? -1 : 0)); } 

const unsigned MAXDEPTH = 23; // Maximum depth for recursion.  Using floats means 23 bits precision max

const double BEPSILON = ldexp(1.0,(-MAXDEPTH-1)); /*Flatness control value */
const double SECANT_EPSILON = 1e-13; // secant method converges much faster, get a bit more precision
/**
 * This function is called _a lot_.  We have included various manual memory management stuff to reduce the amount of mallocing that goes on.  In the future it is possible that this will hurt performance.
 **/
class Bernsteins{
public:
    double *Vtemp;
    unsigned N,degree;
    std::vector<double> &solutions;
    bool use_secant;
    Bernsteins(int degr, std::vector<double> &so) : N(degr+1), degree(degr),solutions(so), use_secant(false) {
        Vtemp = new double[N*2];
    }
    ~Bernsteins() {
        delete[] Vtemp;
    }
    void subdivide(double const *V,
                   double t,
                   double *Left,
                   double *Right);

    double horner(const double *b, double t);
    
    unsigned 
    control_poly_flat_enough(double const *V);

    void
    find_bernstein_roots(double const *w, /* The control points  */
                         unsigned depth,  /* The depth of the recursion */
                         double left_t, double right_t);
};
/*
 *  find_bernstein_roots : Given an equation in Bernstein-Bernstein form, find all 
 *    of the roots in the open interval (0, 1).  Return the number of roots found.
 */
void
find_bernstein_roots(double const *w, /* The control points  */
                     unsigned degree,	/* The degree of the polynomial */
                     std::vector<double> &solutions, /* RETURN candidate t-values */
                     unsigned depth,	/* The depth of the recursion */
                     double left_t, double right_t, bool use_secant)
{  
    Bernsteins B(degree, solutions);
    B.use_secant = use_secant;
    B.find_bernstein_roots(w, depth, left_t, right_t);
}

void
Bernsteins::find_bernstein_roots(double const *w, /* The control points  */
                     unsigned depth,	/* The depth of the recursion */
                     double left_t, double right_t) 
{

    unsigned 	n_crossings = 0;	/*  Number of zero-crossings */
    
    int old_sign = SGN(w[0]);
    for (unsigned i = 1; i < N; i++) {
        int sign = SGN(w[i]);
        if (sign) {
            if (sign != old_sign && old_sign) {
               n_crossings++;
            }
            old_sign = sign;
        }
    }
    
    if (n_crossings == 0) // no solutions here
        return;
	
    if (n_crossings == 1) {
 	/* Unique solution	*/
        /* Stop recursion when the tree is deep enough	*/
        /* if deep enough, return 1 solution at midpoint  */
        if (depth >= MAXDEPTH) {
            //printf("bottom out %d\n", depth);
            const double Ax = right_t - left_t;
            const double Ay = w[degree] - w[0];
            
            solutions.push_back(left_t - Ax*w[0] / Ay);
            return;
            solutions.push_back((left_t + right_t) / 2.0);
            return;
        }
        
        // I thought secant method would be faster here, but it'aint. -- njh
        // Actually, it was, I just was using the wrong method for bezier evaluation.  Horner's rule results in a very efficient algorithm - 10* faster (20080816)
        // Future work: try using brent's method
        if(use_secant) { // false position
            double s = 0;double t = 1;
            double e = 1e-10;
            int n,side=0;
            double r,fr,fs = w[0],ft = w[degree];
 
            for (n = 1; n <= 100; n++)
            {
                r = (fs*t - ft*s) / (fs - ft);
                if (fabs(t-s) < e*fabs(t+s)) break;
                fr = horner(w, r);
 
                if (fr * ft > 0)
                {
                    t = r; ft = fr;
                    if (side==-1) fs /= 2;
                    side = -1;
                }
                else if (fs * fr > 0)
                {
                    s = r;  fs = fr;
                    if (side==+1) ft /= 2;
                    side = +1;
                }
                else break;
            }
            solutions.push_back(r*right_t + (1-r)*left_t);
            return;
        }
    }

    /* Otherwise, solve recursively after subdividing control polygon  */
    double Left[N],	/* New left and right  */
           Right[N];	/* control polygons  */
    const double t = 0.5;


/*
 *  Bernstein : 
 *	Evaluate a Bernstein function at a particular parameter value
 *      Fill in control points for resulting sub-curves.
 * 
 */
    for (unsigned i = 0; i < N; i++)
        Vtemp[i] = w[i];

    /* Triangle computation	*/
    const double omt = (1-t);
    Left[0] = Vtemp[0];
    Right[degree] = Vtemp[degree];
    double *prev_row = Vtemp;
    double *row = Vtemp + N;
    for (unsigned i = 1; i < N; i++) {
        for (unsigned j = 0; j < N - i; j++) {
            row[j] = omt*prev_row[j] + t*prev_row[j+1];
        }
        Left[i] = row[0];
        Right[degree-i] = row[degree-i];
        std::swap(prev_row, row);
    }
    
    double mid_t = left_t*(1-t) + right_t*t;
    
    find_bernstein_roots(Left, depth+1, left_t, mid_t);
            
    /* Solution is exactly on the subdivision point. */
    if (Right[0] == 0)
        solutions.push_back(mid_t);
        
    find_bernstein_roots(Right, depth+1, mid_t, right_t);
}

/*
 *  control_poly_flat_enough :
 *	Check if the control polygon of a Bernstein curve is flat enough
 *	for recursive subdivision to bottom out.
 *
 */
unsigned 
Bernsteins::control_poly_flat_enough(double const *V)
{
    /* Find the perpendicular distance from each interior control point to line connecting V[0] and
     * V[degree] */

    /* Derive the implicit equation for line connecting first */
    /*  and last control points */
    const double a = V[0] - V[degree];

    double max_distance_above = 0.0;
    double max_distance_below = 0.0;
    double ii = 0, dii = 1./degree;
    for (unsigned i = 1; i < degree; i++) {
        ii += dii;
        /* Compute distance from each of the points to that line */
        const double d = (a + V[i]) * ii  - a;
        double dist = d*d;
    // Find the largest distance
        if (d < 0.0)
            max_distance_below = std::min(max_distance_below, -dist);
        else
            max_distance_above = std::max(max_distance_above, dist);
    }
    
    const double abSquared = 1./((a * a) + 1);

    const double intercept_1 = (a - max_distance_above * abSquared);
    const double intercept_2 = (a - max_distance_below * abSquared);

    /* Compute bounding interval*/
    const double left_intercept = std::min(intercept_1, intercept_2);
    const double right_intercept = std::max(intercept_1, intercept_2);

    const double error = 0.5 * (right_intercept - left_intercept);
    //printf("error %g %g %g\n", error, a, BEPSILON * a);
    return error < BEPSILON * a;
}

// suggested by Sederberg.
double Bernsteins::horner(const double *b, double t) {
    int n = degree;
    double u, bc, tn, tmp;
    int i;
    u = 1.0 - t;
    bc = 1;
    tn = 1;
    tmp = b[0]*u;
    for(i=1; i<n; i++){
        tn = tn*t;
        bc = bc*(n-i+1)/i;
        tmp = (tmp + tn*bc*b[i])*u;
    }
    return (tmp + tn*t*b[n]);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

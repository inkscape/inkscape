
#include <2geom/solver.h>
#include <2geom/choose.h>
#include <2geom/bezier.h>
#include <2geom/point.h>

#include <cmath>
#include <algorithm>
//#include <valarray>

/*** Find the zeros of the bernstein function.  The code subdivides until it is happy with the
 * linearity of the function.  This requires an O(degree^2) subdivision for each step, even when
 * there is only one solution.
 */

namespace Geom{

template<class t>
static int SGN(t x) { return (x > 0 ? 1 : (x < 0 ? -1 : 0)); }

//const unsigned MAXDEPTH = 23; // Maximum depth for recursion.  Using floats means 23 bits precision max

//const double BEPSILON = ldexp(1.0,(-MAXDEPTH-1)); /*Flatness control value */
//const double SECANT_EPSILON = 1e-13; // secant method converges much faster, get a bit more precision
/**
 * This function is called _a lot_.  We have included various manual memory management stuff to reduce the amount of mallocing that goes on.  In the future it is possible that this will hurt performance.
 **/
class Bernsteins{
public:
    static const size_t MAX_DEPTH = 53;
    size_t degree, N;
    std::vector<double> &solutions;
    //std::vector<double> bc;
    BinomialCoefficient<double> bc;

    Bernsteins(size_t _degree, std::vector<double> & sol)
        : degree(_degree), N(degree+1), solutions(sol), bc(degree)
    {
    }

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
                     unsigned degree,   /* The degree of the polynomial */
                     std::vector<double> &solutions, /* RETURN candidate t-values */
                     unsigned depth,    /* The depth of the recursion */
                     double left_t, double right_t, bool /*use_secant*/)
{
    Bernsteins B(degree, solutions);
    B.find_bernstein_roots(w, depth, left_t, right_t);
}

void
find_bernstein_roots(std::vector<double> &solutions, /* RETURN candidate t-values */
                     Geom::Bezier const &bz, /* The control points  */
                     double left_t, double right_t)
{
    Bernsteins B(bz.degree(), solutions);
    Geom::Bezier& bzl = const_cast<Geom::Bezier&>(bz);
    double* w = &(bzl[0]);
    B.find_bernstein_roots(w, 0, left_t, right_t);
}



void Bernsteins::find_bernstein_roots(double const *w, /* The control points  */
                          unsigned depth,    /* The depth of the recursion */
                          double left_t,
                          double right_t)
{

    size_t n_crossings = 0;

    int old_sign = SGN(w[0]);
    //std::cout << "w[0] = " << w[0] << std::endl;
    for (size_t i = 1; i < N; i++)
    {
        //std::cout << "w[" << i << "] = " << w[i] << std::endl;
        int sign = SGN(w[i]);
        if (sign != 0)
        {
            if (sign != old_sign && old_sign != 0)
            {
               ++n_crossings;
            }
            old_sign = sign;
        }
    }
    //std::cout << "n_crossings = " << n_crossings << std::endl;
    if (n_crossings == 0)  return; // no solutions here

    if (n_crossings == 1) /* Unique solution  */
    {
        //std::cout << "depth = " << depth << std::endl;
        /* Stop recursion when the tree is deep enough  */
        /* if deep enough, return 1 solution at midpoint  */
        if (depth > MAX_DEPTH)
        {
            //printf("bottom out %d\n", depth);
            const double Ax = right_t - left_t;
            const double Ay = w[degree] - w[0];

            solutions.push_back(left_t - Ax*w[0] / Ay);
            return;
        }


        double s = 0, t = 1;
        double e = 1e-10;
        int side = 0;
        double r, fs = w[0], ft = w[degree];

        for (size_t n = 0; n < 100; ++n)
        {
            r = (fs*t - ft*s) / (fs - ft);
            if (fabs(t-s) < e * fabs(t+s))  break;

            double fr = bernstein_value_at(r, w, degree);

            if (fr * ft > 0)
            {
                t = r; ft = fr;
                if (side == -1) fs /= 2;
                side = -1;
            }
            else if (fs * fr > 0)
            {
                s = r;  fs = fr;
                if (side == +1) ft /= 2;
                side = +1;
            }
            else break;
        }
        solutions.push_back(r*right_t + (1-r)*left_t);
        return;

    }

    /* Otherwise, solve recursively after subdividing control polygon  */
//    double Left[N], /* New left and right  */
//           Right[N];    /* control polygons  */
    //const double t = 0.5;
    double* LR = new double[2*N];
    double* Left = LR;
    double* Right = LR + N;

    std::copy(w, w + N, Right);

    Left[0] = Right[0];
    for (size_t i = 1; i < N; ++i)
    {
        for (size_t j = 0; j < N-i; ++j)
        {
            Right[j] = (Right[j] + Right[j+1]) * 0.5;
        }
        Left[i] = Right[0];
    }

    double mid_t = (left_t + right_t) * 0.5;


    find_bernstein_roots(Left, depth+1, left_t, mid_t);


    /* Solution is exactly on the subdivision point. */
    if (Right[0] == 0)
    {
        solutions.push_back(mid_t);
    }

    find_bernstein_roots(Right, depth+1, mid_t, right_t);
    delete[] LR;
}

#if 0
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
#endif


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

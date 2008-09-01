


#include <2geom/basic-intersection.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/exception.h>


#include <gsl/gsl_vector.h>
#include <gsl/gsl_multiroots.h>


unsigned intersect_steps = 0;

using std::vector;
namespace Geom {

class OldBezier {
public:
    std::vector<Geom::Point> p;
    OldBezier() {
    }
    void split(double t, OldBezier &a, OldBezier &b) const;
    Point operator()(double t) const;

    ~OldBezier() {}

    void bounds(double &minax, double &maxax,
                double &minay, double &maxay) {
        // Compute bounding box for a
        minax = p[0][X];	 // These are the most likely to be extremal
        maxax = p.back()[X];
        if( minax > maxax )
            std::swap(minax, maxax);
        for(unsigned i = 1; i < p.size()-1; i++) {
            if( p[i][X] < minax )
                minax = p[i][X];
            else if( p[i][X] > maxax )
                maxax = p[i][X];
        }

        minay = p[0][Y];	 // These are the most likely to be extremal
        maxay = p.back()[Y];
        if( minay > maxay )
            std::swap(minay, maxay);
        for(unsigned i = 1; i < p.size()-1; i++) {
            if( p[i][Y] < minay )
                minay = p[i][Y];
            else if( p[i][Y] > maxay )
                maxay = p[i][Y];
        }

    }

};

static std::vector<std::pair<double, double> >
find_intersections( OldBezier a, OldBezier b);

static std::vector<std::pair<double, double> >
find_self_intersections(OldBezier const &Sb, D2<SBasis> const & A);

std::vector<std::pair<double, double> >
find_intersections( vector<Geom::Point> const & A,
                    vector<Geom::Point> const & B) {
    OldBezier a, b;
    a.p = A;
    b.p = B;
    return find_intersections(a,b);
}

std::vector<std::pair<double, double> >
find_self_intersections(OldBezier const &/*Sb*/) {
    THROW_NOTIMPLEMENTED();
}

std::vector<std::pair<double, double> >
find_self_intersections(D2<SBasis> const & A) {
    OldBezier Sb;
    sbasis_to_bezier(Sb.p, A);
    return find_self_intersections(Sb, A);
}


static std::vector<std::pair<double, double> >
find_self_intersections(OldBezier const &Sb, D2<SBasis> const & A) {


    vector<double> dr = roots(derivative(A[X]));
    {
        vector<double> dyr = roots(derivative(A[Y]));
        dr.insert(dr.begin(), dyr.begin(), dyr.end());
    }
    dr.push_back(0);
    dr.push_back(1);
    // We want to be sure that we have no empty segments
    sort(dr.begin(), dr.end());
    unique(dr.begin(), dr.end());

    std::vector<std::pair<double, double> > all_si;

    vector<OldBezier> pieces;
    {
        OldBezier in = Sb, l, r;
        for(unsigned i = 0; i < dr.size()-1; i++) {
            in.split((dr[i+1]-dr[i]) / (1 - dr[i]), l, r);
            pieces.push_back(l);
            in = r;
        }
    }
    for(unsigned i = 0; i < dr.size()-1; i++) {
        for(unsigned j = i+1; j < dr.size()-1; j++) {
            std::vector<std::pair<double, double> > section =
                find_intersections( pieces[i], pieces[j]);
            for(unsigned k = 0; k < section.size(); k++) {
                double l = section[k].first;
                double r = section[k].second;
// XXX: This condition will prune out false positives, but it might create some false negatives.  Todo: Confirm it is correct.
                if(j == i+1)
                    if((l == 1) && (r == 0))
                        continue;
                all_si.push_back(std::make_pair((1-l)*dr[i] + l*dr[i+1],
                                                (1-r)*dr[j] + r*dr[j+1]));
            }
        }
    }

    // Because i is in order, all_si should be roughly already in order?
    //sort(all_si.begin(), all_si.end());
    //unique(all_si.begin(), all_si.end());

    return all_si;
}

/* The value of 1.0 / (1L<<14) is enough for most applications */
const double INV_EPS = (1L<<14);

/*
 * split the curve at the midpoint, returning an array with the two parts
 * Temporary storage is minimized by using part of the storage for the result
 * to hold an intermediate value until it is no longer needed.
 */
void OldBezier::split(double t, OldBezier &left, OldBezier &right) const {
    const unsigned sz = p.size();
    Geom::Point Vtemp[sz][sz];

    /* Copy control points	*/
    std::copy(p.begin(), p.end(), Vtemp[0]);

    /* Triangle computation	*/
    for (unsigned i = 1; i < sz; i++) {
        for (unsigned j = 0; j < sz - i; j++) {
            Vtemp[i][j] = lerp(t, Vtemp[i-1][j], Vtemp[i-1][j+1]);
        }
    }

    left.p.resize(sz);
    right.p.resize(sz);
    for (unsigned j = 0; j < sz; j++)
        left.p[j]  = Vtemp[j][0];
    for (unsigned j = 0; j < sz; j++)
        right.p[j] = Vtemp[sz-1-j][j];
}

#if 0
/*
 * split the curve at the midpoint, returning an array with the two parts
 * Temporary storage is minimized by using part of the storage for the result
 * to hold an intermediate value until it is no longer needed.
 */
Point OldBezier::operator()(double t) const {
    const unsigned sz = p.size();
    Geom::Point Vtemp[sz][sz];

    /* Copy control points	*/
    std::copy(p.begin(), p.end(), Vtemp[0]);

    /* Triangle computation	*/
    for (unsigned i = 1; i < sz; i++) {
        for (unsigned j = 0; j < sz - i; j++) {
            Vtemp[i][j] = lerp(t, Vtemp[i-1][j], Vtemp[i-1][j+1]);
        }
    }
    return Vtemp[sz-1][0];
}
#endif

// suggested by Sederberg.
Point OldBezier::operator()(double t) const {
    int n = p.size()-1;
    double u, bc, tn, tmp;
    int i;
    Point r;
    for(int dim = 0; dim < 2; dim++) {
        u = 1.0 - t;
        bc = 1;
        tn = 1;
        tmp = p[0][dim]*u;
        for(i=1; i<n; i++){
            tn = tn*t;
            bc = bc*(n-i+1)/i;
            tmp = (tmp + tn*bc*p[i][dim])*u;
        }
        r[dim] = (tmp + tn*t*p[n][dim]);
    }
    return r;
}


/*
 * Test the bounding boxes of two OldBezier curves for interference.
 * Several observations:
 *	First, it is cheaper to compute the bounding box of the second curve
 *	and test its bounding box for interference than to use a more direct
 *	approach of comparing all control points of the second curve with
 *	the various edges of the bounding box of the first curve to test
 * 	for interference.
 *	Second, after a few subdivisions it is highly probable that two corners
 *	of the bounding box of a given Bezier curve are the first and last
 *	control point.  Once this happens once, it happens for all subsequent
 *	subcurves.  It might be worth putting in a test and then short-circuit
 *	code for further subdivision levels.
 *	Third, in the final comparison (the interference test) the comparisons
 *	should both permit equality.  We want to find intersections even if they
 *	occur at the ends of segments.
 *	Finally, there are tighter bounding boxes that can be derived. It isn't
 *	clear whether the higher probability of rejection (and hence fewer
 *	subdivisions and tests) is worth the extra work.
 */

bool intersect_BB( OldBezier a, OldBezier b ) {
    double minax, maxax, minay, maxay;
    a.bounds(minax, maxax, minay, maxay);
    double minbx, maxbx, minby, maxby;
    b.bounds(minbx, maxbx, minby, maxby);
    // Test bounding box of b against bounding box of a
    // Not >= : need boundary case
    return not( ( minax > maxbx ) || ( minay > maxby )
                || ( minbx > maxax ) || ( minby > maxay ) );
}

/*
 * Recursively intersect two curves keeping track of their real parameters
 * and depths of intersection.
 * The results are returned in a 2-D array of doubles indicating the parameters
 * for which intersections are found.  The parameters are in the order the
 * intersections were found, which is probably not in sorted order.
 * When an intersection is found, the parameter value for each of the two
 * is stored in the index elements array, and the index is incremented.
 *
 * If either of the curves has subdivisions left before it is straight
 *	(depth > 0)
 * that curve (possibly both) is (are) subdivided at its (their) midpoint(s).
 * the depth(s) is (are) decremented, and the parameter value(s) corresponding
 * to the midpoints(s) is (are) computed.
 * Then each of the subcurves of one curve is intersected with each of the
 * subcurves of the other curve, first by testing the bounding boxes for
 * interference.  If there is any bounding box interference, the corresponding
 * subcurves are recursively intersected.
 *
 * If neither curve has subdivisions left, the line segments from the first
 * to last control point of each segment are intersected.  (Actually the
 * only the parameter value corresponding to the intersection point is found).
 *
 * The apriori flatness test is probably more efficient than testing at each
 * level of recursion, although a test after three or four levels would
 * probably be worthwhile, since many curves become flat faster than their
 * asymptotic rate for the first few levels of recursion.
 *
 * The bounding box test fails much more frequently than it succeeds, providing
 * substantial pruning of the search space.
 *
 * Each (sub)curve is subdivided only once, hence it is not possible that for
 * one final line intersection test the subdivision was at one level, while
 * for another final line intersection test the subdivision (of the same curve)
 * was at another.  Since the line segments share endpoints, the intersection
 * is robust: a near-tangential intersection will yield zero or two
 * intersections.
 */
void recursively_intersect( OldBezier a, double t0, double t1, int deptha,
			   OldBezier b, double u0, double u1, int depthb,
			   std::vector<std::pair<double, double> > &parameters)
{
    intersect_steps ++;
    if( deptha > 0 )
    {
        OldBezier A[2];
        a.split(0.5, A[0], A[1]);
	double tmid = (t0+t1)*0.5;
	deptha--;
	if( depthb > 0 )
        {
	    OldBezier B[2];
            b.split(0.5, B[0], B[1]);
	    double umid = (u0+u1)*0.5;
	    depthb--;
	    if( intersect_BB( A[0], B[0] ) )
		recursively_intersect( A[0], t0, tmid, deptha,
				      B[0], u0, umid, depthb,
				      parameters );
	    if( intersect_BB( A[1], B[0] ) )
		recursively_intersect( A[1], tmid, t1, deptha,
				      B[0], u0, umid, depthb,
				      parameters );
	    if( intersect_BB( A[0], B[1] ) )
		recursively_intersect( A[0], t0, tmid, deptha,
				      B[1], umid, u1, depthb,
				      parameters );
	    if( intersect_BB( A[1], B[1] ) )
		recursively_intersect( A[1], tmid, t1, deptha,
				      B[1], umid, u1, depthb,
				      parameters );
        }
	else
        {
	    if( intersect_BB( A[0], b ) )
		recursively_intersect( A[0], t0, tmid, deptha,
				      b, u0, u1, depthb,
				      parameters );
	    if( intersect_BB( A[1], b ) )
		recursively_intersect( A[1], tmid, t1, deptha,
				      b, u0, u1, depthb,
				      parameters );
        }
    }
    else
	if( depthb > 0 )
        {
	    OldBezier B[2];
            b.split(0.5, B[0], B[1]);
	    double umid = (u0 + u1)*0.5;
	    depthb--;
	    if( intersect_BB( a, B[0] ) )
		recursively_intersect( a, t0, t1, deptha,
				      B[0], u0, umid, depthb,
				      parameters );
	    if( intersect_BB( a, B[1] ) )
		recursively_intersect( a, t0, t1, deptha,
				      B[0], umid, u1, depthb,
				      parameters );
        }
	else // Both segments are fully subdivided; now do line segments
        {
	    double xlk = a.p.back()[X] - a.p[0][X];
	    double ylk = a.p.back()[Y] - a.p[0][Y];
	    double xnm = b.p.back()[X] - b.p[0][X];
	    double ynm = b.p.back()[Y] - b.p[0][Y];
	    double xmk = b.p[0][X] - a.p[0][X];
	    double ymk = b.p[0][Y] - a.p[0][Y];
	    double det = xnm * ylk - ynm * xlk;
	    if( 1.0 + det == 1.0 )
		return;
	    else
            {
		double detinv = 1.0 / det;
		double s = ( xnm * ymk - ynm *xmk ) * detinv;
		double t = ( xlk * ymk - ylk * xmk ) * detinv;
		if( ( s < 0.0 ) || ( s > 1.0 ) || ( t < 0.0 ) || ( t > 1.0 ) )
		    return;
		parameters.push_back(std::pair<double, double>(t0 + s * ( t1 - t0 ),
                                                         u0 + t * ( u1 - u0 )));
            }
        }
}

inline double log4( double x ) { return log(x)/log(4.); }

/*
 * Wang's theorem is used to estimate the level of subdivision required,
 * but only if the bounding boxes interfere at the top level.
 * Assuming there is a possible intersection, recursively_intersect is
 * used to find all the parameters corresponding to intersection points.
 * these are then sorted and returned in an array.
 */

double Lmax(Point p) {
    return std::max(fabs(p[X]), fabs(p[Y]));
}

unsigned wangs_theorem(OldBezier a) {
    return 6; // seems a good approximation!
    double la1 = Lmax( ( a.p[2] - a.p[1] ) - (a.p[1] - a.p[0]) );
    double la2 = Lmax( ( a.p[3] - a.p[2] ) - (a.p[2] - a.p[1]) );
    double l0 = std::max(la1, la2);
    unsigned ra;
    if( l0 * 0.75 * M_SQRT2 + 1.0 == 1.0 )
        ra = 0;
    else
        ra = (unsigned)ceil( log4( M_SQRT2 * 6.0 / 8.0 * INV_EPS * l0 ) );
    std::cout << ra << std::endl;
    return ra;
}

struct rparams
{
    OldBezier &A;
    OldBezier &B;
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

typedef union dbl_64{
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


static void intersect_polish_root (OldBezier &A, double &s,
                            OldBezier &B, double &t) {
    const gsl_multiroot_fsolver_type *T;
    gsl_multiroot_fsolver *sol;

    int status;
    size_t iter = 0;

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


std::vector<std::pair<double, double> > find_intersections( OldBezier a, OldBezier b)
{
    std::vector<std::pair<double, double> > parameters;
    if( intersect_BB( a, b ) )
    {
	recursively_intersect( a, 0., 1., wangs_theorem(a),
                               b, 0., 1., wangs_theorem(b),
                               parameters);
    }
    for(unsigned i = 0; i < parameters.size(); i++)
        intersect_polish_root(a, parameters[i].first,
                              b, parameters[i].second);
    std::sort(parameters.begin(), parameters.end());
    return parameters;
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

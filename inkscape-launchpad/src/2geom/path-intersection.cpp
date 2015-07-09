#include <2geom/path-intersection.h>

#include <2geom/ord.h>

//for path_direction:
#include <2geom/sbasis-geometric.h>
#include <2geom/line.h>
#ifdef HAVE_GSL
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multiroots.h>
#endif

namespace Geom {

/// Compute winding number of the path at the specified point
int winding(Path const &path, Point const &p) {
    return path.winding(p);
}

/**
 * This function should only be applied to simple paths (regions), as otherwise
 * a boolean winding direction is undefined.  It returns true for fill, false for
 * hole.  Defaults to using the sign of area when it reaches funny cases.
 */
bool path_direction(Path const &p) {
    if(p.empty()) return false;

    /*goto doh;
    //could probably be more efficient, but this is a quick job
    double y = p.initialPoint()[Y];
    double x = p.initialPoint()[X];
    Cmp res = cmp(p[0].finalPoint()[Y], y);
    for(unsigned i = 1; i < p.size(); i++) {
        Cmp final_to_ray = cmp(p[i].finalPoint()[Y], y);
        Cmp initial_to_ray = cmp(p[i].initialPoint()[Y], y);
        // if y is included, these will have opposite values, giving order.
        Cmp c = cmp(final_to_ray, initial_to_ray);
        if(c != EQUAL_TO) {
            std::vector<double> rs = p[i].roots(y, Y);
            for(unsigned j = 0; j < rs.size(); j++) {
                double nx = p[i].valueAt(rs[j], X);
                if(nx > x) {
                    x = nx;
                    res = c;
                }
            }
        } else if(final_to_ray == EQUAL_TO) goto doh;
    }
    return res < 0;
    
    doh:*/
        //Otherwise fallback on area
        
        Piecewise<D2<SBasis> > pw = p.toPwSb();
        double area;
        Point centre;
        Geom::centroid(pw, centre, area);
        return area > 0;
}

//pair intersect code based on njh's pair-intersect

/** A little sugar for appending a list to another */
template<typename T>
void append(T &a, T const &b) {
    a.insert(a.end(), b.begin(), b.end());
}

/**
 * Finds the intersection between the lines defined by A0 & A1, and B0 & B1.
 * Returns through the last 3 parameters, returning the t-values on the lines
 * and the cross-product of the deltas (a useful byproduct).  The return value
 * indicates if the time values are within their proper range on the line segments.
 */
bool
linear_intersect(Point const &A0, Point const &A1, Point const &B0, Point const &B1,
                 double &tA, double &tB, double &det) {
    bool both_lines_non_zero = (!are_near(A0, A1)) && (!are_near(B0, B1));

    // Cramer's rule as cross products
    Point Ad = A1 - A0,
          Bd = B1 - B0,
           d = B0 - A0;
    det = cross(Ad, Bd);

    double det_rel = det; // Calculate the determinant of the normalized vectors
    if (both_lines_non_zero) {
        det_rel /= Ad.length();
        det_rel /= Bd.length();
    }

    if( fabs(det_rel) < 1e-12 ) { // If the cross product is NEARLY zero,
        // Then one of the linesegments might have length zero
        if (both_lines_non_zero) {
            // If that's not the case, then we must have either:
            // - parallel lines, with no intersections, or
            // - coincident lines, with an infinite number of intersections
            // Either is quite useless, so we'll just bail out
            return false;
        } // Else, one of the linesegments is zero, and we might still be able to calculate a single intersection point
    } // Else we haven't bailed out, and we'll try to calculate the intersections

    double detinv = 1.0 / det;
    tA = cross(d, Bd) * detinv;
    tB = cross(d, Ad) * detinv;
    return (tA >= 0.) && (tA <= 1.) && (tB >= 0.) && (tB <= 1.);
}


#if 0
typedef union dbl_64{
    long long i64;
    double d64;
};

static double EpsilonOf(double value)
{
    dbl_64 s;
    s.d64 = value;
    if(s.i64 == 0)
    {
        s.i64++;
        return s.d64 - value;
    }
    else if(s.i64-- < 0)
        return s.d64 - value;
    else
        return value - s.d64;
}
#endif

#ifdef HAVE_GSL
struct rparams {
    Curve const &A;
    Curve const &B;
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

static void 
intersect_polish_root (Curve const &A, double &s, Curve const &B, double &t)
{
    std::vector<Point> as, bs;
    as = A.pointAndDerivatives(s, 2);
    bs = B.pointAndDerivatives(t, 2);
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

        if (ns<0) ns=0;
        else if (ns>1) ns=1;
        if (nt<0) nt=0;
        else if (nt>1) nt=1;
        
        as = A.pointAndDerivatives(ns, 2);
        bs = B.pointAndDerivatives(nt, 2);
        F = as[0] - bs[0];
        double trial = dot(F, F);
        if (trial > best*0.1) // we have standards, you know
            // At this point we could do a line search
            break;
        best = trial;
        s = ns;
        t = nt;
    }

#ifdef HAVE_GSL
    if(0) { // the GSL version is more accurate, but taints this with GPL
        int status;
        size_t iter = 0;
        const size_t n = 2;
        struct rparams p = {A, B};
        gsl_multiroot_function f = {&intersect_polish_f, n, &p};
     
        double x_init[2] = {s, t};
        gsl_vector *x = gsl_vector_alloc (n);
     
        gsl_vector_set (x, 0, x_init[0]);
        gsl_vector_set (x, 1, x_init[1]);
     
        const gsl_multiroot_fsolver_type *T = gsl_multiroot_fsolver_hybrids;
        gsl_multiroot_fsolver *sol = gsl_multiroot_fsolver_alloc (T, 2);
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
    }
#endif
}

/**
 * This uses the local bounds functions of curves to generically intersect two.
 * It passes in the curves, time intervals, and keeps track of depth, while
 * returning the results through the Crossings parameter.
 */
void pair_intersect(Curve const & A, double Al, double Ah, 
                    Curve const & B, double Bl, double Bh,
                    Crossings &ret,  unsigned depth = 0) {
   // std::cout << depth << "(" << Al << ", " << Ah << ")\n";
    OptRect Ar = A.boundsLocal(Interval(Al, Ah));
    if (!Ar) return;

    OptRect Br = B.boundsLocal(Interval(Bl, Bh));
    if (!Br) return;
    
    if(! Ar->intersects(*Br)) return;
    
    //Checks the general linearity of the function
    if((depth > 12)) { // || (A.boundsLocal(Interval(Al, Ah), 1).maxExtent() < 0.1 
                    //&&  B.boundsLocal(Interval(Bl, Bh), 1).maxExtent() < 0.1)) {
        double tA, tB, c;
        if(linear_intersect(A.pointAt(Al), A.pointAt(Ah), 
                            B.pointAt(Bl), B.pointAt(Bh), 
                            tA, tB, c)) {
            tA = tA * (Ah - Al) + Al;
            tB = tB * (Bh - Bl) + Bl;
            intersect_polish_root(A, tA,
                                  B, tB);
            if(depth % 2)
                ret.push_back(Crossing(tB, tA, c < 0));
            else
                ret.push_back(Crossing(tA, tB, c > 0));
            return;
        }
    }
    if(depth > 12) return;
    double mid = (Bl + Bh)/2;
    pair_intersect(B, Bl, mid,
                    A, Al, Ah,
                    ret, depth+1);
    pair_intersect(B, mid, Bh,
                    A, Al, Ah,
                    ret, depth+1);
}

Crossings pair_intersect(Curve const & A, Interval const &Ad,
                         Curve const & B, Interval const &Bd) {
    Crossings ret;
    pair_intersect(A, Ad.min(), Ad.max(), B, Bd.min(), Bd.max(), ret);
    return ret;
}

/** A simple wrapper around pair_intersect */
Crossings SimpleCrosser::crossings(Curve const &a, Curve const &b) {
    Crossings ret;
    pair_intersect(a, 0, 1, b, 0, 1, ret);
    return ret;
}


//same as below but curves not paths
void mono_intersect(Curve const &A, double Al, double Ah,
                    Curve const &B, double Bl, double Bh,
                    Crossings &ret, double tol = 0.1, unsigned depth = 0) {
    if( Al >= Ah || Bl >= Bh) return;
    //std::cout << " " << depth << "[" << Al << ", " << Ah << "]" << "[" << Bl << ", " << Bh << "]";

    Point A0 = A.pointAt(Al), A1 = A.pointAt(Ah),
          B0 = B.pointAt(Bl), B1 = B.pointAt(Bh);
    //inline code that this implies? (without rect/interval construction)
    Rect Ar = Rect(A0, A1), Br = Rect(B0, B1);
    if(!Ar.intersects(Br) || A0 == A1 || B0 == B1) return;

    if(depth > 12 || (Ar.maxExtent() < tol && Ar.maxExtent() < tol)) {
        double tA, tB, c;
        if(linear_intersect(A.pointAt(Al), A.pointAt(Ah), 
                            B.pointAt(Bl), B.pointAt(Bh), 
                            tA, tB, c)) {
            tA = tA * (Ah - Al) + Al;
            tB = tB * (Bh - Bl) + Bl;
            intersect_polish_root(A, tA,
                                  B, tB);
            if(depth % 2)
                ret.push_back(Crossing(tB, tA, c < 0));
            else
                ret.push_back(Crossing(tA, tB, c > 0));
            return;
        }
    }
    if(depth > 12) return;
    double mid = (Bl + Bh)/2;
    mono_intersect(B, Bl, mid,
              A, Al, Ah,
              ret, tol, depth+1);
    mono_intersect(B, mid, Bh,
              A, Al, Ah,
              ret, tol, depth+1);
}

Crossings mono_intersect(Curve const & A, Interval const &Ad,
                         Curve const & B, Interval const &Bd) {
    Crossings ret;
    mono_intersect(A, Ad.min(), Ad.max(), B, Bd.min(), Bd.max(), ret);
    return ret;
}

/**
 * Takes two paths and time ranges on them, with the invariant that the
 * paths are monotonic on the range.  Splits A when the linear intersection
 * doesn't exist or is inaccurate.  Uses the fact that it is monotonic to
 * do very fast local bounds.
 */
void mono_pair(Path const &A, double Al, double Ah,
               Path const &B, double Bl, double Bh,
               Crossings &ret, double /*tol*/, unsigned depth = 0) {
    if( Al >= Ah || Bl >= Bh) return;
    std::cout << " " << depth << "[" << Al << ", " << Ah << "]" << "[" << Bl << ", " << Bh << "]";

    Point A0 = A.pointAt(Al), A1 = A.pointAt(Ah),
          B0 = B.pointAt(Bl), B1 = B.pointAt(Bh);
    //inline code that this implies? (without rect/interval construction)
    Rect Ar = Rect(A0, A1), Br = Rect(B0, B1);
    if(!Ar.intersects(Br) || A0 == A1 || B0 == B1) return;

    if(depth > 12 || (Ar.maxExtent() < 0.1 && Ar.maxExtent() < 0.1)) {
        double tA, tB, c;
        if(linear_intersect(A0, A1, B0, B1,
                            tA, tB, c)) {
            tA = tA * (Ah - Al) + Al;
            tB = tB * (Bh - Bl) + Bl;
            if(depth % 2)
                ret.push_back(Crossing(tB, tA, c < 0));
            else
                ret.push_back(Crossing(tA, tB, c > 0));
            return;
        }
    }
    if(depth > 12) return;
    double mid = (Bl + Bh)/2;
    mono_pair(B, Bl, mid,
              A, Al, Ah,
              ret, depth+1);
    mono_pair(B, mid, Bh,
              A, Al, Ah,
              ret, depth+1);
}

/** This returns the times when the x or y derivative is 0 in the curve. */
std::vector<double> curve_mono_splits(Curve const &d) {
    Curve* deriv = d.derivative();
    std::vector<double> rs = deriv->roots(0, X);
    append(rs, deriv->roots(0, Y));
    delete deriv;
    std::sort(rs.begin(), rs.end());
    return rs;
}

/** Convenience function to add a value to each entry in a vector of doubles. */
std::vector<double> offset_doubles(std::vector<double> const &x, double offs) {
    std::vector<double> ret;
    for(unsigned i = 0; i < x.size(); i++) {
        ret.push_back(x[i] + offs);
    }
    return ret;
}

/**
 * Finds all the monotonic splits for a path.  Only includes the split between
 * curves if they switch derivative directions at that point.
 */
std::vector<double> path_mono_splits(Path const &p) {
    std::vector<double> ret;
    if(p.empty()) return ret;
    
    bool pdx=2, pdy=2;  //Previous derivative direction
    for(unsigned i = 0; i < p.size(); i++) {
        std::vector<double> spl = offset_doubles(curve_mono_splits(p[i]), i);
        bool dx = p[i].initialPoint()[X] > (spl.empty()? p[i].finalPoint()[X] :
                                                         p.valueAt(spl.front(), X));
        bool dy = p[i].initialPoint()[Y] > (spl.empty()? p[i].finalPoint()[Y] :
                                                         p.valueAt(spl.front(), Y));
        //The direction changed, include the split time
        if(dx != pdx || dy != pdy) {
            ret.push_back(i);
            pdx = dx; pdy = dy;
        }
        append(ret, spl);
    }
    return ret;
}

/**
 * Applies path_mono_splits to multiple paths, and returns the results such that 
 * time-set i corresponds to Path i.
 */
std::vector<std::vector<double> > paths_mono_splits(PathVector const &ps) {
    std::vector<std::vector<double> > ret;
    for(unsigned i = 0; i < ps.size(); i++)
        ret.push_back(path_mono_splits(ps[i]));
    return ret;
}

/**
 * Processes the bounds for a list of paths and a list of splits on them, yielding a list of rects for each.
 * Each entry i corresponds to path i of the input.  The number of rects in each entry is guaranteed to be the
 * number of splits for that path, subtracted by one.
 */
std::vector<std::vector<Rect> > split_bounds(PathVector const &p, std::vector<std::vector<double> > splits) {
    std::vector<std::vector<Rect> > ret;
    for(unsigned i = 0; i < p.size(); i++) {
        std::vector<Rect> res;
        for(unsigned j = 1; j < splits[i].size(); j++)
            res.push_back(Rect(p[i].pointAt(splits[i][j-1]), p[i].pointAt(splits[i][j])));
        ret.push_back(res);
    }
    return ret;
}

/**
 * This is the main routine of "MonoCrosser", and implements a monotonic strategy on multiple curves.
 * Finds crossings between two sets of paths, yielding a CrossingSet.  [0, a.size()) of the return correspond
 * to the sorted crossings of a with paths of b.  The rest of the return, [a.size(), a.size() + b.size()],
 * corresponds to the sorted crossings of b with paths of a.
 *
 * This function does two sweeps, one on the bounds of each path, and after that cull, one on the curves within.
 * This leads to a certain amount of code complexity, however, most of that is factored into the above functions
 */
CrossingSet MonoCrosser::crossings(PathVector const &a, PathVector const &b) {
    if(b.empty()) return CrossingSet(a.size(), Crossings());
    CrossingSet results(a.size() + b.size(), Crossings());
    if(a.empty()) return results;
    
    std::vector<std::vector<double> > splits_a = paths_mono_splits(a), splits_b = paths_mono_splits(b);
    std::vector<std::vector<Rect> > bounds_a = split_bounds(a, splits_a), bounds_b = split_bounds(b, splits_b);
    
    std::vector<Rect> bounds_a_union, bounds_b_union; 
    for(unsigned i = 0; i < bounds_a.size(); i++) bounds_a_union.push_back(union_list(bounds_a[i]));
    for(unsigned i = 0; i < bounds_b.size(); i++) bounds_b_union.push_back(union_list(bounds_b[i]));
    
    std::vector<std::vector<unsigned> > cull = sweep_bounds(bounds_a_union, bounds_b_union);
    Crossings n;
    for(unsigned i = 0; i < cull.size(); i++) {
        for(unsigned jx = 0; jx < cull[i].size(); jx++) {
            unsigned j = cull[i][jx];
            unsigned jc = j + a.size();
            Crossings res;
            
            //Sweep of the monotonic portions
            std::vector<std::vector<unsigned> > cull2 = sweep_bounds(bounds_a[i], bounds_b[j]);
            for(unsigned k = 0; k < cull2.size(); k++) {
                for(unsigned lx = 0; lx < cull2[k].size(); lx++) {
                    unsigned l = cull2[k][lx];
                    mono_pair(a[i], splits_a[i][k-1], splits_a[i][k],
                              b[j], splits_b[j][l-1], splits_b[j][l],
                              res, .1);
                }
            }
            
            for(unsigned k = 0; k < res.size(); k++) { res[k].a = i; res[k].b = jc; }
            
            merge_crossings(results[i], res, i);
            merge_crossings(results[i], res, jc);
        }
    }

    return results;
}

/* This function is similar codewise to the MonoCrosser, the main difference is that it deals with
 * only one set of paths and includes self intersection
CrossingSet crossings_among(PathVector const &p) {
    CrossingSet results(p.size(), Crossings());
    if(p.empty()) return results;
    
    std::vector<std::vector<double> > splits = paths_mono_splits(p);
    std::vector<std::vector<Rect> > prs = split_bounds(p, splits);
    std::vector<Rect> rs;
    for(unsigned i = 0; i < prs.size(); i++) rs.push_back(union_list(prs[i]));
    
    std::vector<std::vector<unsigned> > cull = sweep_bounds(rs);
    
    //we actually want to do the self-intersections, so add em in:
    for(unsigned i = 0; i < cull.size(); i++) cull[i].push_back(i);
    
    for(unsigned i = 0; i < cull.size(); i++) {
        for(unsigned jx = 0; jx < cull[i].size(); jx++) {
            unsigned j = cull[i][jx];
            Crossings res;
            
            //Sweep of the monotonic portions
            std::vector<std::vector<unsigned> > cull2 = sweep_bounds(prs[i], prs[j]);
            for(unsigned k = 0; k < cull2.size(); k++) {
                for(unsigned lx = 0; lx < cull2[k].size(); lx++) {
                    unsigned l = cull2[k][lx];
                    mono_pair(p[i], splits[i][k-1], splits[i][k],
                              p[j], splits[j][l-1], splits[j][l],
                              res, .1);
                }
            }
            
            for(unsigned k = 0; k < res.size(); k++) { res[k].a = i; res[k].b = j; }
            
            merge_crossings(results[i], res, i);
            merge_crossings(results[j], res, j);
        }
    }
    
    return results;
}
*/


Crossings curve_self_crossings(Curve const &a) {
    Crossings res;
    std::vector<double> spl;
    spl.push_back(0);
    append(spl, curve_mono_splits(a));
    spl.push_back(1);
    for(unsigned i = 1; i < spl.size(); i++)
        for(unsigned j = i+1; j < spl.size(); j++)
            pair_intersect(a, spl[i-1], spl[i], a, spl[j-1], spl[j], res);
    return res;
}

/*
void mono_curve_intersect(Curve const & A, double Al, double Ah, 
                          Curve const & B, double Bl, double Bh,
                          Crossings &ret,  unsigned depth=0) {
   // std::cout << depth << "(" << Al << ", " << Ah << ")\n";
    Point A0 = A.pointAt(Al), A1 = A.pointAt(Ah),
          B0 = B.pointAt(Bl), B1 = B.pointAt(Bh);
    //inline code that this implies? (without rect/interval construction)
    if(!Rect(A0, A1).intersects(Rect(B0, B1)) || A0 == A1 || B0 == B1) return;
     
    //Checks the general linearity of the function
    if((depth > 12) || (A.boundsLocal(Interval(Al, Ah), 1).maxExtent() < 0.1 
                    &&  B.boundsLocal(Interval(Bl, Bh), 1).maxExtent() < 0.1)) {
        double tA, tB, c;
        if(linear_intersect(A0, A1, B0, B1, tA, tB, c)) {
            tA = tA * (Ah - Al) + Al;
            tB = tB * (Bh - Bl) + Bl;
            if(depth % 2)
                ret.push_back(Crossing(tB, tA, c < 0));
            else
                ret.push_back(Crossing(tA, tB, c > 0));
            return;
        }
    }
    if(depth > 12) return;
    double mid = (Bl + Bh)/2;
    mono_curve_intersect(B, Bl, mid,
                         A, Al, Ah,
                         ret, depth+1);
    mono_curve_intersect(B, mid, Bh,
                         A, Al, Ah,
                         ret, depth+1);
}

std::vector<std::vector<double> > curves_mono_splits(Path const &p) {
    std::vector<std::vector<double> > ret;
    for(unsigned i = 0; i <= p.size(); i++) {
        std::vector<double> spl;
        spl.push_back(0);
        append(spl, curve_mono_splits(p[i]));
        spl.push_back(1);
        ret.push_back(spl);
    }
}

std::vector<std::vector<Rect> > curves_split_bounds(Path const &p, std::vector<std::vector<double> > splits) {
    std::vector<std::vector<Rect> > ret;
    for(unsigned i = 0; i < splits.size(); i++) {
        std::vector<Rect> res;
        for(unsigned j = 1; j < splits[i].size(); j++)
            res.push_back(Rect(p.pointAt(splits[i][j-1]+i), p.pointAt(splits[i][j]+i)));
        ret.push_back(res);
    }
    return ret;
}

Crossings path_self_crossings(Path const &p) {
    Crossings ret;
    std::vector<std::vector<unsigned> > cull = sweep_bounds(bounds(p));
    std::vector<std::vector<double> > spl = curves_mono_splits(p);
    std::vector<std::vector<Rect> > bnds = curves_split_bounds(p, spl);
    for(unsigned i = 0; i < cull.size(); i++) {
        Crossings res;
        for(unsigned k = 1; k < spl[i].size(); k++)
            for(unsigned l = k+1; l < spl[i].size(); l++)
                mono_curve_intersect(p[i], spl[i][k-1], spl[i][k], p[i], spl[i][l-1], spl[i][l], res);
        offset_crossings(res, i, i);
        append(ret, res);
        for(unsigned jx = 0; jx < cull[i].size(); jx++) {
            unsigned j = cull[i][jx];
            res.clear();
            
            std::vector<std::vector<unsigned> > cull2 = sweep_bounds(bnds[i], bnds[j]);
            for(unsigned k = 0; k < cull2.size(); k++) {
                for(unsigned lx = 0; lx < cull2[k].size(); lx++) {
                    unsigned l = cull2[k][lx];
                    mono_curve_intersect(p[i], spl[i][k-1], spl[i][k], p[j], spl[j][l-1], spl[j][l], res);
                }
            }
            
            //if(fabs(int(i)-j) == 1 || fabs(int(i)-j) == p.size()-1) {
                Crossings res2;
                for(unsigned k = 0; k < res.size(); k++) {
                    if(res[k].ta != 0 && res[k].ta != 1 && res[k].tb != 0 && res[k].tb != 1) {
                        res.push_back(res[k]);
                    }
                }
                res = res2;
            //}
            offset_crossings(res, i, j);
            append(ret, res);
        }
    }
    return ret;
}
*/

Crossings self_crossings(Path const &p) {
    Crossings ret;
    std::vector<std::vector<unsigned> > cull = sweep_bounds(bounds(p));
    for(unsigned i = 0; i < cull.size(); i++) {
        Crossings res = curve_self_crossings(p[i]);
        offset_crossings(res, i, i);
        append(ret, res);
        for(unsigned jx = 0; jx < cull[i].size(); jx++) {
            unsigned j = cull[i][jx];
            res.clear();
            pair_intersect(p[i], 0, 1, p[j], 0, 1, res);
            
            //if(fabs(int(i)-j) == 1 || fabs(int(i)-j) == p.size()-1) {
                Crossings res2;
                for(unsigned k = 0; k < res.size(); k++) {
                    if(res[k].ta != 0 && res[k].ta != 1 && res[k].tb != 0 && res[k].tb != 1) {
                        res2.push_back(res[k]);
                    }
                }
                res = res2;
            //}
            offset_crossings(res, i, j);
            append(ret, res);
        }
    }
    return ret;
}

void flip_crossings(Crossings &crs) {
    for(unsigned i = 0; i < crs.size(); i++)
        crs[i] = Crossing(crs[i].tb, crs[i].ta, crs[i].b, crs[i].a, !crs[i].dir);
}

CrossingSet crossings_among(PathVector const &p) {
    CrossingSet results(p.size(), Crossings());
    if(p.empty()) return results;
    
    SimpleCrosser cc;
    
    std::vector<std::vector<unsigned> > cull = sweep_bounds(bounds(p));
    for(unsigned i = 0; i < cull.size(); i++) {
        Crossings res = self_crossings(p[i]);
        for(unsigned k = 0; k < res.size(); k++) { res[k].a = res[k].b = i; }
        merge_crossings(results[i], res, i);
        flip_crossings(res);
        merge_crossings(results[i], res, i);
        for(unsigned jx = 0; jx < cull[i].size(); jx++) {
            unsigned j = cull[i][jx];
            
            Crossings res = cc.crossings(p[i], p[j]);
            for(unsigned k = 0; k < res.size(); k++) { res[k].a = i; res[k].b = j; }
            merge_crossings(results[i], res, i);
            merge_crossings(results[j], res, j);
        }
    }
    return results;
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

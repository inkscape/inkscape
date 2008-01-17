/* From Sanchez-Reyes 1997
   W_{j,k} = W_{n0j, n-k} = choose(n-2k-1, j-k)choose(2k+1,k)/choose(n,j)
     for k=0,...,q-1; j = k, ...,n-k-1
   W_{q,q} = 1 (n even)

This is wrong, it should read
   W_{j,k} = W_{n0j, n-k} = choose(n-2k-1, j-k)/choose(n,j)
     for k=0,...,q-1; j = k, ...,n-k-1
   W_{q,q} = 1 (n even)

*/
#include "sbasis-to-bezier.h"
#include "choose.h"
#include "svg-path.h"
#include <iostream>
#include "exception.h"

namespace Geom{

double W(unsigned n, unsigned j, unsigned k) {
    unsigned q = (n+1)/2;
    if((n & 1) == 0 && j == q && k == q)
        return 1;
    if(k > n-k) return W(n, n-j, n-k);
    assert((k <= q));
    if(k >= q) return 0;
    //assert(!(j >= n-k));
    if(j >= n-k) return 0;
    //assert(!(j < k));
    if(j < k) return 0;
    return choose<double>(n-2*k-1, j-k) /
        choose<double>(n,j);
}

// this produces a degree 2q bezier from a degree k sbasis
Bezier
sbasis_to_bezier(SBasis const &B, unsigned q) {
    if(q == 0) {
        q = B.size();
        /*if(B.back()[0] == B.back()[1]) {
            n--;
            }*/
    }
    unsigned n = q*2;
    Bezier result = Bezier(Bezier::Order(n-1));
    if(q > B.size())
        q = B.size();
    n--;
    for(unsigned k = 0; k < q; k++) {
        for(unsigned j = 0; j <= n-k; j++) {
            result[j] += (W(n, j, k)*B[k][0] +
                          W(n, n-j, k)*B[k][1]);
        }
    }
    return result;
}

double mopi(int i) {
    return (i&1)?-1:1;
}

// this produces a degree k sbasis from a degree 2q bezier
SBasis
bezier_to_sbasis(Bezier const &B) {
    unsigned n = B.size();
    unsigned q = (n+1)/2;
    SBasis result;
    result.resize(q+1);
    for(unsigned k = 0; k < q; k++) {
        result[k][0] = result[k][1] = 0;
        for(unsigned j = 0; j <= n-k; j++) {
            result[k][0] += mopi(int(j)-int(k))*W(n, j, k)*B[j];
            result[k][1] += mopi(int(j)-int(k))*W(n, j, k)*B[j];
            //W(n, n-j, k)*B[k][1]);
        }
    }
    return result;
}

// this produces a 2q point bezier from a degree q sbasis
std::vector<Geom::Point>
sbasis_to_bezier(D2<SBasis> const &B, unsigned qq) {
    std::vector<Geom::Point> result;
    if(qq == 0) {
        qq = sbasis_size(B);
    }
    unsigned n = qq * 2;
    result.resize(n, Geom::Point(0,0));
    n--;
    for(unsigned dim = 0; dim < 2; dim++) {
        unsigned q = qq;
        if(q > B[dim].size())
            q = B[dim].size();
        for(unsigned k = 0; k < q; k++) {
            for(unsigned j = 0; j <= n-k; j++) {
                result[j][dim] += (W(n, j, k)*B[dim][k][0] +
                             W(n, n-j, k)*B[dim][k][1]);
                }
        }
    }
    return result;
}
/*
template <unsigned order>
D2<Bezier<order> > sbasis_to_bezier(D2<SBasis> const &B) {
    return D2<Bezier<order> >(sbasis_to_bezier<order>(B[0]), sbasis_to_bezier<order>(B[1]));
}
*/

#if 0 // using old path
//std::vector<Geom::Point>
// mutating
void
subpath_from_sbasis(Geom::OldPathSetBuilder &pb, D2<SBasis> const &B, double tol, bool initial) {
    assert(B.is_finite());
    if(B.tail_error(2) < tol || B.size() == 2) { // nearly cubic enough
        if(B.size() == 1) {
            if (initial) {
                pb.start_subpath(Geom::Point(B[0][0][0], B[1][0][0]));
            }
            pb.push_line(Geom::Point(B[0][0][1], B[1][0][1]));
        } else {
            std::vector<Geom::Point> bez = sbasis_to_bezier(B, 2);
            if (initial) {
                pb.start_subpath(bez[0]);
            }
            pb.push_cubic(bez[1], bez[2], bez[3]);
        }
    } else {
        subpath_from_sbasis(pb, compose(B, Linear(0, 0.5)), tol, initial);
        subpath_from_sbasis(pb, compose(B, Linear(0.5, 1)), tol, false);
    }
}

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
    assert(B[0].is_finite());
    assert(B[1].is_finite());
    
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

void build_from_sbasis(Geom::PathBuilder &pb, D2<SBasis> const &B, double tol) {
    if (!B.isFinite()) {
        throwException("assertion failed: B.isFinite()");
    }
    if(tail_error(B, 2) < tol || sbasis_size(B) == 2) { // nearly cubic enough
        if(sbasis_size(B) <= 1) {
            pb.lineTo(B.at1());
        } else {
            std::vector<Geom::Point> bez = sbasis_to_bezier(B, 2);
            pb.curveTo(bez[1], bez[2], bez[3]);
        }
    } else {
        build_from_sbasis(pb, compose(B, Linear(0, 0.5)), tol);
        build_from_sbasis(pb, compose(B, Linear(0.5, 1)), tol);
    }
}

Path
path_from_sbasis(D2<SBasis> const &B, double tol) {
    PathBuilder pb;
    pb.moveTo(B.at0());
    build_from_sbasis(pb, B, tol);
    pb.finish();
    return pb.peek().front();
}

//TODO: some of this logic should be lifted into svg-path
std::vector<Geom::Path>
path_from_piecewise(Geom::Piecewise<Geom::D2<Geom::SBasis> > const &B, double tol) {
    Geom::PathBuilder pb;
    if(B.size() == 0) return pb.peek();
    Geom::Point start = B[0].at0();
    pb.moveTo(start);
    for(unsigned i = 0; ; i++) {
        if(i+1 == B.size() || !are_near(B[i+1].at0(), B[i].at1(), tol)) {
            //start of a new path
            if(are_near(start, B[i].at1()) && sbasis_size(B[i]) <= 1) {
                //last line seg already there
                goto no_add;
            }
            build_from_sbasis(pb, B[i], tol);
            if(are_near(start, B[i].at1())) {
                //it's closed
                pb.closePath();
            }
          no_add:
            if(i+1 >= B.size()) break;
            start = B[i+1].at0();
            pb.moveTo(start);
        } else {
            build_from_sbasis(pb, B[i], tol);
        }
    }
    pb.finish();
    return pb.peek();
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

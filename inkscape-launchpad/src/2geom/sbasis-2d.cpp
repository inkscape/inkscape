#include <2geom/sbasis-2d.h>
#include <2geom/sbasis-geometric.h>

namespace Geom{

SBasis extract_u(SBasis2d const &a, double u) {
    SBasis sb(a.vs, Linear());
    double s = u*(1-u);
    
    for(unsigned vi = 0; vi < a.vs; vi++) {
        double sk = 1;
        Linear bo(0,0);
        for(unsigned ui = 0; ui < a.us; ui++) {
            bo += (extract_u(a.index(ui, vi), u))*sk;
            sk *= s;
        }
        sb[vi] = bo;
    }
    
    return sb;
}

SBasis extract_v(SBasis2d const &a, double v) {
    SBasis sb(a.us, Linear());
    double s = v*(1-v);
    
    for(unsigned ui = 0; ui < a.us; ui++) {
        double sk = 1;
        Linear bo(0,0);
        for(unsigned vi = 0; vi < a.vs; vi++) {
            bo += (extract_v(a.index(ui, vi), v))*sk;
            sk *= s;
        }
        sb[ui] = bo;
    }
    
    return sb;
}

SBasis compose(Linear2d const &a, D2<SBasis> const &p) {
    D2<SBasis> omp(-p[X] + 1, -p[Y] + 1);
    return multiply(omp[0], omp[1])*a[0] +
           multiply(p[0], omp[1])*a[1] +
           multiply(omp[0], p[1])*a[2] +
           multiply(p[0], p[1])*a[3];
}

SBasis 
compose(SBasis2d const &fg, D2<SBasis> const &p) {
    SBasis B;
    SBasis s[2];
    SBasis ss[2];
    for(unsigned dim = 0; dim < 2; dim++) 
        s[dim] = p[dim]*(Linear(1) - p[dim]);
    ss[1] = Linear(1);
    for(unsigned vi = 0; vi < fg.vs; vi++) {
        ss[0] = ss[1];
        for(unsigned ui = 0; ui < fg.us; ui++) {
            unsigned i = ui + vi*fg.us;
            B += ss[0]*compose(fg[i], p);
            ss[0] *= s[0];
        }
        ss[1] *= s[1];
    }
    return B;
}

D2<SBasis>
compose_each(D2<SBasis2d> const &fg, D2<SBasis> const &p) {
    return D2<SBasis>(compose(fg[X], p), compose(fg[Y], p));
}

SBasis2d partial_derivative(SBasis2d const &f, int dim) {
    SBasis2d result;
    for(unsigned i = 0; i < f.size(); i++) {
        result.push_back(Linear2d(0,0,0,0));
    }
    result.us = f.us;
    result.vs = f.vs;

    for(unsigned i = 0; i < f.us; i++) {
        for(unsigned j = 0; j < f.vs; j++) {
            Linear2d lin = f.index(i,j);
            Linear2d dlin(lin[1+dim]-lin[0], lin[1+2*dim]-lin[dim], lin[3-dim]-lin[2*(1-dim)], lin[3]-lin[2-dim]);
            result[i+j*result.us] += dlin;
            unsigned di = dim?j:i;
            if (di>=1){
                float motpi = dim?-1:1;
                Linear2d ds_lin_low( lin[0], -motpi*lin[1], motpi*lin[2], -lin[3] );
                result[(i+dim-1)+(j-dim)*result.us] += di*ds_lin_low;
                
                Linear2d ds_lin_hi( lin[1+dim]-lin[0], lin[1+2*dim]-lin[dim], lin[3]-lin[2-dim], lin[3-dim]-lin[2-dim] );
                result[i+j*result.us] += di*ds_lin_hi;                
            }
        }
    }
    return result;
}

/**
 * Finds a path which traces the 0 contour of f, traversing from A to B as a single d2<sbasis>.
 * degmax specifies the degree (degree = 2*degmax-1, so a degmax of 2 generates a cubic fit).
 * The algorithm is based on dividing out derivatives at each end point and does not use the curvature for fitting.
 * It is less accurate than sb2d_cubic_solve, although this may be fixed in the future. 
 */
D2<SBasis>
sb2dsolve(SBasis2d const &f, Geom::Point const &A, Geom::Point const &B, unsigned degmax){
    //g_warning("check f(A)= %f = f(B) = %f =0!", f.apply(A[X],A[Y]), f.apply(B[X],B[Y]));

    SBasis2d dfdu = partial_derivative(f, 0);
    SBasis2d dfdv = partial_derivative(f, 1);
    Geom::Point dfA(dfdu.apply(A[X],A[Y]),dfdv.apply(A[X],A[Y]));
    Geom::Point dfB(dfdu.apply(B[X],B[Y]),dfdv.apply(B[X],B[Y]));
    Geom::Point nA = dfA/(dfA[X]*dfA[X]+dfA[Y]*dfA[Y]);
    Geom::Point nB = dfB/(dfB[X]*dfB[X]+dfB[Y]*dfB[Y]);

    D2<SBasis>result(SBasis(degmax, Linear()), SBasis(degmax, Linear()));
    double fact_k=1;
    double sign = 1.;
    for(int dim = 0; dim < 2; dim++)
        result[dim][0] = Linear(A[dim],B[dim]);
    for(unsigned k=1; k<degmax; k++){
        // these two lines make the solutions worse!
        //fact_k *= k;
        //sign = -sign;
        SBasis f_on_curve = compose(f,result);
        Linear reste = f_on_curve[k];
        double ax = -reste[0]/fact_k*nA[X];
        double ay = -reste[0]/fact_k*nA[Y];
        double bx = -sign*reste[1]/fact_k*nB[X];
        double by = -sign*reste[1]/fact_k*nB[Y];

        result[X][k] = Linear(ax,bx);
        result[Y][k] = Linear(ay,by);
        //sign *= 3;
    }    
    return result;
}

/**
 * Finds a path which traces the 0 contour of f, traversing from A to B as a single cubic d2<sbasis>.
 * The algorithm is based on matching direction and curvature at each end point.
 */
//TODO: handle the case when B is "behind" A for the natural orientation of the level set.
//TODO: more generally, there might be up to 4 solutions. Choose the best one!
D2<SBasis>
sb2d_cubic_solve(SBasis2d const &f, Geom::Point const &A, Geom::Point const &B){
    D2<SBasis>result;//(Linear(A[X],B[X]),Linear(A[Y],B[Y]));
    //g_warning("check 0 = %f = %f!", f.apply(A[X],A[Y]), f.apply(B[X],B[Y]));

    SBasis2d f_u  = partial_derivative(f  , 0);
    SBasis2d f_v  = partial_derivative(f  , 1);
    SBasis2d f_uu = partial_derivative(f_u, 0);
    SBasis2d f_uv = partial_derivative(f_v, 0);
    SBasis2d f_vv = partial_derivative(f_v, 1);

    Geom::Point dfA(f_u.apply(A[X],A[Y]),f_v.apply(A[X],A[Y]));
    Geom::Point dfB(f_u.apply(B[X],B[Y]),f_v.apply(B[X],B[Y]));

    Geom::Point V0 = rot90(dfA);
    Geom::Point V1 = rot90(dfB);
    
    double D2fVV0 = f_uu.apply(A[X],A[Y])*V0[X]*V0[X]+
                  2*f_uv.apply(A[X],A[Y])*V0[X]*V0[Y]+
                    f_vv.apply(A[X],A[Y])*V0[Y]*V0[Y];
    double D2fVV1 = f_uu.apply(B[X],B[Y])*V1[X]*V1[X]+
                  2*f_uv.apply(B[X],B[Y])*V1[X]*V1[Y]+
                    f_vv.apply(B[X],B[Y])*V1[Y]*V1[Y];

    std::vector<D2<SBasis> > candidates = cubics_fitting_curvature(A,B,V0,V1,D2fVV0,D2fVV1);
    if (candidates.empty()) {
        return D2<SBasis>(SBasis(Linear(A[X],B[X])), SBasis(Linear(A[Y],B[Y])));
    }
    //TODO: I'm sure std algorithm could do that for me...
    double error = -1;
    unsigned best = 0;
    for (unsigned i=0; i<candidates.size(); i++){
        Interval bounds = *bounds_fast(compose(f,candidates[i]));
        double new_error = (fabs(bounds.max())>fabs(bounds.min()) ? fabs(bounds.max()) : fabs(bounds.min()) );
        if ( new_error < error || error < 0 ){
            error = new_error;
            best = i;
        }
    }
    return candidates[best];
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

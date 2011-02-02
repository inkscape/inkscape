#include <2geom/chebyshev.h>

#include <2geom/sbasis.h>
#include <2geom/sbasis-poly.h>

#include <vector>
using std::vector;

#include <gsl/gsl_math.h>
#include <gsl/gsl_chebyshev.h>

namespace Geom{

SBasis cheb(unsigned n) {
    static std::vector<SBasis> basis;
    if(basis.empty()) {
        basis.push_back(Linear(1,1));
        basis.push_back(Linear(0,1));
    }
    for(unsigned i = basis.size(); i <= n; i++) {
        basis.push_back(Linear(0,2)*basis[i-1] - basis[i-2]);
    }
    
    return basis[n];
}

SBasis cheb_series(unsigned n, double* cheb_coeff) {
    SBasis r;
    for(unsigned i = 0; i < n; i++) {
        double cof = cheb_coeff[i];
        //if(i == 0)
        //cof /= 2;
        r += cheb(i)*cof;
    }
    
    return r;
}

SBasis clenshaw_series(unsigned m, double* cheb_coeff) {
    /** b_n = a_n
        b_n-1 = 2*x*b_n + a_n-1
        b_n-k = 2*x*b_{n-k+1} + a_{n-k} - b_{n - k + 2}
        b_0 = x*b_1 + a_0 - b_2
    */
    
    double a = -1, b = 1;
    SBasis d, dd;
    SBasis y = (Linear(0, 2) - (a+b)) / (b-a);
    SBasis y2 = 2*y;
    for(int j = m - 1; j >= 1; j--) {
        SBasis sv = d;
        d = y2*d - dd + cheb_coeff[j];
        dd = sv;
    }
    
    return y*d - dd + 0.5*cheb_coeff[0];
}

SBasis chebyshev_approximant (double (*f)(double,void*), int order, Interval in, void* p) {
    gsl_cheb_series *cs = gsl_cheb_alloc (order+2);

    gsl_function F;

    F.function = f;
    F.params = p;

    gsl_cheb_init (cs, &F, in[0], in[1]);
    
    SBasis r = compose(clenshaw_series(order, cs->c), Linear(-1,1));
        
    gsl_cheb_free (cs);
    return r;
}

struct wrap {
    double (*f)(double,void*);
    void* pp;
    double fa, fb;
    Interval in;
};

double f_interp(double x, void* p) {
    struct wrap *wr = (struct wrap *)p;
    double z = (x - wr->in[0]) / (wr->in[1] - wr->in[0]);
    return (wr->f)(x, wr->pp) - ((1 - z)*wr->fa + z*wr->fb);
}

SBasis chebyshev_approximant_interpolating (double (*f)(double,void*), 
                                            int order, Interval in, void* p) {
    double fa = f(in[0], p);
    double fb = f(in[1], p);
    struct wrap wr;
    wr.fa = fa;
    wr.fb = fb;
    wr.in = in;
    printf("%f %f\n", fa, fb);
    wr.f = f;
    wr.pp = p;
    return compose(Linear(in[0], in[1]), Linear(fa, fb)) + chebyshev_approximant(f_interp, order, in, &wr) + Linear(fa, fb);
}

SBasis chebyshev(unsigned n) {
    static std::vector<SBasis> basis;
    if(basis.empty()) {
        basis.push_back(Linear(1,1));
        basis.push_back(Linear(0,1));
    }
    for(unsigned i = basis.size(); i <= n; i++) {
        basis.push_back(Linear(0,2)*basis[i-1] - basis[i-2]);
    }
    
    return basis[n];
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

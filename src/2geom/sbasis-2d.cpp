#include "sbasis-2d.h"

namespace Geom{

SBasis extract_u(SBasis2d const &a, double u) {
    SBasis sb;
    double s = u*(1-u);
    
    for(unsigned vi = 0; vi < a.vs; vi++) {
        double sk = 1;
        Linear bo(0,0);
        for(unsigned ui = 0; ui < a.us; ui++) {
            bo += (extract_u(a.index(ui, vi), u))*sk;
            sk *= s;
        }
        sb.push_back(bo);
    }
    
    return sb;
}

SBasis extract_v(SBasis2d const &a, double v) {
    SBasis sb;
    double s = v*(1-v);
    
    for(unsigned ui = 0; ui < a.us; ui++) {
        double sk = 1;
        Linear bo(0,0);
        for(unsigned vi = 0; vi < a.vs; vi++) {
            bo += (extract_v(a.index(ui, vi), v))*sk;
            sk *= s;
        }
        sb.push_back(bo);
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

};

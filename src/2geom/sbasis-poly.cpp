#include <2geom/sbasis-poly.h>

namespace Geom{

/** Changes the basis of p to be sbasis.
 \param p the Monomial basis polynomial
 \returns the Symmetric basis polynomial

This algorithm is horribly slow and numerically terrible.  Only for testing.
*/
SBasis poly_to_sbasis(Poly const & p) {
    SBasis x = Linear(0, 1);
    SBasis r;
    
    for(int i = p.size()-1; i >= 0; i--) {
        r = SBasis(Linear(p[i], p[i])) + multiply(x, r);
    }
    r.normalize();
    return r;
	
}

/** Changes the basis of p to be monomial.
 \param p the Symmetric basis polynomial
 \returns the Monomial basis polynomial

This algorithm is horribly slow and numerically terrible.  Only for testing.
*/
Poly sbasis_to_poly(SBasis const & sb) {
    if(sb.isZero())
        return Poly();
    Poly S; // (1-x)x = -1*x^2 + 1*x + 0
    Poly A, B;
    B.push_back(0);
    B.push_back(1);
    A.push_back(1);
    A.push_back(-1);
    S = A*B;
    Poly r;
    
    for(int i = sb.size()-1; i >= 0; i--) {
        r = S*r + sb[i][0]*A + sb[i][1]*B;
    }
    r.normalize();
    return r;
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

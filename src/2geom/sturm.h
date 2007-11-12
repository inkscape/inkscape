#ifndef LIB2GEOM_STURM_HEADER
#define LIB2GEOM_STURM_HEADER

#include "poly.h"
#include "utils.h"

namespace Geom {

class sturm : public std::vector<Poly>{
public:
    sturm(Poly const &X) {
        push_back(X);
        push_back(derivative(X));
        Poly Xi = back();
        Poly Xim1 = X;
        std::cout << "sturm:\n" << Xim1 << std::endl;
        std::cout << Xi << std::endl;
        while(Xi.size() > 1) {
            Poly r;
            divide(Xim1, Xi, r);
            std::cout << r << std::endl;
            assert(r.size() < Xi.size());
            Xim1 = Xi;
            Xi = -r;
            assert(Xim1.size() > Xi.size());
            push_back(Xi);
        }
    }
    
    unsigned count_signs(double t) {
        unsigned n_signs = 0;/*  Number of sign-changes */
        const double big = 1e20; // a number such that practical polys would overflow on evaluation
        if(t >= big) {
            int old_sign = sgn((*this)[0].back());
            for (unsigned i = 1; i < size(); i++) {
                int sign = sgn((*this)[i].back());
                if (sign != old_sign)
                    n_signs++;
                old_sign = sign;
            }
        } else {
            int old_sign = sgn((*this)[0].eval(t));
            for (unsigned i = 1; i < size(); i++) {
                int sign = sgn((*this)[i].eval(t));
                if (sign != old_sign)
                    n_signs++;
                old_sign = sign;
            }
        }
        return n_signs;
    }
    
    unsigned n_roots_between(double l, double r) {
        return count_signs(l) - count_signs(r);
    }
};

} //namespace Geom

#endif
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

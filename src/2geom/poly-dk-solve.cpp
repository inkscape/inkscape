#include "poly-dk-solve.h"
#include <iterator>

/*** implementation of the Durand-Kerner method.  seems buggy*/

std::complex<double> evalu(Poly const & p, std::complex<double> x) {
    std::complex<double> result = 0;
    std::complex<double> xx = 1;

    for(unsigned i = 0; i < p.size(); i++) {
        result += p[i]*xx;
        xx *= x;
    }
    return result;
}

std::vector<std::complex<double> > DK(Poly const & ply, const double tol) {
    std::vector<std::complex<double> > roots;
    const int N = ply.degree();

    std::complex<double> b(0.4, 0.9);
    std::complex<double> p = 1;
    for(int i = 0; i < N; i++) {
        roots.push_back(p);
        p *= b;
    }
    assert(roots.size() == ply.degree());

    double error = 0;
    int i;
    for( i = 0; i < 30; i++) {
        error = 0;
        for(int r_i = 0; r_i < N; r_i++) {
            std::complex<double> denom = 1;
            std::complex<double> R = roots[r_i];
            for(int d_i = 0; d_i < N; d_i++) {
                if(r_i != d_i)
                    denom *= R-roots[d_i];
            }
            assert(norm(denom) != 0);
            std::complex<double> dr = evalu(ply, R)/denom;
            error += norm(dr);
            roots[r_i] = R - dr;
        }
        /*std::copy(roots.begin(), roots.end(), std::ostream_iterator<std::complex<double> >(std::cout, ",\t"));
          std::cout << std::endl;*/
        if(error < tol)
            break;
    }
    //std::cout << error << ", " << i<< std::endl;
    return roots;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

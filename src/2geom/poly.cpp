#include "poly.h"

Poly Poly::operator*(const Poly& p) const {
    Poly result; 
    result.resize(degree() +  p.degree()+1);
    
    for(unsigned i = 0; i < size(); i++) {
        for(unsigned j = 0; j < p.size(); j++) {
            result[i+j] += (*this)[i] * p[j];
        }
    }
    return result;
}
#ifdef HAVE_GSL
#include <gsl/gsl_poly.h>
#endif

/*double Poly::eval(double x) const {
    return gsl_poly_eval(&coeff[0], size(), x);
    }*/

void Poly::normalize() {
    while(back() == 0)
        pop_back();
}

void Poly::monicify() {
    normalize();
    
    double scale = 1./back(); // unitize
    
    for(unsigned i = 0; i < size(); i++) {
        (*this)[i] *= scale;
    }
}


#ifdef HAVE_GSL
std::vector<std::complex<double> > solve(Poly const & pp) {
    Poly p(pp);
    p.normalize();
    gsl_poly_complex_workspace * w 
        = gsl_poly_complex_workspace_alloc (p.size());
       
    gsl_complex_packed_ptr z = new double[p.degree()*2];
    double* a = new double[p.size()];
    for(int i = 0; i < p.size(); i++)
        a[i] = p[i];
    std::vector<std::complex<double> > roots;
    //roots.resize(p.degree());
    
    gsl_poly_complex_solve (a, p.size(), w, z);
    delete[]a;
     
    gsl_poly_complex_workspace_free (w);
     
    for (int i = 0; i < p.degree(); i++) {
        roots.push_back(std::complex<double> (z[2*i] ,z[2*i+1]));
        //printf ("z%d = %+.18f %+.18f\n", i, z[2*i], z[2*i+1]);
    }    
    delete[] z;
    return roots;
}

std::vector<double > solve_reals(Poly const & p) {
    std::vector<std::complex<double> > roots = solve(p);
    std::vector<double> real_roots;
    
    for(int i = 0; i < roots.size(); i++) {
        if(roots[i].imag() == 0) // should be more lenient perhaps
            real_roots.push_back(roots[i].real());
    }
    return real_roots;
}
#endif

double polish_root(Poly const & p, double guess, double tol) {
    Poly dp = derivative(p);
    
    double fn = p(guess);
    while(fabs(fn) > tol) {
        guess -= fn/dp(guess);
        fn = p(guess);
    }
    return guess;
}

Poly integral(Poly const & p) {
    Poly result;
    
    result.reserve(p.size()+1);
    result.push_back(0); // arbitrary const
    for(unsigned i = 0; i < p.size(); i++) {
        result.push_back(p[i]/(i+1));
    }
    return result;

}

Poly derivative(Poly const & p) {
    Poly result;
    
    if(p.size() <= 1)
        return Poly(0);
    result.reserve(p.size()-1);
    for(unsigned i = 1; i < p.size(); i++) {
        result.push_back(i*p[i]);
    }
    return result;
}

Poly compose(Poly const & a, Poly const & b) {
    Poly result;
    
    for(unsigned i = a.size(); i > 0; i--) {
        result = Poly(a[i-1]) + result * b;
    }
    return result;
    
}

/* This version is backwards - dividing taylor terms
Poly divide(Poly const &a, Poly const &b, Poly &r) {
    Poly c;
    r = a; // remainder
    
    const unsigned k = a.size();
    r.resize(k, 0);
    c.resize(k, 0);

    for(unsigned i = 0; i < k; i++) {
        double ci = r[i]/b[0];
        c[i] += ci;
        Poly bb = ci*b;
        std::cout << ci <<"*" << b << ", r= " << r << std::endl;
        r -= bb.shifted(i);
    }
    
    return c;
}
*/

Poly divide(Poly const &a, Poly const &b, Poly &r) {
    Poly c;
    r = a; // remainder
    assert(b.size() > 0);
    
    const unsigned k = a.degree();
    const unsigned l = b.degree();
    c.resize(k, 0.);
    
    for(unsigned i = k; i >= l; i--) {
        assert(i >= 0);
        double ci = r.back()/b.back();
        c[i-l] += ci;
        Poly bb = ci*b;
        //std::cout << ci <<"*(" << b.shifted(i-l) << ") = " 
        //          << bb.shifted(i-l) << "     r= " << r << std::endl;
        r -= bb.shifted(i-l);
        r.pop_back();
    }
    //std::cout << "r= " << r << std::endl;
    r.normalize();
    c.normalize();
    
    return c;
}

Poly gcd(Poly const &a, Poly const &b, const double tol) {
    if(a.size() < b.size())
        return gcd(b, a);
    if(b.size() <= 0)
        return a;
    if(b.size() == 1)
        return a;
    Poly r;
    divide(a, b, r);
    return gcd(b, r);
}



/*Poly divide_out_root(Poly const & p, double x) {
    assert(1);
    }*/


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

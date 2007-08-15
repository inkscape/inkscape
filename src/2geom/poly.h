#ifndef SEEN_POLY_H
#define SEEN_POLY_H
#include <assert.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <complex>
#include "utils.h"

class Poly : public std::vector<double>{
public:
    // coeff; // sum x^i*coeff[i]
    
    //unsigned size() const { return coeff.size();}
    unsigned degree() const { return size()-1;}

    //double operator[](const int i) const { return (*this)[i];}
    //double& operator[](const int i) { return (*this)[i];}
    
    Poly operator+(const Poly& p) const {
        Poly result;
        const unsigned out_size = std::max(size(), p.size());
        const unsigned min_size = std::min(size(), p.size());
        //result.reserve(out_size);
        
        for(unsigned i = 0; i < min_size; i++) {
            result.push_back((*this)[i] + p[i]);
        }
        for(unsigned i = min_size; i < size(); i++)
            result.push_back((*this)[i]);
        for(unsigned i = min_size; i < p.size(); i++)
            result.push_back(p[i]);
        assert(result.size() == out_size);
        return result;
    }
    Poly operator-(const Poly& p) const {
        Poly result;
        const unsigned out_size = std::max(size(), p.size());
        const unsigned min_size = std::min(size(), p.size());
        result.reserve(out_size);
        
        for(unsigned i = 0; i < min_size; i++) {
            result.push_back((*this)[i] - p[i]);
        }
        for(unsigned i = min_size; i < size(); i++)
            result.push_back((*this)[i]);
        for(unsigned i = min_size; i < p.size(); i++)
            result.push_back(-p[i]);
        assert(result.size() == out_size);
        return result;
    }
    Poly operator-=(const Poly& p) {
        const unsigned out_size = std::max(size(), p.size());
        const unsigned min_size = std::min(size(), p.size());
        resize(out_size);
        
        for(unsigned i = 0; i < min_size; i++) {
            (*this)[i] -= p[i];
        }
        for(unsigned i = min_size; i < out_size; i++)
            (*this)[i] = -p[i];
        return *this;
    }
    Poly operator-(const double k) const {
        Poly result;
        const unsigned out_size = size();
        result.reserve(out_size);
        
        for(unsigned i = 0; i < out_size; i++) {
            result.push_back((*this)[i]);
        }
        result[0] -= k;
        return result;
    }
    Poly operator-() const {
        Poly result;
        result.resize(size());
        
        for(unsigned i = 0; i < size(); i++) {
            result[i] = -(*this)[i];
        }
        return result;
    }
    Poly operator*(const double p) const {
        Poly result;
        const unsigned out_size = size();
        result.reserve(out_size);
        
        for(unsigned i = 0; i < out_size; i++) {
            result.push_back((*this)[i]*p);
        }
        assert(result.size() == out_size);
        return result;
    }
// equivalent to multiply by x^terms, discard negative terms
    Poly shifted(unsigned terms) const { 
        Poly result;
        // This was a no-op and breaks the build on x86_64, as it's trying
        // to take maximum of 32-bit and 64-bit integers
        //const unsigned out_size = std::max(unsigned(0), size()+terms);
        const size_type out_size = size() + terms;
        result.reserve(out_size);
        
        if(terms < 0) {
            for(unsigned i = 0; i < out_size; i++) {
                result.push_back((*this)[i-terms]);
            }
        } else {
            for(unsigned i = 0; i < terms; i++) {
                result.push_back(0.0);
            }
            for(unsigned i = 0; i < size(); i++) {
                result.push_back((*this)[i]);
            }
        }
        
        assert(result.size() == out_size);
        return result;
    }
    Poly operator*(const Poly& p) const;
    
    template <typename T>
    T eval(T x) const {
        T r = 0;
        for(int k = size()-1; k >= 0; k--) {
            r = r*x + T((*this)[k]);
        }
        return r;
    }
    
    template <typename T>
    T operator()(T t) const { return (T)eval(t);}
    
    void normalize();
    
    void monicify();
    Poly() {}
    Poly(const Poly& p) : std::vector<double>(p) {}
    Poly(const double a) {push_back(a);}
    
public:
    template <class T, class U>
    void val_and_deriv(T x, U &pd) const {
        pd[0] = back();
        int nc = size() - 1;
        int nd = pd.size() - 1;
        for(unsigned j = 1; j < pd.size(); j++)
            pd[j] = 0.0;
        for(int i = nc -1; i >= 0; i--) {
            int nnd = std::min(nd, nc-i);
            for(int j = nnd; j >= 1; j--)
                pd[j] = pd[j]*x + operator[](i);
            pd[0] = pd[0]*x + operator[](i);
        }
        double cnst = 1;
        for(int i = 2; i <= nd; i++) {
            cnst *= i;
            pd[i] *= cnst;
        }
    }
    
    static Poly linear(double ax, double b) {
        Poly p;
        p.push_back(b);
        p.push_back(ax);
        return p;
    }
};

inline Poly operator*(double a, Poly const & b) { return b * a;}

Poly integral(Poly const & p);
Poly derivative(Poly const & p);
Poly divide_out_root(Poly const & p, double x);
Poly compose(Poly const & a, Poly const & b);
Poly divide(Poly const &a, Poly const &b, Poly &r);
Poly gcd(Poly const &a, Poly const &b, const double tol=1e-10);

/*** solve(Poly p)
 * find all p.degree() roots of p.
 * This function can take a long time with suitably crafted polynomials, but in practice it should be fast.  Should we provide special forms for degree() <= 4?
 */
std::vector<std::complex<double> > solve(const Poly & p);

/*** solve_reals(Poly p)
 * find all real solutions to Poly p.
 * currently we just use solve and pick out the suitably real looking values, there may be a better algorithm.
 */
std::vector<double> solve_reals(const Poly & p);
double polish_root(Poly const & p, double guess, double tol);

inline std::ostream &operator<< (std::ostream &out_file, const Poly &in_poly) {
    if(in_poly.size() == 0)
        out_file << "0";
    else {
        for(int i = (int)in_poly.size()-1; i >= 0; --i) {
            if(i == 1) {
                out_file << "" << in_poly[i] << "*x";
                out_file << " + ";
            } else if(i) {
                out_file << "" << in_poly[i] << "*x^" << i;
                out_file << " + ";
            } else
                out_file << in_poly[i];
            
        }
    }
    return out_file;
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
#endif

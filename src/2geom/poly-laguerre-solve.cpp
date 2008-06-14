#include "poly-laguerre-solve.h"
#include <iterator>

typedef std::complex<double> cdouble;

cdouble laguerre_internal_complex(Poly const & p,
                                  double x0,
                                  double tol,
                                  bool & quad_root) {
    cdouble a = 2*tol;
    cdouble xk = x0;
    double n = p.degree();
    quad_root = false;
    const unsigned shuffle_rate = 10;
    //static double shuffle[] = {0, 0.5, 0.25, 0.75, 0.125, 0.375, 0.625, 0.875, 1.0};
    unsigned shuffle_counter = 0;
    while(std::norm(a) > (tol*tol)) {
        //std::cout << "xk = " << xk << std::endl;
        cdouble b = p.back();
        cdouble d = 0, f = 0;
        double err = abs(b);
        double abx = abs(xk);
        for(int j = p.size()-2; j >= 0; j--) {
            f = xk*f + d;
            d = xk*d + b;
            b = xk*b + p[j];
            err = abs(b) + abx*err;
        }

        err *= 1e-7; // magic epsilon for convergence, should be computed from tol

        cdouble px = b;
        if(abs(b) < err)
            return xk;
        //if(std::norm(px) < tol*tol)
        //    return xk;
        cdouble G = d / px;
        cdouble H = G*G - f / px;

        //std::cout << "G = " << G << "H = " << H;
        cdouble radicand = (n - 1)*(n*H-G*G);
        //assert(radicand.real() > 0);
        if(radicand.real() < 0)
            quad_root = true;
        //std::cout << "radicand = " << radicand << std::endl;
        if(G.real() < 0) // here we try to maximise the denominator avoiding cancellation
            a = - sqrt(radicand);
        else
            a = sqrt(radicand);
        //std::cout << "a = " << a << std::endl;
        a = n / (a + G);
        //std::cout << "a = " << a << std::endl;
        if(shuffle_counter % shuffle_rate == 0)
            ;//a *= shuffle[shuffle_counter / shuffle_rate];
        xk -= a;
        shuffle_counter++;
        if(shuffle_counter >= 90)
            break;
    }
    //std::cout << "xk = " << xk << std::endl;
    return xk;
}

double laguerre_internal(Poly const & p,
                         Poly const & pp,
                         Poly const & ppp,
                         double x0,
                         double tol,
                         bool & quad_root) {
    double a = 2*tol;
    double xk = x0;
    double n = p.degree();
    quad_root = false;
    while(a*a > (tol*tol)) {
        //std::cout << "xk = " << xk << std::endl;
        double px = p(xk);
        if(px*px < tol*tol)
            return xk;
        double G = pp(xk) / px;
        double H = G*G - ppp(xk) / px;

        //std::cout << "G = " << G << "H = " << H;
        double radicand = (n - 1)*(n*H-G*G);
        assert(radicand > 0);
        //std::cout << "radicand = " << radicand << std::endl;
        if(G < 0) // here we try to maximise the denominator avoiding cancellation
            a = - sqrt(radicand);
        else
            a = sqrt(radicand);
        //std::cout << "a = " << a << std::endl;
        a = n / (a + G);
        //std::cout << "a = " << a << std::endl;
        xk -= a;
    }
    //std::cout << "xk = " << xk << std::endl;
    return xk;
}


std::vector<cdouble >
laguerre(Poly p, const double tol) {
    std::vector<cdouble > solutions;
    //std::cout << "p = " << p << " = ";
    while(p.size() > 1)
    {
        double x0 = 0;
        bool quad_root = false;
        cdouble sol = laguerre_internal_complex(p, x0, tol, quad_root);
        //if(abs(sol) > 1) break;
        Poly dvs;
        if(quad_root) {
            dvs.push_back((sol*conj(sol)).real());
            dvs.push_back(-(sol + conj(sol)).real());
            dvs.push_back(1.0);
            //std::cout << "(" <<  dvs << ")";
            //solutions.push_back(sol);
            //solutions.push_back(conj(sol));
        } else {
            //std::cout << sol << std::endl;
            dvs.push_back(-sol.real());
            dvs.push_back(1.0);
            solutions.push_back(sol);
            //std::cout << "(" <<  dvs << ")";
        }
        Poly r;
        p = divide(p, dvs, r);
        //std::cout << r << std::endl;
    }
    return solutions;
}

std::vector<double>
laguerre_real_interval(Poly const & ply,
                       const double lo, const double hi,
                       const double tol) 
{
    /* not implemented*/
    assert(false);
    std::vector<double> result;
    return result;
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

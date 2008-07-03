#include <2geom/poly.h>
#include <complex>

std::vector<std::complex<double> > 
laguerre(Poly ply, const double tol=1e-10);

std::vector<double> 
laguerre_real_interval(Poly  ply, 
		       const double lo, const double hi,
		       const double tol=1e-10);

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

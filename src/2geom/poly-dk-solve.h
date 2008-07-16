#ifndef LIB2GEOM_SEEN_POLY_DK_SOLVE_H
#define LIB2GEOM_SEEN_POLY_DK_SOLVE_H

#include <2geom/poly.h>
#include <complex>

namespace Geom {

std::vector<std::complex<double> > 
DK(Poly const & ply, const double tol=1e-10);

} // namespace Geom

#endif // LIB2GEOM_SEEN_POLY_DK_SOLVE_H

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

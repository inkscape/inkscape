#ifndef _CHEBYSHEV
#define _CHEBYSHEV

#include <2geom/sbasis.h>
#include <2geom/interval.h>

/*** Conversion between Chebyshev approximation and SBasis.
 * 
 */

namespace Geom{

SBasis chebyshev_approximant (double (*f)(double,void*), int order, Interval in, void* p=0);
SBasis chebyshev_approximant_interpolating (double (*f)(double,void*), int order, Interval in, void* p=0);
SBasis chebyshev(unsigned n);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

#endif

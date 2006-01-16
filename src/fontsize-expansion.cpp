#include "libnr/nr-matrix-ops.h"

/**
 * Returns the distance from the transformed baseline to the
 * transformation of a point at (0, 1); or 0 if \a m has no inverse.
 */
double
fontsize_expansion(NR::Matrix const &m)
{
    double const denom(hypot(m[0], m[1]));
    if (!(denom > 1e-100)) {
        return 0.;
    }
    double const numer(m.descrim2());
    if (!(numer > 1e-100)) {
        return 0.;
    }
    return numer / denom;
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

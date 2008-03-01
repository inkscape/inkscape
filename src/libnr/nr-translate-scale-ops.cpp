#include "libnr/nr-matrix-ops.h"

NR::Matrix
operator*(NR::translate const &t, NR::scale const &s)
{
    using NR::X; using NR::Y;

    NR::Matrix ret(s);
    ret[4] = t[X] * s[X];
    ret[5] = t[Y] * s[Y];
    return ret;
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

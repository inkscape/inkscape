#include "libnr/nr-matrix-ops.h"

NR::Matrix
operator*(NR::scale const &s, NR::Matrix const &m)
{
    using NR::X; using NR::Y;
    NR::Matrix ret(m);
    ret[0] *= s[X];
    ret[1] *= s[X];
    ret[2] *= s[Y];
    ret[3] *= s[Y];
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

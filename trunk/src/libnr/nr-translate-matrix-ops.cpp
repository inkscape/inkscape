#include "libnr/nr-matrix-ops.h"

namespace NR {

Matrix
operator*(translate const &t, Matrix const &m)
{
    Matrix ret(m);
    ret[4] += m[0] * t[X] + m[2] * t[Y];
    ret[5] += m[1] * t[X] + m[3] * t[Y];
    return ret;
}

}  /* namespace NR */


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

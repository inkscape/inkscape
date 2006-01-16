#include "libnr/nr-matrix-ops.h"

namespace NR {

Matrix
operator*(Matrix const &m, translate const &t)
{
    Matrix ret(m);
    ret[4] += t[X];
    ret[5] += t[Y];
    assert_close( ret, m * Matrix(t) );
    return ret;
}

}  // namespace NR

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

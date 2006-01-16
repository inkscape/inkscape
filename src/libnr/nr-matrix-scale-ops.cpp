#include "libnr/nr-matrix-ops.h"

NR::Matrix
operator/(NR::Matrix const &m, NR::scale const &s)
{
    using NR::X; using NR::Y;
    NR::Matrix ret(m);
    ret[0] /= s[X]; ret[1] /= s[Y];
    ret[2] /= s[X]; ret[3] /= s[Y];
    ret[4] /= s[X]; ret[5] /= s[Y];
    assert_close( ret, m * NR::Matrix(s.inverse()) );
    return ret;
}

NR::Matrix
operator*(NR::Matrix const &m, NR::scale const &s)
{
    using NR::X; using NR::Y;
    NR::Matrix ret(m);
    ret[0] *= s[X]; ret[1] *= s[Y];
    ret[2] *= s[X]; ret[3] *= s[Y];
    ret[4] *= s[X]; ret[5] *= s[Y];
    assert_close( ret, m * NR::Matrix(s) );
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

#include "libnr/nr-matrix-translate-ops.h"

NR::Matrix
operator*(NR::scale const &s, NR::translate const &t)
{
    return NR::Matrix(s) * t;
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

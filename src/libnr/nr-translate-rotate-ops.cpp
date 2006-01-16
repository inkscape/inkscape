#include <libnr/nr-matrix-translate-ops.h>
#include <libnr/nr-rotate-ops.h>

NR::Matrix
operator*(NR::translate const &a, NR::rotate const &b)
{
    return NR::Matrix(b) * NR::translate(a.offset * b);
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

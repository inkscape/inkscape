#include "libnr/nr-matrix-ops.h"
#include "libnr/nr-point-matrix-ops.h"

NR::Point operator/(NR::Point const &p, NR::Matrix const &m) {
    return p * m.inverse();
}

NR::Matrix operator/(NR::Matrix const &a, NR::Matrix const &b) {
    return a * b.inverse();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

#ifndef SEEN_LIBNR_NR_MATRIX_DIV_H
#define SEEN_LIBNR_NR_MATRIX_DIV_H

#include "libnr/nr-forward.h"

NR::Point operator/(NR::Point const &, NR::Matrix const &);

NR::Matrix operator/(NR::Matrix const &, NR::Matrix const &);

#endif /* !SEEN_LIBNR_NR_MATRIX_DIV_H */

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

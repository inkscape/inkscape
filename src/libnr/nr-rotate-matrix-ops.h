#ifndef SEEN_LIBNR_NR_ROTATE_MATRIX_OPS_H
#define SEEN_LIBNR_NR_ROTATE_MATRIX_OPS_H

#include <libnr/nr-forward.h>


NR::Matrix operator*(NR::rotate const &a, NR::Matrix const &b);


#endif /* !SEEN_LIBNR_NR_ROTATE_MATRIX_OPS_H */

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

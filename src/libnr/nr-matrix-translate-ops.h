#ifndef SEEN_LIBNR_NR_MATRIX_TRANSLATE_OPS_H
#define SEEN_LIBNR_NR_MATRIX_TRANSLATE_OPS_H

/** \file 
 * Declarations (and definition if inline) of operator 
 * blah (NR::Matrix, NR::translate).
 */

#include "libnr/nr-matrix.h"
#include "libnr/nr-translate.h"

//NR::Matrix operator*(NR::Matrix const &m, NR::translate const &t);

namespace NR {
Matrix operator*(Matrix const &m, translate const &t);
}

inline NR::Matrix
operator/(NR::Matrix const &numer, NR::translate const &denom)
{
    return numer * NR::translate(-denom.offset);
}


#endif /* !SEEN_LIBNR_NR_MATRIX_TRANSLATE_OPS_H */

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

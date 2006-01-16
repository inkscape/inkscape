#ifndef SEEN_LIBNR_NR_MATRIX_SCALE_OPS_H
#define SEEN_LIBNR_NR_MATRIX_SCALE_OPS_H
/** \file 
 * Declarations (and definition if inline) of operator blah (NR::Matrix, NR::scale). 
 */

#include "libnr/nr-forward.h"

NR::Matrix operator/(NR::Matrix const &m, NR::scale const &s);

NR::Matrix operator*(NR::Matrix const &m, NR::scale const &s);


#endif /* !SEEN_LIBNR_NR_MATRIX_SCALE_OPS_H */

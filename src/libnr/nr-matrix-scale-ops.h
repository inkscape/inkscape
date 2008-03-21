#ifndef SEEN_LIBNR_NR_MATRIX_SCALE_OPS_H
#define SEEN_LIBNR_NR_MATRIX_SCALE_OPS_H
/** \file 
 * Declarations (and definition if inline) of operator blah (NR::Matrix, NR::scale). 
 */

#include "libnr/nr-forward.h"

namespace NR {

inline Matrix &operator/=(Matrix &m, scale const &s) {
    m[0] /= s[X]; m[1] /= s[Y];
    m[2] /= s[X]; m[3] /= s[Y];
    m[4] /= s[X]; m[5] /= s[Y];
    return m;
}

inline Matrix &operator*=(Matrix &m, scale const &s) {
    m[0] *= s[X]; m[1] *= s[Y];
    m[2] *= s[X]; m[3] *= s[Y];
    m[4] *= s[X]; m[5] *= s[Y];
    return m;
}

inline Matrix operator/(Matrix const &m, scale const &s) { Matrix ret(m); ret /= s; return ret; }

inline Matrix operator*(Matrix const &m, scale const &s) { Matrix ret(m); ret *= s; return ret; }

}

#endif /* !SEEN_LIBNR_NR_MATRIX_SCALE_OPS_H */
